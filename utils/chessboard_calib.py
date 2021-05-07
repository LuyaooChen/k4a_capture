
import numpy as np
import cv2
from cv2 import aruco

def rtvec2transMat(rvec, tvec):
    """
    把旋转向量和平移向量转换成变换矩阵
    """
    rotationMat, _ = cv2.Rodrigues(rvec)
    # print(rotationMat, tvec)
    transformMat = np.concatenate((rotationMat, tvec), axis=1)
    # print(transformMat)
    transformMat = np.concatenate(
        (transformMat, np.array([[0, 0, 0, 1]])), axis=0)
    return transformMat


def transMat2rtvec(transformMat):
    """
    把变换矩阵转换成旋转向量和平移向量
    return rvec, tvec
    """
    rotationMat = transformMat[0:3, 0:3]
    rvec = cv2.Rodrigues(rotationMat)[0].reshape(3, 1)
    tvec = transformMat[0:3, 3].reshape(3, 1)
    return rvec, tvec

class Camera_params:
    """
    描述相机的内外参数
    """

    def __init__(self, cameraMatrix, distCoeffs, rvec=None, tvec=None):
        self.cameraMatrix = cameraMatrix
        self.distCoeffs = distCoeffs
        if rvec is None:
            self.rvec = np.zeros((3, 1), dtype=float)
        else:
            self.rvec = rvec
        if tvec is None:
            self.tvec = np.zeros((3, 1), dtype=float)
        else:
            self.tvec = tvec
        # if rvec is not None and tvec is not None:
        self.transformMat = rtvec2transMat(self.rvec, self.tvec)
        # else:
        #     self.transformMat = None

    def transMat2rtvec(self):
        """
        把变换矩阵转换成旋转向量和平移向量
        """
        self.rvec, self.tvec = transMat2rtvec(self.transformMat)

    def rtvec2transMat(self):
        """
        把旋转平移向量转换成变换矩阵
        """
        self.transformMat = rtvec2transMat(self.rvec, self.tvec)

    def save(self, path):
        """
        保存参数至path路径文件
        """
        filesaver = cv2.FileStorage()
        filesaver.open(path, cv2.FileStorage_WRITE)
        filesaver.write('camreaMatrix', self.cameraMatrix)
        filesaver.write('distCoeffs', self.distCoeffs)
        filesaver.write('rvec', self.rvec)
        filesaver.write('tvec', self.tvec)
        filesaver.write('transformMat', self.transformMat)
        filesaver.release()

class Chessboard:
    """
    棋盘格。Input: 行角点数， 列角点数， 方格实际边长(m)
    """

    def __init__(self, cbrow, cbcol, cblen):
        self.pattern_size = (cbrow, cbcol)
        self.criteria = (cv2.TERM_CRITERIA_EPS +
                         cv2.TERM_CRITERIA_MAX_ITER, 30, 0.1)
        self.objPoints = np.zeros((np.prod(self.pattern_size), 3), np.float32)
        self.objPoints[:, :2] = np.indices(self.pattern_size).T.reshape(-1, 2)
        self.objPoints *= cblen

class N_cam_calib:
    """
    读入两个相机对同一标定板拍摄的照片，计算相机二相对相机一的外参。相机一的外参应预先给定或初始化。
    每读入一组图像应更新一次参数。
    """

    def __init__(self, camera_paramses:list, board, method='CHESSBOARD'):
        self.camera_paramses = camera_paramses
        self.N_cam = len(camera_paramses)
        self.board = board
        self.method = method
        self.cnt = 0
        self.cam2board_rvecs = []  # transform from cam to board
        self.cam2board_tvecs = []

    def pose_estimation(self, imgs):
        """
        估计标定板相对N个相机的位姿，并在图中画出坐标轴；N幅图都估计成功，则返回True；
        """
        self.cam2board_rvecs = []
        self.cam2board_tvecs = []
        success_flags=[]
        for i in range(self.N_cam):
            assert (imgs[i].shape[2] == 3)  # channnels == 3
            estimate_success = False
            gray_img = cv2.cvtColor(imgs[i], cv2.COLOR_BGR2GRAY)
            if self.method == 'CHESSBOARD':
                ret, corners = cv2.findChessboardCorners(
                    gray_img, self.board.pattern_size)
                if ret == True:
                    corners = cv2.cornerSubPix(
                        gray_img, corners, (11, 11), (-1, -1), self.board.criteria)
                    cv2.drawChessboardCorners(
                        imgs[i], self.board.pattern_size, corners, ret)
                    estimate_success, cam2board_rvec, cam2board_tvec = cv2.solvePnP(
                        self.board.objPoints, corners, self.camera_paramses[i].cameraMatrix, self.camera_paramses[i].distCoeffs)
                else:
                    print('findChessboardCorners failed!'+str(i))
            success_flags.append(estimate_success)
            if estimate_success:
                aruco.drawAxis(imgs[i], self.camera_paramses[i].cameraMatrix,
                            self.camera_paramses[i].distCoeffs, cam2board_rvec, cam2board_tvec, 0.1)
                self.cam2board_rvecs.append(cam2board_rvec)
                self.cam2board_tvecs.append(cam2board_tvec)
            else:
                print('estimate failed!'+str(i))

        if False in success_flags:
            return False
        else:
            return True

    def step(self):
        """
        每调用一次就更新一次sub_camera的外参；需要N幅图都拍摄到标定板；
        """

        # 把0号相机作为坐标原点
        transformMat_zero2board = rtvec2transMat(
            self.cam2board_rvecs[0], self.cam2board_tvecs[0])
        transformMat_zero2cam = []
        for i in range(1,self.N_cam):
            transformMat_cam2board = rtvec2transMat(
                self.cam2board_rvecs[i], self.cam2board_tvecs[i])
            transformMat_zero2cam.append(np.matmul(
                transformMat_zero2board, np.linalg.inv(transformMat_cam2board)))

            if self.cnt == 0:
                self.camera_paramses[i].transformMat = transformMat_zero2cam[i-1]
            else:
                self.camera_paramses[i].transformMat = (
                    self.camera_paramses[i].transformMat*self.cnt + transformMat_zero2cam[i-1]) / (self.cnt + 1)

        self.cnt += 1
    
    def save_camParam(self,path):
        for i in range(self.N_cam):
            self.camera_paramses[i].transMat2rtvec()
            self.camera_paramses[i].save(os.path.join(path,str(i),str(i)+'.yaml'))
    


if __name__ == "__main__":
    """
    目录结构
    imgs
    --4cam
      |--0
      |--1
      |--2
      |--3
      |--res
        |--0
        |--1
        |--2
        |--3
    """
    CAMERA_MATRIXS = []
    DIST_COEFFSES = []

    # 720p分辨率的内参
    # CAMERA_MATRIXS.append(np.array(
    #     [[612.513, 0,  638.342],
    #     [0,  612.383,  366.08],
    #     [0, 0, 1]]))
    # DIST_COEFFSES.append(np.array([0.6377023458480835, -2.6658477783203125, 0.00048410258023068309, -0.00033505461760796607,
    #                             1.4428961277008057, 0.51688140630722046,-2.4993424415588379,1.3787573575973511]))

    # CAMERA_MATRIXS.append(np.array(
    #     [[612.902, 0,  638.628],
    #     [0,  612.694,  368.928],
    #     [0, 0, 1]]))
    # DIST_COEFFSES.append(np.array([0.57293963432312012, -2.6344637870788574, 0.00069375935709103942, -0.00045344670070335269,
    #                                1.4527934789657593, 0.45141249895095825,-2.4632573127746582,1.3854954242706299, ]))

    # CAMERA_MATRIXS.append(np.array(
    #     [[613.292, 0,  637.187],
    #     [0,   613.268, 363.199],
    #     [0, 0, 1]]))
    # DIST_COEFFSES.append(np.array([0.24720779061317444, -2.5398986339569092, 0.00031602478702552617, -0.0002125041646650061,
    #                                1.5901056528091431, 0.12699282169342041,-2.3463912010192871,1.5033758878707886]))

    # CAMERA_MATRIXS.append(np.array(
    #     [[606.487, 0,   639.651],
    #     [0,  606.267, 367.937],
    #     [0, 0, 1]]))
    # DIST_COEFFSES.append(np.array([0.32710307836532593, -2.45015549659729, 0.0014043597038835287, -0.00051846145652234554,
    #                             1.4576127529144287, 0.2109769880771637,-2.2757446765899658,1.3838359117507935]))

    ############################################

    CAMERA_MATRIXS.append(np.array(
        [[6.2352789333151281e+02, 0., 6.3745467097488859e+02],
        [0,  6.2357961226318287e+02, 3.6474047119510431e+02],
        [0, 0, 1]]))
    DIST_COEFFSES.append(np.array([9.8451763617300098e-02, -7.4028796336095604e-02,
       -5.1430224923343761e-04, -3.8625308493277823e-04,
       2.1024566729153222e-02]))

    # CAMERA_MATRIXS.append(np.array(
    #     [[6.2405000536364116e+02, 0., 6.3671270266455349e+02],
    #     [0,  6.2410305616626522e+02, 3.6857735453560787e+02],
    #     [0, 0, 1]]))
    # DIST_COEFFSES.append(np.array([ 1.0215714525581411e-01, -9.7220430974020736e-02,
    #    1.8170056356242966e-04, -7.5531562197025687e-04,
    #    5.0563223167652202e-02 ]))

    # CAMERA_MATRIXS.append(np.array(
    #     [[6.2315934715498418e+02, 0., 6.3520369443883419e+02],
    #     [0,   6.2324970613489950e+02, 3.6382198007909483e+02],
    #     [0, 0, 1]]))
    # DIST_COEFFSES.append(np.array([9.7229263351965273e-02, -7.6834886582235709e-02,
    #    1.9508723048470780e-04, -9.4131488794134378e-04,
    #    2.9646836529399655e-02]))

    CAMERA_MATRIXS.append(np.array(
        [[6.1660870472920874e+02, 0., 6.3913875689503197e+02],
        [0,  6.1654884739064948e+02, 3.6646525733855822e+02],
        [0, 0, 1]]))
    DIST_COEFFSES.append(np.array([1.0250247683057216e-01, -1.0066981875197754e-01,
       5.6029563670811976e-04, -3.7253780151465200e-04,
       5.6730746052155431e-02]))



    import os
    path_prefix = os.path.join('imgs', '4cam')
    N_CAM = 2
    img_lists = []
    for i in range(N_CAM):
        img_lists.append(os.listdir(os.path.join(path_prefix, str(i))))
        img_lists[-1].sort()
        print(img_lists[-1])
    
    board = Chessboard(11, 8, 0.045)

    cam_paramses = []
    for i in range(N_CAM):
        cam_paramses.append(Camera_params(CAMERA_MATRIXS[i],DIST_COEFFSES[i]))
    calib = N_cam_calib(cam_paramses, board)

    for i in range(len(img_lists[0])):
        print('处理第'+str(i)+'组图片...')
        imgs = []
        for j in range(N_CAM):
            img=cv2.imread(os.path.join(path_prefix,str(j),img_lists[j][i]))
            imgs.append(img)
        if calib.pose_estimation(imgs):
            calib.step()
        for j in range(N_CAM):
            cv2.imwrite(os.path.join(path_prefix, 'res', str(j),str(i)+'.jpg'), imgs[j])
            
    for i in range(N_CAM):
        calib.save_camParam(os.path.join(path_prefix,'res'))


