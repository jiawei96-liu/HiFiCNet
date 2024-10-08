#include "host.h"
#include "legacy_node.h"
#include "route_table.h"
#include "arp_table.h"
#include "lib/util.h"
#include <log/log.h>
#include <uthash/utlist.h>

#define IDLENGTH 108

#define ETH_ADDR_FMT                                                    \
    "%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8":%02"PRIx8
#define ETH_ADDR_ARGS(ea)                                   \
    (ea)[0], (ea)[1], (ea)[2], (ea)[3], (ea)[4], (ea)[5]

struct host {
    struct legacy_node ep; /* Node is an endpoint */
    struct app *apps;      /* Hash Map of applications */
    struct exec *execs;    /* List of apps to be executed */
    uint32_t IPV4_addr;
    uint32_t netmask;
    char device_id[IDLENGTH];
};

struct host *
host_new(void)
{
    struct host *h = xmalloc(sizeof(struct host));
    legacy_node_init(&h->ep, HOST);
    h->ep.base.recv_netflow = host_recv_netflow;
    h->ep.base.send_netflow = host_send_netflow;
    h->apps = NULL;
    h->execs = NULL;
    h->IPV4_addr = 0;
    h->netmask = 0;
    memset(h->device_id,0,sizeof(h->device_id));
//    log_info("create host success");
    return h;
}
void
host_set_device_id(struct host * h, char * device_id)
{
    memcpy(h->device_id,device_id,strlen(device_id));
}

uint64_t 
host_device_id_cmp(struct node * n,char * device_id)
{
    struct host * h = (struct host *)n;
    if(memcmp(h->device_id,device_id,strlen(device_id)) == 0)
        return h->ep.base.uuid;
    return 0;
}

void 
host_destroy(struct host *h)
{
    struct exec *exec, *exec_tmp;
    struct app *app, *app_tmp;
    legacy_node_clean(&h->ep);
    
    HASH_ITER(hh, h->apps, app, app_tmp) {
        HASH_DEL(h->apps, app);
        app_destroy(app);
    }
    HASH_ITER(hh, h->execs, exec, exec_tmp) {
        HASH_DEL(h->execs, exec);
        app_destroy_exec(exec);
    }
    free(h);
}

void 
host_add_port(struct host *h, uint32_t port_id, 
              uint8_t eth_addr[ETH_LEN], uint32_t speed, 
              uint32_t curr_speed)
{   
    node_add_port( (struct node*) h, port_id, eth_addr, speed, curr_speed);
    //for(int i = 0; i<ETH_LEN;++i)
      //  log_info("%d %d",eth_addr[i]/16,eth_addr[i]%16);
}


void
host_set_intf_ipv4(struct host *h, uint32_t port_id, 
                   uint32_t addr, uint32_t netmask){
    
    h->IPV4_addr = addr;
    h->netmask = netmask;
    legacy_node_set_intf_ipv4(&h->ep, port_id, addr, netmask);   
}

uint32_t 
get_host_vlan_tag(struct host *h)
{
    return h->IPV4_addr & h->netmask;
}


struct netflow*
host_recv_netflow(struct node *n, struct netflow *flow)
{
    struct host *h = (struct host*) n;
    /* Check MAC and IP addresses. Only returns true if
       both have the node destination MAC and IP       */
    uint16_t eth_type = flow->match.eth_type;
    struct netflow* nf = l2_recv_netflow(&h->ep, flow); 
    if(nf == NULL)
        return NULL;
    if (eth_type == ETH_TYPE_IP || eth_type == ETH_TYPE_IPV6) {
        if (l3_recv_netflow(&h->ep, nf)){
            log_info("l3 rec ok %d",n->uuid);
            struct app *app;
            uint16_t ip_proto = flow->match.ip_proto;
            HASH_FIND(hh, h->apps, &ip_proto, sizeof(uint16_t), app);
            /* If gets here send to application */
            log_debug("APP %d %p", ip_proto, app);
            if (app){
                if (app->handle_netflow(nf)){
                    return find_forwarding_ports(&h->ep, nf);
                }
            } 
        }
    }
    else if (eth_type == ETH_TYPE_ARP){
        /* End here if there is not further processing */
        return nf;
    }
    return NULL;
}

void 
host_send_netflow(struct node *n, struct netflow *flow, uint32_t out_port)
{
    node_update_port_stats(n, flow, out_port);
}

void 
host_add_app(struct host *h, uint16_t type)
{
    struct app *a =  app_creator(type);
    HASH_ADD(hh, h->apps, type, sizeof(uint16_t), a);
}

void 
host_add_app_exec(struct host *h, uint64_t id, uint32_t type,
                  uint32_t execs_num, uint64_t start_time, void *args, 
                  size_t arg_size)
{
    struct app *app = NULL;
    /* Guarantees the app can be executed */
    HASH_FIND(hh, h->apps, &type, sizeof(uint16_t), app);
    if (app) {
        struct exec *exec = app_new_exec(id, type, execs_num, start_time, 
                                         args, arg_size);
        HASH_ADD(hh, h->execs, id, sizeof(uint64_t), exec);
    }
}

struct netflow* 
host_execute_app(struct host *h, struct exec *exec)
{
    struct app *app;
    HASH_FIND(hh, h->apps, &exec->type, sizeof(uint16_t), app);
    struct netflow *flow = NULL;
    /* If app still has remaining executions */
    if (app) {
        flow = app->start(exec->start_time, exec->args);
        flow->exec_id = exec->id;
        log_info("Flow Start time APP %ld", flow-> start_time);
        exec->exec_cnt -= 1;
        return find_forwarding_ports(&h->ep, flow);
    }
    return NULL;
}

void host_set_name(struct host* h, char *name)
{
    memcpy(h->ep.base.name, name, MAX_NODE_NAME);
}

/* Access functions*/
char *host_name(struct host *h)
{
    return h->ep.base.name;
}

/* Retrieve a datapath port */
struct port* 
host_port(const struct host *h, uint32_t port_id)
{
    struct port *p = node_port( (struct node*) h, port_id);
    return p;
}

uint64_t 
host_uuid(const struct host* h)
{
    return h->ep.base.uuid;
}

struct app *host_apps(const struct host *h)
{
    return h->apps;
}

struct exec *host_execs(const struct host *h)
{
    return h->execs;
}
