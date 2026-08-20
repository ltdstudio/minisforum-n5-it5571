# v0.2.0 — Experimental N5 Pro and N5 Air profiles

English | 中文见下方

## English

- Adds exact DMI profiles for N5 Pro (`N5 PRO` / `F8NAA`) and N5 Air
  (`N5A` or `N5 AIR` / `F8NAB`).
- These new profiles are **experimental and read-only by default**. Temperature
  and RPM data are exposed first; PWM nodes require explicit
  `experimental_write=1` opt-in.
- N5/F8NAA remains the only maintainer hardware-validated profile.
- If a fan stops, runs unexpectedly, controls the wrong header, disappears, or
  temperatures become abnormal, **stop immediately**, unload the module, and
  return to BIOS control. Never leave an experimental test unattended.
- Module unload now restores only channels that this module actually attempted
  to modify.
- One user reports good operation on a Minisforum MS-A2. This is community
  feedback, not maintainer validation; MS-A2 is not auto-whitelisted until its
  exact DMI and channel mapping are collected.

See the README for the staged test and opt-in procedure.

## 中文

- 新增 N5 Pro（`N5 PRO` / `F8NAA`）和 N5 Air（`N5A` 或 `N5 AIR` /
  `F8NAB`）的精确 DMI 配置。
- 两款新增机型均为**实验性且默认只读**：先开放温度与 RPM，只有显式设置
  `experimental_write=1` 后才开放 PWM 节点。
- N5/F8NAA 仍是唯一由维护者完成实机验证的配置。
- 如出现风扇停转、异常运行、控制到错误接口、通道消失或温度异常，必须
  **立即终止**、卸载模块并恢复 BIOS 控制；实验期间不得无人值守。
- 模块卸载现在只恢复本次确实尝试修改过的通道。
- 已有一位用户反馈 Minisforum MS-A2 使用良好；这属于社区反馈而非维护者
  实机验证，在收集准确 DMI 和通道映射前不会自动加入白名单。

分阶段测试与显式启用方法请阅读 README。
