#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include "nlink.h"
#include "filter.h"
#include "nat.h"



static const struct nf_hook_ops nf_xwall_ops[] = {
    // {
    //     .hook     = fw_filter,
    //     .pf       = PF_INET,
    //     .hooknum  = NF_INET_PRE_ROUTING,
    //     .priority = NF_IP_PRI_LAST,
    // },
    {
        .hook     = fw_filter,
        .pf       = PF_INET,
        .hooknum  = NF_INET_FORWARD,
        .priority = NF_IP_PRI_FIRST,
    },
    // {
    //     .hook     = fw_filter,
    //     .pf       = PF_INET,
    //     .hooknum  = NF_INET_POST_ROUTING,
    //     .priority = NF_IP_PRI_FIRST,
    // },
    {
        .hook     = fw_nat_in,
        .pf       = PF_INET,
        .hooknum  = NF_INET_PRE_ROUTING,
        .priority = NF_IP_PRI_NAT_DST,
    },
    {
        .hook     = fw_nat_out,
        .pf       = PF_INET,
        .hooknum  = NF_INET_POST_ROUTING,
        .priority = NF_IP_PRI_NAT_SRC,
    },
};


static int __init firewall_init(void)
{
    int err;
    struct netlink_kernel_cfg nl_cfg = {
        .input = netlink_recv,
    };

    nl_sk_cmd = netlink_kernel_create(&init_net, NETLINK_CMD, &nl_cfg);
    if (!nl_sk_cmd) {
        PR_INFO("nl_sk_cmd init failed.\n");
        err = -ENOMEM;
        goto err_out;
    }

    nl_sk_log = netlink_kernel_create(&init_net, NETLINK_LOG, NULL);
    if (!nl_sk_log) {
        PR_INFO("nl_sk_log init failed.\n");
        err = -ENOMEM;
        goto err_release_sock_cmd;
    }

    conn_table = hashtable_create();
    if (!conn_table) {
        pr_err("Connection hashtable create failed.\n");
        err = -ENOMEM;
        goto err_release_sock_log;
    }

    rule_table = ruletable_create();
    if (!rule_table) {
        pr_err("Rule table create failed.\n");
        err = -ENOMEM;
        goto err_free_hashtable;
    }

    err = nf_register_net_hooks(&init_net, nf_xwall_ops,
                                ARRAY_SIZE(nf_xwall_ops));
    if (err != 0) {
        pr_err("Netfilter register net hooks failed.\n");
        goto err_free_ruletable;
    }

    PR_INFO("Firewall module init.\n");
    /* 
     * A non 0 return means init_module failed; module can't be loaded. 
     */
    // timer_setup(&conn_timer, fw_timer_callback, 0);
    // conn_timer.expires = jiffies + FW_CLEAN_CONN_INVERVAL_SEC * HZ;
    // add_timer(&conn_timer);

    return 0;

err_free_ruletable:
    ruletable_clear(rule_table);
    kfree(rule_table);
err_free_hashtable:
    hashtable_clear(conn_table);
    kfree(conn_table);
err_release_sock_log:
    sock_release(nl_sk_log->sk_socket);
err_release_sock_cmd:
    sock_release(nl_sk_cmd->sk_socket);
err_out:
    return err;

}

static void __exit firewall_exit(void)
{
    nf_unregister_net_hooks(&init_net, nf_xwall_ops, ARRAY_SIZE(nf_xwall_ops));
    ruletable_clear(rule_table);
    kfree(rule_table);
    hashtable_clear(conn_table);
    kfree(conn_table);
    sock_release(nl_sk_log->sk_socket);
    sock_release(nl_sk_cmd->sk_socket);
    del_timer(&conn_timer);
    PR_INFO("Firewall module exited.\n");
}


module_init(firewall_init);
module_exit(firewall_exit);

MODULE_LICENSE("GPL");