# k4a_capture
多个Azure Kinect DK的同步采集、显示、标定等。
![window](http://github.com/LuyaooChen/k4a_capture/blob/main/window.png)

## 更新记录  
#### v0.5.1  
优化  
#### v0.5
增加backgroundMattingV2背景分割，使用libtorch(TorchScript)部署  
由于libtorch与QT的宏有冲突，添加了QT_NO_KEYWORDS选项，并修改了相关内容。  
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
加入颜色控制的部分功能。  
#### v0.1
首个版本，ui基本完成。  

