# k4a_capture
多个Azure Kinect DK的同步采集、显示、标定等。
![window](https://raw.githubusercontent.com/LuyaooChen/k4a_capture/main/window.png)

## 依赖
1. [Azure Kinect Sensor SDK](https://github.com/microsoft/Azure-Kinect-Sensor-SDK)  1.4.1
2. Qt5.9
3. OpenCV 3.2
4. [Open3D](https://github.com/intel-isl/Open3D) 0.12
5. libtorch 1.8.1

## 使用说明
4个窗口分别对应设备号从0-3的相机  
背景抠图功能基于[BackgroundMattingV2](https://github.com/PeterL1n/BackgroundMattingV2),需要去原仓库先下载模型权重文件，只能基于固定背景。点击`Set BG`按钮会保存当前图片为背景并开始分割  
点击`2D\3D`按钮可以显示点云，本功能会调用Open3D的窗口（目前存在打开时可能无内容的bug，可多尝试几次）  
`Refine Registration`会调用Open3D的colored_ICP以尝试对齐点云，但在我的应用中因相机重叠视野过小而失败，所以暂时没有进行进一步测试  
保存图片、保存点云、调整曝光值和白平衡等功能  
注意：当使用线缆同步时应最后启动Master相机

## 更新记录  
#### v0.5.2
增加了一个录制按钮和一个打印相机参数的工具
#### v0.5.1  
优化  
#### v0.5
增加backgroundMattingV2背景分割，使用libtorch(TorchScript)部署  
由于libtorch与QT的宏有冲突，添加了QT_NO_KEYWORDS选项，并修改了相关内容。(ps.后来我才知道，实际上只需要在包含torch头文件的位置处#undef slot即可)  
修复上个版本FPS实际显示的为处理时间
#### v0.4.2
一键启动按钮和保存图片按钮  
状态栏显示FPS  
以及一些小改动
#### v0.4.1
相机启动自动读取标定外参  
点云配准功能（但还不好用）  
修复了一个同步模式切换时的bug
#### v0.4
点云可视化内容  
改用CMake
#### v0.3
修改了多线程的方式以期望能多相机同步获取图像；  
重写了UI相关的内容，分组处理相同功能的控件，代码更精简了。  
#### v0.2 
加入颜色控制的部分功能。(ps.官方建议在进行线缆同步时使用手动曝光以避免帧率不稳定)  
#### v0.1
首个版本，ui基本完成。  

