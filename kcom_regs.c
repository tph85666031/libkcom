#include "kcom_regs.h"

unsigned long kcom_regs_get_param(struct pt_regs* regs, int n)
{
    switch(n)
    {
#if defined(CONFIG_X86_64)
        case 1:
            return regs->di;
        case 2:
            return regs->si;
        case 3:
            return regs->dx;
        case 4:
            return regs->cx;
        case 5:
            return regs->r8;
        case 6:
            return regs->r9;
#elif defined(CONFIG_CPU_LOONGSON3)
        case 1:  // a0
        case 2:  // a1
        case 3:  // a2
        case 4:  // a3
            return *(unsigned long*)((char*)regs + (3 + n) * 8);
#elif defined(CONFIG_SW)
        case 1:
            return regs->r16;
        case 2:
            return regs->r17;
        case 3:
            return regs->r18;
        case 4:
            return regs->r19;
#elif defined(CONFIG_ARM64)
        case 1:
            return regs->regs[0];
        case 2:
            return regs->regs[1];
        case 3:
            return regs->regs[2];
        case 4:
            return regs->regs[3];
        case 5:
            return regs->regs[4];
        case 6:
            return regs->regs[5];
#elif defined(CONFIG_MIPS)
        case 1:
            return regs->regs[4];
        case 2:
            return regs->regs[5];
        case 3:
            return regs->regs[6];
        case 4:
            return regs->regs[7];
        case 5:
            return regs->regs[8];
#elif defined(CONFIG_LOONGARCH)
        case 1:
            return regs->regs[4];
        case 2:
            return regs->regs[5];
        case 3:
            return regs->regs[6];
        case 4:
            return regs->regs[7];
        case 5:
            return regs->regs[8];
        case 6:
            return regs->regs[9];
#endif // CONFIG_X86_64
        default:
            break;
    }
    return 0;
}
EXPORT_SYMBOL(kcom_regs_get_param);

void kcom_regs_set_param(struct pt_regs* regs, int n, unsigned long val)
{
    switch(n)
    {
#if defined(CONFIG_X86_64)
        case 1:
            regs->di = val;
            break;
        case 2:
            regs->si = val;
            break;
        case 3:
            regs->dx = val;
            break;
        case 4:
            regs->cx = val;
            break;
        case 5:
            regs->r8 = val;
            break;
        case 6:
            regs->r9 = val;
            break;
#elif defined(CONFIG_CPU_LOONGSON3)
        case 1:  // a0
        case 2:  // a1
        case 3:  // a2
        case 4:  // a3
            *(unsigned long*)((char*)regs + (3 + n) * 8) = val;
            break;
#elif defined(CONFIG_SW)
        case 1:
            regs->r16 = val;
            break;
        case 2:
            regs->r17 = val;
            break;
        case 3:
            regs->r18 = val;
            break;
        case 4:
            regs->r19 = val;
            break;
#elif defined(CONFIG_ARM64)
        case 1:
            regs->regs[0] = val;
            break;
        case 2:
            regs->regs[1] = val;
            break;
        case 3:
            regs->regs[2] = val;
            break;
        case 4:
            regs->regs[3] = val;
            break;
        case 5:
            regs->regs[4] = val;
            break;
        case 6:
            regs->regs[5] = val;
            break;
#elif defined(CONFIG_MIPS)
        case 1:
            regs->regs[4] = val;
            break;
        case 2:
            regs->regs[5] = val;
            break;
        case 3:
            regs->regs[6] = val;
            break;
        case 4:
            regs->regs[7] = val;
            break;
        case 5:
            regs->regs[8] = val;
            break;
#elif defined(CONFIG_LOONGARCH)
        case 1:
            regs->regs[4] = val;
            break;
        case 2:
            regs->regs[5] = val;
            break;
        case 3:
            regs->regs[6] = val;
            break;
        case 4:
            regs->regs[7] = val;
            break;
        case 5:
            regs->regs[8] = val;
            break;
        case 6:
            regs->regs[9] = val;
            break;
#endif // CONFIG_X86_64
        default:
            break;
    }
}
EXPORT_SYMBOL(kcom_regs_set_param);

unsigned long kcom_regs_get_return_value(struct pt_regs* regs)
{
    return regs_return_value(regs);
}
EXPORT_SYMBOL(kcom_regs_get_return_value);

void kcom_regs_set_return_value(struct pt_regs* regs, unsigned long val)
{
    regs_set_return_value(regs, val);
}
EXPORT_SYMBOL(kcom_regs_set_return_value);

