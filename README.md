# SDui-LVGL

<p>
  <img src="https://img.shields.io/badge/licence-MIT-yellow.svg" />
  <img src="https://img.shields.io/badge/language-C-blue.svg" />
  <img src="https://img.shields.io/badge/platform-Linux-lightgrey.svg" />
  <img src="https://img.shields.io/badge/framework-LVGL-blue.svg" />
</p>

SDui-LVGL is a drawing board program based on the LVGL graphics framework, utilizing the stable diffusion API to call a remote server for text-to-image and image-to-image generation. This was my undergraduate thesis project, which has now concluded, so I intend to open-source it.

The principle behind it is roughly as follows: first, the canvas buffer is extracted and converted to BASE64 encoding. Based on user-selected prompts, generation modes, and network control types within the interface, it synthesizes JSON. Then, the image content is filled into appropriate locations and sent to either a local or remote server, with reception involving the inverse operation.

I’ve only been learning Linux for six months, and I haven’t previously worked with LVGL on any projects, so this code may appear rudimentary. However, with your efforts, it can improve.

## 1. 项目说明

本项目的前身是作为我的本科毕业设计，如今已经结束了相关流程，因此开源了此工程。

项目想法起源于各大平台都有调用 stableDiffusion API 的功能实现绘图，但是在嵌入式平台中却没有找到类似的项目(公开的)，由于本人对于 Linux 和 LVGL 的认知尚浅，因此想通过本项目提升技术。也因此，本项目有很多地方的代码并不高明，仅作为抛砖引玉的作用，也希望各位大佬可以迭代出更强大的版本！

对于项目的作用，大概在数位屏上比较贴切吧？（雾）

### 开发套件

屏幕分辨率为`800x480`，开发板为鲁班猫2（无所谓的），使用WSL+VScode开发，WSL也承担了交叉编译，在`makefile`中使用`sshpass`自动补全密码（美滋滋）。

对于屏幕触摸的标志位，单点触控和多点触控是不一样的，各位需要通过`evtest`获知自己的标志位是怎么样的。

如果想移植到单片机，那么各种回调函数和涉及linux相关的内容都需要改写。

### 基础界面

<div align="center">
  <img src="https://github.com/johnsionFarry/SDui-LVGL/blob/main/README.assets/1.jpg" alt="Image 1">
  <p>基础界面（直接选择风格和提示词进行文生图）</p>
</div>


<div align="center">
  <img src="https://github.com/johnsionFarry/SDui-LVGL/blob/main/README.assets/2.jpg" alt="Image 2">
  <p>图生图</p>
</div>

<div align="center">
  <img src="https://github.com/johnsionFarry/SDui-LVGL/blob/main/README.assets/3.jpg" alt="Image 3">
  <p>图生图</p>
</div>

<div align="center">
  <img src="https://github.com/johnsionFarry/SDui-LVGL/blob/main/README.assets/4.jpg" alt="Image 4">
  <p>带控制网的文生图</p>
</div>

<div align="center">
  <img src="https://github.com/johnsionFarry/SDui-LVGL/blob/main/README.assets/5.jpg" alt="Image 5">
  <p>带控制网的文生图</p>
</div>

<div align="center">
  <img src="https://github.com/johnsionFarry/SDui-LVGL/blob/main/README.assets/6.jpg" alt="Image 5">
  <p>开发界面</p>
</div>

<div align="center">
  <img src="https://github.com/johnsionFarry/SDui-LVGL/blob/main/README.assets/7.png" alt="Image 5">
  <p>调试界面</p>
</div>

PS：左下角黑色的小方块其实是光标，我在另一款开源的C++贪吃蛇中见到过使用printf方式隐藏的方式，但是忘了，后续补上

### 大致流程

只有一个屏幕，其他所有控件都是以活动屏幕为父对象创建的，并且简单粗暴地用全局变量保存各对象的句柄、标志位等。经过我的测试，发送到服务器的图片允许使用BMP格式（文件较大），但返回时只能为PNG格式（需要自己解码）。

画板部分套用了百问网的接口，其他按钮、下拉菜单、进图条、复选框为自己开发。画板初始化时需要制定一个缓存作为画布保存内容的空间，因此我们可以在缓存上做文章。通过Linux的文件IO、系统编程、网络编程等操作：

1. 将缓存变换为BMP格式的图片，再通过BASE64编码放在一旁备用（A）；
2. 根据下拉菜单和复选框的内容，条件合成JSON内容（主要是提示词、各种设置、控制网等，你在WEBui上能看到的，JSON上都是可调的），其中也包含要填入图像数据的地方（B）；
3. 将A和B数据进行组合，一般在JSON中有三个位置可以插入图像：最外部的`init_image`（基础图像）、在控制网中的`mask`（遮罩）和`input_image`（控制网输入图像）等；
4. 对于buffer->BMP的过程，LVGL的色彩通道顺序是BGR，而BMP的色彩通道顺序为RGB，因此只需要采用经典合成文件头+信息头+色彩顺序的方式，即可得到BMP图像
5. 对于PNG->BMP的过程，我采用调用其他第三方程序的方式实现（偷懒了）：使用`sprintf`合成指令，用`system`执行。但不知道为什么，我采用这种方式得到的BMP文件虽然不是调色板模式，但却有调色板的无效数据，因此需要额外更改偏移位才可以正常读取内容（代码中有体现）；
6. 服务器通讯刚开始使用了`Curl`库，但编译文件多了好多，我也仅仅是进行UDP通信（SD服务器会报，但兼容），因此一咬牙还是自己手搓了套接字通讯。
7. 接受流程大致为发送流程的逆操作；
8. TODU ...

### 代码逻辑解释

TODU. ..

## 2. 工程前期准备		

### 百问网画板接口

如果您阅读过百问网的相关代码，您会发现“**画板**”部分非常的相像：

- 百问网LVGL中文文档：[什么是Lvgl？ — 百问网LVGL中文教程手册文档 1.0 文档 (100ask.net)](https://lvgl.100ask.net/7.11/documentation/01_intro/intro.html)
- 百问网画板接口：[src/lv_100ask_sketchpad · 韦东山/lv_lib_100ask - 码云 - 开源中国 (gitee.com)](https://gitee.com/weidongshan/lv_lib_100ask/tree/master/src/lv_100ask_sketchpad)

关于画板中的色轮（颜色选择器），默认会触发长按选择色相时，跳转到饱和度选择，非常影响体验，参考正点原子的LVGL教程：[第45讲 部件篇-色环部件_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1CG4y157Px?p=45&vd_source=00fe280cdfc4a645876630b6c032bf30)的解释，添加以下代码可以屏蔽相关跳转：

`lv_colorwheel_set_mode_fixed(cw, true);`

### SD接口

我为了图方便，直接使用**秋葉aaaki**的整合包：[秋葉aaaki的个人空间-秋葉aaaki个人主页-哔哩哔哩视频 (bilibili.com)](https://space.bilibili.com/12566101?spm_id_from=333.788.0.0)

API 方面，如今已经有 v3 版本的接口，但是我使用的本地 SD 只有 v1 的版本。如果您在各大提供 post 等测试的平台看过 v3 接口的具体内容，其实具体细节的变化并不大，因此可以先研究最初始的，也可以触类旁通。

具体的 v1 接口示范：`http://你的IP:你的端口/docs`

网上也有相关的说明文档：

- (v1) 很详细：[全网最全stable diffusion webui API调用示例，包含controlneth和segment anything的API](https://blog.csdn.net/Python_anning/article/details/135269356)
- (v1) 有实操：[stable diffusion 远端跑图—— Api基础知识掌握](https://zhuanlan.zhihu.com/p/624042359)
- (v2)：[stable diffusion API 调用，超级详细代码示例和说明](https://juejin.cn/post/7265666505101164603) | [AI绘画专栏之 SDXL controlnet  API教程(36)](https://cloud.tencent.com/developer/article/2359971)

### LVGL框架移植

LVGL库：[lvgl/lvgl: Embedded graphics library to create beautiful UIs for any MCU, MPU and display type. (github.com)](https://github.com/lvgl/lvgl)

LVGL 框架版本选择：本文撰写时已经有 `9.0.0` 版本，而互联网上很多文章可能还在用 `8.3.x` 甚至是 `7.x.x`，此时并不需要惊慌，因为很多接口是通用的。当你使用一个函数，如果你的编译器会提供自动补全，而且有弹出相关的参数定义，那么您输入的函数大概率是可以使用的。但是不同版本的函数对参数的定义可能会不一样，因此这部分需要各位大佬自行解决。

前文提到我使用了百问网的画板接口，它并不兼容我的 `9.x.x` 版本，但是有取巧的方法：网上提供的接口为了稳定性，会在对应的头文件中加入版本控制的宏判断语句，对应的宏在框架的 有记录。因此我们只需要删除掉相关的接口判断语句即可，如果编译不通过再继续找问题。

对于LVGL调用硬件的方式：你可以把自己屏幕驱动中的画点函数，注册到LVGL框架中；也可以使用SDL（如在Win中模拟）、Frame Buffer（如Linux）等方式。

## 3. 常用的命令和项目依赖

### 命令

- 截图：`fbgrab -d /dev/fb0 /home/cat/play/1.png`
- 关闭图形显示：`systemctl set-default multi-user.target`
- 打开图形显示：`systemctl set-default graphical.target`

### 依赖库

- **imagemagick**（处理 PNG 和 BMP 转换）
  - 安装：`sudo apt install imagemagick`
  - 用法：
    - PNG转BMP：`sprintf(command, "convert %s %s", input, output);`

- **coreutils**（负责 BASE64 与文件的转换）
  - 安装：`sudo apt install coreutils`
  - 用法：
    - 编码到文件：`sprintf(base64_decode, "base64 -d %s > %s", input, output);`
    - 文件到编码：`sprintf(command, "base64 -w 0 %s > %s", bmp_file, base_name);`

TODO ...
