# Minisforum N5 / IT5571 PWM 与 RPM 读取接口逆向

日期：2026-08-08  
固件：F8NAA EC V0.14（`ecfw.bin`）

## 结论

N5 固件已经实现三路风扇转速读取与换算，并通过 **PMC2 主机接口的 `0xD5` 命令族**提供六条单字节子命令。Linux 不需要直接访问 EC 内部 XDATA，也不需要自己读取不稳定的 TACH 硬件计数器。

| 通道 | 低字节命令 | 高字节命令 | 固件 RPM 缓存 |
|---|---:|---:|---:|
| TACH1 | `0x18` | `0x19` | `XDATA 0x8880-0x8881` |
| TACH2 | `0x16` | `0x17` | `XDATA 0x8882-0x8883` |
| TACH3 | `0x14` | `0x15` | `XDATA 0x88D0-0x88D1` |

组合方式：

```text
RPM = (high_byte << 8) | low_byte
```

TACH1/TACH2/TACH3 与 BIOS 中 CPU/SSD/HDD Fan 的显示顺序仍需在实机上对照一次，因此工具暂时使用 TACH 名称，避免误标。

PWM 当前请求值仍可从标准 ACPI EC RAM `0x34` 读取。此前已经实测：写 `0x00` 停转、写 `0x64` 全速，说明该字节使用 0-100 百分比语义。它是固件控制请求值，不等于硬件 DCR 的原始计数值。

## PMC2 端口与协议

IT5571 的 PMC2 为逻辑设备 `LDN 0x12`：

- Super I/O 配置端口：`0x2E/0x2F`
- `LDN 0x12` 的 `0x60/0x61`：PMC2 data port
- `LDN 0x12` 的 `0x62/0x63`：PMC2 command/status port
- 数据手册默认/legacy 映射：data `0x68`，command/status `0x6C`

读取命令的流程：

1. 等待 status 的 `IBF`（bit 1）清零。
2. 向 PMC2 command port 写命令族 `0xD5`。
3. 再次等待 `IBF` 清零。
4. 向 PMC2 data port 写 `0x14-0x19` 子命令。
5. 等待 status 的 `OBF`（bit 0）置位。
6. 从 PMC2 data port 读取响应字节。

PMC2 中断处理程序在 `0xF9BD` 将顶层命令 `0xD5` 设为两阶段事务；收到 data 子命令后，`0xF999` 把 `0xD5` 分派到 `0xF6FD`。子命令表位于 `0xF70B`，对应处理函数如下：

```text
0x14 -> 0xF772 -> TACH3 low
0x15 -> 0xF777 -> TACH3 high
0x16 -> 0xF77C -> TACH2 low
0x17 -> 0xF781 -> TACH2 high
0x18 -> 0xF786 -> TACH1 low
0x19 -> 0xF78E -> TACH1 high
```

响应由 `0xF6F7` 写入 `XDATA 0x1511`，即 PMC2 Data Out 寄存器。

## TACH 硬件与固件换算证据

IT5571 PWM 模块的 EC XDATA 基址是 `0x1800`：

| 功能 | 地址 |
|---|---:|
| PWM0-7 duty（DCR0-7） | `0x1802-0x1809` |
| TACH1 LSB/MSB | `0x181E/0x181F` |
| TACH2 LSB/MSB | `0x1820/0x1821` |
| TACH3 LSB/MSB | `0x1845/0x1846` |

固件读取位置：

- `0x9EDC`：读取 TACH1
- `0xA1D1`：读取 TACH2
- `0x9D28`：读取 TACH3

固件在 `0xAC1D` 准备常数 `0x0020E6DA = 2,156,250`，随后调用 `0x53B7` 做除法：

```text
RPM = 2,156,250 / tach_count
```

这与数据手册公式完全一致：EC 时钟 9.2 MHz，TACH 采样时钟为 `9.2 MHz / 128 = 71,875 Hz`，风扇每转 2 个脉冲：

```text
RPM = 60 * 71,875 / (tach_count * 2)
    = 2,156,250 / tach_count
```

## 对旧成果的纠正

旧 `ecfw.asm` 所用反汇编器在 `MOV DPTR,#imm16` 中只打印了高字节。例如：

```text
原始字节 90 18 46
错误显示 mov dptr, 0x18
正确显示 mov dptr, #0x1846
```

因此旧报告把若干 `0x96xx` 地址误判为 `XDATA 0x96`，并进一步把 `0x96` 判断为内部 PWM 执行寄存器。该内部地址结论无效。已修正反汇编器并生成 `ecfw.corrected.asm`。`EC RAM 0x34` 的实机读写结论不受这个反汇编错误影响。

## 工具

本成果包含：

- `n5_fan.c`：源码
- `n5_fan`：x86-64 Linux 静态二进制

用法：

```bash
chmod +x n5_fan
sudo ./n5_fan probe
sudo ./n5_fan status
sudo ./n5_fan rpm
sudo ./n5_fan pwm
sudo ./n5_fan raw 0x18
```

`status` 同时输出 `EC[0x34]` 的 PWM 请求百分比和三路 RPM。程序先从 `LDN 0x11/0x12` 动态读取 PMC1/PMC2 端口，不硬编码假设；读取 16 位 RPM 时使用 high-low-high 三次采样，避免固件刷新导致撕裂值。

## 实机验证状态

已通过 SSH `44322` 在目标 Unraid 实机完成只读验证：

- 主机：Unraid `7.3.2`，Linux `6.18.38-Unraid`，x86-64
- EC：ITE `IT5571`
- PMC1：data `0x62`，command/status `0x66`
- PMC2：data `0x68`，command/status `0x6C`
- 工具位置：`/mnt/user/public/N5-fan-reader-2026-08-08/n5_fan`
- SHA-256：`cbfe021d87419abfe33c3cc5f2ba5223bb06f9c1a9470ebd5db1ba689021e2d3`

2026-08-09 CST（2026-08-08 PDT）的一次 `status` 输出：

```text
PWM request: 100% (EC[0x34]=0x64)
TACH1: 1987 RPM
TACH2: 2255 RPM
TACH3: 1272 RPM
```

随后连续五次、间隔两秒采样的范围：TACH1 `1981-2002 RPM`、TACH2 `2236-2264 RPM`、TACH3 `1269-1285 RPM`，全部成功，无超时。独立运行 `pwm` 和 `rpm` 子命令也成功。

旧 BIOS 记录按 CPU/SSD/HDD 顺序约为 `2498/2234/1141 RPM`；据数值顺序可初步推定 TACH1=CPU、TACH2=SSD、TACH3=HDD，但本次无法同时查看 BIOS 页面，工具仍保留 TACH 标签，避免把推定写成已证实映射。

连接与读取命令：

```bash
ssh -p <SSH_PORT> <SSH_USER>@<UNRAID_HOST>
/mnt/user/public/N5-fan-reader-2026-08-08/n5_fan probe
/mnt/user/public/N5-fan-reader-2026-08-08/n5_fan status
/mnt/user/public/N5-fan-reader-2026-08-08/n5_fan pwm
/mnt/user/public/N5-fan-reader-2026-08-08/n5_fan rpm
```

验证过程没有写 PWM、没有改变 EC 值，也没有修改 Unraid 配置。
