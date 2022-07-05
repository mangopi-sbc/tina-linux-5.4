/*
 * nand_lib.h for  SUNXI NAND .
 *
 * Copyright (C) 2016 Allwinner.
 *
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __BSP_NAND_H__
#define __BSP_NAND_H__

#include "../phy-nand/nand-partition3/sunxi_nand_partitions.h"
//#include "../phy-nand/nand-partition/phy.h"
//#include "../phy-nand/spinand/spinand_type.h"
#include "nand_type.h"

struct _nftl_blk {
	unsigned int nftl_logic_size;
	struct _nand_partition *nand;
	struct _nftl_blk *nftl_blk_next;
	void *nftl_zone;
	void *cfg;
	unsigned int time;
	unsigned int ops_time; /* add ecape time for ops nftl */
	unsigned int time_flush;
	void *nftl_thread;
	void *blk_lock;
	int (*read_data)(struct _nftl_blk *nftl_blk, unsigned int sector,
			 unsigned int len, unsigned char *buf);
	int (*write_data)(struct _nftl_blk *nftl_blk, unsigned int sector,
			  unsigned int len, unsigned char *buf);
	int (*flush_write_cache)(struct _nftl_blk *nftl_blk,
				 unsigned int num);
	int (*discard)(struct _nftl_blk *nftl_blk, unsigned int sector,
		       unsigned int len);
	int (*shutdown_op)(struct _nftl_blk *nftl_blk);
	int (*read_sector_data)(struct _nftl_blk *nftl_blk,
				unsigned int sector, unsigned int len,
				unsigned char *buf);
	int (*write_sector_data)(struct _nftl_blk *nftl_blk,
				 unsigned int sector, unsigned int len,
				 unsigned char *buf);
	int (*flush_sector_write_cache)(struct _nftl_blk *nftl_blk,
					unsigned int num);
};
#endif
