# HttpMenu/网页菜单

这是一个测试程序模块，用于实践测试http菜单的可行性，主要用到的依赖库有[jansson](https://github.com/akheron/jansson)和[cpp-httplib](https://github.com/yhirose/cpp-httplib)

本项目仅为测试与实践可行性使用，您也可以将其修改后用于您自己的项目

### 基本原理

客户端向 服务器IP/api/control 发送get请求获取相关变量，浏览器将相关变量显示在网页上

浏览器修改相关变量的值后发送post请求到同一地址实现修改服务器程序中的变量值

### 测试环境

服务器端：Ubuntu22.04.1

客户机端：Chrome On Windows11（理论上支持使用浏览器即可）

在服务器端下载源码编译后，打开程序，会显示本机的IP地址。

客户端于服务器端处于同一网络下，使用客户端浏览器打开服务器的IP地址+端口即可（服务器端需允许相应端口的出入站）