#ifndef __KCOM_MAP_H__
#define __KCOM_MAP_H__

#include <linux/hashtable.h>
#include <linux/stringhash.h>

uint64_t kcom_hash_string(const char* str);
uint64_t kcom_hash_data(const void* data, int data_size);
struct hlist_head* kcom_map_create(int hash_bits);
void kcom_map_remove(struct hlist_head* map, const char* key);
void* kcom_map_add(struct hlist_head* map, const char* key, const void* value, int value_size);
char* kcom_map_add_string(struct hlist_head* map, const char* key, const char* value);
bool kcom_map_add_int64(struct hlist_head* map, const char* key, int64_t value);
void* kcom_map_get(struct hlist_head* map, const char* key);
int64_t kcom_map_get_int64(struct hlist_head* map, const char* key, int64_t default_val);
bool kcom_map_exist(struct hlist_head* map, const char* key);
void kcom_map_clear(struct hlist_head* map);
void kcom_map_destroy(struct hlist_head* map);

#endif /* __KCOM_MAP_H__ */

