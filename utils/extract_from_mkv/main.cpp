// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <stdio.h>
#include <malloc.h>
#include <k4a/k4a.h>
#include <k4arecord/playback.h>

#include <vector>
#include <opencv2/opencv.hpp>

#include <turbojpeg.h>

#include "../../depthundistorthelper.h"

static int K4A_COLOR_RESOLUTIONS[7][2]= {{0,0}, {1280,720},{1920,1080},{2560,1440},{2048,1536},{3840,2160},{4096,3072}};

typedef struct
{
    char *filename;
    k4a_playback_t handle;
    k4a_record_configuration_t record_config;
    k4a_capture_t capture;
} recording_t;

static uint64_t first_capture_timestamp(k4a_capture_t capture)
{
    uint64_t min_timestamp = (uint64_t)-1;
    k4a_image_t images[3];
    images[0] = k4a_capture_get_color_image(capture);
    images[1] = k4a_capture_get_depth_image(capture);
    images[2] = k4a_capture_get_ir_image(capture);

    for (int i = 0; i < 3; i++)
    {
        if (images[i] != NULL)
        {
            uint64_t timestamp = k4a_image_get_device_timestamp_usec(images[i]);
            if (timestamp < min_timestamp)
            {
                min_timestamp = timestamp;
            }
            k4a_image_release(images[i]);
            images[i] = NULL;
        }
    }

    return min_timestamp;
}

static void print_capture_info(k4a_capture_t capture)
{
    k4a_image_t images[3];
    images[0] = k4a_capture_get_color_image(capture);
    images[1] = k4a_capture_get_depth_image(capture);
    images[2] = k4a_capture_get_ir_image(capture);

    // printf("%-32s", file->filename);
    for (int i = 0; i < 3; i++)
    {
        if (images[i] != NULL)
        {
            uint64_t timestamp = k4a_image_get_device_timestamp_usec(images[i]);
            printf("  %7ju usec", timestamp);
            k4a_image_release(images[i]);
            images[i] = NULL;
        }
        else
        {
            printf("  %12s", "");
        }
    }
    printf("\n");
}

static int check_capture_info(recording_t *file)
{
    int ret=1;
    k4a_image_t images[3];
    images[0] = k4a_capture_get_color_image(file->capture);
    images[1] = k4a_capture_get_depth_image(file->capture);
    images[2] = k4a_capture_get_ir_image(file->capture);

    // printf("%-32s", file->filename);
    for (int i = 0; i < 3; i++)
    {
        if (images[i] != NULL)
        {
            // uint64_t timestamp = k4a_image_get_device_timestamp_usec(images[i]);
            // printf("  %7ju usec", timestamp);
            k4a_image_release(images[i]);
            images[i] = NULL;
        }
        else
        {
            // printf("  %12s", "");
            ret=-1;
        }
    }
    // printf("\n");
    return ret;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: playback_external_sync.exe <master.mkv> <sub1.mkv>...\n");
        return 1;
    }

    size_t file_count = (size_t)(argc - 1);
    bool master_found = false;
    k4a_result_t result = K4A_RESULT_SUCCEEDED;

    // Allocate memory to store the state of N recordings.
    recording_t *files = (recording_t *)malloc(sizeof(recording_t) * file_count);
    if (files == NULL)
    {
        printf("Failed to allocate memory for playback (%zu bytes)\n", sizeof(recording_t) * file_count);
        return 1;
    }
    memset(files, 0, sizeof(recording_t) * file_count);

    std::vector<k4a_calibration_t> calibrations(file_count);
    std::vector<k4a_transformation_t> transformations(file_count);
    std::vector<k4a_image_t> undistorteds(file_count,NULL);
    std::vector<k4a_image_t> luts(file_count,NULL);
    std::vector<pinhole_t> pinholes(file_count);

    // Open each recording file and validate they were recorded in master/subordinate mode.
    for (size_t i = 0; i < file_count; i++)
    {
        files[i].filename = argv[i + 1];

        result = k4a_playback_open(files[i].filename, &files[i].handle);
        if (result != K4A_RESULT_SUCCEEDED)
        {
            printf("Failed to open file: %s\n", files[i].filename);
            break;
        }

        result = k4a_playback_get_record_configuration(files[i].handle, &files[i].record_config);
        if (result != K4A_RESULT_SUCCEEDED)
        {
            printf("Failed to get record configuration for file: %s\n", files[i].filename);
            break;
        }

        if (files[i].record_config.wired_sync_mode == K4A_WIRED_SYNC_MODE_MASTER)
        {
            printf("Opened master recording file: %s\n", files[i].filename);
            if (master_found)
            {
                printf("ERROR: Multiple master recordings listed!\n");
                result = K4A_RESULT_FAILED;
                break;
            }
            else
            {
                master_found = true;
            }
        }
        else if (files[i].record_config.wired_sync_mode == K4A_WIRED_SYNC_MODE_SUBORDINATE)
        {
            printf("Opened subordinate recording file: %s\n", files[i].filename);
        }
        else
        {
            printf("ERROR: Recording file was not recorded in master/sub mode: %s\n", files[i].filename);
            result = K4A_RESULT_FAILED;
            break;
        }

        // Read the first capture of each recording into memory.
        k4a_stream_result_t stream_result = k4a_playback_get_next_capture(files[i].handle, &files[i].capture);
        if (stream_result == K4A_STREAM_RESULT_EOF)
        {
            printf("ERROR: Recording file is empty: %s\n", files[i].filename);
            result = K4A_RESULT_FAILED;
            break;
        }
        else if (stream_result == K4A_STREAM_RESULT_FAILED)
        {
            printf("ERROR: Failed to read first capture from file: %s\n", files[i].filename);
            result = K4A_RESULT_FAILED;
            break;
        }

        //获取各文件的calibration和transform
        k4a_playback_get_calibration(files[i].handle, &calibrations[i]);
        transformations[i] = k4a_transformation_create(&calibrations[i]);

        //为畸变矫正做准备 prepare for undistortion
        // k4a_image_t undistorted = NULL;
        // k4a_image_t lut=NULL;
        // pinhole_t pinhole;

        pinholes[i] = depthUndistortHelper::create_pinhole_from_xy_range(&calibrations[i],K4A_CALIBRATION_TYPE_COLOR);
        k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
                        pinholes[i].width,
                        pinholes[i].height,
                        pinholes[i].width * (int)sizeof(coordinate_t),
                        &luts[i]);
        depthUndistortHelper::create_undistortion_lut(&calibrations[i], K4A_CALIBRATION_TYPE_COLOR, &pinholes[i], luts[i], INTERPOLATION_BILINEAR_DEPTH);
        k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,pinholes[i].width,pinholes[i].height,pinholes[i].width*sizeof(uint16_t), &undistorteds[i]);
        //prepare for undistortion end
    }

    tjhandle m_decoder = tjInitDecompress();

    if (result == K4A_RESULT_SUCCEEDED)
    {
        printf("%-32s  %12s  %12s  %12s\n", "Source file", "COLOR", "DEPTH", "IR");
        printf("==========================================================================\n");

        int cnt=0;
        std::vector<k4a_capture_t> captures_t(file_count);  //缓存当前一组的capture
        uint64_t pre_timestamp = 0;

        int frame_cnt=0;

        // Print the first 25 captures in order of timestamp across all the recordings.
        // for (int frame = 0; frame < 10; frame++)
        while(frame_cnt<390)
        {
            uint64_t min_timestamp = (uint64_t)-1;
            // recording_t *min_file = NULL;
            size_t min_file_i;

            // Find the lowest timestamp out of each of the current captures.
            for (size_t i = 0; i < file_count; i++) //从N个文件中找出时间戳最早的
            {
                if (files[i].capture != NULL)
                {
                    uint64_t timestamp = first_capture_timestamp(files[i].capture);
                    if (timestamp < min_timestamp)
                    {
                        min_timestamp = timestamp;
                        // min_file = &files[i];
                        min_file_i=i;
                    }
                }
            }
            // printf("pre:%d,cur:%d\n",pre_timestamp, min_timestamp);
            if(min_timestamp-pre_timestamp>10000)   //假设两帧之间肯定大于10ms间隔
            {
                if(cnt==file_count)
                {
                    //保存图片
                    for(int i=0; i<file_count; i++)
                    {
                        printf("file id: %d\t",i);
                        print_capture_info(captures_t[i]);

                        int height = K4A_COLOR_RESOLUTIONS[files[i].record_config.color_resolution][1];
                        int width = K4A_COLOR_RESOLUTIONS[files[i].record_config.color_resolution][0];
                        k4a_image_t images[3];
                        images[0] = k4a_capture_get_color_image(captures_t[i]);

                        images[1] = k4a_capture_get_depth_image(captures_t[i]);

                        k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16,width,height,width*sizeof(uint16_t),&images[2]);
                        k4a_transformation_depth_image_to_color_camera(transformations[i],images[1],images[2]);


                        depthUndistortHelper::remap(images[2],luts[i],undistorteds[i],INTERPOLATION_BILINEAR_DEPTH);
                        uchar* depth_image_data = k4a_image_get_buffer(undistorteds[i]);
                        // uchar* depth_image_data = k4a_image_get_buffer(iamges[2]);
                        cv::Mat tmp2(height,width,CV_16UC1,depth_image_data);

                        //@todo: undistort color image
                        uchar* color_image_data = k4a_image_get_buffer(images[0]);
                        cv::Mat tmp(height,width,CV_8UC4);
                        tjDecompress2(m_decoder,color_image_data,k4a_image_get_size(images[0]),tmp.data,width,0,height,TJPF_BGRA,TJFLAG_FASTDCT|TJFLAG_FASTUPSAMPLE);
                        // cv::cvtColor(tmp,tmp,cv::COLOR_BGRA2BGR);
                        char filename[32];
                        sprintf(filename,"data/%d/depth_%04d.png",i,frame_cnt);
                        cv::imwrite(filename,tmp2);
                        // sprintf(filename,"data/%d/color_%04d.png",i,frame_cnt);
                        // cv::imwrite(filename,tmp);

                        for (int i = 0; i < 3; i++)
                        {
                            if (images[i] != NULL)
                            {
                                k4a_image_release(images[i]);
                                images[i] = NULL;
                            }
                        }

                    }
                    frame_cnt++;
                }
                else printf("drop!\n");

                for(int i=0; i<file_count; i++) //如果没到cnt说明某个文件丢帧或该帧缺图了，这一组都不要了
                {   //释放资源

                    if(captures_t[i]!=NULL)
                    {
                        k4a_capture_release(captures_t[i]);
                        captures_t[i] = NULL;
                    }
                }

                cnt=0;
            }
            pre_timestamp=min_timestamp;

            recording_t *min_file = &files[min_file_i];
            int is_valid = check_capture_info(min_file);

            captures_t[min_file_i]=min_file->capture;
            if(is_valid==1) cnt++;


            k4a_stream_result_t stream_result = k4a_playback_get_next_capture(min_file->handle, &(min_file->capture));
            if (stream_result == K4A_STREAM_RESULT_FAILED)  //似乎读到尾了也不会break。。。
            {
                printf("ERROR: Failed to read next capture from file: %s\n", min_file->filename);
                result = K4A_RESULT_FAILED;
                break;
            }
        }
    }

    for (size_t i = 0; i < file_count; i++)
    {
        if (files[i].handle != NULL)
        {
            k4a_playback_close(files[i].handle);
            files[i].handle = NULL;
        }
    }
    free(files);
    tjDestroy(m_decoder);
    return result == K4A_RESULT_SUCCEEDED ? 0 : 1;
}