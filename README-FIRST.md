# Minisforum N5 EC / IT5571 完整开发归档

English summary follows the Chinese section.

## 中文说明

这是截至 2026-08-08 的完整开发归档，包含 Minisforum N5/F8NAA 的 EC 风扇、
转速和温度接口逆向成果、Linux hwmon 驱动原型、诊断工具、构建脚本、已验证
内核模块以及中英文 GitHub/Unraid 发布文案。

### 当前状态

- 已验证硬件：Minisforum N5、F8NAA、BIOS 1.04、ITE IT5571 C。
- 已验证系统：Unraid 7.3.2、Linux 6.18.38-Unraid。
- 已验证原型模块名：`n5_ec_hwmon`。
- 计划公开模块名：`minisforum_n5_it5571`。
- 计划插件 ID：`minisforum-n5-it5571`。
- 当前 `.ko` 仅适用于 `6.18.38-Unraid`，不能用于其他内核。
- 最终公开改名、Community Applications `.plg`、干净环境重新编译和发布前
  回归测试尚未完成。

因此，本归档是开发和保存资料，不是可以直接提交 Community Applications 的
最终插件包。

### 推荐使用组合

- 本驱动：提供标准 hwmon PWM、RPM 和 EC 温度节点。
- FanCtrl Plus 2：推荐用于四路风扇自动调速。
- Dynamix System Temperature：推荐用于 CPU、System、Board、Ambient 温度展示。

同一个 `pwmN` 只能由一个风扇控制程序负责。不要同时让 FanCtrl Plus 2、
FanCtrl Plus 或 Dynamix Fan Auto Control 写入同一个 PWM。

### 安全说明

诊断工具和驱动能够直接访问 EC/Super I/O 端口。只应在已经验证的 N5/F8NAA
上以 root 使用。错误写入可能造成风扇停转、过热或主板功能异常。

归档已排除密码、IP 地址、SSH 会话、厂商 BIOS/EC 固件、固件反汇编、ITE
保密数据手册、Linux 源码树和编译缓存。

## English summary

This is the complete development archive as of 2026-08-08. It contains the
Minisforum N5/F8NAA EC fan, tachometer, and temperature research; the Linux
hwmon driver prototype; diagnostic source; build scripts; the validated kernel
module; and bilingual GitHub/Unraid publication drafts.

The validated prototype is still named `n5_ec_hwmon`. The planned public module
name is `minisforum_n5_it5571`, and the planned plugin ID is
`minisforum-n5-it5571`. The included `.ko` is only for `6.18.38-Unraid`.
Final public renaming, the Community Applications `.plg`, a clean rebuild, and
release regression testing are still pending. This archive must not be
submitted directly as the final CA plugin.

Recommended stack: this driver supplies hwmon nodes, FanCtrl Plus 2 performs
automatic fan control, and Dynamix System Temperature displays the EC sensors.
Assign each PWM channel to one controller only.

Vendor firmware, firmware disassembly, confidential documentation, credentials,
host addresses, build caches, and kernel source trees are intentionally excluded.
