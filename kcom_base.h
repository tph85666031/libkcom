#ifndef __KCOM_BASE_H__
#define __KCOM_BASE_H__

#include <linux/dcache.h>

char* kcom_ipv4_to_string(__be32 ip, char* buf, int buf_size);
__be32 kcom_ipv4_from_string(const char* ipv4_str);
char* kcom_file_path_from_path(const struct path* path, const struct dentry* dentry, char* buf, int buf_size);
char* kcom_strdup(char* str);

#endif /* __KCOM_BASE_H__ */

