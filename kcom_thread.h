#ifndef __KCOM_THREAD_H__
#define __KCOM_THREAD_H__

char* kcom_process_get_path(struct task_struct* task, char* buf, int buf_size);
char* kcom_process_get_name(struct task_struct* task, char* buf, int buf_size);

char* kcom_process_get_path_by_pid(int pid, char* buf, int buf_size);
char* kcom_process_get_name_by_pid(int pid, char* buf, int buf_size);

#endif /* __KCOM_THREAD_H__ */

