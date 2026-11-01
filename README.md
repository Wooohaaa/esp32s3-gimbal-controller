# ESP32S3 Gimbal Controller

ESP32S3 云台控制固件：接收 RK3566 发送的目标坐标帧，按双轴比例微分控制更新舵机角度，并在长时间无目标时缓慢回到中心位置。

This repository contains an independently implemented ESP32S3 controller for the RK3566 tracker. The serial frame is compatible with the documented 7-byte interface; the protocol parser and PID controller here are original implementations.

RK3566 vision companion: [Wooohaaa/rk3566-vision-tracker](https://github.com/Wooohaaa/rk3566-vision-tracker).

## 硬件与接线

| ESP32S3 | 连接 | RK3566 / 外设 |
| --- | --- | --- |
| GPIO17 / UART1 RX | ← | Pin 8 / UART3_TX_M1 |
| GPIO18 / UART1 TX | → | Pin 10 / UART3_RX_M1 |
| GPIO5 | → | PAN 水平舵机信号 |
| GPIO4 | → | TILT 垂直舵机信号 |
| GND | ↔ | RK3566 与舵机电源共地 |

舵机建议使用独立、稳定的电源；接线前确认电压、电流能力和共地方式。实际 GPIO 与舵机方向可能需要按具体开发板和机构安装调整。

## 通信帧

```text
[0x2C, 7, x, y, 3, 4, 0x5B]
```

- `(0, 0)`：当前没有有效目标，保持画面中心输入。
- `(255, 255)`：云台回中命令。
- 其他坐标：目标位置，坐标基准为 320×240 画面中心 `(160, 120)`。

UART 使用 115200 baud、8-N-1。引脚、帧格式、控制周期、舵机脉宽和限位可在 `src/main.cpp` 顶部集中调整。

## 构建与烧录

安装 PlatformIO Core 后，在本目录运行：

```bash
pio run
pio run --target upload --upload-port <串口设备>
pio device monitor --baud 115200 --port <串口设备>
```

上传串口需要按当前电脑实际枚举结果指定，配置文件不绑定某台电脑的 COM 端口。

## 主要参数

当前工程默认使用 320×240 坐标中心、50 ms 控制周期、GPIO 4/5 舵机输出、GPIO 17/18 UART、500–2500 μs 舵机脉宽区间。机械限位、舵机方向及 PID 参数应结合实际云台逐项校准。

## 来源说明

早期系统借鉴了公开的 OpenMV/STM32 视觉云台项目，并使用兼容的串口帧格式。本仓库仅包含面向 ESP32S3 的独立实现，没有打包该上游仓库的源码。

## License

MIT. See [LICENSE](LICENSE).
