#ifndef ROUTER_H
#define ROUTER_H 1

#include "node.h"
#include "flow_table.h"
#include "lib/util.h"
#include "lib/openflow.h"
#include "lib/of_unpack.h"
#include "lib/of_pack.h"
#include "dp_actions.h"
#include "datapath.h"
#include "arp_table.h"

#define NON_PORT 0x00ffffff


typedef enum action_router
{
    DROP = 0,
    RESUBMIT = 1,
    LEARN = 2, // modify or add
};

struct router; 
struct port_ip;
struct forward_table;
struct arq_ip_mac;
struct arq_ip_mac_table;
struct router *router_new(void);

struct netflow* router_recv_netflow_openflow(struct node *n, struct netflow *flow);
void set_recv_netflow(struct node *n,uint32_t lev);
void ser_port_attri(struct node *n,uint32_t port_id,uint8_t attri);
uint32_t find_arp_table_index(struct router * r,uint32_t ipv4_address);
void insert_arq_ip_mac_table(struct node *n,uint32_t ipv4_address,uint8_t mac_addr[ETH_LEN]);
void ser_port_attri(struct node *n,uint32_t port_id,uint8_t attri);
uint32_t find_forward_table(struct forward_table * ft, uint8_t *mac_addr);
void update__forward_table(struct forward_table * ft,uint32_t port_id, uint8_t *mac_addr);
void router_destroy(struct router *h);
void router_add_port(struct router *h, uint32_t port_id, 
                 uint8_t eth_addr[ETH_LEN], uint32_t speed, 
                 uint32_t curr_speed);
void router_add_port(struct router *r, uint32_t port_id,
                     uint8_t eth_addr[ETH_LEN], uint32_t speed, 
                     uint32_t curr_speed);
//void router_set_intf_ipv4(struct router *r, uint32_t port_id,
                          //uint32_t addr, uint32_t netmask);
struct netflow* router_recv_netflow(struct node *n, struct netflow *flow);

void add_flag_to_port(struct node *n,uint32_t port_id,bool flag);
uint32_t find_alloc_port(struct node *n);
void router_send_netflow(struct node *n, struct netflow *flow,
                         uint32_t out_port);
void router_set_name(struct router* r, char *name);
char *router_name(struct router *r);
struct port* router_port(const struct router *r, uint32_t port_id);
uint64_t router_uuid(const struct router* r);
#endif