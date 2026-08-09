# 铭凡 N5 / N5 Pro 风扇控制调研报告
### ——IT5571 EC 的封闭现状、逆向证据与可行路径

> 写给「明矾 NAS 交流群」群友。本文所有结论均有实测/源码证据，可自行复核。
> 调查日期：2026-08

---

## 一、问题背景

N5 / N5 Pro 在 Linux（Unraid / TrueNAS / Proxmox）下**无法读取风扇转速、无法控制风扇**，BIOS 里只有手动 PWM 设定，低温时风扇也保持固定转速、噪音大。群友普遍呼吁官方开放 IT5571 传感器驱动。

## 二、硬件真相（已确认）

| 项 | 结论 |
|---|---|
| 风扇控制芯片 | **ITE IT5571**（嵌入式控制器 EC，非传统 Super I/O） |
| Linux 识别 | `sensors-detect` 报 `Found unknown chip with ID 0x5571` |
| 内核驱动 | **不存在**。it87 驱动不支持；官方 datasheet 之前是机密 |
| 芯片规格 | 8 路 PWM 输出、**3 组风扇转速计**、11 路 ADC、PECI（IT5571_C_V0.3.3 手册，现已公开） |
| EC 访问方式 | 标准 **ACPI EC 接口（端口 0x62/0x66）**，用户态可读写（nbfc/ec_probe 原理） |
| 官方立场 | 铭凡客服回复：风扇由 BIOS 按 HDD/SSD 温度自动控制 |

关键点：**EC 的温度/PWM 数据存在 EC 私有 RAM 里，不通过标准 hwmon 接口暴露给操作系统**——所以 Linux 下 `/sys/class/hwmon` 里根本没有 PWM 节点。

## 三、重大发现：连 MinisCloud OS 自己都读不到 EC（本次逆向）

我们解包了官方 **MinisCloud OS 2.1.10 完整镜像（10.7GB）**，结论：

1. **系统底座**：Debian 系，内核 `6.12.64+`，带 linux-headers（支持 DKMS 编译模块）
2. **内核模块清单**：只有标准驱动（`acpi/fan.ko`、hwmon 全家桶、`cros_ec` 等），**没有任何 IT5571 专用驱动**
3. **四有云后端**（Go 1.25 静态二进制，88MB，含 `/os/fan/list`、`/os/fan/strategy` API）：
   - 温度读取走 **gopsutil（标准 sysfs hwmon）**
   - **没有任何 EC 端口（0x62/0x66）读写代码**，无 `/dev/port` 访问
   - 有 `PWMFan`/`GpioFan` 硬件抽象模型，但那是给**有 hwmon 驱动的主板**用的
4. **官方 Windows 版 MinisCloud APP**（Inno Setup 6.1 → Flutter `app.so`）：同样是前端 UI（`PwmFan`/`pwmStrategy` 模型），无 EC 代码

**结论：铭凡把风扇控制 100% 封闭在 BIOS/EC 固件里，连自家 MinisCloud OS 都拿不到硬件监控数据**（这也解释了 NAS Compares 评测吐槽它"SSD 温度/SMART 监控不一致"）。

## 四、社区现状（大家都在同一困境）

- **lm-sensors issue #400**（67 条评论）：IT5571 主讨论帖，覆盖 ZOTAC/Gigabyte/HP/铭凡全系
- **frankcrawford/it87 #8 #49**：it87 驱动维护者确认 IT557x 无 datasheet 时无法支持（datasheet 现已公开，2025-10 有人上传）
- **成功先例**：lm-sensors #400 有人用 **nbfc-linux + ec_probe** 在 IT5571 机器（HP AiO）上成功控制风扇——EC PWM 寄存器可写，但**每台机器寄存器布局不同，需自己探测**
- **it5570-fan 项目**：IT5570（同系列）的 hwmon 驱动，N5 是 IT5571 寄存器不同，不通用
- 铭凡系受影响机型：X7 Ti、MS02 Ultra、AI X1 Pro、N5 Air、N5/N5 Pro 全中招

## 五、可行路径（按推荐顺序）

### 1️⃣ BIOS 路线（最现实，零风险）
- 更新 BIOS：N5 用 **F8NAA_HPT 分支**（CPU-2XX），N5 Pro 用 F8NAA_STX 分支，别刷错
- 手动设低转速（30-40%），靠**磁盘 SMART 温度**监控（硬盘 35-50°C 安全）
- 背后双风扇主要吹硬盘，低转速足够

### 2️⃣ EC 寄存器逆向（要动手，有先例）
- IT5571 datasheet 已公开；EC 走标准 0x62/0x66 端口
- Unraid 上 `modprobe ec_sys write_support=1` → 读 `/sys/kernel/debug/ec/ec0/io`（256 字节 EC RAM）
- 边写边听风扇，找 PWM 寄存器 → 找温度寄存器 → 配 nbfc 或写脚本自动控温
- 探测脚本已写好（见群文件/附后）

### 3️⃣ 物理方案（兜底）
- 换 Noctua 低速风扇 / 3-pin 减速线

## 六、给官方的呼吁

1. 开放 IT5571 EC **寄存器映射文档**（或 ACPI 表暴露），让社区写 hwmon 驱动
2. 或直接提供 **Linux 风扇驱动**（MinisCloud OS 里都没有，说明你们自己也没做）
3. 或像华硕一样提供 **WMI/ACPI 控制接口**（asus-wmi 模式）

数据手册已经公开了（IT5571_C_V0.3.3），驱动只是时间问题——**官方给一份寄存器表，全系机型 Linux 用户都能受益**。

---

*参考链接：lm-sensors#400、frankcrawford/it87#8#49、nbfc-linux、it5570-fan、Unraid 论坛 186390 帖、MinisCloud OS 镜像逆向（本报告）*
