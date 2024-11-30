#include <linux/netlink.h>
#include "kcom_netlink.h"
#include "kcom_log.h"

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

