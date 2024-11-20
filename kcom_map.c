#include <linux/slab.h>
#include "kcom_map.h"

#define HASH_HEAD_COUNT(map) *((long*)map-1)

typedef struct
{
    struct hlist_node node;
    int key_size;
    int data_size;
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
EXPORT_SYMBOL(kcom_hash_string);

inline uint64_t kcom_hash_data(const void* data, int data_size)
{
    if(unlikely(data == NULL || data_size <= 0))
    {
        return 0;
    }
    return hashlen_create(full_name_hash(NULL, data, data_size), data_size);
}
EXPORT_SYMBOL(kcom_hash_data);

struct hlist_head* kcom_map_create(int hash_bits)
{
    if(hash_bits <= 0)
    {
        hash_bits = 3;
    }
    long* ret = (long*)kcalloc(1, sizeof(long) + sizeof(struct hlist_head) * (1 << hash_bits), GFP_KERNEL);
    if(ret == NULL)
    {
        return NULL;
    }
    ret[0] = 1 << hash_bits;
    struct hlist_head* head = (struct hlist_head*)(ret + 1);
    __hash_init(head, 1 << hash_bits);
    return head;
}
EXPORT_SYMBOL(kcom_map_create);

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
EXPORT_SYMBOL(kcom_map_remove);

void* kcom_map_add(struct hlist_head* map, const char* key, const void* value, int value_size)
{
    if(unlikely(map == NULL || key == NULL || value == NULL || value_size <= 0))
    {
        return NULL;
    }
    kcom_map_remove(map, key);
    int key_size = strlen(key) + 1;
    KCOM_HASH_NODE* node = kcalloc(1, sizeof(KCOM_HASH_NODE) + key_size + value_size, GFP_KERNEL);
    if(unlikely(node == NULL))
    {
        return NULL;
    }
    node->key_size = key_size;
    node->data_size = value_size;
    memcpy(node->data, key, key_size);
    memcpy(node->data + key_size, value, value_size);
    hlist_add_head(&node->node, &map[hash_min(kcom_hash_data(key, key_size - 1), ilog2(HASH_HEAD_COUNT(map)))]);
    return (node->data + node->key_size);
}
EXPORT_SYMBOL(kcom_map_add);

char* kcom_map_add_string(struct hlist_head* map, const char* key, const char* value)
{
    return (char*)kcom_map_add(map, key, value, strlen(value) + 1);
}
EXPORT_SYMBOL(kcom_map_add_string);

bool kcom_map_add_int64(struct hlist_head* map, const char* key, int64_t value)
{
    return kcom_map_add(map, key, &value, sizeof(value)) != NULL;
}
EXPORT_SYMBOL(kcom_map_add_int64);

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
EXPORT_SYMBOL(kcom_map_get);

int64_t kcom_map_get_int64(struct hlist_head* map, const char* key, int64_t default_val)
{
    if(unlikely(map == NULL || key == NULL))
    {
        return default_val;
    }
    KCOM_HASH_NODE* node = NULL;
    hlist_for_each_entry(node, &map[hash_min(kcom_hash_string(key), ilog2(HASH_HEAD_COUNT(map)))], node)
    {
        if(memcmp(key, node->data, node->key_size) == 0 && node->data_size == sizeof(int64_t))
        {
            return *((int64_t*)(node->data + node->key_size));
        }
    }
    return default_val;
}
EXPORT_SYMBOL(kcom_map_get_int64);

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
EXPORT_SYMBOL(kcom_map_exist);

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
EXPORT_SYMBOL(kcom_map_clear);

void kcom_map_destroy(struct hlist_head* map)
{
    if(map != NULL)
    {
        kcom_map_clear(map);
        kfree((long*)map - 1);
    }
}
EXPORT_SYMBOL(kcom_map_destroy);

