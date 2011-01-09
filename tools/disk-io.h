#ifndef __DISKIO_H__
#define __DISKIO_H__

#define EPII_SUPER_INFO_OFFSET (64 * 1024)
#define EPII_SUPER_INFO_SIZE   4096

#define EPII_SUPER_MIRROR_MAX 3
#define EPII_SUPER_MIRROR_SHIFT 12 

static inline u64 epii_sb_offset(int mirror)
{
	u64 start = 16 * 1024;
	if (mirror)
		return start << (EPII_SUPER_MIRROR_SHIFT * mirror);

	return EPII_SUPER_INFO_OFFSET;
}

#endif


