#ifndef __SUPER_H__
#define __SUPER_H__

#define EPII_MAGIC  "e^(PIi)+1=0"

struct epii_super_block {
	__le64 s_uuid;
	__le16 s_blocksize; 	/* block size */
	char s_magic[16];   /* magic "e^(PIi)+1=0" */	
	__le64 s_root;

} __attribute__ ((__packed__));	

#endif
