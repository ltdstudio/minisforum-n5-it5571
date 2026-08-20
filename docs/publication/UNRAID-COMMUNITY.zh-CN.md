# Unraid Community Applications 中文发布文案

> 发布前替换所有方括号占位符，并确认正式模块名和安装包已经完成实机测试。

## 应用名称

```text
Minisforum N5 EC / IT5571 Driver
```

## 推荐分类

```text
Drivers:System
```

## 简短描述

```text
适用于 Minisforum N5 系列 ITE IT5571 EC 的专用 hwmon 驱动。N5/F8NAA 已验证；N5 Pro/F8NAA、N5 Air/F8NAB 为实验性且默认只读。提供四路 PWM、三路转速与四路温度，推荐搭配 FanCtrl Plus 2 和 Dynamix System Temperature。
```

## 完整描述

```text
该插件为 Minisforum N5（F8NAA 主板、ITE IT5571 EC）安装专用 Linux hwmon 驱动。

提供：
• CPU、SSD、HDD、PCIe 四路 PWM 控制；
• CPU、SSD、HDD 三路真实 RPM；
• CPU、System、Board、Ambient 四路温度；
• 推荐搭配 FanCtrl Plus 2，实现硬盘、NVMe、CPU、NVIDIA GPU 等温度源到四路 PWM 的独立联动；
• 推荐搭配 Dynamix System Temperature，在 Unraid WebGUI/Dashboard 显示 CPU、System、Board、Ambient 温度；
• 兼容 Dynamix Fan Auto Control。

PCIe Fan 的 PWM 已经过物理风扇停转/全速验证，但主板固件没有提供可安全使用的 PCIe RPM 反馈，因此该通道显示 0 RPM。插件不会伪造转速。

推荐映射为：CPU 温度 → CPU Fan，NVMe 温度组 → SSD Fan，阵列硬盘温度组 → HDD Fan Group，NVIDIA GPU 或目标 PCIe 设备辅助温度 → PCIe Fan。本机 NVIDIA Tesla P4 已验证能够读取 GPU 温度。FanCtrl Plus 2 需单独安装；不得与上游 FanCtrl Plus 同时运行，也不得让多个控制插件同时写入相同 PWM。

推荐软件分工：Minisforum N5 EC / IT5571 Driver 提供底层 hwmon 节点，FanCtrl Plus 2 负责自动调速，Dynamix System Temperature 负责温度选择和界面展示。

已识别配置包括 N5/F8NAA（已验证）、N5 PRO/F8NAA（实验性）以及 N5A 或 N5 AIR/F8NAB（实验性）。实验性配置默认只开放温度/RPM，必须显式设置 `experimental_write=1` 才开放 PWM。已有一位用户反馈 MS-A2 使用良好，但在收集准确 DMI 与通道映射前不自动加入白名单。内核模块必须与当前 Unraid 内核完全匹配。
```

## 安装警告

```text
此插件直接控制嵌入式控制器风扇输出，N5 Pro 与 N5 Air 支持仍属实验性。如出现风扇停转、异常全速、控制到错误接口、通道消失或温度异常，必须立即终止、卸载模块并恢复 BIOS 控制；实验期间不得无人值守。安装前确认内核包匹配，同一个 PWM 只能由一个控制器负责。
```

## 首次发布更新日志

```text
### 0.2.0

- 新增实验性 N5 Pro/F8NAA 与 N5 Air/F8NAB 配置。
- 实验性配置默认只读，PWM 写入需要用户显式确认。
- 新增异常立即终止说明，并只恢复本次实际修改的通道。
- 记录 MS-A2 用户正面反馈，但暂不自动加入白名单。

### 0.1.0

- Initial public release for Minisforum N5/F8NAA.
- Added four PWM channels: CPU, SSD, HDD, and PCIe.
- Added CPU, SSD, and HDD RPM monitoring.
- Added CPU, System, Board, and Ambient temperatures.
- Added EC automatic-mode restoration on unload.
- Documented recommended FanCtrl Plus 2 integration and four-zone mapping.
- Documented recommended Dynamix System Temperature integration.
- PCIe PWM validated on hardware; PCIe RPM is not available.
- Validated on Unraid 7.3.2 / 6.18.38-Unraid / BIOS 1.04.
```

## 支持帖标题

```text
[Support] Minisforum N5 EC / IT5571 Driver - Fan PWM, RPM and Temperature Sensors
```

## 支持帖正文

```text
本帖用于支持 Minisforum N5 EC / IT5571 Driver。

支持硬件：
• Minisforum N5
• 主板 F8NAA
• BIOS 1.04（已验证）
• ITE IT5571 EC

功能：
• CPU Fan：PWM + RPM
• SSD Fan：PWM + RPM
• HDD Fan Group：PWM + RPM
• PCIe Fan：PWM，无 RPM 反馈
• CPU/System/Board/Ambient 温度

推荐控制插件：
• FanCtrl Plus 2：https://github.com/andrebrait/fanctrlplus
• CPU Temp → CPU Fan
• NVMe 温度组 → SSD Fan
• 阵列硬盘温度组 → HDD Fan Group
• NVIDIA GPU/目标 PCIe 设备温度 → PCIe Fan
• 不要同时运行 FanCtrl Plus、FanCtrl Plus 2 或其他写入相同 PWM 的控制配置

推荐温度显示插件：
• Dynamix System Temperature
• 可选择 CPU Temp、System Temp，并按需显示 Board Temp、Ambient Temp
• 不需要 `it87` 或 `nct6775`；选择本驱动提供的 EC 温度节点

PCIe Fan 说明：
已使用真实 PWM 风扇验证 pwm4=0 可以停转，pwm4=255 可以全速。IT5571 虽然有三个可切换 A/B 输入的测速计数器，但该主板固件只配置并公开三路 A 输入；PCIe 通道没有可安全使用的 RPM 读取路径。

报告问题时请提供：
1. Unraid 版本
2. uname -r
3. BIOS 版本
4. dmidecode -t system -t baseboard
5. modinfo minisforum_n5_it5571
6. sensors
7. dmesg | grep -i minisforum_n5

项目源码：https://github.com/ltdstudio/minisforum-n5-it5571
问题跟踪：https://github.com/ltdstudio/minisforum-n5-it5571/issues
```

## Community Applications 元数据建议

| 字段 | 建议值 |
|---|---|
| Name | Minisforum N5 EC / IT5571 Driver |
| Repository | `https://raw.githubusercontent.com/ltdstudio/minisforum-n5-it5571/main/minisforum-n5-it5571.plg` |
| Support | `https://github.com/ltdstudio/minisforum-n5-it5571/issues` |
| Project | `https://github.com/ltdstudio/minisforum-n5-it5571` |
| Category | Drivers:System |
| License | GPL-2.0-only |
| Min Unraid | 7.3.2（首个仅验证版本） |

## 手动发布检查表

- [ ] 公开仓库中模块名统一为 `minisforum_n5_it5571`。
- [ ] 删除所有密码、IP 地址、SSH 记录和本机路径。
- [ ] 不上传 EC 固件或 IT5571 保密 PDF。
- [ ] 添加 `LICENSE`、中英文 README、源码和构建说明。
- [ ] 为每个支持的 `uname -r` 发布单独驱动包。
- [ ] `.plg` 下载后校验 SHA-256 或 MD5。
- [ ] 不匹配的 DMI 或内核必须拒绝安装。
- [ ] 在干净环境验证安装、重启、升级、卸载、重装。
- [ ] 卸载和失败路径均恢复 EC 自动风扇模式。
- [ ] RAW `.plg` URL 可通过 Unraid Plugins 页面手动安装。
- [ ] 创建 Unraid 支持帖并把 URL 写回 `.plg`。
- [ ] 最后再提交 Community Applications 收录。
