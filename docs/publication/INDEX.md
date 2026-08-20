# Minisforum N5 EC / IT5571 发布文案索引

本目录只包含发布文案草稿，没有执行 GitHub 或 Unraid Community Applications
上传。发布工作由维护者手动完成。

## 最终命名约定

| 用途 | 名称 |
|---|---|
| 公开显示名 | `Minisforum N5 EC / IT5571 Driver` |
| Linux 模块名 | `minisforum_n5_it5571` |
| Unraid 插件 ID | `minisforum-n5-it5571` |
| 建议 `.plg` 文件 | `minisforum-n5-it5571.plg` |

显示名明确标出 IT5571；N5/F8NAA 已验证，N5 Pro/F8NAA 与 N5 Air/F8NAB
作为默认只读、显式开启 PWM 的实验性配置发布。
其他 IT5571 设备不能仅凭芯片型号直接安装，需要建立并验证单独的主板配置。
README 和社区文案可列出 Avalue EMX-EHLP、System76 Pangolin（pang13）等公开采用
IT5571 的设备作为潜在适配目标，但不得将其表述为已经兼容。

推荐用户态风扇控制层写作 `FanCtrl Plus 2`。发布说明应强调它是独立安装的
配套社区项目，并且同一个 PWM 不得同时交给多个风扇控制插件。

推荐温度显示层写作 `Dynamix System Temperature`：本驱动提供 hwmon 节点，
System Temperature 负责在 Unraid WebGUI/Dashboard 中选择和展示温度。

## 文件说明

- `RESULTS.zh-CN.md`：完整中文逆向与实机验证成果。
- `RESULTS.en.md`：完整英文逆向与实机验证成果。
- `README.zh-CN.md`：GitHub 中文 README 发布稿。
- `README.md`：GitHub 英文 README 发布稿。
- `UNRAID-COMMUNITY.zh-CN.md`：中文 CA 描述、支持帖和检查表。
- `UNRAID-COMMUNITY.en.md`：英文 CA 描述、支持帖和检查表。

## 占位符说明

本归档已将所有占位符替换为仓库 `ltdstudio/minisforum-n5-it5571` 的真实值（版本 v0.2.0）。
如迁移到新仓库，请重新替换：版本号、raw GitHub URL、`.plg` URL、
仓库 URL、Issues URL、Unraid 支持帖 URL。

## 重要边界

- 可以公开：自行编写的驱动、构建脚本、寄存器结论和测试数据。
- 不要公开：厂商 EC 固件镜像、IT5571 Confidential 数据手册、密码、IP、
  SSH 日志或开发机绝对路径。
- 首个公开版本应明确写成只验证于 Unraid 7.3.2、内核
  `6.18.38-Unraid`、BIOS 1.04。
