```
Source file                                          COLOR         DEPTH            IR
==========================================================================================
/home/cly/Documents/dataset/k4a_mine/output3.mkv                 241144 usec   241144 usec	//丢color图； 缺0 1 2； 不可用

/home/cly/Documents/dataset/k4a_mine/output1.mkv   268500 usec   266822 usec   266822 usec	//缺文件0，这3帧都不可用
/home/cly/Documents/dataset/k4a_mine/output2.mkv   268488 usec   266977 usec   266977 usec
/home/cly/Documents/dataset/k4a_mine/output3.mkv   268500 usec   267144 usec   267144 usec	

/home/cly/Documents/dataset/k4a_mine/output1.mkv   301822 usec   300166 usec   300166 usec	//这组是正常的
/home/cly/Documents/dataset/k4a_mine/output2.mkv   301822 usec   300322 usec   300322 usec
/home/cly/Documents/dataset/k4a_mine/output3.mkv   301833 usec   300488 usec   300488 usec
/home/cly/Documents/dataset/k4a_mine/output.mkv    301866 usec   301866 usec   301866 usec

/home/cly/Documents/dataset/k4a_mine/output.mkv    335200 usec   335155 usec   335155 usec	//丢图，这一组不可用
/home/cly/Documents/dataset/k4a_mine/output2.mkv   335155 usec                            
/home/cly/Documents/dataset/k4a_mine/output1.mkv   335166 usec                            
/home/cly/Documents/dataset/k4a_mine/output3.mkv   335166 usec                            

/home/cly/Documents/dataset/k4a_mine/output.mkv    368488 usec   368533 usec   368533 usec
```
使用k4arecorder同步录制数据，原始数据可能会有丢失。  
该代码参考自[官方example](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/tree/develop/examples/playback_external_sync)  
把完整的一帧数据保存下来，残缺的丢弃。确保同序号的文件是同时刻拍摄的。  
录制的color图是mjpg格式，需要解码，这里用turbojpeg库，ffmpeg也可  
注意其中有些路径和图片数量是硬编码  
另外代码目前似乎不能正常退出。。不影响功能