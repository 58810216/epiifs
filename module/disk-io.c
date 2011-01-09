
#include <linux/types.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>

#include "super.h"
#include "disk-io.h"


struct buffer_head *epii_read_dev_super(struct block_device *bdev)
{
	struct buffer_head *bh;
	struct epii_super_block *super;
	u64 offset;
	int len = 0;

	offset = epii_sb_offset(0);
	bh = __bread(bdev, offset / 4096, 4096);
	if (!bh)
		goto error_out;
	
	len = i_size_read(bdev->bd_inode);

	printk(KERN_WARNING "len is %d\n", len);
		
    super = (struct epii_super_block *)bh->b_data;

	if (strncmp(super->s_magic, EPII_MAGIC, sizeof(EPII_MAGIC))) {
		printk(KERN_WARNING "uuid %Lx, magic error, %s", 
			super->s_uuid,super->s_magic);
		brelse(bh);
		goto error_out;
	}
	
	printk(KERN_WARNING "super ok uuid %Lx, magic %s\n", super->s_uuid, super->s_magic); 
	return bh;	

error_out:	
	printk(KERN_WARNING "read sb from bdev fail \n");
	return NULL;
}
