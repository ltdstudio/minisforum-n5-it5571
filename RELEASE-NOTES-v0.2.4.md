# v0.2.4 — Unraid 7.3.4 / 6.18.58-Unraid

## English

Unraid 7.3.4 changes the kernel to `6.18.58-Unraid`. Earlier plugin releases
have no matching package for that kernel, so the plugin fails to install and
its temperature, RPM, and PWM nodes are unavailable.

This release adds a module built for `6.18.58-Unraid` from the existing driver
source and the running Unraid kernel configuration. It also provides 0.2.4
packages for `6.18.38-Unraid` and `6.18.54-Unraid`, retaining their previously
validated modules. Each kernel package has a SHA-256 sidecar.

Driver EC/PWM behavior, the DMI allowlist, experimental read-only profiles,
verified boot-flash cache, and the bundled Plugins list icon are unchanged.
The plugin and package version is 0.2.4; the unchanged driver source still
reports MODULE_VERSION 0.2.0.

Validation: N5 / F8NAA / BIOS 1.04, Unraid 7.3.4 / `6.18.58-Unraid`;
module load, four temperatures, three RPM inputs, four PWM nodes, and the
existing FanCtrl Plus 2 bindings. Installer regression cases cover valid
offline cache, corrupt cache refresh, missing package, invalid checksum,
package-installation failure, wrong kernel, and module-load failure for all
three supported kernel names. The older modules are repackaged without
recompilation. A full machine reboot and fresh installations of 7.3.2/7.3.3
were not repeated for this maintenance release.

The PCIe header still has no usable RPM feedback; zero RPM remains expected.

## 中文

Unraid 7.3.4 将内核升级至 `6.18.58-Unraid`，此前的插件缺少匹配包，导致
安装失败，无法提供温度、转速和 PWM 节点。

v0.2.4 使用原驱动源码及运行内核的原始配置编译新模块，同时保留
`6.18.38-Unraid`、`6.18.54-Unraid` 的独立安装包及 SHA-256 校验文件。
旧内核继续使用原已验证模块，仅更新插件/WebGUI 包版本。

EC/PWM 行为、DMI 限制、实验机型只读模式、校验缓存及插件列表本地图标均
保持不变。插件/包版本为 0.2.4，驱动 MODULE_VERSION 仍为 0.2.0。

已在 N5 / F8NAA / BIOS 1.04 的 Unraid 7.3.4 验证模块加载、四路温度、三路
转速、四路 PWM 节点和原 FanCtrl Plus 2 配置恢复。三个支持内核名称分别运行
七项安装器回归测试。此次维护未重复整机重启或旧版系统全新安装测试。

PCIe 风扇仍没有可用转速反馈，零 RPM 为预期行为。
