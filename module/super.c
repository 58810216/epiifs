#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mount.h>


static int llfs_test_sb(struct super_block *s, void *data)
{
	int test = 0;

	test = (int)data;

	printk(KERN_WARNING "data is %d\n", test);

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
static int llfs_get_sb(struct file_system_type *fs_type, int flags,
		const char *dev_name, void *data, struct vfsmount *mnt)
{
	struct super_block *s;
	struct dentry *root;

	int error;
	
	s = sget(fs_type, llfs_test_sb, set_anon_super, (void *)0);
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

	return 0;

error_s:
	error = PTR_ERR(s);
error:
	printk(KERN_WARNING "[%s,%d]get sb fail %d \n", __FUNCTION__, __LINE__, error);
	return error;
}


static struct file_system_type llfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "llfs",
	.get_sb		= llfs_get_sb,
	.kill_sb	= kill_anon_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int hello_init(void)
{
	int err;

	err = register_filesystem(&llfs_fs_type);
	if (err) 
		return err;
	printk(KERN_INFO "Hello world\n");
	return 0;
}

static void hello_exit(void)
{
	unregister_filesystem(&llfs_fs_type);

	printk(KERN_INFO"Goodbye,world\n");

}

module_init(hello_init);

module_exit(hello_exit);
MODULE_LICENSE("GPL");

