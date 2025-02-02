/* Copyright (c) 2016-2017, Eder Leao Fernandes
 * All rights reserved.
 *
 * The contents of this file are subject to the license defined in
 * file 'doc/LICENSE'.
 *
 *
 * Author: Eder Leao Fernandes <e.leao@qmul.ac.uk>
 */

#include "sim_event.h"
#include <log/log.h>
struct sim_event*
sim_event_new(uint64_t time)
{
    struct sim_event *ev = xmalloc(sizeof(struct sim_event));
    ev->time = time;
    return ev;
}

void sim_event_free(struct sim_event* ev){
    
    switch (ev->type){
        case EVENT_FLOW_RECV:{
            free((struct sim_event_flow_recv*) ev);
            break;
        } 
        case EVENT_FLOW_SEND: {
            /* The pointer fields should be released in 
               the end of their lifetime */
            free((struct sim_event_flow_send*) ev);
            break;
        }
        case EVENT_OF_MSG_IN:
        case EVENT_OF_MSG_OUT: {
            free((struct sim_event_of*) ev);
            break;
        }
        case EVENT_APP_START: {
            free((struct sim_event_app_start*) ev);
            break;
        }
        case EVENT_END :{
            free(ev);
            break;
        }
        default: {
            //fprintf(stderr, "Unknown Event %d\n", ev->type);
            break;
        }
    } 
}
 
struct sim_event_flow_recv *sim_event_flow_recv_new(uint64_t time, 
                                               uint64_t node_id, uint32_t rate) 
{
    struct sim_event_flow_recv *flow = xmalloc(sizeof(struct sim_event_flow_recv));
    flow->hdr.time = time;
    flow->hdr.type = EVENT_FLOW_RECV;
    flow->node_id = node_id;
    flow->rate = rate;
    return flow;   
}

struct sim_event_flow_send *sim_event_flow_send_new(uint64_t time, 
                                          uint64_t node_id, uint32_t out_port)
{
    struct sim_event_flow_send *flow = xmalloc(sizeof(struct sim_event_flow_send));
    flow->hdr.time = time;
    flow->hdr.type = EVENT_FLOW_SEND;
    flow->node_id = node_id;
    flow->out_port = out_port;
    return flow;  
}

static struct sim_event_of* 
of_msg_new(uint64_t time, uint64_t dp_id, void *data, 
           size_t len, uint8_t type)
{
    struct sim_event_of *msg = xmalloc(sizeof(struct sim_event_of));
    msg->hdr.time = time;
    msg->hdr.type = type;
    msg->dp_id = dp_id;
    msg->data = data;
    msg->len = len;
    return msg;        
} 

struct sim_event_of* 
sim_event_of_msg_in_new(uint64_t time, uint64_t dp_id, 
                         void *data, size_t len)
{
    return of_msg_new(time, dp_id, data, len, EVENT_OF_MSG_IN);      
}

struct sim_event_of* 
sim_event_of_msg_out_new(uint64_t time, uint64_t dp_id, 
                         void *data, size_t len)
{
    return of_msg_new(time, dp_id, data, len, EVENT_OF_MSG_OUT);      
}

struct sim_event_host_add * 
sim_event_host_add_new(struct node * rt,uint64_t time,uint32_t ip,uint32_t netmask,uint8_t eth_addr[ETH_LEN],char * device_id)
{
    //uint32_t port_id_router = find_alloc_port(rt);// nums + 1;  need 

   // uint8_t mac_host[6] = {0,0,0,1,router_id,port_id_router};

    struct sim_event_host_add *add = xmalloc(sizeof(struct sim_event_host_add));
    add->hdr.time = time;
    add->hdr.type = EVENT_HOST_ADD;
    add->ip = ip;
    add->netmask = netmask;
    add->rt = rt;
    strcpy(add->eth_addr,eth_addr);
    memcpy(add->device_id,device_id,strlen(device_id));
//    log_info("add event create ok");
    return add;

}

struct sim_event_host_del * 
 sim_event_host_del_new(char * device_id,uint64_t time)
 {
    struct sim_event_host_del *del = xmalloc(sizeof(struct sim_event_host_del));
    //del->h = h;
    //del->uuid = uuid;
    del->hdr.time = time;
    del->hdr.type = EVENT_HOST_DEL;
    strcpy(del->device_id,device_id);
    return del;
 }

struct sim_event_app_start*
sim_event_app_start_new(uint64_t time, uint64_t node_id, struct exec *exec)
{
    struct sim_event_app_start *ev = xmalloc(sizeof(
                                             struct sim_event_app_start));
    ev->hdr.time = time;
    ev->hdr.type = EVENT_APP_START;
    ev->node_id = node_id;
    ev->exec = exec;
    return ev;
}
