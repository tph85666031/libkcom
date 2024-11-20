#ifndef __KCOM_REGS_H__
#define __KCOM_REGS_H__

#include <linux/ptrace.h>

unsigned long kcom_regs_get_param(struct pt_regs* regs, int n);
void kcom_regs_set_param(struct pt_regs* regs, int n, unsigned long val);
unsigned long kcom_regs_get_return_value(struct pt_regs* regs);
void kcom_regs_set_return_value(struct pt_regs* regs, unsigned long val);

#endif /* __KCOM_REGS_H__ */

