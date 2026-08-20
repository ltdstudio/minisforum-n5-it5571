# Minisforum N5 EC / IT5571 完整开发归档

> **免责声明 DISCLAIMER：** 本项目为社区爱好者独立开发，与 Minisforum（铭凡）
> 官方无关；包含未验证因素，按“现状”提供，不作任何担保。使用即表示您自行承担
> 全部风险。完整条款见 [DISCLAIMER.md](DISCLAIMER.md)。

English summary follows the Chinese section.

## 中文说明

这是 Minisforum N5 系列 IT5571 EC 风扇、
转速和温度接口探索成果、Linux hwmon 驱动原型、诊断工具、构建脚本、已验证
内核模块以及中英文 GitHub/Unraid 发布文案。

### 当前状态

- 已验证硬件：Minisforum N5、F8NAA、BIOS 1.04、ITE IT5571 C。
- 实验性配置：N5 Pro/F8NAA、N5 Air/F8NAB（默认只读，PWM 需显式确认）。
- 社区反馈：一台 MS-A2 使用良好；尚未加入自动白名单。
- 已验证系统：Unraid 7.3.2、Linux 6.18.38-Unraid。
- 已验证原型模块名：`minisforum_n5_it5571`。
- 公开模块名：`minisforum_n5_it5571`。
- 插件 ID：`minisforum-n5-it5571`。
- 当前 `.ko` 仅适用于 `6.18.38-Unraid`，不能用于其他内核。
- v0.2.0 已完成源码编译、模块元数据、包结构、脚本与 XML 检查；新增机型仍需
  用户完成物理风扇通道验证。

N5 Pro/N5 Air 如出现风扇停转、异常运行、通道错配或温度异常，必须立即终止、
卸载模块并恢复 BIOS 控制。

### 推荐使用组合

- 本驱动：提供标准 hwmon PWM、RPM 和 EC 温度节点。
- FanCtrl Plus 2：推荐用于四路风扇自动调速。
- Dynamix System Temperature：推荐用于 CPU、System、Board、Ambient 温度展示。

同一个 `pwmN` 只能由一个风扇控制程序负责。不要同时让 FanCtrl Plus 2、
FanCtrl Plus 或 Dynamix Fan Auto Control 写入同一个 PWM。

### 安全说明

诊断工具和驱动能够直接访问 EC/Super I/O 端口。实验性机型默认只读；只有完成
读数核对并明确接受风险后才可开启 PWM。错误写入可能造成风扇停转、过热或主板
功能异常。

归档已排除密码、IP 地址、SSH 会话、厂商 BIOS/EC 固件、固件反汇编、ITE
保密数据手册、Linux 源码树和编译缓存。

## English summary

This archive contains Minisforum N5-family EC fan, tachometer, and temperature
research; the Linux
hwmon driver prototype; diagnostic source; build scripts; the validated kernel
module; and bilingual GitHub/Unraid publication drafts.

The public module is `minisforum_n5_it5571`, and the plugin ID is
`minisforum-n5-it5571`. N5/F8NAA is validated. N5 Pro/F8NAA and N5 Air/F8NAB
are experimental read-only-first profiles. One MS-A2 user reports success,
but MS-A2 is not auto-whitelisted. The included `.ko` is only for
`6.18.38-Unraid`.

Recommended stack: this driver supplies hwmon nodes, FanCtrl Plus 2 performs
automatic fan control, and Dynamix System Temperature displays the EC sensors.
Assign each PWM channel to one controller only.

Vendor firmware, firmware disassembly, confidential documentation, credentials,
host addresses, build caches, and kernel source trees are intentionally excluded.
