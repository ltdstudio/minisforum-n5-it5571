// SPDX-License-Identifier: GPL-2.0-only
/*
 * Minisforum N5 (F8NAA) / ITE IT5571 hwmon driver.
 *
 * Reverse engineered from F8NAA EC firmware V0.14 and validated on an N5
 * running Unraid 7.3.2 / Linux 6.18.38-Unraid.
 *
 * The EC exposes four logical fan-control channels through PMC2 command 0xd5:
 *   1: CPU fan       (DCR1, TACH1)
 *   2: SSD fan       (DCR2, TACH2)
 *   3: HDD fan group (DCR3 + DCR4, TACH3)
 *   4: PCIe fan      (DCR5, no tachometer input)
 *
 * ACPI EC RAM provides four useful temperature bytes:
 *   0x09 CPU, 0x04 system, 0x05 board/aux, 0x06 ambient.
 */

#include <linux/delay.h>
#include <linux/dmi.h>
#include <linux/errno.h>
#include <linux/hwmon.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/types.h>

#define N5_DRIVER_NAME          "minisforum_n5_it5571"
#define N5_EC_DATA              0x62
#define N5_EC_STATUS_COMMAND    0x66
#define N5_EC_READ_COMMAND      0x80
#define N5_PMC2_DATA            0x68
#define N5_PMC2_STATUS_COMMAND  0x6c
#define N5_PMC2_COMMAND         0xd5
#define N5_STATUS_OBF           BIT(0)
#define N5_STATUS_IBF           BIT(1)
#define N5_WAIT_LOOPS           5000

#define N5_PWM_CHANNELS         4
#define N5_FAN_CHANNELS         4
#define N5_TEMP_CHANNELS        4

static bool force;
module_param(force, bool, 0444);
MODULE_PARM_DESC(force, "Load without the Minisforum N5/F8NAA DMI match");

struct n5_data {
	struct device *hwmon_dev;
	struct mutex lock;
	long fan_min[N5_FAN_CHANNELS];
	u8 pwm[N5_PWM_CHANNELS];
	u8 pwm_enable[N5_PWM_CHANNELS];
};

static struct platform_device *n5_pdev;

static const u8 n5_temp_reg[N5_TEMP_CHANNELS] = { 0x09, 0x04, 0x05, 0x06 };

static const char * const n5_temp_label[N5_TEMP_CHANNELS] = {
	"CPU Temp",
	"System Temp",
	"Board Temp",
	"Ambient Temp",
};

static const char * const n5_fan_label[N5_FAN_CHANNELS] = {
	"CPU Fan",
	"SSD Fan",
	"HDD Fan",
	"PCIe Fan (no tach)",
};

static int n5_wait_status(unsigned short status_port, u8 mask, bool set)
{
	int i;

	for (i = 0; i < N5_WAIT_LOOPS; i++) {
		if (!!(inb(status_port) & mask) == set)
			return 0;
		usleep_range(20, 50);
	}

	return -ETIMEDOUT;
}

static void n5_drain_output(unsigned short data_port,
			    unsigned short status_port)
{
	int i;

	for (i = 0; i < 16; i++) {
		if (!(inb(status_port) & N5_STATUS_OBF))
			break;
		(void)inb(data_port);
	}
}

/* Caller holds data->lock. */
static int n5_pmc2_begin(u8 subcommand)
{
	int ret;

	n5_drain_output(N5_PMC2_DATA, N5_PMC2_STATUS_COMMAND);
	ret = n5_wait_status(N5_PMC2_STATUS_COMMAND, N5_STATUS_IBF, false);
	if (ret)
		return ret;

	outb(N5_PMC2_COMMAND, N5_PMC2_STATUS_COMMAND);
	ret = n5_wait_status(N5_PMC2_STATUS_COMMAND, N5_STATUS_IBF, false);
	if (ret)
		return ret;

	outb(subcommand, N5_PMC2_DATA);
	return n5_wait_status(N5_PMC2_STATUS_COMMAND, N5_STATUS_IBF, false);
}

/* Caller holds data->lock. */
static int n5_pmc2_read(u8 subcommand, u8 *value)
{
	int ret;

	ret = n5_pmc2_begin(subcommand);
	if (ret)
		return ret;

	ret = n5_wait_status(N5_PMC2_STATUS_COMMAND, N5_STATUS_OBF, true);
	if (ret)
		return ret;

	*value = inb(N5_PMC2_DATA);
	return 0;
}

/* Caller holds data->lock. */
static int n5_pmc2_write(u8 subcommand, u8 value)
{
	int ret;

	ret = n5_pmc2_begin(subcommand);
	if (ret)
		return ret;

	outb(value, N5_PMC2_DATA);
	return n5_wait_status(N5_PMC2_STATUS_COMMAND, N5_STATUS_IBF, false);
}

/* Caller holds data->lock. */
static int n5_pmc2_action(u8 subcommand)
{
	return n5_pmc2_begin(subcommand);
}

/* Caller holds data->lock. */
static int n5_ec_read_locked(u8 address, u8 *value)
{
	int ret;

	n5_drain_output(N5_EC_DATA, N5_EC_STATUS_COMMAND);
	ret = n5_wait_status(N5_EC_STATUS_COMMAND, N5_STATUS_IBF, false);
	if (ret)
		return ret;

	outb(N5_EC_READ_COMMAND, N5_EC_STATUS_COMMAND);
	ret = n5_wait_status(N5_EC_STATUS_COMMAND, N5_STATUS_IBF, false);
	if (ret)
		return ret;

	outb(address, N5_EC_DATA);
	ret = n5_wait_status(N5_EC_STATUS_COMMAND, N5_STATUS_OBF, true);
	if (ret)
		return ret;

	*value = inb(N5_EC_DATA);
	return 0;
}

static int n5_read_fan_locked(int channel, long *rpm)
{
	static const u8 low_command[] = { 0x18, 0x16, 0x14 };
	static const u8 high_command[] = { 0x19, 0x17, 0x15 };
	u8 high_before, high_after, low;
	int attempt, ret;

	/* The N5 firmware exposes DCR5 for PCIe PWM, but no TACH4 input. */
	if (channel == 3) {
		*rpm = 0;
		return 0;
	}

	for (attempt = 0; attempt < 4; attempt++) {
		ret = n5_pmc2_read(high_command[channel], &high_before);
		if (ret)
			return ret;
		ret = n5_pmc2_read(low_command[channel], &low);
		if (ret)
			return ret;
		ret = n5_pmc2_read(high_command[channel], &high_after);
		if (ret)
			return ret;
		if (high_before == high_after) {
			*rpm = ((long)high_before << 8) | low;
			return 0;
		}
	}

	return -EAGAIN;
}

/* Caller holds data->lock. */
static int n5_set_pwm_locked(int channel, u8 pwm)
{
	static const u8 cpu_curve_pwm[] = { 0x23, 0x25, 0x27, 0x29 };
	static const u8 pcie_curve_pwm[] = { 0x33, 0x35, 0x37, 0x39 };
	int i, ret;

	switch (channel) {
	case 0:
		/* Override all four CPU fan-curve output points. */
		for (i = 0; i < ARRAY_SIZE(cpu_curve_pwm); i++) {
			ret = n5_pmc2_write(cpu_curve_pwm[i], pwm);
			if (ret)
				return ret;
		}
		return n5_pmc2_action(0x20);
	case 1:
		ret = n5_pmc2_write(0x2f, pwm);
		return ret ? ret : n5_pmc2_action(0x2d);
	case 2:
		/* One logical HDD control drives DCR3 and DCR4 together. */
		ret = n5_pmc2_write(0x2c, pwm);
		return ret ? ret : n5_pmc2_action(0x2a);
	case 3:
		/* Override all four PCIe fan-curve output points. */
		for (i = 0; i < ARRAY_SIZE(pcie_curve_pwm); i++) {
			ret = n5_pmc2_write(pcie_curve_pwm[i], pwm);
			if (ret)
				return ret;
		}
		return n5_pmc2_action(0x30);
	default:
		return -EINVAL;
	}
}

/* Caller holds data->lock. */
static int n5_set_auto_locked(int channel)
{
	static const u8 auto_command[N5_PWM_CHANNELS] = { 0x21, 0x2e, 0x2b, 0x31 };

	if (channel < 0 || channel >= N5_PWM_CHANNELS)
		return -EINVAL;
	return n5_pmc2_action(auto_command[channel]);
}

static umode_t n5_is_visible(const void *drvdata,
			     enum hwmon_sensor_types type, u32 attr,
			     int channel)
{
	switch (type) {
	case hwmon_temp:
		switch (attr) {
		case hwmon_temp_input:
		case hwmon_temp_label:
			return 0444;
		default:
			return 0;
		}
	case hwmon_fan:
		switch (attr) {
		case hwmon_fan_input:
		case hwmon_fan_label:
			return 0444;
		case hwmon_fan_min:
			return 0644;
		default:
			return 0;
		}
	case hwmon_pwm:
		switch (attr) {
		case hwmon_pwm_input:
		case hwmon_pwm_enable:
			return 0644;
		default:
			return 0;
		}
	default:
		return 0;
	}
}

static int n5_read(struct device *dev, enum hwmon_sensor_types type,
		   u32 attr, int channel, long *value)
{
	struct n5_data *data = dev_get_drvdata(dev);
	u8 temp;
	int ret = 0;

	switch (type) {
	case hwmon_temp:
		if (attr != hwmon_temp_input || channel >= N5_TEMP_CHANNELS)
			return -EOPNOTSUPP;
		mutex_lock(&data->lock);
		ret = n5_ec_read_locked(n5_temp_reg[channel], &temp);
		mutex_unlock(&data->lock);
		if (ret)
			return ret;
		if (temp == 0 || temp == 0xff)
			return -ENODATA;
		*value = (long)(s8)temp * 1000;
		return 0;
	case hwmon_fan:
		if (channel >= N5_FAN_CHANNELS)
			return -EOPNOTSUPP;
		if (attr == hwmon_fan_min) {
			*value = data->fan_min[channel];
			return 0;
		}
		if (attr != hwmon_fan_input)
			return -EOPNOTSUPP;
		mutex_lock(&data->lock);
		ret = n5_read_fan_locked(channel, value);
		mutex_unlock(&data->lock);
		return ret;
	case hwmon_pwm:
		if (channel >= N5_PWM_CHANNELS)
			return -EOPNOTSUPP;
		if (attr == hwmon_pwm_input)
			*value = data->pwm[channel];
		else if (attr == hwmon_pwm_enable)
			*value = data->pwm_enable[channel];
		else
			return -EOPNOTSUPP;
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static int n5_read_string(struct device *dev, enum hwmon_sensor_types type,
			  u32 attr, int channel, const char **str)
{
	if (type == hwmon_temp && attr == hwmon_temp_label &&
	    channel < N5_TEMP_CHANNELS) {
		*str = n5_temp_label[channel];
		return 0;
	}
	if (type == hwmon_fan && attr == hwmon_fan_label &&
	    channel < N5_FAN_CHANNELS) {
		*str = n5_fan_label[channel];
		return 0;
	}
	return -EOPNOTSUPP;
}

static int n5_write(struct device *dev, enum hwmon_sensor_types type,
		    u32 attr, int channel, long value)
{
	struct n5_data *data = dev_get_drvdata(dev);
	int ret = 0;

	if (type == hwmon_fan && attr == hwmon_fan_min) {
		if (channel >= N5_FAN_CHANNELS || value < 0 || value > 65535)
			return -EINVAL;
		data->fan_min[channel] = value;
		return 0;
	}

	if (type != hwmon_pwm || channel >= N5_PWM_CHANNELS)
		return -EOPNOTSUPP;

	mutex_lock(&data->lock);
	if (attr == hwmon_pwm_input) {
		if (value < 0 || value > 255) {
			ret = -EINVAL;
			goto out;
		}
		if (data->pwm_enable[channel] != 1) {
			ret = -EBUSY;
			goto out;
		}
		ret = n5_set_pwm_locked(channel, value);
		if (!ret)
			data->pwm[channel] = value;
	} else if (attr == hwmon_pwm_enable) {
		switch (value) {
		case 0:
			/* Full-speed fail-safe mode. */
			ret = n5_set_pwm_locked(channel, 255);
			if (!ret) {
				data->pwm[channel] = 255;
				data->pwm_enable[channel] = 0;
			}
			break;
		case 1:
			/* Entering manual mode starts at full speed for safety. */
			if (data->pwm_enable[channel] != 1) {
				ret = n5_set_pwm_locked(channel, 255);
				if (!ret)
					data->pwm[channel] = 255;
			}
			if (!ret)
				data->pwm_enable[channel] = 1;
			break;
		case 2:
			ret = n5_set_auto_locked(channel);
			if (!ret)
				data->pwm_enable[channel] = 2;
			break;
		default:
			ret = -EINVAL;
			break;
		}
	} else {
		ret = -EOPNOTSUPP;
	}
out:
	mutex_unlock(&data->lock);
	return ret;
}

static const struct hwmon_ops n5_hwmon_ops = {
	.is_visible = n5_is_visible,
	.read = n5_read,
	.read_string = n5_read_string,
	.write = n5_write,
};

static const struct hwmon_channel_info * const n5_hwmon_info[] = {
	HWMON_CHANNEL_INFO(temp,
		HWMON_T_INPUT | HWMON_T_LABEL,
		HWMON_T_INPUT | HWMON_T_LABEL,
		HWMON_T_INPUT | HWMON_T_LABEL,
		HWMON_T_INPUT | HWMON_T_LABEL),
	HWMON_CHANNEL_INFO(fan,
		HWMON_F_INPUT | HWMON_F_MIN | HWMON_F_LABEL,
		HWMON_F_INPUT | HWMON_F_MIN | HWMON_F_LABEL,
		HWMON_F_INPUT | HWMON_F_MIN | HWMON_F_LABEL,
		HWMON_F_INPUT | HWMON_F_MIN | HWMON_F_LABEL),
	HWMON_CHANNEL_INFO(pwm,
		HWMON_PWM_INPUT | HWMON_PWM_ENABLE,
		HWMON_PWM_INPUT | HWMON_PWM_ENABLE,
		HWMON_PWM_INPUT | HWMON_PWM_ENABLE,
		HWMON_PWM_INPUT | HWMON_PWM_ENABLE),
	NULL
};

static const struct hwmon_chip_info n5_chip_info = {
	.ops = &n5_hwmon_ops,
	.info = n5_hwmon_info,
};

static bool n5_dmi_supported(void)
{
	const char *product = dmi_get_system_info(DMI_PRODUCT_NAME);
	const char *board = dmi_get_system_info(DMI_BOARD_NAME);

	return product && board && !strcmp(product, "N5") && !strcmp(board, "F8NAA");
}

static int n5_probe(struct platform_device *pdev)
{
	struct n5_data *data;
	int i;

	if (!force && !n5_dmi_supported())
		return dev_err_probe(&pdev->dev, -ENODEV,
				     "unsupported system; expected Minisforum N5/F8NAA\n");

	if (!request_region(N5_EC_DATA, 1, N5_DRIVER_NAME))
		return dev_err_probe(&pdev->dev, -EBUSY,
				     "EC data port 0x62 is busy\n");
	if (!request_region(N5_EC_STATUS_COMMAND, 1, N5_DRIVER_NAME)) {
		release_region(N5_EC_DATA, 1);
		return dev_err_probe(&pdev->dev, -EBUSY,
				     "EC status/command port 0x66 is busy\n");
	}
	if (!request_region(N5_PMC2_DATA, 1, N5_DRIVER_NAME)) {
		release_region(N5_EC_STATUS_COMMAND, 1);
		release_region(N5_EC_DATA, 1);
		return dev_err_probe(&pdev->dev, -EBUSY,
				     "PMC2 data port 0x68 is busy\n");
	}
	if (!request_region(N5_PMC2_STATUS_COMMAND, 1, N5_DRIVER_NAME)) {
		release_region(N5_PMC2_DATA, 1);
		release_region(N5_EC_STATUS_COMMAND, 1);
		release_region(N5_EC_DATA, 1);
		return dev_err_probe(&pdev->dev, -EBUSY,
				     "PMC2 status/command port 0x6c is busy\n");
	}

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		release_region(N5_PMC2_STATUS_COMMAND, 1);
		release_region(N5_PMC2_DATA, 1);
		release_region(N5_EC_STATUS_COMMAND, 1);
		release_region(N5_EC_DATA, 1);
		return -ENOMEM;
	}

	mutex_init(&data->lock);
	for (i = 0; i < N5_PWM_CHANNELS; i++) {
		data->pwm[i] = 255;
		data->pwm_enable[i] = 2;
	}
	platform_set_drvdata(pdev, data);

	data->hwmon_dev = devm_hwmon_device_register_with_info(&pdev->dev,
			N5_DRIVER_NAME, data, &n5_chip_info, NULL);
	if (IS_ERR(data->hwmon_dev)) {
		release_region(N5_PMC2_STATUS_COMMAND, 1);
		release_region(N5_PMC2_DATA, 1);
		release_region(N5_EC_STATUS_COMMAND, 1);
		release_region(N5_EC_DATA, 1);
		return PTR_ERR(data->hwmon_dev);
	}

	dev_info(&pdev->dev,
		 "registered 4 PWM, 4 fan and 4 temperature channels\n");
	return 0;
}

static void n5_remove(struct platform_device *pdev)
{
	struct n5_data *data = platform_get_drvdata(pdev);
	int i;

	/* Never leave the board in manual mode when the module is unloaded. */
	mutex_lock(&data->lock);
	for (i = 0; i < N5_PWM_CHANNELS; i++)
		(void)n5_set_auto_locked(i);
	mutex_unlock(&data->lock);

	release_region(N5_PMC2_STATUS_COMMAND, 1);
	release_region(N5_PMC2_DATA, 1);
	release_region(N5_EC_STATUS_COMMAND, 1);
	release_region(N5_EC_DATA, 1);
}

static struct platform_driver n5_driver = {
	.driver = {
		.name = N5_DRIVER_NAME,
	},
	.probe = n5_probe,
	.remove = n5_remove,
};

static int __init n5_init(void)
{
	int ret;

	ret = platform_driver_register(&n5_driver);
	if (ret)
		return ret;

	n5_pdev = platform_device_register_simple(N5_DRIVER_NAME,
						    PLATFORM_DEVID_NONE, NULL, 0);
	if (IS_ERR(n5_pdev)) {
		ret = PTR_ERR(n5_pdev);
		platform_driver_unregister(&n5_driver);
		return ret;
	}
	return 0;
}

static void __exit n5_exit(void)
{
	platform_device_unregister(n5_pdev);
	platform_driver_unregister(&n5_driver);
}

module_init(n5_init);
module_exit(n5_exit);

MODULE_AUTHOR("ltdstudio, reverse engineered for the Minisforum N5");
MODULE_DESCRIPTION("Minisforum N5/F8NAA IT5571 fan and temperature hwmon driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.0");
