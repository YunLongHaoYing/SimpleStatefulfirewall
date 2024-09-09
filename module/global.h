#include <linux/tcp.h>

#define pr_fmt(fmt) "%s[%25s]: " fmt, KBUILD_MODNAME, __func__
#ifdef BENCHMARK
#define PR_INFO(...)
#else
#define PR_INFO(...) pr_info(__VA_ARGS__)
#endif

#ifdef __BIG_ENDIAN__
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) (((__u64)htonl((x)&0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) (((__u64)ntohl((x)&0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

#define IS_SYN(tcp)             ((tcp_flag_word(tcp) & TCP_FLAG_SYN) != 0)
#define IS_ECHO_REQUEST(icmp)   (icmp->type == ICMP_ECHO && icmp->code == 0)
#define ktime_cur_before(kt)    (ktime_before(ktime_get_real(), (kt)))
#define ktime_add_sec(kt, sval) (ktime_add_ns((kt), (sval)*NSEC_PER_SEC))

#define FW_HASHTABLE_BITS          (10)
#define FW_CLEAN_CONN_INVERVAL_SEC (10)

#define ip_to_be32(a, b, c, d) (a + (b << 8) + (c << 16) + (d << 24))

#ifdef BENCHMARK
static bool default_logging = false;
#else
static bool default_logging = true;
#endif
static unsigned int default_action        = NF_ACCEPT;
static int default_timeout_tcp            = 300;
static int default_timeout_udp            = 180;
static int default_timeout_icmp           = 180;
static int default_timeout_others         = 180;

/* Match package and connection. */
#define existing_connection_tcp(old_conn, new_conn)                            \
    (old_conn->saddr == new_conn->saddr &&                                     \
     old_conn->daddr == new_conn->daddr &&                                     \
     old_conn->tcp.sport == new_conn->tcp.sport &&                             \
     old_conn->tcp.dport == new_conn->tcp.dport)

#define existing_connection_others(old_conn, new_conn)                         \
    ((old_conn->saddr == new_conn->saddr &&                                    \
      old_conn->daddr == new_conn->daddr) ||                                   \
     (old_conn->saddr == new_conn->daddr &&                                    \
      old_conn->daddr == new_conn->saddr))

/* UTILES */
// 展示数据包
void show_packet(struct iphdr *iph)
{
    switch (iph->protocol) {
    case IPPROTO_TCP:
        {struct tcphdr *tcph = (struct tcphdr *)((char *)iph + iph->ihl * 4);
        PR_INFO("TCP: %pI4,%d > %pI4,%d\n", &iph->saddr,
                      ntohs(tcph->source), &iph->daddr, ntohs(tcph->dest));
        break;}
    case IPPROTO_UDP:
        {struct udphdr *udph = (struct udphdr *)((char *)iph + iph->ihl * 4);
        PR_INFO("UDP: %pI4,%d > %pI4,%d\n", &iph->saddr,
                      ntohs(udph->source), &iph->daddr, ntohs(udph->dest));
        break;}
    case IPPROTO_ICMP:
        {struct icmphdr *icmph = (struct icmphdr *)((char *)iph + iph->ihl * 4);
        PR_INFO("ICMP: %pI4 > %pI4, type=%d, code=%d\n", &iph->saddr,
                      &iph->daddr, icmph->type, icmph->code);
        break;}
    default:
        PR_INFO("Unknown protocol: %d", iph->protocol);
    }
}

/* connection */
#define FW_CONNECTION_FILE "/tmp/conn"
#define FW_RULE_FILE_PRIV 0644
static struct file *conn_fp = NULL;
/* connection */


/* netlink */
#define NETLINK_CMD 31
#define NETLINK_LOG 17
#define LOG_LEN 256

static struct sock *nl_sk_cmd = NULL;
static struct sock *nl_sk_log = NULL;

static int pid;

// netlink响应类型
enum RESPONSE_TYPE {
    FW_TYPE_OK,
    FW_TYPE_ERROR,
    FW_TYPE_RULE,
    FW_TYPE_LOG,
    FW_TYPE_MLOG,
    FW_TYPE_CONN,
    FW_TYPE_NAT
};

// netlink响应
struct Response {
    __u8 type;
    __be32 len;
    __u8 msg[0];
};

int netlink_send(struct sock *nl_sk, int pid, int seq, char *data, int len);
struct Response *response_create(enum RESPONSE_TYPE type, int len, char *data);
/* netlink */

/* nat */
#define FW_LAN_PORT_ICMP           (11803)
#define FW_NAT_PORT_START          (11803 + 1)
#define FW_MAX_NAT_ENTRY_NUM       (65536)

static __be16 nat_port     = FW_NAT_PORT_START;
static int nat_timeout_sec = 60;
static __be32 ip_lan_mask  = ip_to_be32(255, 255, 255, 0);
static __be32 ip_wan_mask  = ip_to_be32(255, 255, 255, 0);
static __be32 fw_ip_lan = ip_to_be32(192, 168, 171, 0);
static __be32 fw_ip_wan = ip_to_be32(10, 0, 12, 2);
/* nat */