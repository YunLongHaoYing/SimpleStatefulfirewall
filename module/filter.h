unsigned int fw_filter(void *priv, struct sk_buff *skb,
                          const struct nf_hook_state *state)
{
    // TODO: catch the TCP state accurately.

    /* Step 1: init connection information. */
    bool new_conn                 = false;
    unsigned int res              = default_action;
    struct iphdr *iph             = ip_hdr(skb);
    struct Conn_index *idx       = index_create(iph);
    struct Connection *conn = connection_create(iph);
    struct Rule *rule       = NULL;
    struct Package_log *log         = NULL;
    show_packet(iph);

    /* Step 2: find current connection in connection table. */
    if (hashtable_match(conn_table, conn, idx)) {
        PR_INFO("    Connection existed, accept.\n");
        res = NF_ACCEPT;
        if (default_logging) {
            log = package_log_create(skb, NF_ACCEPT);
            // 发送log
            if (log)
            {
                struct Response *resp = response_create(FW_TYPE_LOG, sizeof(*log), (char *)log);
                if (resp)
                {
                    netlink_send(nl_sk_log, pid, 0, (char *)resp, sizeof(*resp) + sizeof(*log));
                    kfree(resp);
                }
                kfree(log);
            }
        }
        goto out;
    }

    /* Step 3: if TCP, must be SYN to continue to match rules. */
    if (iph->protocol == IPPROTO_TCP && !IS_SYN((void *)iph + iph->ihl * 4)) {
        PR_INFO("    No such TCP connection and not SYN, drop.");
        res = NF_DROP;
        if (default_logging) {
            log = package_log_create(skb, NF_DROP);
            // 发送log
            if (log)
            {
                struct Response *resp = response_create(FW_TYPE_LOG, sizeof(*log), (char *)log);
                if (resp)
                {
                    netlink_send(nl_sk_log, pid, 0, (char *)resp, sizeof(*resp) + sizeof(*log));
                    kfree(resp);
                }
                kfree(log);
            }
        }
        goto out;
    }

    /* Step 4: if ICMP, must be Echo request to continue to match rules. */
    if (iph->protocol == IPPROTO_ICMP) {
        struct icmphdr *icmph = (struct icmphdr *)((char *)iph + iph->ihl * 4);
        if (!IS_ECHO_REQUEST(icmph)) {
            PR_INFO(
                "    No such ICMP connection and not ECHO REQUEST, drop.");
            res = NF_DROP;
            if (default_logging) {
                log = package_log_create(skb, NF_DROP);
                // 发送log
                if (log)
                {
                    struct Response *resp = response_create(FW_TYPE_LOG, sizeof(*log), (char *)log);
                    if (resp)
                    {
                        netlink_send(nl_sk_log, pid, 0, (char *)resp, sizeof(*resp) + sizeof(*log));
                        kfree(resp);
                    }
                    kfree(log);
                }
            }
            goto out;
        }
    }

    /* Step 5: find current connection in rule table. */
    rule = ruletable_match(rule_table, conn);
    if (rule) {
        res = ntohl(rule->action);
        if (rule->logging) {
            log = package_log_create(skb, ntohl(rule->action));
            // 发送log
            if (log)
            {
                struct Response *resp = response_create(FW_TYPE_LOG, sizeof(*log), (char *)log);
                if (resp)
                {
                    netlink_send(nl_sk_log, pid, 0, (char *)resp, sizeof(*resp) + sizeof(*log));
                    kfree(resp);
                }
                kfree(log);
            }
        }
        PR_INFO("    Match rule, use rule action.\n");
        if (res == NF_ACCEPT) {
            new_conn = true;
            hashtable_add(conn_table, conn, idx);
            PR_INFO("    Rule action is accept, add new connection.\n");
        }
    } else {
        res = default_action;
        if (default_logging) {
            log = package_log_create(skb, default_action);
            // 发送log
            if (log)
            {
                struct Response *resp = response_create(FW_TYPE_LOG, sizeof(*log), (char *)log);
                if (resp)
                {
                    netlink_send(nl_sk_log, pid, 0, (char *)resp, sizeof(*resp) + sizeof(*log));
                    kfree(resp);
                }
                kfree(log);
            }
        }
        PR_INFO("    No matching rule, use default action.\n");
        if (res == NF_ACCEPT) {
            new_conn = true;
            hashtable_add(conn_table, conn, idx);
            PR_INFO(
                "    Default action is accept, add new connection.\n");
        }
    }

out:
    if (!new_conn)
        kfree(conn);
    kfree(idx);
    return res;
}