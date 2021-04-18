import cv2
import numpy as np
import pyk4a
from pyk4a import Config, PyK4A

import os.path as osp
import datetime

DEV_ID=0

def main():
    k4a_config=Config(
            color_resolution=pyk4a.ColorResolution.RES_3072P,
            depth_mode=pyk4a.DepthMode.OFF,
            synchronized_images_only=False,
            camera_fps=pyk4a.FPS.FPS_15,
        )
    k4a = PyK4A(
        k4a_config,device_id=DEV_ID
    )
    k4a.start()

    # getters and setters directly get and set on device
    k4a.whitebalance = 4500
    assert k4a.whitebalance == 4500
    k4a.whitebalance = 4510
    assert k4a.whitebalance == 4510

    while 1:
        capture = k4a.get_capture()
        cv2.namedWindow("k4a",cv2.WINDOW_NORMAL)
        if np.any(capture.color):
            cv2.imshow("k4a", capture.color[:, :, :3])
            key = cv2.waitKey(10)
            if key == ord(' '):
                time = datetime.datetime.strftime(datetime.datetime.now(), '%Y-%m-%d-%H-%M-%S')
                cv2.imwrite(osp.join('imgs','4cam',str(DEV_ID),time+'.jpg'), capture.color[:,:,:3])
                print('save img!'+str(DEV_ID))
            elif key != -1:
                cv2.destroyAllWindows()
                break
    k4a.stop()


if __name__ == "__main__":
    main()
