#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <net/netlink.h>

#include "kcom_netlink.h"
#include "kcom_log.h"

#define KCOM_NETLINK_GENERIC_CMD_BIN   1
#define KCOM_NETLINK_GENERIC_ATTR_BIN  1

typedef struct
{
    struct genl_family family;
    struct genl_multicast_group group;
    struct genl_ops option[1];
    struct nla_policy policy[2];
} KCOM_NETLINK_GENERIC_HANDLE;

struct sock* kcom_netlink_open(int protocol_type, unsigned int group, void (*cb_on_recv)(struct sk_buff*))
{
    struct netlink_kernel_cfg cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.input = cb_on_recv;
    cfg.groups = group;
    return netlink_kernel_create(&init_net, protocol_type, &cfg);
}
EXPORT_SYMBOL(kcom_netlink_open);

void kcom_netlink_close(struct sock* sk)
{
    if(sk != NULL)
    {
        netlink_kernel_release(sk);
    }
}
EXPORT_SYMBOL(kcom_netlink_close);

int kcom_netlink_send(struct sock* sk, int remote_id, const void* data, int data_size)
{
    struct sk_buff* skb = NULL;
    struct nlmsghdr* msg = NULL;
    if(sk == NULL || data == NULL || data_size <= 0)
    {
        KLOG_E("arg incorrect");
        return -1;
    }

    skb = nlmsg_new(data_size, GFP_NOWAIT);
    if(skb == NULL)
    {
        KLOG_E("nlmsg_new failed");
        return -2;
    }

    msg = nlmsg_put(skb, 0, 0, NLMSG_DONE, data_size, 0);
    if(msg == NULL)
    {
        nlmsg_free(skb);
        KLOG_E("nlmsg_put failed");
        return -3;
    }

    NETLINK_CB(skb).portid = 0;//sender
    NETLINK_CB(skb).dst_group = 0;
    memcpy(nlmsg_data(msg), data, data_size);
    return nlmsg_unicast(sk, skb, remote_id);//nlmsg_unicast will free skb
}
EXPORT_SYMBOL(kcom_netlink_send);

int kcom_netlink_send_broadcast(struct sock* sk, int group, const void* data, int data_size)
{
    struct sk_buff* skb = NULL;
    struct nlmsghdr* msg = NULL;
    if(sk == NULL || data == NULL || data_size <= 0)
    {
        KLOG_E("arg incorrect");
        return -1;
    }

    skb = nlmsg_new(data_size, GFP_NOWAIT);
    if(skb == NULL)
    {
        KLOG_E("nlmsg_new failed");
        return -2;
    }

    msg = nlmsg_put(skb, 0, 0, NLMSG_DONE, data_size, 0);
    if(msg == NULL)
    {
        nlmsg_free(skb);
        KLOG_E("nlmsg_put failed");
        return -3;
    }

    NETLINK_CB(skb).portid = 0;//sender
    NETLINK_CB(skb).dst_group = group;
    memcpy(nlmsg_data(msg), data, data_size);
    return netlink_broadcast(sk, skb, 0, group, GFP_NOWAIT); //netlink_broadcast will free skb
}
EXPORT_SYMBOL(kcom_netlink_send_broadcast);

void* kcom_netlink_generic_open(const char* name, int (*cb_on_recv)(struct sk_buff*, struct genl_info*))
{
    KCOM_NETLINK_GENERIC_HANDLE* handle = (KCOM_NETLINK_GENERIC_HANDLE*)kzalloc(sizeof(KCOM_NETLINK_GENERIC_HANDLE), GFP_NOWAIT);
    if(handle == NULL)
    {
        return NULL;
    }

    handle->policy[KCOM_NETLINK_GENERIC_ATTR_BIN].type = NLA_BINARY;

    handle->option[0].cmd = KCOM_NETLINK_GENERIC_CMD_BIN;
    handle->option[0].doit = cb_on_recv;
    handle->option[0].policy = handle->policy;

    strncpy(handle->family.name, name, sizeof(handle->family.name) - 1);
    handle->family.version = 1;
    handle->family.maxattr = 2;
    handle->family.module = THIS_MODULE;
    handle->family.n_ops = 1;
    handle->family.ops = &handle->option[0];

    int ret = genl_register_family(&handle->family);
    if(ret != 0)
    {
        KLOG_E("failed to init generic netlink,ret=%d", ret);
        kfree(handle);
        return NULL;
    }

    return handle;
}
EXPORT_SYMBOL(kcom_netlink_generic_open);

void kcom_netlink_generic_close(void* handle)
{
    if(handle != NULL)
    {
        genl_unregister_family(&((KCOM_NETLINK_GENERIC_HANDLE*)handle)->family);
        kfree(handle);
    }
}
EXPORT_SYMBOL(kcom_netlink_generic_close);

int kcom_netlink_generic_send(void* handle, int remote_id, const void* data, int data_size)
{
    if(handle == NULL || data == NULL || data_size <= 0 || remote_id <= 0)
    {
        return -1;
    }
    const struct genl_family* family = &((KCOM_NETLINK_GENERIC_HANDLE*)handle)->family;
    struct sk_buff* skb = genlmsg_new(nla_total_size(data_size), GFP_NOWAIT);
    genlmsg_put(skb, 0, 0, family, 0, KCOM_NETLINK_GENERIC_CMD_BIN);
    nla_put(skb, KCOM_NETLINK_GENERIC_ATTR_BIN, data_size, data);

    return genlmsg_unicast(&init_net, skb, remote_id);
}
EXPORT_SYMBOL(kcom_netlink_generic_send);

int kcom_netlink_generic_send_broadcast(void* handle, int group, const void* data, int data_size)
{
    if(handle == NULL || data == NULL || data_size <= 0)
    {
        return -1;
    }
    const struct genl_family* family = &((KCOM_NETLINK_GENERIC_HANDLE*)handle)->family;
    struct sk_buff* skb = genlmsg_new(nla_total_size(data_size), GFP_NOWAIT);
    genlmsg_put(skb, 0, 0, family, 0, KCOM_NETLINK_GENERIC_CMD_BIN);
    nla_put(skb, KCOM_NETLINK_GENERIC_ATTR_BIN, data_size, data);

    return genlmsg_multicast(family, skb, 0, group, GFP_NOWAIT);
}
EXPORT_SYMBOL(kcom_netlink_generic_send_broadcast);

