#include <linux/netlink.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/ip.h>
#include <linux/fs.h>
#include <net/sock.h>
#include "global.h"
#include "hash.h"
#include "rule.h"

// 过滤的数据包
struct Package_log {
    __be32 idx; // unsigned int
    __be64 ts;  // ktime_t
    __be32 saddr;
    __be32 daddr;
    __u8 protocol;
    union {
        struct {
            __u8 type;
            __u8 code;
        } icmp;
        struct {
            __be16 sport;
            __be16 dport;
            __u8 state;
        } tcp;
        struct {
            __be16 sport;
            __be16 dport;
        } udp;
    };
    __be16 len;    // unsigned short
    __be32 action; // unsigned int
    struct list_head node;
};


// netlink请求操作码
enum REQUEST_OP {
    FW_INIT,
    FW_ADDRULE,
    FW_DELRULE,
    FW_READRULE,
    FW_DEFACT,
    FW_READDEFACT,
    FW_ADDNAT,
    FW_DELNAT,
    FW_READNAT
};



// netlink请求
struct Request {
    __u8 opcode;
    union {
        struct Rule rule_add;
        __be32 rule_del_idx; // unsigned int
        struct {
            __be32 start_idx; // unsigned int
            __be32 end_idx;   // unsigned int
        } read_nat, read_mlog;
        __be32 def_act; // unsigned int
    } msg;
};

struct Response *response_create(enum RESPONSE_TYPE type,
                                    int len, char *data)
{
    struct Response *resp = kvzalloc(sizeof(*resp) + len, GFP_KERNEL);
    if (!resp)
        return NULL;

    resp->type = type;
    resp->len  = htonl(len);
    memcpy(resp->msg, data, len);

    return resp;
}

/* LOG */
struct Package_log *package_log_create(struct sk_buff *skb, unsigned int action)
{
    struct iphdr *iph     = ip_hdr(skb);
    struct Package_log *log = kvzalloc(sizeof(*log), GFP_KERNEL);
    if (!log)
        return NULL;

    log->ts       = htonll(ktime_get_real());
    log->saddr    = iph->saddr;
    log->daddr    = iph->daddr;
    log->protocol = iph->protocol;
    log->len      = iph->tot_len;
    log->action   = htonl(action);
    switch (iph->protocol) {
    case IPPROTO_TCP:
    {
        struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
        log->tcp.sport      = tcph->source;
        log->tcp.dport      = tcph->dest;
        break;
    }
    case IPPROTO_UDP:
    {
        struct udphdr *udph = (void *)iph + iph->ihl * 4;
        log->udp.sport      = udph->source;
        log->udp.dport      = udph->dest;
        break;
    }
    case IPPROTO_ICMP:
    {
        struct icmphdr *icmph = (void *)iph + iph->ihl * 4;
        log->icmp.type        = icmph->type;
        log->icmp.code        = icmph->code;
        break;
    }
    default:
        break;
    }

    return log;
}

int netlink_send(struct sock *nl_sk, int pid, int seq, char *data, int len)
{
    int ret;
    struct sk_buff *skb;
    struct nlmsghdr *nlh;

    skb = nlmsg_new(len, GFP_ATOMIC);
    if (skb == NULL) {
        pr_err("Alloc reply nlmsg skb failed!\n");
        ret = -ENOMEM;
        goto rt;
    }

    nlh = nlmsg_put(skb, 0, seq, 0, NLMSG_SPACE(len) - NLMSG_HDRLEN, 0);
    if (!nlh) {
        pr_err("Put msg to skb failed!\n");
        ret = -EPERM;
        goto free;
    } 

    memcpy(NLMSG_DATA(nlh), data, len);

    // NETLINK_CB(skb).pid = 0;
    // NETLINK_CB(skb).dst_group = 0;

    ret = netlink_unicast(nl_sk, skb, pid, MSG_DONTWAIT);
    PR_INFO("Data send to user %d.\n", pid);
free:
    //nlmsg_free(skb);
rt:
    return ret;
}

int msg_hdr(int pid, int seq, char *data, int len)
{
    /* TODO: if operation failed, reply the wrong message. */
    PR_INFO("Data from user %s.\n", data);

    int ret = 0, data_len = 0;
    char *buff_heap = NULL, *buff = NULL;
    struct Response *resp = NULL;
    struct Request *req   = (struct Request *)data;

    netlink_send(nl_sk_cmd, pid, seq, "hello world      ", 18);
    netlink_send(nl_sk_log, pid, 0, "log", 6);
    
    /* Do things. */
    switch (req->opcode) {
    case FW_INIT:
    {
        timer_setup(&conn_timer, fw_timer_callback, 0);
        conn_timer.expires = jiffies + FW_CLEAN_CONN_INVERVAL_SEC * HZ;
        add_timer(&conn_timer);
        break;
    }
    case FW_ADDRULE:
    {
        struct Rule *rule = kvzalloc(sizeof(*rule), GFP_KERNEL);
        memcpy(rule, &req->msg.rule_add, sizeof(*rule));
        rule->timeout = htonll(ktime_add_sec(ktime_get_real(), 3600 * 24));
        ruletable_add(rule_table, rule);
        buff     = "Add rule success!";
        data_len = strlen(buff);
        resp     = response_create(FW_TYPE_OK, data_len, buff);
        if (!resp) {
            pr_err("[ADDRULE] Response create failed.\n");
            ret = -ENOMEM;
            goto out;
        }
        ret = netlink_send(nl_sk_cmd, pid, seq, (char *)resp, sizeof(*resp) + data_len);
        break;
    }
    case FW_DELRULE:
    {
        bool del_ret = ruletable_del_by_idx(rule_table,
                                                  ntohl(req->msg.rule_del_idx));
        // hashtable_clear(conn_table);
        buff     = del_ret ? "Delete rule success!" : "No such rule!";
        data_len = strlen(buff);
        resp = response_create(del_ret ? FW_TYPE_OK : FW_TYPE_ERROR,
                                     data_len, buff);
        if (!resp) {
            pr_err("[DELRULE] Response create failed.\n");
            ret = -ENOMEM;
            goto out;
        }
        ret = netlink_send(nl_sk_cmd, pid, seq, (char *)resp, sizeof(*resp) + data_len);
        break;
    }
    // case READRULE:
    //     {buff_heap = xwall_ruletable_read(
    //         rule_table, ntohl(req->msg.read_rule.start_idx),
    //         ntohl(req->msg.read_rule.end_idx), &data_len);
    //     if (!buff_heap) {
    //         pr_err("[READRULE] Log buffer create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     resp = response_create(XWALL_TYPE_RULE, data_len, buff_heap);
    //     if (!resp) {
    //         pr_err("[READRULE] Response create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     ret = xwall_netlink_send(pid, (char *)resp, sizeof(*resp) + data_len);
    //     break;}
    // case XWALL_OP_SAVERULE:
    //     {ret      = xwall_save_rule(rule_table);
    //     buff     = ret ? "Save rules to " XWALL_RULE_FILE " success!"
    //                    : "Save rules to " XWALL_RULE_FILE " fail!";
    //     data_len = strlen(buff);
    //     resp     = response_create(ret ? XWALL_TYPE_OK : XWALL_TYPE_ERROR,
    //                                  data_len, buff);
    //     if (!resp) {
    //         pr_err("[SAVERULE] Response create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     ret = xwall_netlink_send(pid, (char *)resp, sizeof(*resp) + data_len);
    //     break;}
    // case XWALL_OP_READLOG:
    //     {buff_heap =
    //         xwall_logtable_read(log_table, ntohl(req->msg.read_log.start_idx),
    //                             ntohl(req->msg.read_log.end_idx), &data_len);
    //     if (!buff_heap) {
    //         pr_err("[READLOG] Log buffer create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     resp = response_create(XWALL_TYPE_LOG, data_len, buff_heap);
    //     if (!resp) {
    //         pr_err("[READLOG] Response create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     ret = xwall_netlink_send(pid, (char *)resp, sizeof(*resp) + data_len);
    //     break;}
    // case XWALL_OP_CLRLOG:
    //     {xwall_logtable_clear(log_table);
    //     buff     = "Clear log table success!";
    //     data_len = strlen(buff);
    //     resp     = response_create(XWALL_TYPE_OK, data_len, buff);
    //     if (!resp) {
    //         pr_err("[CLRLOG] Response create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     ret = xwall_netlink_send(pid, (char *)resp, sizeof(*resp) + data_len);
    //     break;}
    // case XWALL_OP_READMLOG:
    //     {buff_heap = xwall_mlogtable_read(
    //         mlog_table, ntohl(req->msg.read_mlog.start_idx),
    //         ntohl(req->msg.read_mlog.end_idx), &data_len);
    //     if (!buff_heap) {
    //         pr_err("[READNLOG] Manage log buffer create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     resp = response_create(XWALL_TYPE_MLOG, data_len, buff_heap);
    //     if (!resp) {
    //         pr_err("[READMLOG] Response create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     ret = xwall_netlink_send(pid, (char *)resp, sizeof(*resp) + data_len);
    //     break;}
    // case XWALL_OP_CLRMLOG:
    //     {xwall_mlogtable_clear(mlog_table);
    //     buff     = "Clear manage log table success!";
    //     data_len = strlen(buff);
    //     resp     = response_create(XWALL_TYPE_OK, data_len, buff);
    //     if (!resp) {
    //         pr_err("[CLRMLOG] Response create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     ret = xwall_netlink_send(pid, (char *)resp, sizeof(*resp) + data_len);
    //     break;}
    // case XWALL_OP_READCONN:
    //     {buff_heap = xwall_hashtable_read(conn_table, &data_len);
    //     if (!buff_heap) {
    //         pr_err("[READCONN] Log buffer create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     resp = response_create(XWALL_TYPE_CONN, data_len, buff_heap);
    //     if (!resp) {
    //         pr_err("[READCONN] Response create failed.\n");
    //         ret = -ENOMEM;
    //         goto out;
    //     }
    //     ret = xwall_netlink_send(pid, (char *)resp, sizeof(*resp) + data_len);
    //     break;}
    case FW_DEFACT:
    {
        default_action = (unsigned int)ntohl(req->msg.def_act);
        // if (default_action == NF_DROP)
        //     hashtable_clear(conn_table);
        buff     = "Change default action success!";
        data_len = strlen(buff);
        resp     = response_create(FW_TYPE_OK, data_len, buff);
        if (!resp) {
            pr_err("[DEFACT] Response create failed.\n");
            ret = -ENOMEM;
            goto out;
        }
        ret = netlink_send(nl_sk_cmd, pid, seq, (char *)resp, sizeof(*resp) + data_len);
        break;
    }
    case FW_READDEFACT:
    {
        __be32 tmp = htonl(default_action);
        buff       = (char *)&tmp;
        data_len   = sizeof(default_action);
        resp       = response_create(FW_TYPE_OK, data_len, buff);
        if (!resp) {
            pr_err("[READDEFACT] Response create failed.\n");
            ret = -ENOMEM;
            goto out;
        }
        ret = netlink_send(nl_sk_cmd, pid, seq, (char *)resp, sizeof(*resp) + data_len);
        break;
    }
    default:
        {buff     = "Invalid opcode!";
        data_len = strlen(buff);
        resp     = response_create(FW_TYPE_ERROR, data_len, buff);
        if (!resp) {
            pr_err("[DEFAULT] Response create failed.\n");
            ret = -ENOMEM;
            goto out;
        }
        ret = netlink_send(nl_sk_cmd, pid, seq, (char *)resp, sizeof(*resp) + data_len);
        break;}
    }

out:
    if (buff_heap)
        kfree(buff_heap);
    if (resp)
        kfree(resp);
    return ret;
}

void netlink_recv(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    char *data;
    int len, seq;

    nlh = nlmsg_hdr(skb);
    if ((nlh->nlmsg_len < NLMSG_HDRLEN) || (skb->len < nlh->nlmsg_len)) {
        pr_err("Illegal netlink packet!\n");
        return;
    }

    data = (char *)NLMSG_DATA(nlh);
    pid  = nlh->nlmsg_pid;
    len  = nlh->nlmsg_len - NLMSG_SPACE(0);
    seq  = nlh->nlmsg_seq;
    PR_INFO("Data recv from user %d.\n", pid);
    msg_hdr(pid, seq, data, len);
}