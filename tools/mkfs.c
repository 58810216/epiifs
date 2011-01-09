#define _XOPEN_SOURCE 600
#define __USE_XOPEN2K
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <mntent.h>
#include <linux/loop.h>
#include <linux/major.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "super.h"
#include "epii_types.h"
#include "disk-io.h"


int is_existing_blk_or_reg_file(const char* filename)
{
	struct stat st_buf;

	if(stat(filename, &st_buf) < 0) {
		if(errno == ENOENT)
			return 0;
		else
			return -errno;
	}

	return (S_ISBLK(st_buf.st_mode) || S_ISREG(st_buf.st_mode));
}


int is_loop_device (const char* device) 
{
	struct stat statbuf;

	if(stat(device, &statbuf) < 0)
		return -errno;

	return (S_ISBLK(statbuf.st_mode) &&
		MAJOR(statbuf.st_rdev) == LOOP_MAJOR);
}

int is_same_blk_file(const char* a, const char* b)
{
	struct stat st_buf_a, st_buf_b;
	char real_a[PATH_MAX];
	char real_b[PATH_MAX];

	if(!realpath(a, real_a) ||
	   !realpath(b, real_b))
	{
		return -errno;
	}

	/* Identical path? */
	if(strcmp(real_a, real_b) == 0)
		return 1;

	if(stat(a, &st_buf_a) < 0 ||
	   stat(b, &st_buf_b) < 0)
	{
		return -errno;
	}

	/* Same blockdevice? */
	if(S_ISBLK(st_buf_a.st_mode) &&
	   S_ISBLK(st_buf_b.st_mode) &&
	   st_buf_a.st_rdev == st_buf_b.st_rdev)
	{
		return 1;
	}

	/* Hardlink? */
	if (st_buf_a.st_dev == st_buf_b.st_dev &&
	    st_buf_a.st_ino == st_buf_b.st_ino)
	{
		return 1;
	}

	return 0;
}

int resolve_loop_device(const char* loop_dev, char* loop_file, int max_len)
{
	int loop_fd;
	int ret_ioctl;
	struct loop_info loopinfo;

	if ((loop_fd = open(loop_dev, O_RDONLY)) < 0)
		return -errno;

	ret_ioctl = ioctl(loop_fd, LOOP_GET_STATUS, &loopinfo);
	close(loop_fd);

	if (ret_ioctl == 0)
		strncpy(loop_file, loopinfo.lo_name, max_len);
	else
		return -errno;

	return 0;
}

int is_same_loop_file(const char* a, const char* b)
{
	char res_a[PATH_MAX];
	char res_b[PATH_MAX];
	const char* final_a;
	const char* final_b;
	int ret;

	/* Resolve a if it is a loop device */
	if((ret = is_loop_device(a)) < 0) {
	   return ret;
	} else if(ret) {
		if((ret = resolve_loop_device(a, res_a, sizeof(res_a))) < 0)
			return ret;

		final_a = res_a;
	} else {
		final_a = a;
	}

	/* Resolve b if it is a loop device */
	if((ret = is_loop_device(b)) < 0) {
	   return ret;
	} else if(ret) {
		if((ret = resolve_loop_device(b, res_b, sizeof(res_b))) < 0)
			return ret;

		final_b = res_b;
	} else {
		final_b = b;
	}

	return is_same_blk_file(final_a, final_b);
}

int check_mounted(char *file)
{
	int fd;
	int ret;
	struct mntent *mnt;
	FILE *f;

	if ((f = setmntent("/proc/mounts", "r")) == NULL)
		return -errno;

	while ((mnt = getmntent(f)) != NULL) {
		fprintf(stdout, "Check if %s == %s \n", file, mnt->mnt_fsname);
		if((ret = is_existing_blk_or_reg_file(mnt->mnt_fsname)) < 0)
			goto error_check;
		else if (!ret)
			continue;
		
		fprintf(stdout, "Check if %s is loop file \n", mnt->mnt_fsname);	
		ret = is_same_loop_file(mnt->mnt_fsname, file);
		if (ret < 0) 
			goto error_check;
		else if (ret)
			break;
	}

	ret = (mnt != NULL);

error_check:
	endmntent(f);

	return ret;
}

int main(int argc, char *argv[])
{
	struct epii_super_block super;
	char *file;
	int ret;
	int fd;

	if (argc != 2) {
		fprintf(stderr, "give a partition\n");
		ret = -1;
		goto error;
	}

	file = argv[argc - 1];

	fprintf(stdout, "Format %s , check: \n", file);
	
	ret = check_mounted(file);
	if (ret)
		goto error;

	fprintf(stdout, "Format %s , begin format: \n", file);

	fd = open(file, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "unable to open %s\n", file);
		exit(1);
	}

	fprintf(stdout, "Write Super Block \n");
	memset(&super, 0, sizeof(super));

	strncpy(super.s_magic, EPII_MAGIC, sizeof(EPII_MAGIC));
	super.s_uuid = 0x1234567012345;
	super.s_blocksize = 4096;	
	super.s_root = 1 << 20;

	fprintf(stdout, "magic %s %lu\n", super.s_magic, sizeof(super));
	ret = pwrite(fd, &super, sizeof(super), EPII_SUPER_INFO_OFFSET);	
	if (ret != sizeof(super)) {
		fprintf(stderr, "write super fail.\n");
		ret = 0x12;
		goto error;
	}	

	
	return 0;

error:

	fprintf(stderr, "Fomat err %d\n", ret);	
	return -1;
}
