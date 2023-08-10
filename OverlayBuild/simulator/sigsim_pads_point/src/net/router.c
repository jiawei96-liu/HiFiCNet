#include "router.h"
#include <log/log.h>


#define MAX_TABLE_LENGTH 108
#define NONNEXTHOP 0x0000
#define MAX_TABLE_LENGTH_FLOW 128
#define NONMACADDRESS 0x0000
#define PATCHPORT 0x01
#define VXLANPORT 0x02
//#define IDLENGTH 108

struct router
{
	struct node base;
	/* Specific fields may follow */
    struct forward_table * ft;
    uint64_t rt_id; 
    struct port_alloc * pa;
    struct arq_ip_mac_table * at;
    //struct flow_table *tables[MAX_TABLE_LENGTH_FLOW];
    //char device_id[IDLENGTH];
    //uint32_t ipv4_address;
    uint8_t port_attri[MAX_TABLE_LENGTH]; // id , attri
};

struct port_alloc
{
    bool port_id_alloc[MAX_TABLE_LENGTH];
    uint32_t size;
};

struct port_mac
{
    uint32_t port_id;
    uint8_t mac_addr[ETH_LEN];
};

//default route: ft[0]
struct forward_table
{
    struct port_mac ft[MAX_TABLE_LENGTH];
    uint32_t top;  // init top = 0, insert: ++top then insert ft[top]
};

struct arq_ip_mac
{
    uint32_t ipv4_address;
    uint8_t mac_addr[ETH_LEN];
};


struct arq_ip_mac_table
{
    struct arq_ip_mac arp_table[MAX_TABLE_LENGTH];
    uint32_t top; //insert: ++top then insert arp_table[top]
};

void
ser_port_attri(struct node *n,uint32_t port_id,uint8_t attri)
{
    struct router *r = (struct router *)n;
    r->port_attri[port_id] = attri;
}

void 
insert_arq_ip_mac_table(struct node *n,uint32_t ipv4_address,uint8_t mac_addr[ETH_LEN])
{
    struct router *r = (struct router *)n;
    r->at->top++;
    uint32_t top = r->at->top;
    r->at->arp_table[top].ipv4_address = ipv4_address;
    memcpy(r->at->arp_table[top].mac_addr,mac_addr,ETH_LEN);
}
uint32_t 
find_arp_table_index(struct router * r,uint32_t ipv4_address)
{
    uint32_t index = NONMACADDRESS;

    uint32_t top = r->at->top;
    for(uint32_t i = 1; i<=top; ++i)
    {
        if(r->at->arp_table[i].ipv4_address == ipv4_address)
        {
            index = i;
            break;
        }
    }
    return index;
}

struct router *
router_new(void)
{
    struct router *r = xmalloc(sizeof(struct router));
    node_init(&r->base, ROUTER);
    r->ft = NULL;
    r->ft = xmalloc(sizeof(struct forward_table));
    memset(r->ft,0,sizeof(struct forward_table));

    r->pa = NULL;
    r->pa = xmalloc(sizeof(struct port_alloc));
    memset(r->pa,0,sizeof(struct port_alloc));
    r->base.recv_netflow = router_recv_netflow;
    r->base.send_netflow = router_send_netflow;
    r->rt_id = -1;

    r->at = NULL;
    r->at  = xmalloc(sizeof(struct arq_ip_mac_table));
    memset(r->at,0,sizeof(struct arq_ip_mac_table));
    //memset(r->device_id,0,sizeof(r->device_id));
    return r;
}

void
set_recv_netflow(struct node *n,uint32_t lev)
{
    struct router *r = (struct router *)n;
    if(lev == 3)
    {
        r->base.recv_netflow = router_recv_netflow_openflow;
    }
    else
    {
        r->base.recv_netflow = router_recv_netflow;
    }
}

void 
router_destroy(struct router *r)
{
    
    node_destroy_ports(&r->base);

    free(r->ft);
    free(r->pa);
    free(r->at);
    free(r);
}

//defualt port_id 1 2 3 4....
void 
router_add_port(struct router *r, uint32_t port_id, uint8_t eth_addr[ETH_LEN],
                uint32_t speed, uint32_t curr_speed)
{   r->pa->size++;
    r->pa->port_id_alloc[port_id] = true;
    node_add_port( (struct node*) r, port_id, eth_addr, speed, curr_speed);
}

void 
add_flag_to_port(struct node *n,uint32_t port_id,bool flag)
{
    struct router *r = (struct router *)n;
    r->pa->port_id_alloc[port_id] = flag;
}

uint32_t 
find_alloc_port(struct node *n)
{
    struct router *r = (struct router *)n;
    
    for(uint32_t i = 1; i<=r->pa->size;++i)
    {
        if(r->pa->port_id_alloc[i] == true)
            return i;
    }

    uint32_t port_id = r->pa->size + 1;
    uint8_t mac[6] = {0,0,0,0,0,0};

log_info("new pord need");

    struct port* p_last = router_port(r,r->pa->size);
    memcpy(mac,p_last->eth_address,6);

    mac[6]++;
log_info("copy mac  from %d",r->pa->size);
    router_add_port(r,port_id,mac,1000,1000);
    log_info("add port new ok");
    return port_id;
}

/*
void
router_set_intf_ipv4(struct router *r, uint32_t port_id, 
                   uint32_t addr, uint32_t netmask){
    legacy_node_set_intf_ipv4(&r->rt, port_id, addr, netmask);   
}
*/

struct netflow* 
router_recv_netflow(struct node *n, struct netflow *flow)
{
     // When a router receives an IP packet it 
     //    performs a lookup in the routing table 
   // UNUSED(n);
    //UNUSED(flow);
    if(check_vlan_tag(flow) == false)
    {
        return NULL;
    }


    //printf("router rec\n");
    //printf("rcv flow->byte_cnt %d\n",flow->byte_cnt);

    struct router *r = (struct router *)n;
    
    update__forward_table(r->ft,flow->match.in_port,flow->match.eth_src);
 
    uint32_t out_port_id = NON_PORT;
    out_port_id = find_forward_table(r->ft,flow->match.eth_dst);
    /*
    for(int i = 0; i<6; ++i)
    {
        uint8_t tt = flow->match.eth_src[i];
        log_info("%x%x" ,tt/16,tt%16);
    }
    for(int i = 0; i<6; ++i)
    {
        uint8_t tt = flow->match.eth_dst[i];
        log_info("%x%x" ,tt/16,tt%16);
    }
    */
    //log_info("src mac %x dst mac %x",flow->match.eth_src,flow->match.eth_dst);

    if(out_port_id == NON_PORT)
    {
        log_info("route send to all\n");
        //flow->match.dst_to_all = 1;
        // forward all port except in_port
        struct port *p, *tmp_port;
        HASH_ITER(hh, r->base.ports, p, tmp_port) 
        {
            if (p->port_id != flow->match.in_port) 
            {
                netflow_add_out_port(flow, p->port_id);
                log_info("add port to send %d\n",p->port_id);
            }
        }
        
        return flow;
    }
    else
    {
        log_info("route send to one %d",out_port_id);
        //flow->match.dst_to_all = 0;
        netflow_add_out_port(flow, out_port_id);
        return flow;
    }
    return NULL;
}

struct netflow* 
router_recv_netflow_openflow(struct node *n, struct netflow *flow)
{
    struct router *r = (struct router *)n;
    uint32_t in_port = flow->match.in_port;
    if(r->port_attri[in_port] == PATCHPORT)
    {
        log_info("PATCHPORT");
        if(flow->match.eth_type == ETH_TYPE_ARP)
        {
            log_info("ETH_TYPE_ARP");
            if(flow->match.arp_op == ARP_REQUEST)
            {
                log_info("ARP_REQUEST");
                uint32_t index = find_arp_table_index(r,flow->match.arp_tpa);
                if(index == NONMACADDRESS)
                    return NULL;

                log_info("create ARP_REPLY");
                uint32_t ip_dst = flow->match.arp_spa;
                memcpy(flow->match.arp_sha, r->at->arp_table[index].mac_addr, ETH_LEN);
                memcpy(flow->match.arp_tha, flow->match.eth_src, ETH_LEN);
                memcpy(flow->match.eth_dst, flow->match.eth_src, ETH_LEN);
                memcpy(flow->match.eth_src, r->at->arp_table[index].mac_addr, ETH_LEN);
                flow->match.arp_spa = flow->match.arp_tpa;
                flow->match.arp_tpa = ip_dst;
                flow->match.arp_op = ARP_REPLY;
                /* Add outport */
                log_info("send reply to %d",flow->match.in_port);
                netflow_add_out_port(flow, flow->match.in_port);

                return flow;
            }
        }
        else
        {
            struct port *p, *tmp_port;
            HASH_ITER(hh, r->base.ports, p, tmp_port) 
            {
                if (p->port_id != flow->match.in_port && r->port_attri[p->port_id] == VXLANPORT) 
                {
                    netflow_add_out_port(flow, p->port_id);
                    log_info("add port to send %d\n",p->port_id);
                }
            }

            return flow;
        }
    }
    else if(r->port_attri[in_port] == VXLANPORT)
    {
        struct port *p, *tmp_port;
        HASH_ITER(hh, r->base.ports, p, tmp_port) 
        {
            if (p->port_id != flow->match.in_port && r->port_attri[p->port_id] == PATCHPORT) 
            {
                netflow_add_out_port(flow, p->port_id);
                log_info("add port to send %d\n",p->port_id);
            }
        }
        return flow;
    }
    else
    {

    }
    return NULL;
}

void
update__forward_table(struct forward_table * ft,uint32_t port_id, uint8_t *mac_addr) //ipv4
{
    // mac == 0xffffffffffff? return
    log_info("check mac_port");
     uint8_t eth_addr[ETH_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
     if(memcmp(eth_addr, mac_addr, ETH_LEN) == 0)
        return;
log_info("start update mac_port");
    uint32_t *top = &ft->top;
    for(uint32_t i = 1; i <= (*top); ++i)
    {
        if(memcmp(ft->ft[i].mac_addr, mac_addr, ETH_LEN) == 0)
        {
            ft->ft[i].port_id = port_id;
            log_info("%d to %d", mac_addr[5],port_id);
            return;
        }
    }//update ip -- port

    //insert ip--port
    if((*top) < MAX_TABLE_LENGTH - 1)
    {
        (*top) ++;
        log_info("insert a mac_port");
        ft->ft[(*top)].port_id = port_id;
        memcpy(ft->ft[(*top)].mac_addr,mac_addr,ETH_LEN);
        log_info("%d to %d", mac_addr[5],port_id);
    }

}

uint32_t
find_forward_table(struct forward_table * ft, uint8_t *mac_addr)//ipv4
{   


    uint8_t eth_addr[ETH_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
     if(memcmp(eth_addr, mac_addr, ETH_LEN) == 0)
        return NON_PORT;



    for(uint32_t i = 1; i <= ft->top; ++i)
    {
        
        if(memcmp(mac_addr,ft->ft[i].mac_addr,ETH_LEN) == 0)
        {
            
            //for(int i = 0; i<ETH_LEN; ++i)
                //log_info("%x%x",mac_addr[i]/16,mac_addr[i]%16);
            log_info("forward %d %dto %d",mac_addr[5],ft->ft[i].mac_addr[5],ft->ft[i].port_id);
            return ft->ft[i].port_id;
        }
    }
    return NON_PORT;
}

void 
router_send_netflow(struct node *n, struct netflow *flow,
                         uint32_t out_port)
{
    node_update_port_stats(n, flow, out_port);
}

void 
router_set_name(struct router* r, char *name)
{
    memcpy(r->base.name, name, MAX_NODE_NAME);
}

/* Access functions*/
char *
router_name(struct router *r)
{
    return r->base.name;
}

/* Retrieve a datapath port */
struct port* 
router_port(const struct router *r, uint32_t port_id)
{
    struct port *p = node_port( (struct node*) r, port_id);
    return p;
}


uint64_t 
router_uuid(const struct router* r)
{
    return r->base.uuid;
}
