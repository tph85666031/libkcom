#include <linux/stringhash.h>
#include <linux/slab.h>

#include "kcom_base.h"
#include "kcom_map.h"
#include "rapidhash.h"

uint64_t kcom_hash_string(const char* str)
{
    if(unlikely(str == NULL))
    {
        return 0;
    }
    return rapidhash(str, strlen(str));
}
EXPORT_SYMBOL(kcom_hash_string);

uint64_t kcom_hash_data(const void* data, int data_size)
{
    if(unlikely(data == NULL || data_size <= 0))
    {
        return 0;
    }
    return rapidhash(data, data_size);
}
EXPORT_SYMBOL(kcom_hash_data);

KCOM_MAP* kcom_map_create(int hash_count)
{
    if(unlikely(hash_count <= 0))
    {
        hash_count = 16;
    }
    KCOM_MAP* map = (KCOM_MAP*)kzalloc(sizeof(KCOM_MAP) + sizeof(struct hlist_head) * hash_count, GFP_NOWAIT);
    if(unlikely(map == NULL))
    {
        return NULL;
    }
    map->head_count = hash_count;
    spin_lock_init(&map->lock);
    __hash_init(map->heads, map->head_count);
    return map;
}
EXPORT_SYMBOL(kcom_map_create);

void kcom_map_lock(KCOM_MAP* map)
{
    if(unlikely(map == NULL))
    {
        return;
    }
    spin_lock(&map->lock);
}
EXPORT_SYMBOL(kcom_map_lock);

void kcom_map_unlock(KCOM_MAP* map)
{
    if(unlikely(map == NULL))
    {
        return;
    }
    spin_unlock(&map->lock);
}
EXPORT_SYMBOL(kcom_map_unlock);

void kcom_map_remove(KCOM_MAP* map, uint64_t key)
{
    if(unlikely(map == NULL))
    {
        return;
    }
    KCOM_HASH_NODE* node = NULL;
    spin_lock(&map->lock);
    struct hlist_head* head = &map->heads[key % map->head_count];
    hlist_for_each_entry_rcu(node, head, node)
    {
        if(node->key_u64 == key)
        {
            hash_del_rcu(&node->node);
            kfree_rcu(node, rcu);
            atomic_dec(&map->item_count);
        }
    }
    spin_unlock(&map->lock);
}
EXPORT_SYMBOL(kcom_map_remove);

void kcom_map_remove_value(KCOM_MAP* map, uint64_t key, const void* value, int value_size)
{
    if(unlikely(map == NULL || value == NULL || value_size <= 0))
    {
        return;
    }
    KCOM_HASH_NODE* node = NULL;
    spin_lock(&map->lock);
    struct hlist_head* head = &map->heads[key % map->head_count];
    hlist_for_each_entry_rcu(node, head, node)
    {
        if(node->key_u64 == key
                && node->data_size == value_size
                && memcmp(node->data, value, value_size) == 0)
        {
            hash_del_rcu(&node->node);
            kfree_rcu(node, rcu);
            atomic_dec(&map->item_count);
            break;
        }
    }
    spin_unlock(&map->lock);
}
EXPORT_SYMBOL(kcom_map_remove_value);

int kcom_map_count(KCOM_MAP* map)
{
    if(map == NULL)
    {
        return -1;
    }
    return atomic_read(&map->item_count);
}
EXPORT_SYMBOL(kcom_map_count);

bool kcom_map_add(KCOM_MAP* map, uint64_t key, const void* value, int value_size)
{
    if(unlikely(map == NULL || value == NULL || value_size <= 0))
    {
        return false;
    }
    KCOM_HASH_NODE* node = kzalloc(sizeof(KCOM_HASH_NODE) + value_size, GFP_NOWAIT);
    if(unlikely(node == NULL))
    {
        return false;
    }
    kcom_map_remove_value(map, key, value, value_size);
    node->key_u64 = key;
    node->data_size = value_size;
    memcpy(node->data, value, value_size);
    spin_lock(&map->lock);
    hlist_add_tail_rcu(&node->node, &map->heads[key % map->head_count]);
    atomic_inc(&map->item_count);
    spin_unlock(&map->lock);
    return true;
}
EXPORT_SYMBOL(kcom_map_add);

bool kcom_map_add_string(KCOM_MAP* map, uint64_t key, const char* value)
{
    return kcom_map_add(map, key, value, strlen(value) + 1);
}
EXPORT_SYMBOL(kcom_map_add_string);

bool kcom_map_add_int64(KCOM_MAP* map, uint64_t key, int64_t value)
{
    return kcom_map_add(map, key, &value, sizeof(value));
}
EXPORT_SYMBOL(kcom_map_add_int64);

void kcom_map_iterator_init(KCOM_MAP_ITERATOR* it, KCOM_MAP* map)
{
    if(it != NULL && map != NULL)
    {
        it->map = map;
        it->head_pos = 0;
        it->node = NULL;
    }
}
EXPORT_SYMBOL(kcom_map_iterator_init);

bool kcom_map_iterator_next_rcu(KCOM_MAP_ITERATOR* it, uint64_t* key, void** value, int* value_size)
{
    if(unlikely(it == NULL || it->map == NULL || it->head_pos < 0 || key == NULL || value == NULL || value_size == NULL))
    {
        return false;
    }
    for(; it->head_pos < it->map->head_count;)
    {
        if(it->node == NULL)
        {
            hlist_for_each_entry_rcu(it->node, &it->map->heads[it->head_pos], node)
            {
                *key = it->node->key_u64;
                *value = it->node->data + it->node->key_size;
                *value_size = it->node->data_size;
                return true;
            }
            it->head_pos++;
        }
        else
        {
            hlist_for_each_entry_continue_rcu(it->node, node)
            {
                *key = it->node->key_u64;
                *value = it->node->data + it->node->key_size;
                *value_size = it->node->data_size;
                return true;
            }
            it->head_pos++;
        }
    }

    return false;
}
EXPORT_SYMBOL(kcom_map_iterator_next_rcu);

void* kcom_map_get_rcu(KCOM_MAP* map, uint64_t key, void* default_val)
{
    if(unlikely(map == NULL))
    {
        return default_val;
    }
    KCOM_HASH_NODE* node = NULL;
    hlist_for_each_entry_rcu(node, &map->heads[key % map->head_count], node)
    {
        if(node->key_u64 == key)
        {
            return node->data;
        }
    }
    return default_val;
}
EXPORT_SYMBOL(kcom_map_get_rcu);

void* kcom_map_get_rcu_next(void* pre_value, uint64_t key, void* default_val)
{
    if(unlikely(pre_value == NULL))
    {
        return default_val;
    }
    KCOM_HASH_NODE* node = container_of(pre_value, KCOM_HASH_NODE, data);
    hlist_for_each_entry_continue_rcu(node, node)
    {
        if(node->key_u64 == key)
        {
            return node->data;
        }
    }
    return default_val;
}
EXPORT_SYMBOL(kcom_map_get_rcu_next);

void* kcom_map_get_copy(KCOM_MAP* map, uint64_t key, void* buf, int buf_size, void* default_val)
{
    if(unlikely(map == NULL || buf == NULL || buf_size <= 0))
    {
        return default_val;
    }

    KCOM_HASH_NODE* node = NULL;
    rcu_read_lock();
    hlist_for_each_entry_rcu(node, &map->heads[key % map->head_count], node)
    {
        if(node->key_u64 == key)
        {
            memcpy(buf, node->data, min(buf_size, node->data_size));
            break;
        }
    }
    rcu_read_unlock();
    return buf;
}
EXPORT_SYMBOL(kcom_map_get_copy);

int64_t kcom_map_get_int64(KCOM_MAP* map, uint64_t key, int64_t default_val)
{
    if(unlikely(map == NULL))
    {
        return default_val;
    }
    KCOM_HASH_NODE* node = NULL;
    rcu_read_lock();
    hlist_for_each_entry_rcu(node, &map->heads[key % map->head_count], node)
    {
        if(node->key_u64 == key && node->data_size == sizeof(int64_t))
        {
            int64_t value;
            memcpy(&value, node->data, sizeof(int64_t));
            rcu_read_unlock();
            return value;
        }
    }
    rcu_read_unlock();
    return default_val;
}
EXPORT_SYMBOL(kcom_map_get_int64);

bool kcom_map_exist(KCOM_MAP* map, uint64_t key)
{
    if(unlikely(map == NULL))
    {
        return NULL;
    }
    KCOM_HASH_NODE* node = NULL;
    rcu_read_lock();
    hlist_for_each_entry_rcu(node, &map->heads[key % map->head_count], node)
    {
        if(node->key_u64 == key)
        {
            rcu_read_unlock();
            return true;
        }
    }
    rcu_read_unlock();
    return false;
}
EXPORT_SYMBOL(kcom_map_exist);

bool kcom_map_exist_value(KCOM_MAP* map, uint64_t key, const void* value, int value_size)
{
    if(unlikely(map == NULL || value == NULL || value_size <= 0))
    {
        return NULL;
    }
    KCOM_HASH_NODE* node = NULL;
    rcu_read_lock();
    hlist_for_each_entry_rcu(node, &map->heads[key % map->head_count], node)
    {
        if(node->key_u64 == key
                && node->data_size == value_size
                && memcmp(node->data, value, value_size) == 0)
        {
            rcu_read_unlock();
            return true;
        }
    }
    rcu_read_unlock();
    return false;
}
EXPORT_SYMBOL(kcom_map_exist_value);

void kcom_map_clear(KCOM_MAP* map)
{
    if(unlikely(map == NULL))
    {
        return;
    }
    KCOM_HASH_NODE* node = NULL;
    int i;
    spin_lock(&map->lock);
    for(i = 0; i < map->head_count; i++)
    {
        hlist_for_each_entry_rcu(node, &map->heads[i], node)
        {
            hash_del_rcu(&node->node);
            kfree_rcu(node, rcu);
        }
    }
    atomic_set(&map->item_count, 0);
    spin_unlock(&map->lock);
}
EXPORT_SYMBOL(kcom_map_clear);

void kcom_map_destroy(KCOM_MAP* map)
{
    if(map != NULL)
    {
        kcom_map_clear(map);
        kfree(map);
    }
}
EXPORT_SYMBOL(kcom_map_destroy);

void kcom_maps_remove(KCOM_MAP* map, const char* key)
{
    if(unlikely(map == NULL || key == NULL))
    {
        return;
    }
    KCOM_HASH_NODE* node = NULL;
    spin_lock(&map->lock);
    struct hlist_head* head = &map->heads[kcom_hash_string(key) % map->head_count];
    hlist_for_each_entry_rcu(node, head, node)
    {
        if(memcmp(key, node->data, node->key_size) == 0)
        {
            hash_del_rcu(&node->node);
            kfree_rcu(node, rcu);
            atomic_dec(&map->item_count);
        }
    }
    spin_unlock(&map->lock);
}
EXPORT_SYMBOL(kcom_maps_remove);

void kcom_maps_remove_value(KCOM_MAP* map, const char* key, const void* value, int value_size)
{
    if(unlikely(map == NULL || key == NULL || value == NULL || value_size <= 0))
    {
        return;
    }
    KCOM_HASH_NODE* node = NULL;
    spin_lock(&map->lock);
    struct hlist_head* head = &map->heads[kcom_hash_string(key) % map->head_count];
    hlist_for_each_entry_rcu(node, head, node)
    {
        if(memcmp(key, node->data, node->key_size) == 0
                && node->data_size == value_size
                && memcmp(node->data + node->key_size, value, value_size) == 0)
        {
            hash_del_rcu(&node->node);
            kfree_rcu(node, rcu);
            atomic_dec(&map->item_count);
            break;
        }
    }
    spin_unlock(&map->lock);
}
EXPORT_SYMBOL(kcom_maps_remove_value);

bool kcom_maps_add(KCOM_MAP* map, const char* key, const void* value, int value_size)
{
    if(unlikely(map == NULL || key == NULL || value == NULL || value_size <= 0))
    {
        return false;
    }
    int key_size = strlen(key) + 1;
    KCOM_HASH_NODE* node = kzalloc(sizeof(KCOM_HASH_NODE) + key_size + value_size, GFP_NOWAIT);
    if(unlikely(node == NULL))
    {
        return false;
    }
    kcom_maps_remove_value(map, key, value, value_size);
    node->key_size = key_size;
    node->data_size = value_size;
    memcpy(node->data, key, key_size);
    memcpy(node->data + key_size, value, value_size);
    spin_lock(&map->lock);
    hlist_add_tail_rcu(&node->node, &map->heads[kcom_hash_string(key) % map->head_count]);
    atomic_inc(&map->item_count);
    spin_unlock(&map->lock);
    return true;
}
EXPORT_SYMBOL(kcom_maps_add);

bool kcom_maps_add_string(KCOM_MAP* map, const char* key, const char* value)
{
    return kcom_maps_add(map, key, value, strlen(value) + 1);
}
EXPORT_SYMBOL(kcom_maps_add_string);

bool kcom_maps_add_int64(KCOM_MAP* map, const char* key, int64_t value)
{
    return kcom_maps_add(map, key, &value, sizeof(value));
}
EXPORT_SYMBOL(kcom_maps_add_int64);

void* kcom_maps_get_rcu(KCOM_MAP* map, const char* key, void* default_val)
{
    if(unlikely(map == NULL || key == NULL))
    {
        return default_val;
    }
    KCOM_HASH_NODE* node = NULL;
    hlist_for_each_entry_rcu(node, &map->heads[kcom_hash_string(key) % map->head_count], node)
    {
        if(memcmp(node->data, key, node->key_size) == 0)
        {
            return (node->data + node->key_size);
        }
    }
    return default_val;
}
EXPORT_SYMBOL(kcom_maps_get_rcu);

void* kcom_maps_get_rcu_next(void* pre_value, const char* key, void* default_val)
{
    if(unlikely(pre_value == NULL || key == NULL))
    {
        return default_val;
    }
    int key_size = strlen(key) + 1;
    KCOM_HASH_NODE* node = container_of((void*)((char*)pre_value - key_size), KCOM_HASH_NODE, data);
    if(unlikely(node->key_size != key_size))
    {
        return default_val;
    }
    hlist_for_each_entry_continue_rcu(node, node)
    {
        if(memcmp(node->data, key, node->key_size) == 0)
        {
            return (node->data + node->key_size);
        }
    }
    return default_val;
}
EXPORT_SYMBOL(kcom_maps_get_rcu_next);

void* kcom_maps_get_copy(KCOM_MAP* map, const char* key, void* buf, int buf_size, void* default_val)
{
    if(unlikely(map == NULL || key == NULL || buf == NULL || buf_size <= 0))
    {
        return default_val;
    }
    KCOM_HASH_NODE* node = NULL;
    rcu_read_lock();
    hlist_for_each_entry_rcu(node, &map->heads[kcom_hash_string(key) % map->head_count], node)
    {
        if(memcmp(node->data, key, node->key_size) == 0)
        {
            memcpy(buf, node->data + node->key_size, min(buf_size, node->data_size));
            break;
        }
    }
    rcu_read_unlock();
    return buf;
}
EXPORT_SYMBOL(kcom_maps_get_copy);

int64_t kcom_maps_get_int64(KCOM_MAP* map, const char* key, int64_t default_val)
{
    if(unlikely(map == NULL || key == NULL))
    {
        return default_val;
    }
    KCOM_HASH_NODE* node = NULL;
    rcu_read_lock();
    hlist_for_each_entry_rcu(node, &map->heads[kcom_hash_string(key) % map->head_count], node)
    {
        if(node->data_size == sizeof(int64_t) && (memcmp(node->data, key, node->key_size) == 0))
        {
            int64_t value;
            memcpy(&value, node->data + node->key_size, sizeof(int64_t));
            rcu_read_unlock();
            return value;
        }
    }
    rcu_read_unlock();
    return default_val;
}
EXPORT_SYMBOL(kcom_maps_get_int64);

bool kcom_maps_exist(KCOM_MAP* map, const char* key)
{
    if(unlikely(map == NULL || key == NULL))
    {
        return NULL;
    }
    KCOM_HASH_NODE* node = NULL;
    rcu_read_lock();
    hlist_for_each_entry_rcu(node, &map->heads[kcom_hash_string(key) % map->head_count], node)
    {
        if(memcmp(node->data, key, node->key_size) == 0)
        {
            rcu_read_unlock();
            return true;
        }
    }
    rcu_read_unlock();
    return false;
}
EXPORT_SYMBOL(kcom_maps_exist);

bool kcom_maps_exist_value(KCOM_MAP* map, const char* key, const void* value, int value_size)
{
    if(unlikely(map == NULL || key == NULL || value == NULL || value_size <= 0))
    {
        return NULL;
    }
    KCOM_HASH_NODE* node = NULL;
    rcu_read_lock();
    hlist_for_each_entry_rcu(node, &map->heads[kcom_hash_string(key) % map->head_count], node)
    {
        if(memcmp(node->data, key, node->key_size) == 0
                && node->data_size == value_size
                && memcmp(node->data + node->key_size, value, value_size) == 0)
        {
            rcu_read_unlock();
            return true;
        }
    }
    rcu_read_unlock();
    return false;
}
EXPORT_SYMBOL(kcom_maps_exist_value);

bool kcom_maps_iterator_next_rcu(KCOM_MAP_ITERATOR* it, const char** key, void** value, int* value_size)
{
    if(unlikely(it == NULL || it->map == NULL || it->head_pos < 0 || key == NULL || value == NULL || value_size == NULL))
    {
        return false;
    }
    for(; it->head_pos < it->map->head_count;)
    {
        if(it->node == NULL)
        {
            hlist_for_each_entry_rcu(it->node, &it->map->heads[it->head_pos], node)
            {
                *key = it->node->data;
                *value = it->node->data + it->node->key_size;
                *value_size = it->node->data_size;
                return true;
            }
            it->head_pos++;
        }
        else
        {
            hlist_for_each_entry_continue_rcu(it->node, node)
            {
                *key = it->node->data;
                *value = it->node->data + it->node->key_size;
                *value_size = it->node->data_size;
                return true;
            }
            it->head_pos++;
        }
    }

    return false;
}
EXPORT_SYMBOL(kcom_maps_iterator_next_rcu);

