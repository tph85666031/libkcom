#ifndef __KCOM_BASE_H__
#define __KCOM_BASE_H__

#include <linux/socket.h>
#include <linux/dcache.h>
#include <linux/fs.h>

bool kcom_string_match(const char* str, const char* pattern);
char* kcom_sockaddr_storage_to_string(const struct sockaddr_storage* addr, char* buf, int buf_size);
char* kcom_ipv4_to_string(__be32 ip, char* buf, int buf_size);
__be32 kcom_ipv4_from_string(const char* ipv4_str);
char* kcom_strdup(char* str);
char* kcom_path_from_struct_path_dentry(const struct path* path, const struct dentry* dentry, char* buf, int buf_size);
char* kcom_path_from_struct_path(const struct path* path, char* buf, int buf_size);
char* kcom_path_from_struct_dentry(const struct dentry* dentry, char* buf, int buf_size);

#endif /* __KCOM_BASE_H__ */

