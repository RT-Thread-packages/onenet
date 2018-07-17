# OneNET API

## OneNET 初始化

> int onenet_mqtt_init(void);

OneNET 初始化函数，需要在使用 OneNET 功能前调用

| 参数     | 描述    |
| :-----   | :-----  |
|无        | 无   |
| **返回**    | **描述**  |
|0       | 成功 |
|-1        | 获得设备信息失败 |
|-2        | mqtt 客户端初始化失败 |


## mqtt 数据上传

> rt_err_t onenet_mqtt_publish(const char *topic, const uint8_t *msg, size_t len);




