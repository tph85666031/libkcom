#ifndef __KCOM_THREAD_H__
#define __KCOM_THREAD_H__

#include <linux/sched/task.h>

char* kcom_process_get_path(struct task_struct* task, char* buf, int buf_size);
char* kcom_process_get_name(struct task_struct* task, char* buf, int buf_size);

struct task_struct* kcom_find_get_task_by_vpid(pid_t pid);

#endif /* __KCOM_THREAD_H__ */

