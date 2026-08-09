# Minisforum N5 EC / IT5571 Driver for Unraid

<p align="center">
  <img src="icons/minisforum-n5-it5571-512.png" width="256" alt="Minisforum N5 EC / IT5571 Driver">
</p>


[English](README.md) | 简体中文

`minisforum_n5_it5571` 是面向 Minisforum N5（F8NAA 主板、ITE IT5571 EC）的 Linux hwmon
内核驱动，为 Unraid 提供四路风扇 PWM、三路真实转速和四路 EC 温度。

## 功能

- CPU Fan：PWM + RPM。
- SSD Fan：PWM + RPM。
- HDD Fan Group：PWM + RPM。
- PCIe Fan：PWM，无可用 RPM 反馈。
- CPU、System、Board、Ambient 四路温度。
- 推荐搭配 FanCtrl Plus 2，实现多路 PWM 与硬盘、NVMe、CPU、NVIDIA GPU
  等温度源联动。
- 兼容 Dynamix Fan Auto Control。
- 推荐搭配 Dynamix System Temperature，显示 CPU、System、Board、Ambient
  四路 EC 温度。
- 模块卸载或服务停止时恢复 EC/BIOS 自动控制。

## 截图 Screenshots

实机截图（验证环境：Unraid 7.3.2，内核 6.18.38-Unraid）。

**插件状态页** — 四路风扇通道（PWM / 模式 / 转速）与四路 EC 温度：

![插件状态页](screenshots/plugin-page.png)

**Dynamix System Temperature** — 驱动提供的 CPU、主板与阵列风扇传感器：

![Dynamix System Temperature](screenshots/system-temperature.png)

**FanCtrl Plus 2** — 四区 PWM 控制，绑定驱动的 `pwm1`–`pwm4`：

![FanCtrl Plus 2](screenshots/fanctrlplus2.png)

## 兼容性

| 项目 | 已验证版本 |
|---|---|
| 设备 | Minisforum N5 |
| 主板 | F8NAA |
| BIOS | 1.04 |
| Unraid | 7.3.2 |
| Kernel | 6.18.38-Unraid |

内核模块必须与 `uname -r` 完全匹配。升级 Unraid 内核后，需要安装对应的新
驱动包。驱动默认拒绝在非 N5/F8NAA 系统上加载。

使用同一 IT5571 芯片并不等于自动兼容。不同厂商可能使用不同 PMC 命令、
端口、EC RAM 和风扇接线。目前兼容列表只有经过完整实机验证的 N5/F8NAA；
其他 IT5571 主板可以提交 DMI、固件和测试数据申请适配，验证后再加入列表。

公开资料中采用 IT5571 的其他设备包括
[Avalue EMX-EHLP](https://www.avalue.com/en/product/Industrial-Embedded-Motherboard/Mini-ITX/EMX-EHLP)
工业主板和 [System76 Pangolin（pang13）](https://system76.com/tech-docs/models/pang13/README.html)。
这些型号只是“潜在适配目标”，并非当前受支持设备；请勿绕过 DMI 检查强制加载。

## hwmon 通道

| 节点 | 功能 |
|---|---|
| `pwm1`, `fan1_input` | CPU Fan |
| `pwm2`, `fan2_input` | SSD Fan |
| `pwm3`, `fan3_input` | HDD Fan Group |
| `pwm4`, `fan4_input` | PCIe Fan；RPM 固定为 0 |
| `temp1_input` | CPU Temp |
| `temp2_input` | System Temp |
| `temp3_input` | Board Temp |
| `temp4_input` | Ambient Temp |

PCIe Fan 已实机验证能够在 `pwm4=128` 时明显降速、在 `pwm4=0` 时停止，
并能从静止状态通过 `pwm4=255` 可靠起转至全速。重启后四路 PWM 均能恢复
EC 自动模式。
主板固件没有启用或公开可安全使用的 PCIe TACH，因此不会伪造 RPM。

## 安装

### Community Applications

正式收录后，在 Unraid Apps 中搜索：

```text
Minisforum N5 EC / IT5571 Driver
```

### 手动安装插件

在 Unraid 的 **Plugins → Install Plugin** 输入：

```text
https://raw.githubusercontent.com/ltdstudio/minisforum-n5-it5571/main/minisforum-n5-it5571.plg
```

发布者必须为每个支持的 Unraid 内核提供匹配的 release 资产。

## 推荐搭配：Dynamix System Temperature

推荐使用 **Dynamix System Temperature** 作为温度显示层。本驱动负责提供
标准 hwmon 温度节点，Dynamix System Temperature 负责在 Unraid WebGUI 和
Dashboard 中选择并显示温度。

安装后进入：

```text
Settings → System Temperature
```

可用驱动选择 `minisforum_n5_it5571`，然后选择 CPU Temp 和 System Temp。
还可以按需显示 Board Temp 和 Ambient Temp。原先遗留的 `nct6775` 不适用于
本机，应从已选驱动中移除。

推荐分工：

- **Dynamix System Temperature**：温度发现、选择和 Dashboard 展示。
- **FanCtrl Plus 2**：根据温度自动调节四路风扇 PWM。
- **Minisforum N5 EC / IT5571 Driver**：提供底层 PWM、RPM 和 EC 温度节点。

## 推荐搭配：FanCtrl Plus 2

推荐使用社区插件
[FanCtrl Plus 2](https://github.com/andrebrait/fanctrlplus)
作为本驱动的用户态风扇控制层。它能够独立控制多路 PWM，并可读取硬盘、
NVMe、CPU、auxiliary hwmon、StorCLI 和 NVIDIA GPU 温度，适合 N5 的四个
散热区域。

在 Community Applications 中搜索：

```text
FanCtrl Plus 2
```

如果当前 CA feed 尚未显示，可在 **Plugins → Install Plugin** 使用项目提供的
安装地址：

```text
https://raw.githubusercontent.com/andrebrait/fanctrlplus/main/plugin/fanctrlplus2.plg
```

建议映射：

| N5 控制器 | 推荐温度源 |
|---|---|
| CPU Fan / `pwm1` | CPU Temp |
| SSD Fan / `pwm2` | 对应 NVMe 温度组 |
| HDD Fan Group / `pwm3` | 阵列硬盘温度组 |
| PCIe Fan / `pwm4` | NVIDIA GPU 或目标 PCIe 设备的辅助温度 |

本机的 NVIDIA Tesla P4 已验证可由 `nvidia-smi` 读取 GPU 温度，FanCtrl Plus 2
支持 NVIDIA GPU 温度源，因此推荐将 P4 温度绑定到 PCIe Fan。PCIe Fan 没有
RPM 反馈，界面显示 0 RPM 属于正常现象。

FanCtrl Plus 2 与上游 FanCtrl Plus 不能同时运行。也不要让 Dynamix Fan Auto
Control 或其他服务同时控制相同的 `pwmN`，否则多个控制循环会互相覆盖。

### Dynamix Fan Auto Control（备选）

标准 Dynamix Fan Auto Control 仍可识别本驱动的 hwmon PWM；若使用它，必须
先停用 FanCtrl Plus 2，并重新选择 N5 控制器，不能沿用旧 `it87` 路径。

## PCIe 温度源

不同 PCIe 设备的温度传感器不统一。配置时必须选择具体设备与传感器，例如：

- AMD GPU / Edge Temperature / PCI 地址。
- NVIDIA GPU / GPU Temperature / PCI 地址。
- 10GbE NIC / PHY Temperature / PCI 地址。
- NVMe / Composite Temperature / 序列号。

配置 PCIe Fan 前必须确认插座确实连接了风扇，并在 FanCtrl Plus 2 中验证温度
源和 PWM 映射。不同 PCIe 设备不能共用一个固定的温度读取方法。

## 安全说明

- 不要在其他 Minisforum 型号或其他主板上强制加载。
- 不要安装与当前内核不匹配的 `.ko`。
- 修改 PWM 前确认风扇与散热区域的对应关系。
- 同一个 `pwmN` 只能交给一个风扇控制插件。
- 驱动和配套服务退出时会恢复 `pwmN_enable=2`。

## 构建

需要与目标 Unraid 内核完全一致的内核源码和 `.config`：

```bash
make -C /path/to/linux M="$PWD" modules
modinfo minisforum_n5_it5571.ko
```

发布前必须确认 `vermagic` 与目标 `uname -r` 完全一致。

## 许可证

驱动源码计划以 GPL-2.0-only 发布。仓库不包含厂商 EC 固件或 IT5571 保密
数据手册。

## 支持

- GitHub Issues: `https://github.com/ltdstudio/minisforum-n5-it5571/issues`
- Unraid Support Thread: `https://github.com/ltdstudio/minisforum-n5-it5571/issues`

提交问题时请附上 Unraid 版本、`uname -r`、BIOS 版本、DMI 主板信息、
`modinfo minisforum_n5_it5571` 和相关 `dmesg` 输出。
