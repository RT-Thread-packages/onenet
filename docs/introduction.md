# OneNET 软件包介绍 #

OneNET 平台是中国移动基于物联网产业打造的生态平台，可以适配多种网络环境和协议类型，例如MQTT、HTTP、LWM2M等，方便用户数据的管理和设备控制。

该组件包是 RT-Thread 系统针对 OneNET 平台连接的适配，通过这个组件包可以让设备在 RT-Thread 上使用 MQTT 协议连接 OneNet 平台，完成数据的接收、发送、设备的注册和控制等功能。

## 文件目录结构

``` {.c}
OneNET
│   README.md                       // 软件包使用说明
│   SConscript                      // RT-Thread 默认的构建脚本
├───docs 
│   │   api.md                      // API 使用说明
│   │   introduction.md             // 软件包详细介绍
│   │   port.md                     // 移植说明文档
│   └───user-guide.md               // 用户手册
├───inc                             // 头文件
├───src                             // 源文件
├───ports                           // 移植文件                 
│           rt_ota_key_port.c       // 移植文件模板
├───samples                         // 示例代码
│   └───onenet_sample.c             // 软件包应用示例代码
```

## OneNET 功能特点 ##


