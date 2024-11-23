#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#include <linux/version.h>
#include <linux/atomic.h>
#include <linux/fs.h>
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

    struct mm_struct* mm = get_task_mm(task);
    if(unlikely(mm == NULL))
    {
        return NULL;
    }

    char* file_app = NULL;
#if LINUX_VERSION_CODE<KERNEL_VERSION(5,8,0)
    down_read(&mm->mmap_sem);
#else
    mmap_read_lock(mm);
#endif
    if(likely(mm->exe_file))
    {
        file_app = kcom_path_from_struct_path(&mm->exe_file->f_path, buf, buf_size);
    }
#if LINUX_VERSION_CODE<KERNEL_VERSION(5,8,0)
    up_read(&mm->mmap_sem);
#else
    mmap_read_unlock(mm);
#endif
    mmput(mm);

    return file_app;
}
EXPORT_SYMBOL(kcom_process_get_path);

char* kcom_process_get_path_by_pid(int pid, char* buf, int buf_size)
{
    if(unlikely(buf == NULL || buf_size <= 0))
    {
        return NULL;
    }
    struct task_struct* task = NULL;
    if(pid <= 0)
    {
        task = current;
        get_task_struct(task);
    }
    else
    {
        rcu_read_lock();
        task = pid_task(find_vpid(pid), PIDTYPE_PID);
        if(unlikely(task == NULL))
        {
            rcu_read_unlock();
            return NULL;
        }
        get_task_struct(task);
        rcu_read_unlock();
    }

    char* file_path = kcom_process_get_path(task, buf, buf_size);
    put_task_struct(task);
    return file_path;
}
EXPORT_SYMBOL(kcom_process_get_path_by_pid);

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
    char* file_name = file_path;
    while(*file_path != '\0')
    {
        if(*file_path == '/')
        {
            file_name = file_path + 1;
        }
        file_path++;
    }
    return file_name;
}
EXPORT_SYMBOL(kcom_process_get_name);

char* kcom_process_get_name_by_pid(int pid, char* buf, int buf_size)
{
    if(unlikely(buf == NULL || buf_size <= 0))
    {
        return NULL;
    }
    
    struct task_struct* task = NULL;
    if(pid <= 0)
    {
        task = current;
        get_task_struct(task);
    }
    else
    {
        rcu_read_lock();
        task = pid_task(find_vpid(pid), PIDTYPE_PID);
        if(unlikely(task == NULL))
        {
            rcu_read_unlock();
            return NULL;
        }
        get_task_struct(task);
        rcu_read_unlock();
    }

    char* file_name = kcom_process_get_name(task, buf, buf_size);
    put_task_struct(task);
    return file_name;
}
EXPORT_SYMBOL(kcom_process_get_name_by_pid);

