# v0.2.3 — Unraid 7.3.3 kernel package, persistent cache, and icon fix

## English

The Plugins list now displays the bundled N5 icon. Unraid interprets the
`icon` attribute as a local PNG filename, while the previous plugin supplied
a GitHub URL and therefore fell back to the default plugin icon. The attribute
now uses `minisforum-n5-it5571.png`, matching the installed `images/` asset.


Upgrading Unraid 7.3.2 to 7.3.3 changes the kernel from `6.18.38-Unraid` to
`6.18.54-Unraid`. Previous releases did not provide a package for the new
kernel, so plugin installation failed and the hwmon device disappeared.

This release supplies separate, checksum-protected packages for both kernels.
The new module was rebuilt from the existing driver source using the running
Unraid kernel configuration. EC register mappings, PWM behavior, DMI guards,
and experimental read-only profiles are unchanged. The plugin version is
0.2.3; the unchanged driver still reports MODULE_VERSION 0.2.0.

The installer now caches the package and its SHA-256 sidecar on the boot flash,
verifies them before use, and reinstalls the same package into Unraid's RAM
filesystem on startup. It aborts on missing packages, invalid checksums,
package-installation errors, vermagic mismatches, and module-load failures.

Validation:
- N5 / F8NAA / BIOS 1.04, Unraid 7.3.3 / `6.18.54-Unraid`: normal module load,
  four temperature channels, three RPM inputs, four PWM nodes, and restoration
  of existing HDD/SSD FanCtrl Plus 2 bindings.
- Cached same-version installation was exercised on the running N5.
- Installer regression cases cover offline cache, corrupt cache refresh,
  missing package, invalid checksum, package-installation failure, wrong
  kernel, and module-load failure on both supported kernel names.
- The old `6.18.38-Unraid` package retains its previously validated module;
  it is repackaged with the 0.2.3 WebGUI. The full reboot cycle and a fresh
  7.3.2 installation were not repeated for this maintenance release.

The PCIe channel still has no usable tachometer feedback; zero RPM is expected.

## 中文

修复 Unraid“插件”列表图标：Unraid 将 `icon` 属性当作本地 PNG 文件名，
此前填入 GitHub URL 后无法找到文件，回退为默认插件图标。现在改为
`minisforum-n5-it5571.png`，与包内 `images/` 文件一致，无需重新设计图标。


Unraid 从 7.3.2 升级到 7.3.3 后，内核从 `6.18.38-Unraid` 变为
`6.18.54-Unraid`。此前缺少新内核的驱动包，导致插件安装失败、传感器消失。

v0.2.3 为两个内核分别提供匹配包及 SHA-256 校验文件。新模块使用运行内核的
原始配置重新编译；EC 寄存器、PWM 行为、DMI 限制和实验机型只读模式均未改变。
插件版本为 0.2.3，未改动的驱动 MODULE_VERSION 仍为 0.2.0。

安装器新增闪存缓存，每次使用前验证 SHA-256；重启时可将同版本包重新安装到
内存文件系统。缺包、校验失败、安装失败、vermagic 不匹配或模块加载失败均会
明确报错并中止安装。

已在 N5 / F8NAA / BIOS 1.04 的 Unraid 7.3.3 实机验证模块正常加载、四路温度、
三路转速、四路 PWM 节点，以及原 HDD/SSD FanCtrl Plus 2 绑定恢复；已验证缓存
重装。两个支持的内核名称均通过 7 项安装器回归测试。旧内核使用原已验证模块，
仅重新打包 WebGUI。此次维护发布未重复整机重启或 7.3.2 全新安装测试。

PCIe 风扇仍无可用 RPM 反馈，显示零为预期行为。
