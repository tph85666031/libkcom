#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/sched/mm.h>
#include <linux/version.h>
#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/pid.h>

#include "kcom_base.h"
#include "kcom_thread.h"
#include "kcom_log.h"

char* kcom_process_get_path(struct task_struct* task, char* buf, int buf_size)
{
    if(unlikely(task == NULL || buf == NULL || buf_size <= 0))
    {
        return NULL;
    }
#if 1
    return strncpy(buf, task->comm, buf_size);
#else
    struct mm_struct* mm;
    if(unlikely(spin_is_locked(&task->alloc_lock)))
    {
        mm = task->mm;
        if(mm != NULL)
        {
            mmget(mm);
        }
    }
    else
    {
        mm = get_task_mm(task);
    }
    if(mm == NULL)
    {
        return NULL;
    }

    char* file_app = NULL;
#if LINUX_VERSION_CODE<KERNEL_VERSION(5,8,0)
    down_read(&mm->mmap_sem);
#else
    mmap_read_lock(mm);
#endif
    struct file* exe_file = get_mm_exe_file(mm);
    if(!IS_ERR_OR_NULL(exe_file))
    {
        file_app = kcom_path_from_struct_path(&exe_file->f_path, buf, buf_size);
        fput(exe_file);
    }
#if LINUX_VERSION_CODE<KERNEL_VERSION(5,8,0)
    up_read(&mm->mmap_sem);
#else
    mmap_read_unlock(mm);
#endif
    mmput(mm);

    return file_app;
#endif
}
EXPORT_SYMBOL(kcom_process_get_path);

char* kcom_process_get_name(struct task_struct* task, char* buf, int buf_size)
{
    if(unlikely(task == NULL || buf == NULL || buf_size <= 0))
    {
        return NULL;
    }
    if(likely(strlen(task->comm) < sizeof(task->comm) - 2))
    {
        return strncpy(buf, task->comm, buf_size);
    }

    char* file_path = kcom_process_get_path(task, buf, buf_size);
    if(file_path == NULL)
    {
        return strncpy(buf, task->comm, buf_size);
    }
    return (char*)kbasename(file_path);
}
EXPORT_SYMBOL(kcom_process_get_name);

struct task_struct* kcom_find_get_task_by_vpid(pid_t pid)
{
    struct task_struct* task;

    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if(task)
    {
        get_task_struct(task);
    }
    rcu_read_unlock();

    return task;
}
EXPORT_SYMBOL(kcom_find_get_task_by_vpid);

