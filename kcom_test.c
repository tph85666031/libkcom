#include <linux/slab.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>

#include "kcom_map.h"
#include "kcom_log.h"
#include "kcom_netlink.h"

#define __UNIT_TEST__

#ifdef __UNIT_TEST__
static KCOM_MAP* map_1 = NULL;
static KCOM_MAP* map_2 = NULL;
static struct task_struct* threads_1[10];
static struct task_struct* threads_2[10];
static int test_count = 100000;

static int thread_test_1(void* ctx)
{
    KCOM_MAP* map = (KCOM_MAP*)ctx;
    size_t i;
    char buf[16];
    for(i = 0; i < test_count && !kthread_should_stop(); i++)
    {
        snprintf(buf, sizeof(buf), "key%zu", i);
        if(kcom_maps_get_int64(map, buf, -1) == -1)
        {
            KLOG_E("read failed,key=%s", buf);
        }
    }
    KLOG_I("[1]read done,%s", get_current()->comm);
    return 0;
}

static int thread_test_2(void* ctx)
{
    KCOM_MAP* map = (KCOM_MAP*)ctx;
    int i;
    for(i = 0; i < test_count && !kthread_should_stop(); i++)
    {
        if(kcom_map_get_int64(map, i, -1) != i)
        {
            KLOG_E("read failed,key=%d", i);
        }
    }
    KLOG_I("[2]read done,%s", get_current()->comm);
    return 0;
}

static void* netlink_handle = NULL;

static void netlink_test_onrecv(int sender_id, const unsigned char* data, int data_size)
{
    KLOG_I("got message from netlink %d,data=%d:%s", sender_id, data_size, (char*)data);
    kcom_netlink_generic_send(netlink_handle, sender_id, "test back", sizeof("test back"));
}

static int __init kcom_unit_test_init(void)
{
    KLOG_I("called");
    memset(threads_1, 0, sizeof(threads_1));
    memset(threads_2, 0, sizeof(threads_2));
    map_1 = kcom_map_create(1024);
    map_2 = kcom_map_create(1024);
    char buf[16];
    size_t i;
    for(i = 0; i < test_count; i++)
    {
        snprintf(buf, sizeof(buf), "key%zu", i);
        //KLOG_I("add %s to map_1",buf);
        if(kcom_maps_add_int64(map_1, buf, i) == false)
        {
            KLOG_E("failed to add %zu to map_1", i);
        }
        if(kcom_map_add_int64(map_2, i, i) == false)
        {
            KLOG_E("failed to add %zu to map_2", i);
        }
    }
    for(i = 0; i < sizeof(threads_1) / sizeof(threads_1[0]); i++)
    {
        threads_1[i] = kthread_create(thread_test_1, map_1, "thread %zu", i);
        if(IS_ERR_OR_NULL(threads_1[i]))
        {
            KLOG_E("failed to run thread_1 t%zu", i);
            continue;
        }
        get_task_struct(threads_1[i]);
        wake_up_process(threads_1[i]);
    }
    for(i = 0; i < sizeof(threads_2) / sizeof(threads_2[0]); i++)
    {
        threads_2[i] = kthread_create(thread_test_2, map_2, "thread %zu", i);
        if(IS_ERR_OR_NULL(threads_2[i]))
        {
            KLOG_E("failed to run thread_2 t%zu", i);
            continue;
        }
        get_task_struct(threads_2[i]);
        wake_up_process(threads_2[i]);
    }

    KLOG_I("init done");
    netlink_handle = kcom_netlink_generic_open("generic_test", netlink_test_onrecv);
    KLOG_I("netlink init done,netlink_handle=%p", netlink_handle);
    return 0;
}

static void __exit kcom_unit_test_exit(void)
{
    KLOG_I("called");
    size_t i;
    for(i = 0; i < sizeof(threads_1) / sizeof(threads_1[0]); i++)
    {
        if(!IS_ERR_OR_NULL(threads_1[i]))
        {
            kthread_stop(threads_1[i]);
            KLOG_I("thread_1 quit:%s", threads_1[i]->comm);
            put_task_struct(threads_1[i]);
        }
    }
    for(i = 0; i < sizeof(threads_2) / sizeof(threads_2[0]); i++)
    {
        if(!IS_ERR_OR_NULL(threads_2[i]))
        {
            kthread_stop(threads_2[i]);
            KLOG_I("thread_2 quit:%s", threads_2[i]->comm);
            put_task_struct(threads_2[i]);
        }
    }
    kcom_map_destroy(map_1);
    kcom_map_destroy(map_2);
    kcom_netlink_generic_close(netlink_handle);
}

module_init(kcom_unit_test_init);
module_exit(kcom_unit_test_exit);
#endif

MODULE_LICENSE("GPL");
MODULE_VERSION("1.1.0");
MODULE_DESCRIPTION("kcom api");

