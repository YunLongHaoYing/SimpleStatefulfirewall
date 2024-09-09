/* Hash function (Jenkins).
 * REFE: https://troydhanson.github.io/uthash/userguide.html#hash_functions */
#define HASH_JEN_MIX(a, b, c)                                                  \
    do {                                                                       \
        a -= b;                                                                \
        a -= c;                                                                \
        a ^= (c >> 13);                                                        \
        b -= c;                                                                \
        b -= a;                                                                \
        b ^= (a << 8);                                                         \
        c -= a;                                                                \
        c -= b;                                                                \
        c ^= (b >> 13);                                                        \
        a -= b;                                                                \
        a -= c;                                                                \
        a ^= (c >> 12);                                                        \
        b -= c;                                                                \
        b -= a;                                                                \
        b ^= (a << 16);                                                        \
        c -= a;                                                                \
        c -= b;                                                                \
        c ^= (b >> 5);                                                         \
        a -= b;                                                                \
        a -= c;                                                                \
        a ^= (c >> 3);                                                         \
        b -= c;                                                                \
        b -= a;                                                                \
        b ^= (a << 10);                                                        \
        c -= a;                                                                \
        c -= b;                                                                \
        c ^= (b >> 15);                                                        \
    } while (0)

#define HASH_JEN(key, keylen, hashv)                                           \
    do {                                                                       \
        unsigned _hj_i, _hj_j, _hj_k;                                          \
        unsigned const char *_hj_key = (unsigned const char *)(key);           \
        hashv                        = 0xfeedbeefu;                            \
        _hj_i = _hj_j = 0x9e3779b9u;                                           \
        _hj_k         = (unsigned)(keylen);                                    \
        while (_hj_k >= 12U) {                                                 \
            _hj_i +=                                                           \
                (_hj_key[0] + ((unsigned)_hj_key[1] << 8) +                    \
                 ((unsigned)_hj_key[2] << 16) + ((unsigned)_hj_key[3] << 24)); \
            _hj_j +=                                                           \
                (_hj_key[4] + ((unsigned)_hj_key[5] << 8) +                    \
                 ((unsigned)_hj_key[6] << 16) + ((unsigned)_hj_key[7] << 24)); \
            hashv += (_hj_key[8] + ((unsigned)_hj_key[9] << 8) +               \
                      ((unsigned)_hj_key[10] << 16) +                          \
                      ((unsigned)_hj_key[11] << 24));                          \
                                                                               \
            HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                 \
                                                                               \
            _hj_key += 12;                                                     \
            _hj_k -= 12U;                                                      \
        }                                                                      \
        hashv += (unsigned)(keylen);                                           \
        switch (_hj_k) {                                                       \
        case 11:                                                               \
            hashv += ((unsigned)_hj_key[10] << 24);                            \
            fallthrough;                                                       \
        case 10:                                                               \
            hashv += ((unsigned)_hj_key[9] << 16);                             \
            fallthrough;                                                       \
        case 9:                                                                \
            hashv += ((unsigned)_hj_key[8] << 8);                              \
            fallthrough;                                                       \
        case 8:                                                                \
            _hj_j += ((unsigned)_hj_key[7] << 24);                             \
            fallthrough;                                                       \
        case 7:                                                                \
            _hj_j += ((unsigned)_hj_key[6] << 16);                             \
            fallthrough;                                                       \
        case 6:                                                                \
            _hj_j += ((unsigned)_hj_key[5] << 8);                              \
            fallthrough;                                                       \
        case 5:                                                                \
            _hj_j += _hj_key[4];                                               \
            fallthrough;                                                       \
        case 4:                                                                \
            _hj_i += ((unsigned)_hj_key[3] << 24);                             \
            fallthrough;                                                       \
        case 3:                                                                \
            _hj_i += ((unsigned)_hj_key[2] << 16);                             \
            fallthrough;                                                       \
        case 2:                                                                \
            _hj_i += ((unsigned)_hj_key[1] << 8);                              \
            fallthrough;                                                       \
        case 1:                                                                \
            _hj_i += _hj_key[0];                                               \
            fallthrough;                                                       \
        default:;                                                              \
        }                                                                      \
        HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                     \
    } while (0)

#define FW_HASH_JEN(keyptr, keylen, hashv, bits)                            \
    do {                                                                       \
        HASH_JEN(keyptr, keylen, hashv);                                       \
        hashv >>= (32 - bits);                                                 \
    } while (0)

struct Hashtable {
    rwlock_t lock;
    unsigned int conn_num;
    DECLARE_HASHTABLE(hashtable, FW_HASHTABLE_BITS);
};

/* Data structures and enumerations. */
struct Connection {
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
    __be64 timeout; // ktime_t
    struct hlist_node node;
};

struct Conn_index {
    __be32 saddr;
    __be32 daddr;
    __be16 sport;
    __be16 dport;
    __u8 protocol;
    __u8 padding[15];
};

// 创建哈希表
struct Hashtable *hashtable_create(void)
{
    struct Hashtable *table = kvzalloc(sizeof(*table), GFP_KERNEL);
    if (!table)
        return NULL;

    hash_init(table->hashtable);
    rwlock_init(&table->lock);
    return table;
}

// hash表添加节点
void hashtable_add(struct Hashtable *table,
                         struct Connection *conn, struct Conn_index *idx)
{
    u32 hashv;
    FW_HASH_JEN(idx, sizeof(idx), hashv, HASH_BITS(table->hashtable));

    write_lock(&table->lock);
    table->conn_num++;
    /* Use default hash function, need the sizeof(idx) <= 8. */
    // u64 idx_64 = *((u64 *)(&idx));
    // hash_add(table->hashtable, &conn->node, idx_64);

    /* Use custom hash function (Jenkins), need the sizeof(idx) == 12n. */
    hlist_add_head(&conn->node, &(table->hashtable)[hashv]);
    write_unlock(&table->lock);
}

// hash表删除节点
void hashtable_del(struct Hashtable *table,
                         struct Connection *conn)
{
    write_lock(&table->lock);
    hash_del(&conn->node);
    kfree(conn);
    write_unlock(&table->lock);
}

bool _fw_hashtable_match(struct Hashtable *table,
                            struct Connection *conn,
                            struct Conn_index *idx, bool reverse)
{
    u32 hashv;
    bool res = false;
    struct Connection *cur_conn;

    if (reverse) {
        swap(idx->saddr, idx->daddr);
        swap(idx->sport, idx->dport);
        swap(conn->saddr, conn->daddr);
    }
    FW_HASH_JEN(idx, sizeof(idx), hashv, HASH_BITS(table->hashtable));

    read_lock(&table->lock);
    hlist_for_each_entry(cur_conn, &(table->hashtable)[hashv], node)
    {
        if (conn->protocol == IPPROTO_TCP) {
            if (reverse)
                swap(conn->tcp.sport, conn->tcp.dport);
            res = existing_connection_tcp(cur_conn, conn);
            if (reverse)
                swap(conn->tcp.sport, conn->tcp.dport);
            // XWALL_PR_INFO("reverse: %d, res = %d", reverse, res);
            // XWALL_PR_INFO("old: %pI4 %pI4 %d %d", &cur_conn->saddr,
            // &cur_conn->daddr,
            //         ntohs(cur_conn->tcp.sport), ntohs(cur_conn->tcp.dport));
            // XWALL_PR_INFO("new: %pI4 %pI4 %d %d", &conn->saddr, &conn->daddr,
            //         ntohs(conn->tcp.sport), ntohs(conn->tcp.dport));
        } else if (conn->protocol == IPPROTO_UDP) {
            if (reverse)
                swap(conn->udp.sport, conn->udp.dport);
            // XWALL_PR_INFO("cur:  %pI4,%d => %pI4,%d", &cur_conn->saddr,
            //         cur_conn->udp.sport, &cur_conn->daddr,
            //         cur_conn->udp.dport);
            // XWALL_PR_INFO("conn: %pI4,%d => %pI4,%d", &conn->saddr,
            // conn->udp.sport,
            //         &conn->daddr, conn->udp.dport);
            res = (cur_conn->saddr == conn->saddr &&
                   cur_conn->daddr == conn->daddr &&
                   cur_conn->udp.sport == conn->udp.sport &&
                   cur_conn->udp.dport == conn->udp.dport);
            if (reverse)
                swap(conn->udp.sport, conn->udp.dport);
        } else if (conn->protocol == IPPROTO_ICMP) {
            // XWALL_PR_INFO("cur_conn: %pI4 > %pI4", &cur_conn->saddr,
            //         &cur_conn->daddr);
            // XWALL_PR_INFO("conn    : %pI4 > %pI4", &conn->saddr,
            // &conn->daddr); res = existing_connection_icmp(cur_conn, conn);
            if (reverse) {
                res = cur_conn->saddr == conn->saddr &&
                      cur_conn->daddr == conn->daddr &&
                      cur_conn->icmp.type == ICMP_ECHO &&
                      cur_conn->icmp.code == 0 &&
                      conn->icmp.type == ICMP_ECHOREPLY && conn->icmp.code == 0;
            } else {
                res = cur_conn->saddr == conn->saddr &&
                      cur_conn->daddr == conn->daddr &&
                      cur_conn->icmp.type == ICMP_ECHO &&
                      cur_conn->icmp.code == 0 &&
                      conn->icmp.type == ICMP_ECHO && conn->icmp.code == 0;
            }

        } else {
            res = existing_connection_others(cur_conn, conn);
        }
        if (res) {
            if (ktime_cur_before((ktime_t)ntohll(cur_conn->timeout))) {
                switch (cur_conn->protocol) {
                case IPPROTO_ICMP:
                    cur_conn->timeout = htonll(
                        ktime_add_sec(ktime_get_real(), default_timeout_icmp));
                    break;
                case IPPROTO_UDP:
                    cur_conn->timeout = htonll(
                        ktime_add_sec(ktime_get_real(), default_timeout_udp));
                    break;
                case IPPROTO_TCP:
                    cur_conn->timeout = htonll(
                        ktime_add_sec(ktime_get_real(), default_timeout_tcp));
                    break;
                default:
                    cur_conn->timeout = htonll(ktime_add_sec(
                        ktime_get_real(), default_timeout_others));
                }
                break;
            } else {
                res = false;
            }
        }
    }
    read_unlock(&table->lock);

    if (reverse) {
        swap(idx->saddr, idx->daddr);
        swap(idx->sport, idx->dport);
        swap(conn->saddr, conn->daddr);
    }

    return res;
}

// 连接hash表匹配节点
bool hashtable_match(struct Hashtable *table,
                           struct Connection *conn,
                           struct Conn_index *idx)
{
    bool res = false;

    if (conn->protocol == IPPROTO_TCP) {
        res = _fw_hashtable_match(table, conn, idx, false) ||
              _fw_hashtable_match(table, conn, idx, true);
    } else if (conn->protocol == IPPROTO_UDP) {
        res = _fw_hashtable_match(table, conn, idx, false) ||
              _fw_hashtable_match(table, conn, idx, true);
    } else if (conn->protocol == IPPROTO_ICMP) {
        res = _fw_hashtable_match(table, conn, idx, false) ||
              _fw_hashtable_match(table, conn, idx, true);
    } else {
        res = _fw_hashtable_match(table, conn, idx, true);
    }

    return res;
}

// 删除超时连接
void hashtable_clean(struct Hashtable *table)
{
    int i;
    struct hlist_node *tmp        = NULL;
    struct Connection *conn = NULL;
    struct Connection *conn_set = NULL;
    int num = 0;

    write_lock(&table->lock);
    conn_set = (struct Connection *)kvzalloc(sizeof(struct Connection *)*table->conn_num,GFP_KERNEL);
    for (i = 0; i < HASH_SIZE(table->hashtable); ++i) {
        hlist_for_each_entry_safe(conn, tmp, &(table->hashtable)[i], node)
        {
            if (!ktime_cur_before(ntohll(conn->timeout))) {
                switch (conn->protocol) {
                case IPPROTO_TCP:
                    PR_INFO("Delete connection: [TCP] %pI4,%d > %pI4,%d",
                                  &conn->saddr, ntohs(conn->tcp.sport),
                                  &conn->daddr, ntohs(conn->tcp.dport));
                    break;
                case IPPROTO_UDP:
                    PR_INFO("Delete connection: [UDP] %pI4,%d > %pI4,%d",
                                  &conn->saddr, ntohs(conn->udp.sport),
                                  &conn->daddr, ntohs(conn->udp.dport));
                    break;
                case IPPROTO_ICMP:
                    PR_INFO("Delete connection: [ICMP] %pI4 > %pI4",
                                  &conn->saddr, &conn->daddr);
                    break;
                default:
                    break;
                }
                hash_del(&conn->node);
                kfree(conn);
                table->conn_num--;
            } else {
                if (conn_set)
                {
                    memcpy(conn_set+num,conn,sizeof(*conn));
                }
                num++; 
            }
        }
    }
    write_unlock(&table->lock);
    if (conn_set)
    {
        if (num>0)
        {
            struct Response *resp=response_create(FW_TYPE_CONN, sizeof(struct Connection)*num, (char *)conn_set);
            if (resp)
            {
                netlink_send(nl_sk_log, pid, 0, (char *)resp, sizeof(struct Connection)*num + sizeof(*resp));
                kfree(resp);

            }
        }
        kfree(conn_set);
    }
}

// 清空hash表
void hashtable_clear(struct Hashtable *table)
{
    unsigned int i;
    struct hlist_node *tmp        = NULL;
    struct Connection *conn = NULL;

    write_lock(&table->lock);
    table->conn_num = 0;
    for (i = 0; i < HASH_SIZE(table->hashtable); ++i) {
        hlist_for_each_entry_safe(conn, tmp, &(table->hashtable)[i], node)
        {
            hash_del(&conn->node);
            kfree(conn);
        }
    }
    hash_init(table->hashtable);
    write_unlock(&table->lock);
}

/* CONNECTION TABLE */
// 创建hash表索引
struct Conn_index *index_create(struct iphdr *iph)
{
    struct Conn_index *idx = kvzalloc(sizeof(*idx), GFP_KERNEL);
    if (!idx)
        return NULL;

    idx->saddr    = iph->saddr;
    idx->daddr    = iph->daddr;
    idx->protocol = iph->protocol;
    switch (iph->protocol) {
    case IPPROTO_TCP:
        {struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
        idx->sport          = tcph->source;
        idx->dport          = tcph->dest;
        break;}
    case IPPROTO_UDP:
        {struct udphdr *udph = (void *)iph + iph->ihl * 4;
        idx->sport          = udph->source;
        idx->dport          = udph->dest;
        break;}
    case IPPROTO_ICMP:
    default:
        idx->sport = idx->dport = 0x0;
    }

    return idx;
}

// 创建连接
struct Connection *connection_create(struct iphdr *iph)
{
    struct Connection *conn = kvzalloc(sizeof(*conn), GFP_KERNEL);
    if (!conn)
        return NULL;

    conn->saddr    = iph->saddr;
    conn->daddr    = iph->daddr;
    conn->protocol = iph->protocol;
    switch (iph->protocol) {
    case IPPROTO_TCP:
        {struct tcphdr *tcph = (void *)iph + iph->ihl * 4;
        conn->tcp.sport     = tcph->source;
        conn->tcp.dport     = tcph->dest;
        conn->timeout =
            htonll(ktime_add_sec(ktime_get_real(), default_timeout_tcp));
        break;}
    case IPPROTO_UDP:
        {struct udphdr *udph = (void *)iph + iph->ihl * 4;
        conn->udp.sport     = udph->source;
        conn->udp.dport     = udph->dest;
        conn->timeout =
            htonll(ktime_add_sec(ktime_get_real(), default_timeout_udp));
        break;}
    case IPPROTO_ICMP:
        {struct icmphdr *icmph = (void *)iph + iph->ihl * 4;
        conn->icmp.type       = icmph->type;
        conn->icmp.code       = icmph->code;
        conn->timeout =
            htonll(ktime_add_sec(ktime_get_real(), default_timeout_icmp));
        break;}
    default:
        conn->timeout =
            htonll(ktime_add_sec(ktime_get_real(), default_timeout_others));
    }

    return conn;
}

static struct Hashtable *conn_table = NULL;

/* timer */
static struct timer_list conn_timer;

void fw_timer_callback(struct timer_list *t)
{
    PR_INFO("Clean the connection table...");
    hashtable_clean(conn_table);
out:
    conn_timer.expires = jiffies + FW_CLEAN_CONN_INVERVAL_SEC * HZ;
    add_timer(&conn_timer);
}
/* timer */