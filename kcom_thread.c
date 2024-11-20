#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/sched/task.h>
#include <linux/version.h>
#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/pid.h>

#include "kcom_thread.h"

char* kcom_process_get_path(int pid, char* buf, int buf_size)
{
    if(unlikely(buf == NULL || buf_size <= 0))
    {
        return NULL;
    }
    memset(buf, 0, buf_size);
    char* file_app = buf;
    struct task_struct* task = NULL;
    if(likely(pid <= 0))
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
            return file_app;
        }
        get_task_struct(task);
        rcu_read_unlock();
    }
    struct mm_struct* mm = get_task_mm(task);
    if(likely(mm != NULL))
    {
#if LINUX_VERSION_CODE<KERNEL_VERSION(5,8,0)
        down_read(&mm->mmap_sem);
#else
        mmap_read_lock(mm);
#endif
        if(likely(mm->exe_file))
        {
            file_app = d_path(&mm->exe_file->f_path, buf, buf_size);
        }
#if LINUX_VERSION_CODE<KERNEL_VERSION(5,8,0)
        up_read(&mm->mmap_sem);
#else
        mmap_read_unlock(mm);
#endif
    }

    put_task_struct(task);
    return file_app;
}
EXPORT_SYMBOL(kcom_process_get_path);

char* kcom_process_get_name(int pid, char* buf, int buf_size)
{
    buf = kcom_process_get_path(pid, buf, buf_size);
    char* file_name = buf;
    int i;
    for(i = 0; i < buf_size; i++)
    {
        if(buf[i] == '\0')
        {
            break;
        }
        if(buf[i] == '/')
        {
            file_name = buf + i + 1;
        }
    }
    return file_name;
}
EXPORT_SYMBOL(kcom_process_get_name);

