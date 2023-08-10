/* Copyright (c) 2016-2017, Eder Leao Fernandes
 * All rights reserved.
 *
 * The contents of this file are subject to the license defined in
 * file 'doc/LICENSE'.
 *
 *
 * Author: Eder Leao Fernandes <e.leao@qmul.ac.uk>
 */


#include "topology.h"
#include "lib/json_topology.h"
#include <log/log.h>
/* 
*   Pair of node uuid and port. 
*   Key of the hash table of links.
*/

struct node_port_pair {
    uint64_t uuid;
    uint32_t port;
};

/* TODO: Possible split link on its own interface */
struct link {
    struct node_port_pair node1;
    struct node_port_pair node2;
    uint32_t latency;               /* Latency in microseconds */
    uint32_t bandwidth;
    UT_hash_handle hh;  
};

/* Represents the network topology */
struct topology {
    struct node *nodes;             /* Hash table of network nodes. */
    struct link *links;             /* Hash table of links */
    struct dp_node *dps;            /* Access datapath nodes by dpid */
    uint32_t degree[MAX_DPS];       /* number of links connected to dps. */ 
    uint32_t n_dps;                 /* Number of datapaths. */
    uint32_t n_routers;             /* Number of routers. */
    uint32_t n_hosts;               /* Number of hosts. */
    uint32_t n_links;               /* Number of links. */
};

static void
topology_init(struct topology* topo)
{
    topo->nodes = NULL;
    topo->dps = NULL;
    topo->n_dps = 0;
    topo->n_links = 0;
    topo->links = NULL;
}

struct topology* topology_new(void)
{
    struct topology *topo = xmalloc(sizeof(struct topology));
    topology_init(topo);
    return topo;
}

struct topology *
topo_test(struct topology *topo)
{
    uint32_t mask = 0xffffff00;
    log_info("okokok topo");
    struct host * h1 = host_new();
    uint8_t mac_h1[ETH_LEN] = {0,0,0,0,0,1};
    host_add_port(h1,1,mac_h1,1000,1000);
    host_set_intf_ipv4(h1,1,1,mask);

    struct host * h2 = host_new();
    uint8_t mac_h2[ETH_LEN] = {0,0,0,0,0,2};
    host_add_port(h2,1,mac_h2,1000,1000);
    host_set_intf_ipv4(h2,1,2,mask);

    struct host * h3 = host_new();
    uint8_t mac_h3[ETH_LEN] = {0,0,0,0,0,3};
    host_add_port(h3,1,mac_h3,1000,1000);
    host_set_intf_ipv4(h3,1,3,mask);

    struct host * h4 = host_new();
    uint8_t mac_h4[ETH_LEN] = {0,0,0,0,0,4};
    host_add_port(h4,1,mac_h4,1000,1000);
    host_set_intf_ipv4(h4,1,4,mask);

    host_add_app(h1,PINGV4);
    host_add_app(h2,PINGV4);
    host_add_app(h3,PINGV4);
    host_add_app(h4,PINGV4);

    struct router * rr1 = router_new();
    uint8_t mac_rr_base[ETH_LEN] = {0,0,0,1,0,1};
    router_add_port(rr1,1,mac_rr_base,1000,1000);
    mac_rr_base[5]++;
    router_add_port(rr1,2,mac_rr_base,1000,1000);

    struct router * rh1 = router_new();
    mac_rr_base[5]++;
    router_add_port(rh1,1,mac_rr_base,1000,1000);
    mac_rr_base[5]++;
    router_add_port(rh1,2,mac_rr_base,1000,1000);
    mac_rr_base[5]++;
    router_add_port(rh1,3,mac_rr_base,1000,1000);

    struct router * rr2 = router_new();
    mac_rr_base[5]++;
    router_add_port(rr2,1,mac_rr_base,1000,1000);
    mac_rr_base[5]++;
    router_add_port(rr2,2,mac_rr_base,1000,1000);

    struct router * rh2 = router_new();
    mac_rr_base[5]++;
    router_add_port(rh2,1,mac_rr_base,1000,1000);
    mac_rr_base[5]++;
    router_add_port(rh2,2,mac_rr_base,1000,1000);
    mac_rr_base[5]++;
    router_add_port(rh2,3,mac_rr_base,1000,1000);

    ser_port_attri(rr1,1,0x02);
    ser_port_attri(rr2,1,0x02);
    ser_port_attri(rr1,2,0x01);
    ser_port_attri(rr2,2,0x01);

    log_info("okokok topo");
    topology_add_host(topo,h1);
    topology_add_host(topo,h2);
    topology_add_host(topo,h3);
    topology_add_host(topo,h4);
    topology_add_router(topo,rr1);
    topology_add_router(topo,rh1);
    topology_add_router(topo,rr2);
    topology_add_router(topo,rh2);
    //create link
    //uuid 1 2 3 4 5 6 7 8
    topology_add_link(topo,router_uuid(rr1),router_uuid(rr2),1,1,1000,10,false);
    topology_add_link(topo,router_uuid(rr1),router_uuid(rh1),2,1,1000,10,false);
    topology_add_link(topo,router_uuid(rr2),router_uuid(rh2),2,1,1000,10,false);

    topology_add_link(topo,router_uuid(rh1),host_uuid(h1),2,1,1000,10,false);
    topology_add_link(topo,router_uuid(rh1),host_uuid(h2),3,1,1000,10,false);
    topology_add_link(topo,router_uuid(rh2),host_uuid(h3),2,1,1000,10,false);
    topology_add_link(topo,router_uuid(rh2),host_uuid(h4),3,1,1000,10,false);
    log_info("okokok topo");
    set_recv_netflow(rr1,3);
    set_recv_netflow(rr2,3);
    log_info("okokok topo");
    insert_arq_ip_mac_table(rr1,3,mac_h3);
    insert_arq_ip_mac_table(rr1,4,mac_h4);
    insert_arq_ip_mac_table(rr2,1,mac_h1);
    insert_arq_ip_mac_table(rr2,2,mac_h2);
    log_info("okokok topo");
    return topo;
}

void 
topology_add_datapath(struct topology *topo, struct datapath* dp)
{
    struct dp_node *dn = xmalloc(sizeof (struct dp_node));
    dn->dp_id = dp_id(dp);
    dn->dp = dp;
    HASH_ADD(hh, topo->dps, dp_id, sizeof(uint64_t), dn);
    HASH_ADD(hh, topo->nodes, uuid, sizeof(uint64_t), (struct node*) dp);
    topo->n_dps++;
}

void 
topology_add_host(struct topology *topo, struct host *h)
{
    HASH_ADD(hh, topo->nodes, uuid, sizeof(uint64_t), (struct node*) h);
    topo->n_hosts++;
}
void 
topology_add_router(struct topology *topo, struct router *r)
{
    HASH_ADD(hh, topo->nodes, uuid, sizeof(uint64_t), (struct node*)r);
    topo->n_routers++;
}

struct node*
uuid_to_node(struct topology *t,uint64_t uuid)
{
    struct node * n;
    HASH_FIND(hh, t->nodes, &uuid, sizeof(uint64_t), n);
    return n;
}

struct node *
rtname_to_node(struct topology *t,char * name,uint32_t name_size)
{

    struct node *cur_node, *tmp;
    HASH_ITER(hh, t->nodes, cur_node, tmp) {
        if(memcmp(name, cur_node->name, name_size) == 0)
        {
//            log_info("name %s,node name %s",name,cur_node->name);
            return cur_node;
        }
            
    }

    return NULL;
}
/*
struct node *
find_host_with_rt(struct topology *t,struct node * rt,uint32_t ip_address)
{
    struct link *l;
    struct node_port_pair np;
    memset(&np, 0x0, sizeof(struct node_port_pair));
    np.uuid = rt->uuid;

}
*/

void 
topology_add_link(struct topology *t, uint64_t uuidA, uint64_t uuidB, uint32_t portA, uint32_t portB, uint32_t bw, uint32_t latency, bool directed)
{
    struct node *dpA, *dpB;
    struct link *l;
    dpA = dpB = NULL;
    /* Check if A and B exist. */
    HASH_FIND(hh, t->nodes, &uuidA, sizeof(uint64_t), dpA);
    if (dpA == NULL){
        /* TODO: return error message. */
        return;
    }
    HASH_FIND(hh, t->nodes, &uuidB, sizeof(uint64_t), dpB);
    if (dpB == NULL){
        /* TODO: return error message. */
        return;
    }

    if(dpA->type == ROUTER)
    {
        add_flag_to_port(dpA,portA,false);
    }

    /* Fill link configuration. */
    l = (struct link*) xmalloc(sizeof(struct link));
    memset(l, 0x0, sizeof(struct link));

    l->node1.port = portA;
    l->node1.uuid = dpA->uuid;
    l->node2.port = portB;
    l->node2.uuid = dpB->uuid;
    l->latency = latency;
    l->bandwidth = bw;

    

    HASH_ADD(hh, t->links, node1, sizeof(struct node_port_pair), l);
    /* Insert backwards. */
    if (directed == false){
        topology_add_link(t, uuidB, uuidA, portB, portA, bw, latency, true);
    }
    else {
        t->n_links++;
    }
}

void 
topology_remove_node(struct topology *topo,uint64_t uuid)
{
    struct node *n = NULL;
    HASH_FIND(hh, topo->nodes, &uuid, sizeof(uint64_t), n);
    HASH_DEL(topo->nodes,n);
    switch (n->type)
    {
    case DATAPATH/* constant-expression */:
        /* code */
        topo->n_dps--;
        break;
    case ROUTER:
        topo->n_routers--;
        break;
    case HOST:
        topo->n_hosts--;
        break;
    case L2_SWITCH:
        break;
    default:
        break;
    }
}

bool
topology_next_hop(const struct topology *topo, const uint64_t orig_uuid, const uint32_t orig_port, uint64_t *dst_uuid, uint32_t *dst_port, uint32_t *latency)
{
    struct link *l;
    struct node_port_pair np;
    memset(&np, 0x0, sizeof(struct node_port_pair));
    np.uuid = orig_uuid;
    np.port = orig_port;
    HASH_FIND(hh, topo->links, &np, sizeof(struct node_port_pair), l);
    if (l != NULL){
        *dst_uuid = l->node2.uuid;
        *dst_port = l->node2.port;
        *latency = l->latency;
        return true;
    }
    return false;
}

/* Clean the dynamically allocated members of a topology 
*  As there should be a single instance of topology allocated 
*  in the stack it is not necessary to free topo.
*/

void
topology_link_del(struct topology *topo,struct node * n,uint32_t port_id)
{
    struct link *l1;
    struct node_port_pair np1;
    memset(&np1, 0x0, sizeof(struct node_port_pair));
    log_info("copy uuid & port_id");
    np1.uuid = n->uuid;
    np1.port = port_id;
    HASH_FIND(hh, topo->links, &np1, sizeof(struct node_port_pair), l1);


     log_info("find node 2");
    struct link *l2;
    struct node_port_pair np2;
    memset(&np2, 0x0, sizeof(struct node_port_pair));
    np2.uuid = l1->node2.uuid;
    np2.port = l1->node2.port;
    HASH_FIND(hh, topo->links, &np2, sizeof(struct node_port_pair), l2);



log_info("find ok");
    HASH_DEL(topo->links, l1); 
    free(l1);
    log_info("del l1");
    HASH_DEL(topo->links, l2); 
    free(l2);
    log_info("del l2");
    log_info("del links");

    struct node * nn = uuid_to_node(topo,np2.uuid);
    uint32_t port_id_router = np2.port;
    if(nn!=NULL && nn->type == ROUTER)
    {
        add_flag_to_port(n,port_id_router,true);
    }
}

void 
topology_destroy(struct topology *topo)
{
    struct node *cur_node, *tmp;
    struct link *ltmp, *lcurr;
    struct dp_node *dncur, *dntmp;
    /* Clean links */
    HASH_ITER(hh, topo->links, lcurr, ltmp) {
        HASH_DEL(topo->links, lcurr);  
        free(lcurr);
    }
    /* Clean Nodes */
    HASH_ITER(hh, topo->nodes, cur_node, tmp) {
        HASH_DEL(topo->nodes, cur_node);  
        if (cur_node->type == DATAPATH){
            dp_destroy((struct datapath*) cur_node);    
        }
        else if (cur_node->type == HOST){
            host_destroy((struct host*) cur_node);    
        }
    }
    /* Clean datapath map */
    HASH_ITER(hh, topo->dps, dncur, dntmp) {
        HASH_DEL(topo->dps, dncur);  
        free(dncur);
    }
    free(topo);
}

struct node* 
topology_node(const struct topology *topo, uint64_t uuid)
{
    struct node *n = NULL;
    HASH_FIND(hh, topo->nodes, &uuid, sizeof(uint64_t), n);
    return n;
}

struct datapath* 
topology_datapath_by_dpid(const struct topology *topo, uint64_t dp_id)
{
    struct dp_node *dn;
    HASH_FIND(hh, topo->dps, &dp_id, sizeof(uint64_t), dn);
    return dn->dp;
}
/*
struct router*
router_from_json(struct router_json * rt_json)
{
    struct router* rt = router_new();
    router_set_name(rt,rt_json->name);
    struct port_json * p_json = &rt_json->p_json;

    for(uint32_t i = 0; i<=rt_json->num_port; ++i)
    {
        router_add_port(rt,p_json->port_id,p_json->mac_addr,1000,1000);
    }

    return rt;
}

struct host*
host_from_json(struct host_json * h_json)
{
    struct host* h = host_new();
    host_set_name(h,h_json->name);
    
    host_add_port(h,h_json->port.port_id,h_json->port.mac_addr,1000,1000);
    host_set_intf_ipv4(h,h_json->port.port_id,h_json->ipv4,h_json->netmask);
    return h;
}
*/
static
void topology_from_ptopo(struct topology* topo, struct parsed_topology* ptopo)
{
    /*old ;for datapath*/
    //size_t i;
    /* Create Datapaths */
    /*
    for (i = 0; i < ptopo->ndps; ++i){
        struct datapath *dp = dp_new(ptopo->dps[i], "127.0.0.1", 6653);
        uint8_t mac[6] = {0,0,0,0,0,1};
        dp_add_port(dp, 1, mac, 1000000, 1000000);
        dp_add_port(dp, 2, mac, 1000000, 1000000);
        topology_add_datapath(topo, dp);
    }
    */
    /* Create links */

    /*
    for (i = 0; i < ptopo->nlinks; ++i){
        topology_add_link(topo, ptopo->links[i].switchX, ptopo->links[i].switchY, ptopo->links[i].portX, ptopo->links[i].portY, ptopo->links[i].delay, ptopo->links[i].bw, false);
    }
    */
   
   //for node:
    for(uint32_t i = 0; i<ptopo->num_router; ++i)
    {
        struct router* rt = router_new();
        topology_add_router(topo,rt);

        char name[16];
        memset(name,0,sizeof(name));
        name[0] = 'h';
        name[1] = i + '0' + 1;
        router_set_name(rt,name);
        for(uint32_t j = 1; j<=ptopo->num_router + 10000 - 100; ++j)
        {
            uint8_t mac[6] = {0,0,i,j/256/256,(j/256)%256,j%256};
            router_add_port(rt,j,mac,1000,1000);
        }
        uint64_t uuid = rtname_to_node(topo,name,2)->uuid;
        log_info("new rt %d",uuid);
        
    }
    //4 port

    //for link
    for(uint32_t i = 0; i<ptopo->num_links_rt; ++i)
    {
        struct link_rt lrt = ptopo->links_rt[i];
        topology_add_link(topo,lrt.rt_x,lrt.rt_y,lrt.portX,lrt.portY,lrt.bw,lrt.delay,false);
        log_info("new port link %d %d",lrt.rt_x,lrt.rt_y);
    }
    /*new; for emu - sim*/
    /*
    for(uint32_t i = 0; i<ptopo->num_host; ++i)
    {
        struct host * h = host_from_json(&ptopo->h[i]);
        topology_add_host(topo,h);
    }
    for(uint32_t i = 0; i<ptopo->num_router; ++i)
    {
        struct router * rt = router_from_json(&ptopo->rt[i]);
        topology_add_router(topo,rt);
    }
    for(uint32_t i = 0; i<ptopo->num_link_uuid; ++i)
    {
        topology_add_link(topo,ptopo->link_u[i].uuid_x,ptopo->link_u[i].uuid_y,ptopo->link_u[i].portX,ptopo->link_u[i].portY,ptopo->link_u[i].delay,ptopo->link_u[i].bw,false);
    } 
    */
    //free all 
    
}

struct topology* 
from_json(char *json_file)
{
    size_t s;
    struct parsed_topology ptopo;
    struct topology* topo = topology_new();
    char *json = file_to_string(json_file, &s);
    if (json != NULL) {
        parse_topology(json, s, &ptopo);
        topology_from_ptopo(topo, &ptopo);
    }
    return topo;
}


uint64_t 
device_id_to_host_uuid(struct topology *topo,char * device_id)
{
    struct node *cur_node, *tmp;
    HASH_ITER(hh, topo->nodes, cur_node, tmp){
        if(cur_node->type == HOST)
        {
            uint64_t uuid = 0;
            uuid = host_device_id_cmp(cur_node,device_id);
            if(uuid != 0)
            {
                return uuid;
            }
        }
    }

    return 0;
}

/* Get struct members */
uint32_t 
topology_dps_num(const struct topology *topo)
{
    return topo->n_dps;
}

uint32_t topology_links_num(const struct topology *topo)
{
    return topo->n_links;
}
uint32_t topology_hosts_num(const struct topology *topo)
{
    return topo->n_hosts;
}
struct node* topology_nodes(const struct topology *topo)
{
    return topo->nodes;
}
