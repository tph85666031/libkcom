#include <linux/module.h>
#include <linux/slab.h>
#include <linux/ctype.h>

#include "kcom_base.h"
#include "kcom_log.h"

/* Character class matching */
static bool __match_charclass(const char* pat, char c, const char** npat)
{
    bool complement = false, ret = true;

    if(*pat == '!')
    {
        complement = true;
        pat++;
    }
    if(*pat++ == c)     /* First character is special */
    {
        goto end;
    }

    while(*pat && *pat != ']')      /* Matching */
    {
        if(*pat == '-' && *(pat + 1) != ']')    /* Range */
        {
            if(*(pat - 1) <= c && c <= *(pat + 1))
            {
                goto end;
            }
            if(*(pat - 1) > *(pat + 1))
            {
                goto error;
            }
            pat += 2;
        }
        else if(*pat++ == c)
        {
            goto end;
        }
    }
    if(!*pat)
    {
        goto error;
    }
    ret = false;

end:
    while(*pat && *pat != ']')  /* Searching closing */
    {
        pat++;
    }
    if(!*pat)
    {
        goto error;
    }
    *npat = pat + 1;
    return complement ? !ret : ret;

error:
    return false;
}

/* Glob/lazy pattern matching */
static bool __match_glob(const char* str, const char* pat, bool ignore_space,
                         bool case_ins)
{
    while(*str && *pat && *pat != '*')
    {
        if(ignore_space)
        {
            /* Ignore spaces for lazy matching */
            if(isspace(*str))
            {
                str++;
                continue;
            }
            if(isspace(*pat))
            {
                pat++;
                continue;
            }
        }
        if(*pat == '?')     /* Matches any single character */
        {
            str++;
            pat++;
            continue;
        }
        else if(*pat == '[')    /* Character classes/Ranges */
            if(__match_charclass(pat + 1, *str, &pat))
            {
                str++;
                continue;
            }
            else
            {
                return false;
            }
        else if(*pat == '\\')  /* Escaped char match as normal char */
        {
            pat++;
        }
        if(case_ins)
        {
            if(tolower(*str) != tolower(*pat))
            {
                return false;
            }
        }
        else if(*str != *pat)
        {
            return false;
        }
        str++;
        pat++;
    }
    /* Check wild card */
    if(*pat == '*')
    {
        while(*pat == '*')
        {
            pat++;
        }
        if(!*pat)   /* Tail wild card matches all */
        {
            return true;
        }
        while(*str)
        {
            if(__match_glob(str++, pat, ignore_space, case_ins))
            {
                return true;
            }
        }
    }
    return !*str && !*pat;
}

/**
    kcom_string_match - glob expression pattern matching
    @str: the target string to match
    @pat: the pattern string to match

    This returns true if the @str matches @pat. @pat can includes wildcards
    ('*','?') and character classes ([CHARS], complementation and ranges are
    also supported). Also, this supports escape character ('\') to use special
    characters as normal character.

    Note: if @pat syntax is broken, this always returns false.
*/
bool kcom_string_match(const char* str, const char* pattern)
{
    return __match_glob(str, pattern, false, false);
}
EXPORT_SYMBOL(kcom_string_match);

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
EXPORT_SYMBOL(kcom_ipv4_to_string);

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
EXPORT_SYMBOL(kcom_ipv4_from_string);

char* kcom_strdup(char* str)
{
    if(str == NULL || str[0] == '\0')
    {
        return NULL;
    }

    char* buf = (char*)kmalloc(strlen(str) + 1, GFP_KERNEL);
    if(buf == NULL)
    {
        return NULL;
    }

    strcpy(buf, str);
    return buf;
}
EXPORT_SYMBOL(kcom_strdup);

char* kcom_path_from_struct_path_dentry(const struct path* path, const struct dentry* dentry, char* buf, int buf_size)
{
    if(unlikely(buf == NULL || buf_size <= 0 || path == NULL || dentry == NULL || dentry->d_name.name == NULL))
    {
        return NULL;
    }

    int size_name = strlen(dentry->d_name.name) + 1;
    if(buf_size < size_name)
    {
        return NULL;
    }
    char* full_path = d_path(path, buf, buf_size - size_name);
    if(IS_ERR(full_path))
    {
        KLOG_D("failed,error=%ld", PTR_ERR(full_path));
        return NULL;
    }
    strcat(full_path, "/");
    strcat(full_path, dentry->d_name.name);
    buf[buf_size - 1] = '\0';
    return full_path;
}
EXPORT_SYMBOL(kcom_path_from_struct_path_dentry);

char* kcom_path_from_struct_path(const struct path* path, char* buf, int buf_size)
{
    if(unlikely(path == NULL || buf == NULL || buf_size <= 0))
    {
        return NULL;
    }
    char* val = d_path(path, buf, buf_size);
    if(IS_ERR(val))
    {
        KLOG_D("failed,error=%ld", PTR_ERR(val));
        return NULL;
    }
    return val;
}
EXPORT_SYMBOL(kcom_path_from_struct_path);

char* kcom_path_from_struct_dentry(const struct dentry* dentry, char* buf, int buf_size)
{
    if(unlikely(dentry == NULL || buf == NULL || buf_size <= 0))
    {
        return NULL;
    }
    char* val = dentry_path_raw(dentry, buf, buf_size);
    if(IS_ERR(val))
    {
        KLOG_D("failed,error=%ld", PTR_ERR(val));
        return NULL;
    }
    return val;
}
EXPORT_SYMBOL(kcom_path_from_struct_dentry);

