#ifndef __KCOM_BASE_H__
#define __KCOM_BASE_H__

#include <linux/dcache.h>

char* kcom_ipv4_to_string(__be32 ip, char* buf, int buf_size)
{
    if(buf == NULL || buf_size < sizeof("000.000.000.000"))
    {
        return NULL;
    }
    snprintf(buf, buf_size, "%u.%u.%u.%u",
             ((ip) >> 0) & 0xFF, ((ip) >> 8) & 0xFF,
             ((ip) >> 16) & 0xFF, ((ip) >> 24) & 0xFF);
    return buf;
}

__be32 kcom_ipv4_from_string(const char* ipv4_str)
{
    if(ipv4_str == NULL)
    {
        return 0;
    }
    __be32 ip[4];
    int ret = sscanf(ipv4_str, "%u.%u.%u.%u",
                     &ip[0], &ip[1], &ip[2], &ip[3]);
    if(ret != 4)
    {
        return 0;
    }
    if(ip[0] > 255 || ip[1] > 255 || ip[2] > 255 || ip[3] > 255)
    {
        return 0;
    }
    __be32 val = 0;
    val = (ip[0] << 0) | (ip[1] << 8) | (ip[2] << 16) | (ip[3] << 24);
    return val;
}

char* kcom_file_path_from_path(const struct path* path, const struct dentry* dentry, char* buf, int buf_size)
{
    if(path == NULL || dentry == NULL || dentry->d_name.name == NULL || buf == NULL || buf_size <= 0)
    {
        return NULL;
    }
    int size_name = strlen(dentry->d_name.name) + 1;
    if(buf_size < size_name)
    {
        return NULL;
    }
    char* full_path = d_path(path, buf, buf_size - size_name);
    strcat(full_path, "/");
    strcat(full_path, dentry->d_name.name);
    buf[buf_size - 1] = '\0';
    return full_path;
}

#endif /* __KCOM_BASE_H__ */

