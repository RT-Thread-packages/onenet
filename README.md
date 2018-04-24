# OneNET

## 1. 介绍

[OneNET](https://open.iot.10086.cn/) 平台是一个基于物联网产业打造的生态平台，可以适配多种网络环境和协议类型，例如MQTT、HTTP、LWM2M等，方便用户数据的管理和设备控制。

该组件包是 RT-Thread 系统针对 OneNET 平台连接的适配，通过这个组件包可以让设备在 RT-Thread 上使用 MQTT 协议连接 OneNet 平台，完成数据的接受和发送、以及设备的控制等功能，更多 OneNET 平台信息可查看 [OneNET 文档中心](https://open.iot.10086.cn/doc)。

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| inc | 头文件目录 |
| src | 源文件目录 |
| samples | 例程文件目录 |
| port | 用户移植代码目录 |
| LICENSE |  OneNET package 许可证 |

### 1.2 许可证

OneNET package  遵循 GUN GPL 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT_Thread 3.0+
- [paho-mqtt](https://github.com/RT-Thread-packages/paho-mqtt.git)
- [webclient](https://github.com/RT-Thread-packages/webclient.git)
- [cJSON](https://github.com/RT-Thread-packages/cJSON.git)

## 2. 获取方式

使用 `OneNET package` 需要在 RT-Thread 的包管理中选中它，具体路径如下：

    RT-Thread online packages
        IoT - internet of things  --->
            [*] OneNET: China Mobile OneNet cloud SDK for RT-Thread
            [*]   Enable OneNET sample
            [*]   Enable support MQTT protocol 
            (/test_topic) mqtt subtopic
            (device id) device id
            (api key) api key
            (product id) product id
            (auth info) auth info
                onenet version (latest)  --->

`Enable OneNET sample`：使能使用示例代码；  
`Enable support MQTT protocol`：使能 MQTT 协议连接 OneNET；  
`mqtt subtopic`：自定义设备端连接 MQTT 协议时`订阅的主题`信息；  
`device id`：配置云端创建设备时获取的`设备ID`（具体使用和获取方式可参考 OneNET 文档中心[平台概述](https://open.iot.10086.cn/doc/art401.html#97)和[硬件接入](https://open.iot.10086.cn/doc/art454.html#107)）；  
`api key`：配置云端创建设备时获取的 `APIkey`；  
`product id`：配置云端创建产品时获取的`产品ID`；  
`auth info`：配置云端创建产品时`用户自定义的鉴权信息`(每个产品的每个设备唯一)；  
`onenet version`：配置 OneNET 组件包为最新版 `latest`；   

配置完成后让 RT-Thread 的包管理器自动更新，或者使用 pkgs --update 命令更新包到 BSP 中。

## 3. 移植

### 3.1 MQTT 接受数据处理函数实现
```
int onenet_port_data_process(char *recv_data, rt_size_t size)
```
该函数用于设备 MQTT 连接 OneNET 云端之后，对 OneNET 云端下发的数据进行处理，需要用户自定义处理数据方式。

## 4. 使用方式

### 4.1. 示例列表

| 名称 | 说明 |
| ---- | ---- |
| rt_onenet_sample.c | 数据上传 OneNET 云端示例 |

### 4.2. 运行示例

该示例用于设备连接 OneNET 云端后，需要`联网成功`之后 msh 中执行 `onenet_mqtt_init` 初始化MQTT协议设备上线，然后执行 `onenet_upload_cycle` 命令，可5秒一次循环发送数据流名为 `temperature` 的随机数据到 OneNET 云端，云端建立[数据流模板](https://open.iot.10086.cn/doc/art402.html#97)后即可实时查看上传数据信息。具体过程如下：

    msh />onenet_mqtt_init
    Enter mqtt_connect_callback!
    [MQTT] ipv4 address port: 6002
    [MQTT] HOST = '183.230.40.39'
    OneNET MQTT is startup!
    OneNET cloud(V0.1.0) initialize success.
    [MQTT] Subscribe #0 /test_topic OK!
    Enter mqtt_online_callback!             //初始化完成，设备上线成功
    msh />
    msh />onenet_upload_cycle
    buffer : {"temperature":8}
    buffer : {"temperature":56}
    buffer : {"temperature":19}             
    buffer : {"temperature":11}             //循环发送数据到 OneNET 云端
    .....


## 5. 注意事项

- 在 menuconfig 选项中选择 OneNET package 的 `latest` 版本；
- 在 menuconfig 选项中配置的 `device id`、`api key`、`product id`、`auth info` 等信息时需要和 OneNET 云端新建产品和新建设备时获取的信息一致；
- 初始化 OneNET package 之前需要设备`联网成功`；

## 6. 开发资源

- [OneNET 文档中心](https://open.iot.10086.cn/doc)

