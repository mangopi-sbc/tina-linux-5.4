/*
 * A V4L2 driver for GC2385 Raw cameras.
 *
 * Copyright (c) 2018 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Authors:  Zheng ZeQun <zequnzheng@allwinnertech.com>
 *    Liang WeiJie <liangweijie@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <linux/io.h>

#include "camera.h"
#include "sensor_helper.h"

MODULE_AUTHOR("lwj");
MODULE_DESCRIPTION("A low-level driver for GC2385 sensors");
MODULE_LICENSE("GPL");

#define MCLK              (24*1000*1000)
#define V4L2_IDENT_SENSOR 0x2385

/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30

/*
 * The GC0310 i2c address
 */
#define I2C_ADDR 0x6e
#define SENSOR_NUM	0x2
#define SENSOR_NAME	"gc2385_mipi"
#define SENSOR_NAME_2	"gc2385_mipi_2"

struct sensor_exp_gain grobal_exp_gain = {16000, 512};


/*
 * The default register settings
 */

static struct regval_list sensor_default_regs[] = {
};


static struct regval_list sensor_1600x1200p30_regs[] = {
	/*system*/
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xfe, 0x00},
	{0xf2, 0x02},
	{0xf4, 0x03},
	{0xf7, 0x01},
	{0xf8, 0x28},
	{0xf9, 0x02},
	{0xfa, 0x08},
	{0xfc, 0x8e},
	{0xe7, 0xcc},
	{0x88, 0x03},

	/*analog*/
	{0x03, 0x04},
	{0x04, 0x80},
	{0x05, 0x02},
	{0x06, 0x86},
	{0x07, 0x00},
	{0x08, 0x24},
	{0x09, 0x00},
	{0x0a, 0x04},
	{0x0b, 0x00},
	{0x0c, 0x02},
	{0x17, 0x54},
	{0x18, 0x02},
	{0x19, 0x17},
	{0x1c, 0x18},
	{0x20, 0x73},
	{0x21, 0x38},
	{0x22, 0xa2},
	{0x29, 0x20},
	{0x2f, 0x14},
	{0x3f, 0x40},
	{0xcd, 0x94},
	{0xce, 0x45},
	{0xd1, 0x0c},
	{0xd7, 0x9b},
	{0xd8, 0x99},
	{0xda, 0x3b},
	{0xd9, 0xb5},
	{0xdb, 0x75},
	{0xe3, 0x1b},
	{0xe4, 0xf8},

	/*BLk*/
	{0x40, 0x22},
	{0x43, 0x07},
	{0x4e, 0x3c},
	{0x4f, 0x00},
	{0x68, 0x00},

	/*gain*/
	{0xb0, 0x46},
	{0xb1, 0x01},
	{0xb2, 0x00},
	{0xb6, 0x00},

	/*out crop*/
	{0x90, 0x01},
	{0x92, 0x04},
	{0x94, 0x05},
	{0x95, 0x04},
	{0x96, 0xb0},
	{0x97, 0x06},
	{0x98, 0x40},

	/*mipi set*/
	{0xfe, 0x00},
	{0xed, 0x00},
	{0xfe, 0x03},
	{0x01, 0x03},
	{0x02, 0x82},
	{0x03, 0xd0},
	{0x04, 0x04},
	{0x05, 0x00},
	{0x06, 0x80},
	{0x11, 0x2b},
	{0x12, 0xd0},
	{0x13, 0x07},
	{0x15, 0x00},
	{0x1b, 0x10},
	{0x1c, 0x10},
	{0x21, 0x08},
	{0x22, 0x05},
	{0x23, 0x13},
	{0x24, 0x02},
	{0x25, 0x13},
	{0x26, 0x06},
	{0x29, 0x06},
	{0x2a, 0x08},
	{0x2b, 0x06},
	{0xfe, 0x00},
	{0xed, 0x90},
	{0xfe, 0x00},
};

/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 *
 */

static struct regval_list sensor_fmt_raw[] = {

};

/*
 * Code for dealing with controls.
 * fill with different sensor module
 * different sensor module has different settings here
 * if not support the follow function ,retrun -EINVAL
 */

static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	*value = info->exp;
	sensor_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char explow, exphigh;
	unsigned int all_exp;
	struct sensor_info *info = to_state(sd);

	all_exp = exp_val >> 4;
	if (all_exp > 0x3fff)
		all_exp = 0x3fff;
	if (all_exp < 6)
		all_exp = 6;

	sensor_write(sd, 0xfe, 0x00);

	exphigh  = (unsigned char) ((all_exp >> 8) & 0x3f);
	explow  = (unsigned char) (all_exp & 0xff);

	sensor_write(sd, 0x03, exphigh);
	sensor_write(sd, 0x04, explow);

	sensor_dbg("sensor_set_exp = %d, Done!\n", exp_val);

	info->exp = exp_val;
	return 0;
}

static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	*value = info->gain;
	sensor_dbg("sensor_get_gain = %d\n", info->gain);
	return 0;
}

#define ANALOG_GAIN_1 64   // 1.00x
#define ANALOG_GAIN_2 92   // 1.43x
#define ANALOG_GAIN_3 127  // 1.99x
#define ANALOG_GAIN_4 183  // 2.86x
#define ANALOG_GAIN_5 257  // 4.01x
#define ANALOG_GAIN_6 369  // 5.76x
#define ANALOG_GAIN_7 531  //8.30x
#define ANALOG_GAIN_8 750  // 11.72x
#define ANALOG_GAIN_9 1092 // 17.06x

static int sensor_s_gain(struct v4l2_subdev *sd, unsigned int gain_val)
{
	struct sensor_info *info = to_state(sd);
	unsigned int All_gain, Digital_gain;

	All_gain = gain_val*4;
	if (All_gain < 0x40) /* 64 */
		All_gain = 0x40;

	if ((ANALOG_GAIN_1 <= All_gain) && (All_gain < ANALOG_GAIN_2)) {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x73);//0x71
		sensor_write(sd, 0x22, 0xa2);//0xa1
		/* analog gain */
		sensor_write(sd, 0xb6, 0x00);
		Digital_gain = 256*All_gain/ANALOG_GAIN_1;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	} else if ((ANALOG_GAIN_2 <= All_gain) && (All_gain < ANALOG_GAIN_3)) {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x73);//0x72
		sensor_write(sd, 0x22, 0xa2);//0xa1
		/* analog gain */
		sensor_write(sd, 0xb6, 0x01);
		Digital_gain = 256*All_gain/ANALOG_GAIN_2;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	} else if ((ANALOG_GAIN_3 <= All_gain) && (All_gain < ANALOG_GAIN_4)) {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x73);//0x72
		sensor_write(sd, 0x22, 0xa2);//0xa1
		/* analog gain */
		sensor_write(sd, 0xb6, 0x02);
		Digital_gain = 256*All_gain/ANALOG_GAIN_3;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	} else if ((ANALOG_GAIN_4 <= All_gain) && (All_gain < ANALOG_GAIN_5)) {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x73);//0x72
		sensor_write(sd, 0x22, 0xa2);//0xa1
		/* analog gain */
		sensor_write(sd, 0xb6, 0x03);
		Digital_gain = 256*All_gain/ANALOG_GAIN_4;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	} else if ((ANALOG_GAIN_5 <= All_gain) && (All_gain < ANALOG_GAIN_6)) {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x73);//0x72
		sensor_write(sd, 0x22, 0xa3);//0xa2
		/* analog gain */
		sensor_write(sd, 0xb6, 0x04);
		Digital_gain = 256*All_gain/ANALOG_GAIN_5;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	} else if ((ANALOG_GAIN_6 <= All_gain) && (All_gain < ANALOG_GAIN_7)) {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x73);//0x72
		sensor_write(sd, 0x22, 0xa3);//0xa2
		/* analog gain */
		sensor_write(sd, 0xb6, 0x05);
		Digital_gain = 256*All_gain/ANALOG_GAIN_6;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	} else if ((ANALOG_GAIN_7 <= All_gain) && (All_gain < ANALOG_GAIN_8)) {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x74);//0x72
		sensor_write(sd, 0x22, 0xa3);//0xa2
		/* analog gain */
		sensor_write(sd, 0xb6, 0x06);
		Digital_gain = 256*All_gain/ANALOG_GAIN_7;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	} else if ((ANALOG_GAIN_8 <= All_gain) && (All_gain < ANALOG_GAIN_9)) {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x74);//0x72
		sensor_write(sd, 0x22, 0xa3);//0xa2
		/* analog gain */
		sensor_write(sd, 0xb6, 0x07);
		Digital_gain = 256*All_gain/ANALOG_GAIN_8;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	} else {
		sensor_write(sd, 0xfe, 0x00);
		sensor_write(sd, 0x20, 0x74);//0x72
		sensor_write(sd, 0x22, 0xa3);//0xa2
		/* analog gain */
		sensor_write(sd, 0xb6, 0x08);
		Digital_gain = 256*All_gain/ANALOG_GAIN_9;
		sensor_write(sd, 0xb1, Digital_gain >> 8);
		sensor_write(sd, 0xb2, Digital_gain & 0xff);
	}

	sensor_dbg("sensor_set_gain = %d, Done!\n", gain_val);
	info->gain = gain_val;

	return 0;
}

static int sensor_s_exp_gain(struct v4l2_subdev *sd,
				struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val;
	struct sensor_info *info = to_state(sd);

	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;
	grobal_exp_gain.exp_val = exp_gain->exp_val;
	grobal_exp_gain.gain_val = exp_gain->gain_val;

	if (gain_val < 16)
		gain_val = 16;
	if (gain_val > 128*16-1)
		gain_val = 128*16-1;

	if (exp_val > 0x3fff*16)
		exp_val = 0x3fff*16;
	if (exp_val < 6*16)
		exp_val = 6*16;

	sensor_s_exp(sd, exp_val);
	sensor_s_gain(sd, gain_val);

	sensor_dbg("sensor_set_gain exp = %d, gain = %d Done!\n", exp_val, gain_val);

	info->exp = exp_val;
	info->gain = gain_val;
	return 0;
}

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret = 0;
	return ret;
}

/*
 * Stuff that knows about the sensor.
 */
static int sensor_power(struct v4l2_subdev *sd, int on)
{
	int ret;

	switch (on) {
	case STBY_ON:
		sensor_print("STBY_ON!\n");
		cci_lock(sd);
		ret = sensor_s_sw_stby(sd, STBY_ON);
		if (ret < 0)
			sensor_err("soft stby falied!\n");
		usleep_range(1000, 1200);
		cci_unlock(sd);
		break;
	case STBY_OFF:
		sensor_print("STBY_OFF!\n");
		cci_lock(sd);
		usleep_range(1000, 1200);
		ret = sensor_s_sw_stby(sd, STBY_OFF);
		if (ret < 0)
			sensor_err("soft stby falied!\n");
		cci_unlock(sd);
		break;
	case PWR_ON:
		sensor_print(" sd %s,PWR_ON!\n", sd->name);
		cci_lock(sd);
		vin_gpio_set_status(sd, PWDN, 1);
		vin_gpio_set_status(sd, RESET, 1);
		vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		usleep_range(100, 120);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		vin_set_pmu_channel(sd, IOVDD, ON);
		usleep_range(100, 120);
		vin_set_pmu_channel(sd, DVDD, ON);
		usleep_range(100, 120);
		vin_set_pmu_channel(sd, AVDD, ON);
		usleep_range(200, 220);
		vin_set_mclk_freq(sd, MCLK);
		vin_set_mclk(sd, ON);
		usleep_range(200, 220);
		vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		cci_unlock(sd);
		break;
	case PWR_OFF:
		sensor_print("PWR_OFF!\n");
		cci_lock(sd);
		usleep_range(100, 120);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		vin_set_mclk(sd, OFF);
		vin_set_pmu_channel(sd, AVDD, OFF);
		vin_set_pmu_channel(sd, DVDD, OFF);
		vin_set_pmu_channel(sd, IOVDD, OFF);
		cci_unlock(sd);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	switch (val) {
	case 0:
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		usleep_range(100, 120);
		break;
	case 1:
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		usleep_range(100, 120);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	unsigned int SENSOR_ID = 0;
	data_type val;
	int cnt = 0;

	sensor_read(sd, 0xf0, &val);
	SENSOR_ID |= (val << 8);
	sensor_read(sd, 0xf1, &val);
	SENSOR_ID |= (val);
	sensor_print("V4L2_IDENT_SENSOR = 0x%x\n", SENSOR_ID);

	while ((SENSOR_ID != V4L2_IDENT_SENSOR) && (cnt < 5)) {
		sensor_read(sd, 0xf0, &val);
		SENSOR_ID |= (val << 8);
		sensor_read(sd, 0xf1, &val);
		SENSOR_ID |= (val);
		sensor_print("retry = %d, V4L2_IDENT_SENSOR = %x\n", cnt, SENSOR_ID);
		cnt++;
	}

	if (SENSOR_ID != V4L2_IDENT_SENSOR)
		return -ENODEV;

	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	/*Make sure it is a target sensor */
	ret = sensor_detect(sd);
	if (ret) {
		sensor_err("chip found is not an target chip.\n");
		return ret;
	}

	info->focus_status = 0;
	info->low_speed = 0;
	info->width = 1600;
	info->height = 1200;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;
	info->tpf.denominator = 30; /* 30fps */

	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct sensor_info *info = to_state(sd);

	switch (cmd) {
	case GET_CURRENT_WIN_CFG:
		if (info->current_wins != NULL) {
			memcpy(arg, info->current_wins,
				sizeof(struct sensor_win_size));
			ret = 0;
		} else {
			sensor_err("empty wins!\n");
			ret = -1;
		}
		break;
	case SET_FPS:
		ret = 0;
		break;
	case VIDIOC_VIN_SENSOR_EXP_GAIN:
		ret = sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
		break;
	case VIDIOC_VIN_SENSOR_CFG_REQ:
		sensor_cfg_req(sd, (struct sensor_config *)arg);
		break;
	case VIDIOC_VIN_FLASH_EN:
		ret = flash_en(sd, (struct flash_para *)arg);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

/*
 break;
 * Store information about the video data format.
 */
static struct sensor_format_struct sensor_formats[] = {
	{
		.desc = "Raw RGB Bayer",
		.mbus_code = MEDIA_BUS_FMT_SGRBG10_1X10,
		.regs = sensor_fmt_raw,
		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
		.bpp = 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

static struct sensor_win_size sensor_win_sizes[] = {
	{
	.width      = 1600,
	.height     = 1200,
	.hoffset    = 0,
	.voffset    = 0,
	.hts        = 1079,
	.vts        = 1248,
	.pclk       = 41*1000*1000,
	.mipi_bps   = 656*1000*1000,
	.fps_fixed  = 30,
	.bin_factor = 1,
	.intg_min   = 1<<4,
	.intg_max   = 1248<<4,
	.gain_min   = 1<<4,
	.gain_max   = 128<<4,
	.regs       = sensor_1600x1200p30_regs,
	.regs_size  = ARRAY_SIZE(sensor_1600x1200p30_regs),
	.set_size   = NULL,
	},
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2_DPHY;
	cfg->flags = 0 | V4L2_MBUS_CSI2_1_LANE | V4L2_MBUS_CSI2_CHANNEL_0;

	return 0;
}

static int sensor_g_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sensor_info *info =
			container_of(ctrl->handler, struct sensor_info, handler);
	struct v4l2_subdev *sd = &info->sd;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_g_gain(sd, &ctrl->val);
	case V4L2_CID_EXPOSURE:
		return sensor_g_exp(sd, &ctrl->val);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sensor_info *info =
			container_of(ctrl->handler, struct sensor_info, handler);
	struct v4l2_subdev *sd = &info->sd;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_s_gain(sd, ctrl->val);
	case V4L2_CID_EXPOSURE:
		return sensor_s_exp(sd, ctrl->val);
	}
	return -EINVAL;
}

static int sensor_reg_init(struct sensor_info *info)
{
	int ret;
	struct v4l2_subdev *sd = &info->sd;
	struct sensor_format_struct *sensor_fmt = info->fmt;
	struct sensor_win_size *wsize = info->current_wins;
	struct sensor_exp_gain exp_gain;

	ret = sensor_write_array(sd, sensor_default_regs,
				ARRAY_SIZE(sensor_default_regs));
	if (ret < 0) {
		sensor_err("write sensor_default_regs error\n");
		return ret;
	}

	sensor_print("sensor_reg_init\n");

	sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size);

	if (wsize->regs)
		sensor_write_array(sd, wsize->regs, wsize->regs_size);

	if (wsize->set_size)
		wsize->set_size(sd);

	info->width = wsize->width;
	info->height = wsize->height;

	exp_gain.exp_val = grobal_exp_gain.exp_val;
	exp_gain.gain_val = grobal_exp_gain.gain_val;
	sensor_s_exp_gain(sd, &exp_gain);

	sensor_print("s_fmt set width = %d, height = %d\n", wsize->width,
				wsize->height);

	return 0;
}

static int sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct sensor_info *info = to_state(sd);

	sensor_print("%s on = %d, %d*%d fps: %d code: %x\n", __func__, enable,
			info->current_wins->width, info->current_wins->height,
			info->current_wins->fps_fixed, info->fmt->mbus_code);

	if (!enable)
		return 0;

	return sensor_reg_init(info);
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_ctrl_ops sensor_ctrl_ops = {
	.g_volatile_ctrl = sensor_g_ctrl,
	.s_ctrl = sensor_s_ctrl,
};

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = sensor_compat_ioctl32,
#endif
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.s_stream = sensor_s_stream,
	.g_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_pad_ops sensor_pad_ops = {
	.enum_mbus_code = sensor_enum_mbus_code,
	.enum_frame_size = sensor_enum_frame_size,
	.get_fmt = sensor_get_fmt,
	.set_fmt = sensor_set_fmt,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
	.pad = &sensor_pad_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv[] = {
	{
		.name = SENSOR_NAME,
		.addr_width = CCI_BITS_8,
		.data_width = CCI_BITS_8,
	}, {
		.name = SENSOR_NAME_2,
		.addr_width = CCI_BITS_8,
		.data_width = CCI_BITS_8,
	}
};

static int sensor_init_controls(struct v4l2_subdev *sd, const struct v4l2_ctrl_ops *ops)
{
	struct sensor_info *info = to_state(sd);
	struct v4l2_ctrl_handler *handler = &info->handler;
	struct v4l2_ctrl *ctrl;
	int ret = 0;

	v4l2_ctrl_handler_init(handler, 2);

	v4l2_ctrl_new_std(handler, ops, V4L2_CID_GAIN, 1 * 1600,
				256 * 1600, 1, 1 * 1600);
	ctrl = v4l2_ctrl_new_std(handler, ops, V4L2_CID_EXPOSURE, 0,
				65536 * 16, 1, 0);
	if (ctrl != NULL)
		ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;

	if (handler->error) {
		ret = handler->error;
		v4l2_ctrl_handler_free(handler);
	}

	sd->ctrl_handler = handler;

	return ret;
}

static int sensor_dev_id;
static int sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct sensor_info *info;
	int i;

	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;

	if (client) {
		for (i = 0; i < SENSOR_NUM; i++) {
			if (!strcmp(cci_drv[i].name, client->name))
				break;
		}
		cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv[i]);
	} else {
		cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv[sensor_dev_id++]);
	}
	sensor_init_controls(sd, &sensor_ctrl_ops);

	mutex_init(&info->lock);
#ifdef CONFIG_SAME_I2C
	info->sensor_i2c_addr = I2C_ADDR >> 1;
#endif
	info->fmt = &sensor_formats[0];
	info->fmt_pt = &sensor_formats[0];
	info->win_pt = &sensor_win_sizes[0];
	info->fmt_num = N_FMTS;
	info->win_size_num = N_WIN_SIZES;
	info->sensor_field = V4L2_FIELD_NONE;
	info->stream_seq = MIPI_BEFORE_SENSOR;
	info->time_hs = 0x30;
	info->af_first_flag = 1;
	info->exp = 0;
	info->gain = 0;

	return 0;
}

static int sensor_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd;
	int i;

	if (client) {
		for (i = 0; i < SENSOR_NUM; i++) {
			if (!strcmp(cci_drv[i].name, client->name))
				break;
		}
		sd = cci_dev_remove_helper(client, &cci_drv[i]);
	} else {
		sd = cci_dev_remove_helper(client, &cci_drv[sensor_dev_id++]);
	}

	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id sensor_id[] = {
	{SENSOR_NAME, 0},
	{}
};

static const struct i2c_device_id sensor_id_2[] = {
	{SENSOR_NAME_2, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, sensor_id);
MODULE_DEVICE_TABLE(i2c, sensor_id_2);

static struct i2c_driver sensor_driver[] = {
	{
		.driver = {
			   .owner = THIS_MODULE,
			   .name = SENSOR_NAME,
			   },
		.probe = sensor_probe,
		.remove = sensor_remove,
		.id_table = sensor_id,
	}, {
		.driver = {
			   .owner = THIS_MODULE,
			   .name = SENSOR_NAME_2,
			   },
		.probe = sensor_probe,
		.remove = sensor_remove,
		.id_table = sensor_id_2,
	},
};
static __init int init_sensor(void)
{
	int i, ret = 0;

	sensor_dev_id = 0;

	for (i = 0; i < SENSOR_NUM; i++)
		ret = cci_dev_init_helper(&sensor_driver[i]);

	return ret;
}

static __exit void exit_sensor(void)
{
	int i;

	sensor_dev_id = 0;

	for (i = 0; i < SENSOR_NUM; i++)
		cci_dev_exit_helper(&sensor_driver[i]);
}

module_init(init_sensor);
module_exit(exit_sensor);
