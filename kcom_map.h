#ifndef __KCOM_MAP_H__
#define __KCOM_MAP_H__

#include <linux/hashtable.h>
#include <linux/spinlock.h>

typedef struct
{
    spinlock_t lock;
    int head_count;
    struct hlist_head heads[0];
} KCOM_MAP;

uint64_t kcom_hash_string(const char* str);
uint64_t kcom_hash_data(const void* data, int data_size);

/*
create a map
hash_count: 
return: map handle
*/
KCOM_MAP* kcom_map_create(int hash_count);
void kcom_map_clear(KCOM_MAP* map);
void kcom_map_destroy(KCOM_MAP* map);

//key is uint64_t
void kcom_map_remove(KCOM_MAP* map, uint64_t key);
bool kcom_map_add(KCOM_MAP* map, uint64_t key, const void* value, int value_size);
bool kcom_map_add_string(KCOM_MAP* map, uint64_t key, const char* value);
bool kcom_map_add_int64(KCOM_MAP* map, uint64_t key, int64_t value);
void* kcom_map_get_rcu(KCOM_MAP* map, uint64_t key, void* default_val);
void* kcom_map_get_rcu_next(void* pre_value, uint64_t key, void* default_val);
int64_t kcom_map_get_int64(KCOM_MAP* map, uint64_t key, int64_t default_val);
bool kcom_map_exist(KCOM_MAP* map, uint64_t key);

//key is string
void kcom_maps_remove(KCOM_MAP* map, const char* key);
bool kcom_maps_add(KCOM_MAP* map, const char* key, const void* value, int value_size);
bool kcom_maps_add_string(KCOM_MAP* map, const char* key, const char* value);
bool kcom_maps_add_int64(KCOM_MAP* map, const char* key, int64_t value);
void* kcom_maps_get_rcu(KCOM_MAP* map, const char* key, void* default_val);
void* kcom_maps_get_rcu_next(void* pre_value, const char* key, void* default_val);
int64_t kcom_maps_get_int64(KCOM_MAP* map, const char* key, int64_t default_val);
bool kcom_maps_exist(KCOM_MAP* map, const char* key);

#endif /* __KCOM_MAP_H__ */

