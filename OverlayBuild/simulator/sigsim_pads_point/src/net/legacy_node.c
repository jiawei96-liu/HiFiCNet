#include "legacy_node.h"
#include <uthash/utlist.h>
#include <log/log.h>

void
legacy_node_init(struct legacy_node *ln, uint16_t type)
{
    node_init(&ln->base, type);
    arp_table_init(&ln->at);
    route_table_init(&ln->rt);
}

void legacy_node_clean(struct legacy_node *ln)
{
    node_destroy_ports(&ln->base);
    arp_table_clean(&ln->at);
    route_table_clean(&ln->rt);
}

void legacy_node_set_intf_ipv4(struct legacy_node *ln, uint32_t port_id,
                               uint32_t addr, uint32_t netmask) {
    struct port *p = node_port(&ln->base, port_id);
    port_add_v4addr(p, addr, netmask);
    /* Add entry to route table */
    struct route_entry_v4 *e = malloc(sizeof(struct route_entry_v4));
    memset(e, 0x0, sizeof(struct route_entry_v4));
    e->ip = addr & netmask;
    e->netmask = netmask;
    e->iface = port_id;
    add_ipv4_entry(&ln->rt, e);
}

/* Returns the gateway of destination IP */
uint32_t 
ip_lookup(struct legacy_node *ln, struct netflow *flow){
    struct route_entry_v4 *re = ipv4_lookup(&ln->rt, flow->match.ipv4_dst);
    if (!re){
        /* Default gateway */
        re = ipv4_lookup(&ln->rt, 0);
    }
    if (re){
        netflow_add_out_port(flow, re->iface);
        /* IP of the next hop to search in the ARP table */
        return re->gateway == 0? flow->match.ipv4_dst: re->gateway; 
    } 
    /* Could not find a route */
    return 0;   
}
            
struct netflow*
resolve_mac(struct legacy_node *ln, struct netflow *flow, uint32_t ip){
    struct arp_table_entry *ae = arp_table_lookup(&ln->at, ip);
    if (ae){
        /* Fill Missing information */
        struct out_port *op;
        LL_FOREACH(flow->out_ports, op) {
            struct port *p = node_port(&ln->base, op->port);
            flow->match.ipv4_src = p->ipv4_addr->addr;
            memcpy(flow->match.eth_src, p->eth_address, ETH_LEN);
        }
        memcpy(flow->match.eth_dst, ae->eth_addr, ETH_LEN);
    }
    else {
        /*  ARP request before the flow*/
        uint8_t bcast_eth_addr[ETH_LEN] = {0xff, 0xff, 0xff, 
                                            0xff, 0xff, 0xff};
        /* Craft request */
        struct netflow *arp_req = netflow_new();
        arp_req->match.eth_type = ETH_TYPE_ARP;
        struct out_port *op;
        LL_FOREACH(flow->out_ports, op) {
            struct port *p = node_port(&ln->base, op->port);
            /* Need to set missing ip address info */
            flow->match.ipv4_src = p->ipv4_addr->addr;
            memcpy(arp_req->match.eth_src, p->eth_address, ETH_LEN);
            memcpy(arp_req->match.arp_sha, p->eth_address, ETH_LEN);
            arp_req->match.arp_spa = p->ipv4_addr->addr;
            netflow_add_out_port(arp_req, op->port);
        }
        /* Set Destination address */
        memcpy(arp_req->match.eth_dst, bcast_eth_addr, ETH_LEN); 
        arp_req->match.arp_tpa = ip;
        arp_req->match.arp_op = ARP_REQUEST;
        arp_req->start_time = flow->start_time;
        arp_req->pkt_cnt = 1;
        arp_req->byte_cnt = 56;
        /* Put the current packet in the queue and flow becomes ARP */ 
        node_flow_push((struct node*) ln, flow);
        flow = arp_req;
    }
    return flow;
}

struct netflow* 
find_forwarding_ports(struct legacy_node *ln, struct netflow *flow)
{
    uint32_t ip = ip_lookup(ln, flow);
    /* Only forward if the next hop ip is found */
    if (ip) {
        return resolve_mac(ln, flow, ip);    
    }
    return NULL;
}

struct netflow* 
l2_recv_netflow(struct legacy_node *ln, struct netflow *flow)
{
    log_info("host l2 rec");
    struct port *p = node_port(&ln->base, flow->match.in_port);
    if (!p) {
        log_debug("Port %d was not found\n", flow->match.in_port);
        return NULL;
    }
    uint8_t *eth_dst = flow->match.eth_dst;
    uint8_t bcast_eth_addr[ETH_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    /* Return 0 if destination is other */
    if (memcmp(p->eth_address, eth_dst, ETH_LEN) && 
        memcmp(bcast_eth_addr, eth_dst, ETH_LEN)){       
        return NULL;
    }
    log_info("uuid : %d %d %d %x",ln->base.uuid,flow->match.ipv4_src,flow->match.ipv4_dst,flow->match.eth_type);
    /* Handle ARP */
    if (flow->match.eth_type == ETH_TYPE_ARP){
        /* ARP request */
        if (flow->match.arp_op == ARP_REQUEST){

            /* Returns if port is not the target */
            if (p->ipv4_addr != NULL && 
                flow->match.arp_tpa != p->ipv4_addr->addr){
                return 0;
            } 
            log_info("Received ARP Request from %x in %x %ld\n",
                      flow->match.arp_spa, flow->match.arp_tpa, 
                      flow->start_time);
            log_debug("Received ARP Request from %x in %x %ld\n",
                      flow->match.arp_spa, flow->match.arp_tpa, 
                      flow->start_time);
            struct arp_table_entry *ae = arp_table_lookup(&ln->at,
                                                          flow->match.arp_spa);
            /* Learns MAC of the requester */
            if (!ae){
                struct arp_table_entry *e = arp_table_entry_new(
                                            flow->match.arp_spa, 
                                            flow->match.eth_src, 
                                            flow->match.in_port);
                arp_table_add_entry(&ln->at, e);
            }
            /* Craft reply */
            uint32_t ip_dst = flow->match.arp_spa;
            memcpy(flow->match.arp_sha, p->eth_address, ETH_LEN);
            memcpy(flow->match.arp_tha, flow->match.eth_src, ETH_LEN);
            memcpy(flow->match.eth_dst, flow->match.eth_src, ETH_LEN);
            memcpy(flow->match.eth_src, p->eth_address, ETH_LEN);
            flow->match.arp_spa = flow->match.arp_tpa;
            flow->match.arp_tpa = ip_dst;
            flow->match.arp_op = ARP_REPLY;
            /* Add outport */
            netflow_add_out_port(flow, flow->match.in_port);
        }
        /* ARP Reply */
        else if (flow->match.arp_op == ARP_REPLY) {
            /* Add ARP table */
            if (p->ipv4_addr != NULL && 
                flow->match.arp_tpa != p->ipv4_addr->addr){
                return 0;
            } 
            uint64_t start_time = flow->start_time;
            log_debug("Received ARP reply %ld", flow->start_time);
            struct arp_table_entry *e = arp_table_entry_new(
                                        flow->match.arp_spa, 
                                        flow->match.arp_sha, 
                                        flow->match.in_port);
            arp_table_add_entry(&ln->at, e);
            /* Cleans ARP flow */
            netflow_destroy(flow);
            flow = NULL;
            /* Check the stack for a flow waiting for the ARP reply */
            if (!node_is_buffer_empty((struct node*) ln)){
                flow = node_flow_pop((struct node*) ln);
                /* Update the start and end time with the previous ones */ 
                flow->start_time = start_time;
                /* Fill l2 information */
                struct out_port *op;
                LL_FOREACH(flow->out_ports, op) {
                    struct port *p = node_port(&ln->base, op->port);
                    memcpy(flow->match.eth_src, p->eth_address, ETH_LEN);
                }
                memcpy(flow->match.eth_dst, e->eth_addr, ETH_LEN);
            }
            else {
                /* There is nothing else to send */
                return NULL;
            }
        }
    }
    return flow;
}

int 
l3_recv_netflow(struct legacy_node *ln, struct netflow *flow)
{
    struct port *p = node_port(&ln->base, flow->match.in_port);
    /* IPv4 Assigned and flow is IPv4 */

    log_info("uuid : %d port : %d",ln->base.uuid,flow->match.in_port);

    if (p->ipv4_addr && flow->match.ipv4_dst){

        return p->ipv4_addr->addr == flow->match.ipv4_dst;
    }
    else if (p->ipv6_addr){
        return !(memcmp(p->ipv6_addr, flow->match.ipv6_dst, IPV6_LEN));
    }
    return 0;
}