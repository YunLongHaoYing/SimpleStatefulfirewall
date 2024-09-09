#include <linux/ip.h>

struct xwall_nat {
    ktime_t timeout;
    __be32 lan_addr;

    __be16 lan_port;
    bool valid;
};

static struct xwall_nat nat_table[FW_MAX_NAT_ENTRY_NUM];

__be16 fw_nattable_match(__be32 saddr, __be16 sport)
{
    int i = 0;
    for (i = 0; i < FW_MAX_NAT_ENTRY_NUM; i++) {
        if ((nat_table[i].lan_addr == saddr) &&
            (nat_table[i].lan_port == sport) && nat_table[i].valid) {
            if (!ktime_cur_before(nat_table[i].timeout)) {
                PR_INFO("NAT entry with port %d timeout\n", i);
                nat_table[i].valid = false;
                return 0;
            }
            return i;
        }
    }
    return 0;
}

void fw_update_checksum_ip(struct iphdr *iph)
{
    if (!iph)
        return;

    iph->check = 0;
    iph->check = ip_fast_csum((__u8 *)iph, iph->ihl);
}

void fw_update_checksum_tcp(struct sk_buff *skb, struct tcphdr *tcph,
                               struct iphdr *iph)
{
    int tcp_len = 0;
    if (!skb || !iph || !tcph)
        return;
    tcp_len = skb->len - 4 * iph->ihl;

    tcph->check = 0;
    tcph->check =
        csum_tcpudp_magic(iph->saddr, iph->daddr, tcp_len, IPPROTO_TCP,
                          csum_partial(tcph, tcp_len, 0));
}

void fw_update_checksum_udp(struct sk_buff *skb, struct udphdr *udph,
                               struct iphdr *iph)
{
    int udp_len = 0;
    if (!skb || !iph || !udph)
        return;
    udp_len = skb->len - 4 * iph->ihl;

    udph->check = 0;
    udph->check =
        csum_tcpudp_magic(iph->saddr, iph->daddr, udp_len, IPPROTO_UDP,
                          csum_partial(udph, udp_len, 0));
}

void fw_update_checksum_icmp(struct sk_buff *skb, struct icmphdr *icmph,
                                struct iphdr *iph)
{
    int icmp_len = 0;
    if (!skb || !iph || !icmph)
        return;
    icmp_len = skb->len - 4 * iph->ihl;

    icmph->checksum = 0;
    icmph->checksum = csum_fold(csum_partial(icmph, icmp_len, 0));
}

/* NETFILTER */
/* PRE ROUTING hook: Destination network address translation. */
unsigned int fw_nat_in(void *priv, struct sk_buff *skb,
                          const struct nf_hook_state *state)
{
    // return NF_ACCEPT;
    struct iphdr *iph = ip_hdr(skb);

    if (iph->daddr != fw_ip_wan)
        return NF_ACCEPT;

    switch (iph->protocol) {
    case IPPROTO_TCP:
        {struct tcphdr *tcph = (struct tcphdr *)((char *)iph + iph->ihl * 4);
        if (nat_table[tcph->dest].valid) {
            if (!ktime_cur_before(nat_table[tcph->dest].timeout)) {
                nat_table[tcph->dest].valid = false;
                return NF_ACCEPT;
            }
            /* Change dst IP to xwall's WAL IP and src port to WAN NAT port. */
            PR_INFO(
                "DNAT-TCP: (%pI4,%d > %pI4,%d) => (%pI4,%d > %pI4,%d)",
                &iph->saddr, ntohs(tcph->source), &iph->daddr,
                ntohs(tcph->dest), &iph->saddr, ntohs(tcph->source),
                &nat_table[tcph->dest].lan_addr,
                ntohs(nat_table[tcph->dest].lan_port));
            iph->daddr = nat_table[tcph->dest].lan_addr;
            tcph->dest = nat_table[tcph->dest].lan_port;
            /* Update checksum. */
            fw_update_checksum_ip(iph);
            fw_update_checksum_tcp(skb, tcph, iph);
        }
        break;}
    case IPPROTO_UDP:
        {struct udphdr *udph = (struct udphdr *)((char *)iph + iph->ihl * 4);
        if (nat_table[udph->dest].valid) {
            if (!ktime_cur_before(nat_table[udph->dest].timeout)) {
                nat_table[udph->dest].valid = false;
                return NF_ACCEPT;
            }
            /* Change dst IP to xwall's WAL IP and src port to WAN NAT port. */
            PR_INFO(
                "DNAT-UDP: (%pI4,%d > %pI4,%d) => (%pI4,%d > %pI4,%d)",
                &iph->saddr, ntohs(udph->source), &iph->daddr,
                ntohs(udph->dest), &iph->saddr, ntohs(udph->source),
                &nat_table[udph->dest].lan_addr,
                ntohs(nat_table[udph->dest].lan_port));
            iph->daddr = nat_table[udph->dest].lan_addr;
            udph->dest = nat_table[udph->dest].lan_port;
            /* Update checksum. */
            fw_update_checksum_ip(iph);
            fw_update_checksum_udp(skb, udph, iph);
        }
        break;}
    case IPPROTO_ICMP:
        {struct icmphdr *icmph = (struct icmphdr *)((char *)iph + iph->ihl * 4);
        if (nat_table[htons(FW_LAN_PORT_ICMP)].valid) {
            if (!ktime_cur_before(
                    nat_table[htons(FW_LAN_PORT_ICMP)].timeout)) {
                nat_table[htons(FW_LAN_PORT_ICMP)].valid = false;
                return NF_ACCEPT;
            }
            /* Change dst IP to xwall's WAL IP. */
            PR_INFO("DNAT-ICMP: (%pI4 > %pI4) => (%pI4 > %pI4)",
                          &iph->saddr, &iph->daddr, &iph->saddr,
                          &nat_table[htons(FW_LAN_PORT_ICMP)].lan_addr);
            iph->daddr = nat_table[htons(FW_LAN_PORT_ICMP)].lan_addr;
            /* Update checksum. */
            fw_update_checksum_ip(iph);
            fw_update_checksum_icmp(skb, icmph, iph);
        }
        break;}
    default:
        break;
    }
    return NF_ACCEPT;
}

/* POST ROUTING hook: Source network address translation. */
unsigned int fw_nat_out(void *priv, struct sk_buff *skb,
                           const struct nf_hook_state *state)
{
    struct iphdr *iph = ip_hdr(skb);
    __be16 port_wan;

    if ((iph->saddr & ip_lan_mask) != (fw_ip_lan & ip_lan_mask))
        return NF_ACCEPT;

    if ((iph->daddr & ip_wan_mask) != (fw_ip_wan & ip_wan_mask))
        return NF_ACCEPT;

    switch (iph->protocol) {
    case IPPROTO_TCP:
        {struct tcphdr *tcph = (struct tcphdr *)((char *)iph + iph->ihl * 4);
        /* Choose a WAN port. */
        port_wan = fw_nattable_match(iph->saddr, tcph->source);
        if (!port_wan) {
            /* NAT entry doesn't exist, make a new NAT entry. */
            port_wan = htons(nat_port++);
            if (nat_port == 0)
                nat_port = FW_NAT_PORT_START;
            nat_table[port_wan].valid    = true;
            nat_table[port_wan].lan_addr = iph->saddr;
            nat_table[port_wan].lan_port = tcph->source;
            nat_table[port_wan].timeout =
                ktime_add_sec(ktime_get_real(), nat_timeout_sec);
        }
        /* Change src IP to xwall's WAL IP and src port to WAN NAT port. */
        PR_INFO("SNAT-TCP: (%pI4,%d > %pI4,%d) => (%pI4,%d > %pI4,%d)",
                      &iph->saddr, ntohs(tcph->source), &iph->daddr,
                      ntohs(tcph->dest), &fw_ip_wan, ntohs(port_wan),
                      &iph->daddr, ntohs(tcph->dest));
        iph->saddr   = fw_ip_wan;
        tcph->source = port_wan;
        /* Update checksum. */
        fw_update_checksum_ip(iph);
        fw_update_checksum_tcp(skb, tcph, iph);
        break;}
    case IPPROTO_UDP:
        {struct udphdr *udph = (struct udphdr *)((char *)iph + iph->ihl * 4);
        /* Choose a WAN port. */
        port_wan = fw_nattable_match(iph->saddr, udph->source);
        if (!port_wan) {
            /* NAT entry doesn't exist, make a new NAT entry. */
            port_wan = htons(nat_port++);
            if (nat_port == 0)
                nat_port = FW_NAT_PORT_START;
            nat_table[port_wan].valid    = true;
            nat_table[port_wan].lan_addr = iph->saddr;
            nat_table[port_wan].lan_port = udph->source;
            nat_table[port_wan].timeout =
                ktime_add_sec(ktime_get_real(), nat_timeout_sec);
        }
        /* Change src IP to xwall's WAL IP and src port to WAN NAT port. */
        PR_INFO("SNAT-UDP: (%pI4,%d > %pI4,%d) => (%pI4,%d > %pI4,%d)",
                      &iph->saddr, ntohs(udph->source), &iph->daddr,
                      ntohs(udph->dest), &fw_ip_wan, ntohs(port_wan),
                      &iph->daddr, ntohs(udph->dest));
        iph->saddr   = fw_ip_wan;
        udph->source = port_wan;
        /* Update checksum. */
        fw_update_checksum_ip(iph);
        fw_update_checksum_udp(skb, udph, iph);
        break;}
    case IPPROTO_ICMP:
        {struct icmphdr *icmph = (struct icmphdr *)((char *)iph + iph->ihl * 4);
        // /* Choose a WAN port. */
        // port_wan = xwall_nattable_match(iph->saddr, XWALL_LAN_PORT_ICMP);
        // if (!port_wan) {
        //     /* NAT entry doesn't exist, make a new NAT entry. */
        //     port_wan = htons(nat_port++);
        //     if (nat_port == 0)
        //         nat_port = XWALL_NAT_PORT_START;
        //     nat_table[port_wan].valid    = true;
        //     nat_table[port_wan].lan_addr = iph->saddr;
        //     nat_table[port_wan].lan_port = XWALL_LAN_PORT_ICMP;
        //     nat_table[port_wan].timeout =
        //         ktime_add_sec(ktime_get_real(), nat_timeout_sec);
        // }
        port_wan                     = htons(FW_LAN_PORT_ICMP);
        nat_table[port_wan].valid    = true;
        nat_table[port_wan].lan_addr = iph->saddr;
        nat_table[port_wan].lan_port = FW_LAN_PORT_ICMP;
        nat_table[port_wan].timeout =
            ktime_add_sec(ktime_get_real(), nat_timeout_sec);
        /* Change src IP to xwall's WAL IP. */
        PR_INFO("SNAT-ICMP: (%pI4 > %pI4) => (%pI4 > %pI4)", &iph->saddr,
                      &iph->daddr, &fw_ip_wan, &iph->daddr);
        iph->saddr = fw_ip_wan;
        /* Update checksum. */
        fw_update_checksum_ip(iph);
        fw_update_checksum_icmp(skb, icmph, iph);
        break;}
    default:
        /* TODO. */
        break;
    }

    return NF_ACCEPT;
}