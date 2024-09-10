#ifndef __KCOM_NETLINK_H__
#define __KCOM_NETLINK_H__

#include <linux/netlink.h>
#include <net/sock.h>
#include "kcom_log.h"

struct sock* kcom_nl_open(int type, unsigned int group, void (*cb_on_recv)(struct sk_buff*))
{
    struct netlink_kernel_cfg cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.input = cb_on_recv;
    cfg.groups = group;
    return netlink_kernel_create(&init_net, type, &cfg);
}

void kcom_nl_close(struct sock* sk)
{
    if(sk != NULL)
    {
        netlink_kernel_release(sk);
    }
}

int kcom_nl_send(struct sock* sk, int remote_id, const unsigned char* data, int data_size)
{
    struct sk_buff* skb = NULL;
    struct nlmsghdr* msg = NULL;
    if(sk == NULL || data == NULL || data_size <= 0)
    {
        KLOG_E("arg incorrect");
        return -1;
    }

    skb = nlmsg_new(data_size, 0);
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

int kcom_nl_send_broadcast(struct sock* sk, int group, const unsigned char* data, int data_size)
{
    struct sk_buff* skb = NULL;
    struct nlmsghdr* msg = NULL;
    if(sk == NULL || data == NULL || data_size <= 0)
    {
        KLOG_E("arg incorrect");
        return -1;
    }

    skb = nlmsg_new(data_size, 0);
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
    return netlink_broadcast(sk, skb, 0, group, GFP_ATOMIC); //netlink_broadcast will free skb
}

#endif /* __KCOM_NETLINK_H__ */

