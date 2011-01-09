#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/buffer_head.h>

#include "super.h"
#include "disk-io.h"

static int epii_test_sb(struct super_block *s, void *data)
{
	u64 test = 0;

	test = (u64)data;

	printk(KERN_WARNING "data is %Lu\n", test);

	return -ENOMEM;
}

static struct dentry *get_default_root(struct super_block *sb,
				       u64 subvol_objectid)
{
	printk(KERN_WARNING "get root %Lu\n", subvol_objectid);
	return ERR_PTR(-ENOENT);
}


/* Find super block for a known fs 
 * 
 * fill mnt->mnt_sb and mnt->mnt_root
 */
static int epii_get_sb(struct file_system_type *fs_type, int flags,
		const char *dev_name, void *data, struct vfsmount *mnt)
{
	struct super_block *s;
	struct dentry *root;
	struct block_device *bdev;
	struct buffer_head *bh;

	fmode_t mode = FMODE_READ;

	int error;

	/* set param */
	if (!(flags & MS_RDONLY))
		mode |= FMODE_WRITE;
	
	/* read super block */
	/* open device */
	bdev = open_bdev_exclusive(dev_name, flags, fs_type);
	if (IS_ERR(bdev))
		return PTR_ERR(bdev);			

	set_blocksize(bdev, 4096);

		printk(KERN_ERR "begin read sb\n");
	/* read super block of epiifs and check */
	bh = epii_read_dev_super(bdev);
	if (NULL == bh) {
		printk(KERN_ERR "read sb fail\n");
		close_bdev_exclusive(bdev, flags);
		return -EIO; 
	}

	printk(KERN_ERR "read sb OK\n");

	s = sget(fs_type, epii_test_sb, set_anon_super, bdev);
	if (IS_ERR(s))
		goto error_s;

	printk(KERN_WARNING "SGET OK\n");

	root = get_default_root(s, 100);
	if (IS_ERR(root)) {
		error = PTR_ERR(root);
		deactivate_locked_super(s);
		goto error;
	}

	mnt->mnt_sb = s;
	mnt->mnt_root = root;

	printk(KERN_WARNING "get sb ok");
	return 0;

error_s:
	printk(KERN_WARNING "[%s,%d]get sb fail %d \n", __FUNCTION__, __LINE__, error);
	error = PTR_ERR(s);

error:
	printk(KERN_WARNING "[%s,%d]get sb fail %d \n", __FUNCTION__, __LINE__, error);
	brelse(bh);	

	close_bdev_exclusive(bdev, flags);

	return error;
}


static struct file_system_type epii_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "epii",
	.get_sb		= epii_get_sb,
	.kill_sb	= kill_anon_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int epiifs_init(void)
{
	int err;

	err = register_filesystem(&epii_fs_type);
	if (err) 
		return err;
	printk(KERN_INFO "Hello world\n");
	return 0;
}

static void epiifs_exit(void)
{
	unregister_filesystem(&epii_fs_type);

	printk(KERN_INFO"Goodbye,world\n");

}

module_init(epiifs_init);

module_exit(epiifs_exit);
MODULE_LICENSE("GPL");

