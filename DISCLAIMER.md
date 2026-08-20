# DISCLAIMER / 免责声明

**English** · 中文见下方

## English

The `minisforum_n5_it5571` driver and all materials in this repository
(documents, scripts, firmware analysis, measurements) are the result of an
**independent community exploration** by ltdstudio. This project is:

- **NOT affiliated with, endorsed by, or supported by Minisforum** (Shenzhen
  Meigao Electronic Equipment Co., Ltd.).
- **NOT an official product.** Minisforum provides no warranty, support, or
  liability for this project.

This is an early (v0.2.0) community release. Although the driver was fully
tested on the author's hardware (N5 / F8NAA / BIOS 1.04 / Unraid 7.3.2 /
kernel 6.18.38-Unraid), it **contains unverified factors**: hardware batches,
BIOS/EC firmware versions, kernel versions, and third-party software
combinations may behave differently.

**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.**

By installing or using this driver you agree that:

1. You have verified the exact DMI profile. N5/F8NAA is validated; N5
   Pro/F8NAA and N5 Air/F8NAB are experimental, read-only by default, and
   require explicit opt-in before PWM writes are exposed.
2. You assign each fan channel (`pwm1`–`pwm4`) to exactly one controller to
   avoid conflicting control loops.
3. You install a kernel-matched package after every Unraid kernel update.
4. You accept full responsibility for any outcome of using this driver.

If you do not agree with these terms, do not install or use this software.

---

## 中文

`minisforum_n5_it5571` 驱动及本仓库中的全部材料（文档、脚本、固件分析、实测数据）
均为 ltdstudio 的**独立社区探索成果**。本项目：

- **与 Minisforum（铭凡 / 深圳美高电子设备有限公司）无关，未获其认可或支持。**
- **非官方产品。** 官方对本项目不提供任何质保、支持或责任。

本版本为早期（v0.2.0）社区版。虽然驱动已在作者实机（N5 / F8NAA / BIOS 1.04 /
Unraid 7.3.2 / 内核 6.18.38-Unraid）上完成全流程测试，但**仍包含未验证因素**：
不同硬件批次、BIOS/EC 固件版本、内核版本及第三方软件组合下，行为可能存在差异。

**本软件按“现状”提供，不作任何明示或默示担保，包括但不限于适销性、特定用途
适用性与非侵权保证。在任何情况下，作者均不对因使用本软件而产生的任何索赔、
损害或其他责任负责，无论基于合同、侵权或其他原因。**

安装或使用本驱动即表示您同意：

1. 已确认准确 DMI 配置。N5/F8NAA 已验证；N5 Pro/F8NAA 与 N5 Air/F8NAB
   属于实验性配置，默认只读，开放 PWM 写入前必须由用户显式确认。
2. 每路风扇通道（`pwm1`–`pwm4`）只交给一个控制器管理，避免控制环冲突。
3. 每次升级 Unraid 内核后安装与内核严格匹配的新驱动包。
4. 自行承担使用本驱动产生的一切后果。

如不同意以上条款，请勿安装或使用本软件。
