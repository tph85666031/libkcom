#ifndef __KCOM_LOG_H__
#define __KCOM_LOG_H__

#ifndef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif

#define KLOG_D(fmt,...)    pr_debug("(%s:%s:%d) - " fmt "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define KLOG_I(fmt,...)    pr_info("(%s:%s:%d) - " fmt "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define KLOG_W(fmt,...)    pr_warn("(%s:%s:%d) - " fmt "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define KLOG_E(fmt,...)    pr_err("(%s:%s:%d) - " fmt "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)
#define KLOG_F(fmt,...)    pr_crit("(%s:%s:%d) - " fmt "\n", __FILENAME__, __func__, __LINE__, ##__VA_ARGS__)


#endif /* __KCOM_LOG_H__ */

