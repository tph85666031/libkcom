#ifndef __KCOM_MAP_H__
#define __KCOM_MAP_H__

#include <linux/hashtable.h>
#include <linux/stringhash.h>

#define HASH_HEAD_COUNT(map) *((long*)map-1)

typedef struct
{
    struct hlist_node node;
    int key_size;
    char data[0];
} KCOM_HASH_NODE;

uint64_t kcom_hash_string(const char* str)
{
    if(unlikely(str == NULL))
    {
        return 0;
    }
    return hashlen_string(NULL, str);
}

inline uint64_t kcom_hash_data(const void* data, int data_size)
{
    if(unlikely(data == NULL || data_size <= 0))
    {
        return 0;
    }
    return hashlen_create(full_name_hash(NULL, data, data_size), data_size);
}

struct hlist_head* kcom_map_create(int hash_bits)
{
    if(hash_bits <= 0)
    {
        hash_bits = 3;
    }
    long* ret = (long*)kmalloc(sizeof(long) + sizeof(struct hlist_head) * (1 << hash_bits), GFP_KERNEL);
    if(ret == NULL)
    {
        return NULL;
    }
    ret[0] = 1 << hash_bits;
    struct hlist_head* head = (struct hlist_head*)(ret + 1);
    __hash_init(head, 1 << hash_bits);
    return head;
}

void kcom_map_remove(struct hlist_head* map, const char* key)
{
    if(unlikely(map == NULL || key == NULL))
    {
        return;
    }
    KCOM_HASH_NODE* node = NULL;
    hlist_for_each_entry(node, &map[hash_min(kcom_hash_string(key), ilog2(HASH_HEAD_COUNT(map)))], node)
    {
        if(memcmp(key, node->data, node->key_size) == 0)
        {
            hash_del(&node->node);
            kfree(node);
            return;
        }
    }
}

bool kcom_map_add(struct hlist_head* map, const char* key, void* value, int value_size)
{
    if(unlikely(map == NULL || key == NULL || value == NULL || value_size <= 0))
    {
        return false;
    }
    kcom_map_remove(map, key);
    int key_size = strlen(key) + 1;
    KCOM_HASH_NODE* node = kmalloc(sizeof(KCOM_HASH_NODE) + key_size + value_size, GFP_KERNEL);
    if(unlikely(node == NULL))
    {
        return false;
    }
    node->key_size = key_size;
    memcpy(node->data, key, key_size);
    memcpy(node->data + key_size, value, value_size);
    hlist_add_head(&node->node, &map[hash_min(kcom_hash_data(key, key_size - 1), ilog2(HASH_HEAD_COUNT(map)))]);
    return true;
}

inline bool kcom_map_add_string(struct hlist_head* map, const char* key, const char* value)
{
    return kcom_map_add(map, key, value, strlen(value) + 1);
}

void* kcom_map_get(struct hlist_head* map, const char* key)
{
    if(unlikely(map == NULL || key == NULL))
    {
        return NULL;
    }
    KCOM_HASH_NODE* node = NULL;
    hlist_for_each_entry(node, &map[hash_min(kcom_hash_string(key), ilog2(HASH_HEAD_COUNT(map)))], node)
    {
        if(memcmp(key, node->data, node->key_size) == 0)
        {
            return (node->data + node->key_size);
        }
    }
    return NULL;
}

bool kcom_map_exist(struct hlist_head* map, const char* key)
{
    if(unlikely(map == NULL || key == NULL))
    {
        return NULL;
    }
    KCOM_HASH_NODE* node = NULL;
    hlist_for_each_entry(node, &map[hash_min(kcom_hash_string(key), ilog2(HASH_HEAD_COUNT(map)))], node)
    {
        if(memcmp(key, node->data, node->key_size) == 0)
        {
            return true;
        }
    }
    return false;
}

void kcom_map_clear(struct hlist_head* map)
{
    if(unlikely(map == NULL))
    {
        return;
    }
    KCOM_HASH_NODE* node = NULL;
    struct hlist_node* tmp = NULL;
    int i;
    for(i = 0; i < HASH_HEAD_COUNT(map); i++)
    {
        hlist_for_each_entry_safe(node, tmp, &map[i], node)
        {
            hash_del(&node->node);
            kfree(node);
        }
    }
}

void kcom_map_destroy(struct hlist_head* map)
{
    if(map != NULL)
    {
        kcom_map_clear(map);
        kfree((long*)map - 1);
    }
}

#endif /* __KCOM_MAP_H__ */

