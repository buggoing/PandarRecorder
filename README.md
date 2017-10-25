# for windows:

## 1. 下载并解压[WinPcap Developer's Pack](https://www.winpcap.org/devel.htm)和[pthreads-w32-2-9-1-release](ftp://sourceware.org/pub/pthreads-win32)到工程目录下
## 2. 下载并安装vs2010, cmake, [WinPcap](https://www.winpcap.org/install/)和[qt4-vs2010](http://download.qt.io/archive/qt/4.8/4.8.5/)(设置好qt的环境变量)
## 3. 编译
### （1）编译32位版本
* 用cmake-gui配置好编译器为vs2010，
* 打开终端，cd到项目目录，执行以下命令：  
`mkdir build ; cd build; cmake ..`  
* 生成相关文件后用vs2010编译
### （2）编译64位版本(请确保qt4也是64bit版的)
* 将CMakeLists.txt中 *set (BUILD_WIN64 true)* 前的注释去掉
* 用cmake-gui设置编译器为vs2010 Win64
* 打开终端，cd到项目目录，执行以下命令：  
`mkdir build ; cd build; cmake ..`  
* 生成相关文件后用vs2010编译


# for ubuntu
## 1. 安装cmake, libpcap, qt4，执行以下命令:
`sudo apt-get install cmake libpcap-dev libqt4-dev qt4-default`
## 2. 打开终端，cd到项目目录，执行以下命令进行编译：
`makir build; cd build; cmake ..; make`