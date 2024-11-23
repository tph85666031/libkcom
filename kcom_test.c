#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>

#include "kcom_map.h"
#include "kcom_log.h"

#ifdef __UNIT_TEST__
static KCOM_MAP* map_1 = NULL;
static KCOM_MAP* map_2 = NULL;
static struct task_struct* threads_1[10];
static struct task_struct* threads_2[10];

static int thread_test_1(void* ctx)
{
    KCOM_MAP* map = (KCOM_MAP*)ctx;
    int i;
    char buf[16];
    if(strcmp(get_current()->comm, "thread 9") == 0)
    {
        for(i = 0; i < 100000 && !kthread_should_stop(); i += 3)
        {
            snprintf(buf, sizeof(buf), "key%d", i);
            kcom_maps_remove(map, buf);
            //KLOG_I("removed,key=%s", buf);
        }
        KLOG_I("[1]add done,%s", get_current()->comm);
        return 0;
    }

    for(i = 0; i < 100000 && !kthread_should_stop(); i++)
    {
        snprintf(buf, sizeof(buf), "key%d", i);
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
    if(strcmp(get_current()->comm, "thread 9") == 0)
    {
        for(i = 0; i < 100000 && !kthread_should_stop(); i += 3)
        {
            kcom_map_remove(map, i);
            //KLOG_I("removed,key=%s", buf);
        }
        KLOG_I("[2]add done,%s", get_current()->comm);
        return 0;
    }

    for(i = 0; i < 100000 && !kthread_should_stop(); i++)
    {
        if(kcom_map_get_int64(map, i, -1) != i)
        {
            KLOG_E("read failed,key=%d", i);
        }
    }
    KLOG_I("[2]read done,%s", get_current()->comm);
    return 0;
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
    for(i = 0; i < 100000; i++)
    {
        snprintf(buf, sizeof(buf), "key%zu", i);
        kcom_maps_add_int64(map_1, buf, i);
        kcom_map_add_int64(map_2, i, i);
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
}

module_init(kcom_unit_test_init);
module_exit(kcom_unit_test_exit);
#endif

MODULE_LICENSE("GPL");
MODULE_VERSION("1.1.0");
MODULE_DESCRIPTION("kcom api");

