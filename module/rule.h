//规则结构体
struct Rule {
    __be32 idx; // unsigned int
    __be32 saddr;
    __be32 daddr;
    __be32 smask;
    __be32 dmask;
    __be16 sport_min;
    __be16 sport_max;
    __be16 dport_min;
    __be16 dport_max;
    __u8 protocol;
    __be32 action; // unsigned int
    __u8 logging;
    __be64 timeout; // ktime_t
    struct list_head node;
};

struct Ruletable {
    rwlock_t lock;
    unsigned int rule_num;
    struct list_head node;
};

/* RULE TABLE */
// 创建规则表
struct Ruletable *ruletable_create(void)
{
    struct Ruletable *rule_table =
        kvzalloc(sizeof(*rule_table), GFP_KERNEL);
    if (!rule_table)
        return NULL;

    INIT_LIST_HEAD(&rule_table->node);
    rwlock_init(&rule_table->lock);
    return rule_table;
}

void ruletable_add(struct Ruletable *table, struct Rule *rule)
{
    write_lock(&table->lock);
    // rule->idx = htonl(table->rule_num);
    list_add_tail(&rule->node, &table->node);
    table->rule_num++;
    write_unlock(&table->lock);
}

void ruletable_del(struct Ruletable *table, struct Rule *rule)
{
    write_lock(&table->lock);
    list_del(&rule->node);
    kfree(rule);
    // table->rule_num--;
    write_unlock(&table->lock);
}

bool ruletable_del_by_idx(struct Ruletable *table, unsigned int idx)
{
    bool ret                = false;
    struct Rule *rule = NULL;

    write_lock(&table->lock);
    list_for_each_entry(rule, &(table->node), node)
    {
        if ((unsigned int)ntohl(rule->idx) == idx) {
            list_del(&rule->node);
            kfree(rule);
            ret = true;
            break;
        }
    }
    write_unlock(&table->lock);
    return ret;
}

struct Rule *ruletable_match(struct Ruletable *table,
                                         struct Connection *conn)
{
    // TODO: how to use the mask.
    bool flag               = false;
    struct Rule *rule = NULL;

    read_lock(&table->lock);
    list_for_each_entry(rule, &(table->node), node)
    {
        if (!(conn->protocol == rule->protocol &&
              (conn->saddr & rule->smask) == (rule->saddr & rule->smask) &&
              (conn->daddr & rule->dmask) == (rule->daddr & rule->dmask) &&
              ktime_cur_before((ktime_t)ntohll(rule->timeout)))) {
            continue;
        }
        switch (conn->protocol) {
        case IPPROTO_TCP:
            flag = conn->tcp.sport >= rule->sport_min &&
                   conn->tcp.sport <= rule->sport_max &&
                   conn->tcp.dport >= rule->dport_min &&
                   conn->tcp.dport <= rule->dport_max;
            break;
        case IPPROTO_UDP:
            flag = conn->udp.sport >= rule->sport_min &&
                   conn->udp.sport <= rule->sport_max &&
                   conn->udp.dport >= rule->dport_min &&
                   conn->udp.dport <= rule->dport_max;
            break;
        case IPPROTO_ICMP:
            flag = true;
            break;
        default:
            flag = true;
        }
        if (flag)
            break;
    }
    read_unlock(&table->lock);

    return flag ? rule : NULL;
}

void ruletable_clear(struct Ruletable *table)
{
    struct Rule *rule = NULL, *tmp = NULL;

    write_lock(&table->lock);
    list_for_each_entry_safe(rule, tmp, &(table->node), node)
    {
        list_del(&rule->node);
        kfree(rule);
    }
    table->rule_num = 0;
    INIT_LIST_HEAD(&table->node);
    write_unlock(&table->lock);
}

static struct Ruletable *rule_table = NULL;

