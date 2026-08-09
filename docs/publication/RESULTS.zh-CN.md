# Minisforum N5（F8NAA）EC 风扇与温度接口逆向成果

状态：公开发布前成果稿  
实机验证日期：2026-08-08  
目标平台：Minisforum N5 / 主板 F8NAA / BIOS 1.04  
验证系统：Unraid 7.3.2 / Linux 6.18.38-Unraid

## 1. 项目目标

本项目为 Minisforum N5 主板补充 Linux 标准 hwmon 支持，使 Unraid 的
Dynamix Fan Auto Control 和 Dynamix System Temperature 能够使用主板 EC
提供的风扇与温度功能。

目标功能包括：

- CPU、SSD、HDD、PCIe 四路 PWM 控制。
- CPU、SSD、HDD 三路真实风扇转速。
- CPU、System、Board、Ambient 四路温度。
- PCIe 风扇根据选定 PCIe 设备温度联动。
- 安全的手动/自动模式切换和卸载恢复。

## 2. 硬件识别

实机 DMI 信息：

| 项目 | 值 |
|---|---|
| System Product | N5 |
| Board Name | F8NAA |
| Board Vendor | Shenzhen Meigao Electronic Equipment Co., Ltd. |
| EC/Super I/O | ITE IT5571 C Version |
| BIOS | 1.04 |

正式驱动应默认只允许 `N5 + F8NAA` DMI 匹配，避免在其他主板上访问 EC
端口。调试用 `force=1` 不应作为普通用户的安装方式。

## 3. PMC 与 EC 接口

### 3.1 主机接口

| 接口 | Data | Command/Status | 用途 |
|---|---:|---:|---|
| PMC1 / ACPI EC | `0x62` | `0x66` | 温度 EC RAM |
| PMC2 | `0x68` | `0x6c` | PWM 与 RPM 命令 |

PMC2 使用顶层命令 `0xd5`。旧分析中出现过的 `0xd9` 已被实机排除：
`0xd9` 超时，`0xd5` 可稳定工作。

### 3.2 PWM 协议

| Linux 通道 | 物理用途 | 固件 PWM | 写入子命令 | 手动 | EC 自动 |
|---|---|---|---|---:|---:|
| `pwm1` | CPU Fan | DCR1 | `0x23/25/27/29` | `0x20` | `0x21` |
| `pwm2` | SSD Fan | DCR2 | `0x2f` | `0x2d` | `0x2e` |
| `pwm3` | HDD Fan Group | DCR3 + DCR4 | `0x2c` | `0x2a` | `0x2b` |
| `pwm4` | PCIe Fan | DCR5 | `0x33/35/37/39` | `0x30` | `0x31` |

CPU 和 PCIe 通道包含四个固件曲线输出点。进入手动模式时，驱动将四个点
写成相同值，再执行对应的手动动作命令。

### 3.3 已公开 RPM 协议

| 转速 | Low | High | 实机用途 |
|---|---:|---:|---|
| TACH3 | `0x14` | `0x15` | HDD Fan |
| TACH2 | `0x16` | `0x17` | SSD Fan |
| TACH1 | `0x18` | `0x19` | CPU Fan |

组合公式为：

```text
rpm = (high << 8) | low
```

驱动采用 high-low-high 三次采样，避免 EC 刷新 16 位值时产生撕裂读数。

## 4. 温度寄存器

通过 CPU 压力测试和冷却过程验证了以下 EC RAM 字节：

| hwmon | 标签 | EC RAM | 验证表现 |
|---|---|---:|---|
| `temp1_input` | CPU Temp | `0x09` | 随 CPU 负载快速变化 |
| `temp2_input` | System Temp | `0x04` | 随系统热量缓慢变化 |
| `temp3_input` | Board Temp | `0x05` | 主板/辅助区域温度 |
| `temp4_input` | Ambient Temp | `0x06` | 环境/进风附近温度 |

实机示例读数：CPU 44°C、System 42°C、Board 40°C、Ambient 31°C。

该 BIOS 没有向 Linux 注册可供 `ec_read()` 使用的标准 ACPI EC 实例，因此
驱动使用实际工作的 `0x62/0x66` EC 事务读取温度。

## 5. PCIe Fan 实机验证

将一只已知能够输出 TACH 的 SSD PWM 风扇临时接到 PCIe Fan 插座：

- `pwm4=128`：相较全速明显降速，确认支持中间占空比调速。
- `pwm4=0`：约二十秒后完全停转。
- `pwm4=255`：从静止状态正常起转并全速运行。
- `pwm4_enable=2`：测试结束后恢复 EC 自动模式。

这证明 DCR5/第四路 PWM 与物理 PCIe Fan 插座对应。

恢复原始接线并重新启动 Unraid 后，驱动自动加载；CPU、SSD、HDD 三路转速
分别约为 2284、2269、1189 RPM，四路 PWM 均处于 EC 自动模式。重新安装的
NVIDIA Tesla P4 被 PCIe 子系统识别，并可通过 `nvidia-smi` 读取 GPU 温度
（本次启动示例为 46°C）。

### 5.1 PCIe RPM 结论

PCIe 风扇停转与全速时，三路固件公开转速保持不变：

```text
STOP: TACH1≈2005, TACH2=0, TACH3≈1279
FULL: TACH1≈2013, TACH2=0, TACH3≈1278
```

只读 I2EC 检查进一步得到：

| 寄存器 | 值 | 含义 |
|---|---:|---|
| `0x1848` | `0x08` | TACH0/TACH1 选择 A 输入 |
| `0x184f` | `0x02` | TACH2 选择 A 输入 |
| `0x16f4` | `0x00` | TACH0B/TACH1B 未启用 |
| `0x16fe` | `0x00` | TACH2B 未启用 |

IT5571 有三个测速计数器，每个可在 A/B 两个物理引脚间切换，但 N5 固件只
配置并公开三个 A 输入。B 引脚当前为普通 GPIO，强制切换可能影响其他主板
功能，因此不应在公开驱动中尝试。

最终结论：PCIe Fan 支持完整 PWM 控制，但没有安全、已确认的 RPM 反馈。
驱动应显示 `PCIe Fan (no RPM feedback)`，并让 `fan4_input` 返回 0。

## 6. Linux hwmon 设计

建议公开模块名：

```text
minisforum_n5_it5571
```

原型模块名为 `minisforum_n5_it5571`，正式发布前应统一改名。

标准节点：

```text
temp1_input ... temp4_input
fan1_input  ... fan4_input
fan1_min    ... fan4_min
pwm1        ... pwm4
pwm1_enable ... pwm4_enable
```

`pwmN_enable` 语义：

- `0`：全速故障保护。
- `1`：Linux/用户空间手动控制。
- `2`：恢复主板 EC/BIOS 自动策略。

驱动进入手动模式时先设置全速；卸载模块时无条件将四路恢复为 EC 自动模式。

## 7. Unraid 集成

### 7.1 Dynamix System Temperature

可用驱动列表只应保留：

```text
minisforum_n5_it5571
```

旧配置中的 `nct6775` 是遗留项，与该 N5 主板不匹配，应在迁移时删除。

### 7.2 Dynamix Fan Auto Control

插件可发现四个标准 `pwmN` 节点。前三路可通过真实 RPM 检测，PCIe 通道
没有 RPM，不能依赖插件的转速变化自动识别。

PCIe 温度联动应由配套服务完成，并允许选择明确的 hwmon 温度源：设备名称、
PCI 地址或序列号、温度通道和标签。不能假定所有 PCIe 设备都有相同探头。

## 8. 安全措施

- DMI 限制到 Minisforum N5/F8NAA。
- 使用 `request_region()` 独占 EC/PMC 端口。
- 所有 PMC2 事务使用同一互斥锁串行化。
- 写入范围限制为 PWM 0–255。
- 手动模式初始全速。
- 温度源丢失时 PCIe 联动服务全速保护。
- 服务退出和模块卸载均恢复 EC 自动模式。
- 内核版本不匹配时拒绝安装 `.ko`。

## 9. 已验证与未验证范围

已验证：

- Unraid 7.3.2 / `6.18.38-Unraid`。
- BIOS 1.04。
- CPU、SSD、HDD、PCIe 四路 PWM。
- CPU、SSD、HDD 三路 RPM。
- 四路 EC 温度。
- PCIe 无可用 RPM 反馈。
- 模块加载、卸载、手动模式和自动恢复。

正式社区发布前仍需完成：

- 模块统一重命名为 `minisforum_n5_it5571`。
- 制作按内核版本发布的 `.txz` 和 `.plg`。
- 完整测试插件安装、升级、卸载、重装。
- 提供公开源码、GPL-2.0 许可证和构建说明。

## 10. 公开发布注意事项

公开仓库可以包含自行编写的源码、构建脚本、测试记录和寄存器结论。不要上传
厂商 EC 固件镜像或标记为 Confidential 的 IT5571 数据手册。README 可链接
Linux 官方 hwmon 文档：

- https://docs.kernel.org/hwmon/hwmon-kernel-api.html
- https://docs.kernel.org/hwmon/sysfs-interface.html
