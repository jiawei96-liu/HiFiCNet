/* Copyright (c) 2016-2017, Eder Leao Fernandes
 * All rights reserved.
 *
 * The contents of this file are subject to the license defined in
 * file 'doc/LICENSE'.
 *
 *
 * Author: Eder Leao Fernandes <e.leao@qmul.ac.uk>
 */

#include "sim.h"
#include <unistd.h>
#include <time.h>
#include <uthash/utlist.h>
#include "lib/openflow.h"
#include <log/log.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>

#define EV_NUM 1000000

#define NONEVENT 0x1000
#define ADDHOST 0x1001
#define DELHOST 0x1002
#define PING 0x1003

#define Created 0x0010
#define DELETE 0x0020
#define PUT 0x0030

static void *des_mode(void * args);
static void *cont_mode(void* args); 
void monitor_to_packet(void * args);

static pthread_cond_t  mode_cond_var   = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx_mode    = PTHREAD_MUTEX_INITIALIZER;
int last_wrt = 0;
uint64_t last_ctrl = 0;
uint64_t last_stats = 0;

struct temp{
    struct sim * s;
    char * buf;
    uint32_t n;
};

static void 
initial_events(struct sim *s)
{

    struct scheduler *sch = s->evh.sch;
    struct topology *topo = s->evh.topo;
    struct node *node, *tmp;
    /* Not very efficient now... */
    HASH_ITER(hh, topology_nodes(topo), node, tmp){
        if (node->type == HOST){
            struct exec *exec, *exec_tmp;
            HASH_ITER(hh, host_execs((struct host*) node), exec, exec_tmp) {
                struct sim_event_app_start *ev = sim_event_app_start_new(exec->start_time, node->uuid, exec);
                scheduler_insert(sch, (struct sim_event*) ev);
            }   
        }
    }
    /* Event to stop the simulator */
    struct sim_event *end_ev; 
    end_ev = sim_event_new(sim_config_get_end_time(s->config)); 
    end_ev->type = EVENT_END;
    scheduler_insert(sch, end_ev);
}

struct timespec last = {0};
struct timespec now = {0};
FILE *pFile;

static void 
wait_all_switches_connect(struct topology *topo, struct of_manager *om)
{
    uint32_t switches;
    uint32_t time = 0;
    switches = HASH_COUNT(om->of->active_conns);
    while (switches < topology_dps_num(topo))  {
        sleep(1);
        switches = HASH_COUNT(om->of->active_conns);
        time++;
        /* Exit if it takes too long to connect */ 
        if (time > 60) {
            fprintf(stderr, "Connection time expired\n");
            exit(1);
        }
    } 
}

static void
sim_init(struct sim *s, struct topology *topo, struct sim_config *config) 
{   

    s->config = config;
    s->evh.topo = topo;
    s->evh.sch = scheduler_new();
    s->evh.live_flows = NULL;
    s->cont.exec = cont_mode;
    initial_events(s); 
    if (sim_config_get_mode(s->config) == EMU_CTRL){
        struct node *cur_node, *tmp, *nodes;
        struct datapath *dp;
        s->evh.om = of_manager_new(s->evh.sch);
        /* Add of_settings to client */
        nodes = topology_nodes(topo);
        HASH_ITER(hh, nodes, cur_node, tmp) {
            if (cur_node->type == DATAPATH){
                dp = (struct datapath*) cur_node;
                of_client_add_ofsc(s->evh.om->of, dp_settings(dp));
            }
        }
        of_client_start(s->evh.om->of, false);
    }
    pFile = fopen ("bwm.txt","w");
    wait_all_switches_connect(topo, s->evh.om);
    init_timer(&s->cont, (void*)s);
    set_periodic_timer(/* 1 */1000);
    clock_gettime(CLOCK_MONOTONIC_RAW, &last);

    ping_test(1,3,s);

    pthread_create(&s->dataplane, (pthread_attr_t*)0, des_mode, (void*)s);


    pthread_t p;
    log_info("momo start");
   pthread_create(&p,NULL,monitor_to_packet,(void*)s);
   //monitor_to_packet(s);
   //monitor_to_packet_test(s);

/*
    char * buf_t = xmalloc(sizeof(char) * (1024));
    memset(buf_t,0,sizeof(char) * (1024));
    buf_t = "..Created..\"host_id\":\"h1\"..\"device_id\":\"nstest\"..\"ip_address\":\"1.0.0.10\"..\"mac_address\":\"00:11:aa:22:33:44\"....";
    struct temp  te;
    te.buf = buf_t;
    te.n = strlen(buf_t);
    te.s = s;
    packet_to_event(&te);

    char * buf_t0 = xmalloc(sizeof(char) * (1024));
    memset(buf_t0,0,sizeof(char) * (1024));
    buf_t0 = "..Created..\"host_id\":\"h2\"..\"device_id\":\"nstest1\"..\"ip_address\":\"1.0.0.9\"..\"mac_address\":\"00:11:aa:22:33:55\"....";
    struct temp  te0;
    te0.buf = buf_t0;
    te0.n = strlen(buf_t0);
    te0.s = s;
    packet_to_event(&te0);
*/
/*
    char * buf_t1 = xmalloc(sizeof(char) * (1024));
    memset(buf_t1,0,sizeof(char) * (1024));
    buf_t1 = "..DELETE..\"host_id\":\"h2\".\"device_id\":\"nstest1\".";
    struct temp  te1;
    te1.buf = buf_t1;
    te1.n = strlen(buf_t1);
    te1.s = s;
    packet_to_event(&te1);4
    */
            //pthread_create(&p,NULL, packet_to_event, (void*)&te);
/*
     uint32_t ip_dst = 16777225;
    log_info("dst");
    //host_add_app_exec((struct host*) h_base,5,PINGV4,1,s->evh.sch->clock + 1,ip_dst,sizeof(uint32_t));
    struct exec * exec_new = app_new_exec(5,PINGV4,1,s->evh.sch->clock + 2,&ip_dst,sizeof(uint32_t));
    log_info("exec");
    //struct sim_event_app_start* app_start = sim_event_app_start_new(s->evh.sch->clock + 1,h_base->uuid,exec_new);
    struct sim_event_app_start* app_start = sim_event_app_start_new(s->evh.sch->clock + 10,5,exec_new);
   log_info("app event");
    scheduler_insert(s->evh.sch,app_start);
    log_info("insert app");
    */
}

static void 
sim_close(struct sim *s)
{
    pthread_join(s->dataplane, 0);
    fclose (pFile);
    scheduler_destroy(s->evh.sch);
    topology_destroy(s->evh.topo);
    of_manager_destroy(s->evh.om);
}


static void update_stats(struct topology *topo, uint64_t time){

    struct node *cur_node, *tmp, *nodes;
    nodes = topology_nodes(topo);
    HASH_ITER(hh, nodes, cur_node, tmp) {
        if (cur_node->type == DATAPATH){
            dp_write_stats((struct datapath*) cur_node, time, pFile);
        }
    }
}


static struct sim_event* 
make_time_msg(uint64_t time){
    uint8_t *buf = xmalloc(sizeof(struct ofp_experimenter_header) + 8);
    struct ofp_experimenter_header *msg = (struct ofp_experimenter_header*) buf;
    msg->header.version = OFP_VERSION; 
    msg->header.type = OFPT_EXPERIMENTER;
    msg->header.length = htons(24);
    msg->header.xid = 0;
    msg->experimenter = htonl(0xF0A1F0A1);
    msg->exp_type = 0;
    uint64_t *t = (uint64_t*) &buf[16];
    *t = hton64(time);   
    /* TODO: Create a dedicated control channel for the simulator messages */
    struct sim_event_of *ev = sim_event_of_msg_out_new(time, 
                                                      0x00000000000000001, buf, 24);
    // scheduler_insert(sch, (struct sim_event*) ev); 
    return (struct sim_event*) ev;
}


static void *
des_mode(void *args){
/*  Executes DES while controller does nothing
    Waits for some event from controller or current time
    is larger than event time */
    struct sim *s = (struct sim*) args;
    struct scheduler *sch = s->evh.sch;   
    while (1){
        /* Only execute in DES mode */
        pthread_mutex_lock( &mtx_mode );
        while(sch->mode){
            pthread_cond_wait( &mode_cond_var, &mtx_mode );
        }
        pthread_mutex_unlock( &mtx_mode );

        struct sim_event *ev = scheduler_dispatch(sch);
        sch->clock = ev->time;
        /* Update status */
        if ((sch->clock - last_stats) / 1000000){
            update_stats(s->evh.topo, sch->clock);
            last_stats = sch->clock;
        }
        /* Need to make it more sane ... */
        //log_info("select event %d",ev->type);
        if (ev->type != EVENT_END) {
            /* Execute */
            handle_event(&s->evh, ev);
            /* TODO: Very ugly right now*/
            if ((sch->clock - last_wrt) > 10000000 ){
                handle_event(&s->evh, make_time_msg(sch->clock));
                /* Wake up timer */
                pthread_mutex_lock( &mtx_mode );
                last_ctrl = sch->clock;
                sch->mode = CONTINUOUS;
                last_wrt = sch->clock;
                pthread_cond_signal( &mode_cond_var );
                pthread_mutex_unlock( &mtx_mode );
            }
            if (ev->type == EVENT_OF_MSG_OUT ||
                ev->type == EVENT_OF_MSG_IN ) {
                
                /* Wake up timer */
                pthread_mutex_lock( &mtx_mode );
                last_ctrl = sch->clock;
                sch->mode = CONTINUOUS;
                pthread_cond_signal( &mode_cond_var );
                pthread_mutex_unlock( &mtx_mode );
            
            }
            sim_event_free(ev);
        }
        else {
            //log_info("End of the Simulation");
            //sleep(10);
            continue;

            sim_event_free(ev);
            break;
        }
    }
    shutdown_timer(&s->cont);
    return 0;
} 

struct sim_event *cur_ev = NULL;

static void *
cont_mode(void* args) 
{
    struct sim *s = (struct sim*) args;
    struct scheduler *sch = s->evh.sch;
    /* The code below is just a demonstration. */
    /* Increase time and check if there a DP event to execute */
    pthread_mutex_lock( &mtx_mode );
    while(!sch->mode){
        pthread_cond_wait( &mode_cond_var, &mtx_mode );
    }
    pthread_mutex_unlock( &mtx_mode );
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    uint64_t delta_us = (now.tv_sec - last.tv_sec) * 1000000 + 
                        (now.tv_nsec - last.tv_nsec) / 1000;
    cur_ev = scheduler_retrieve(sch); 
    sch->clock += delta_us;     
    /* Time will be shared now */
    if ((sch->clock - last_wrt) > 10000000){
        handle_event(&s->evh, make_time_msg(sch->clock));
        last_wrt = sch->clock;
    }
    while (cur_ev->time <= sch->clock){
        /* Execute */
        if ((cur_ev->time - last_stats) / 1000000){
            update_stats(s->evh.topo, cur_ev->time);
            last_stats = cur_ev->time ;
        }

        if (cur_ev->type == EVENT_OF_MSG_OUT || 
             cur_ev->type == EVENT_OF_MSG_IN ) {
            last_ctrl = cur_ev->time;
        }
        else if(cur_ev->type == EVENT_END){
            goto check_idle;
        }
        handle_event(&s->evh, cur_ev);
        scheduler_delete(sch, cur_ev);
        sim_event_free(cur_ev);
        cur_ev = scheduler_retrieve(sch);
    }

    /* store last time here */
    clock_gettime(CLOCK_MONOTONIC_RAW, &last);
    check_idle:
    /* Check if controller is idle for some time */
    if ( (sch->clock - last_ctrl) > 
        sim_config_get_ctrl_idle_interval(s->config)) {
        pthread_mutex_lock( &mtx_mode );
        sch->mode = DES;
        /* Wake up timer */
        pthread_cond_signal( &mode_cond_var );
        pthread_mutex_unlock( &mtx_mode );
    }
    return 0; 
}

void 
start(struct topology *topo, struct sim_config *config) 
{
    struct sim s; 
    memset(&s, 0x0, sizeof(struct sim));
   // topo = from_json("topology.json");  
    topo = topo_test(topo);
    sim_init(&s, topo, config);
    sim_close(&s);
}


// add to pthread ; monitor all received packet ; add event to simu
void
ping_test(uint64_t uuid,uint32_t dst_ip,struct sim*s)
{
    struct exec * exec_new = app_new_exec(uuid,PINGV4,1,s->evh.sch->clock + 1000,&dst_ip,sizeof(uint32_t));
    struct sim_event_app_start* app_start = sim_event_app_start_new(s->evh.sch->clock + 1002,uuid,exec_new);
    scheduler_insert(s->evh.sch,app_start);
}

void
monitor_to_packet_test(struct sim * s)
{
    
      
    /*
    char * buf_t = xmalloc(sizeof(char) * (1024));
    memset(buf_t,0,sizeof(char) * (1024));
    buf_t = "..Created..\"host_id\":\"h1\"..\"device_id\":\"nstest\"..\"ip_address\":\"1.0.0.10\"..\"mac_address\":\"00:11:aa:22:33:44\"....";
    struct temp  te;
    te.buf = buf_t;
    te.n = strlen(buf_t);
    te.s = s;
    packet_to_event(&te);
    */

   uint32_t len = 10000;
   for(int i = 0; i<len; ++i)
   {
    char * buf_t = xmalloc(sizeof(char) * (1024));
    memset(buf_t,0,sizeof(char) * (1024));
    buf_t = "..Created..\"host_id\":\"h1\"..\"device_id\":\"nstest\"..\"ip_address\":\"1.0.0.10\"..\"mac_address\":\"00:11:aa:22:33:44\"....";
    struct temp  te;
    te.buf = buf_t;
    te.n = strlen(buf_t);
    te.s = s;
    packet_to_event(&te);
//    log_info("%d",i);
   }

/*
    uint32_t ip_dst = 2;
    log_info("dst");
    //host_add_app_exec((struct host*) h_base,5,PINGV4,1,s->evh.sch->clock + 1,ip_dst,sizeof(uint32_t));
    struct exec * exec_new = app_new_exec(5,PINGV4,1,s->evh.sch->clock + 2,&ip_dst,sizeof(uint32_t));
    log_info("exec");
    //struct sim_event_app_start* app_start = sim_event_app_start_new(s->evh.sch->clock + 1,h_base->uuid,exec_new);
    struct sim_event_app_start* app_start = sim_event_app_start_new(s->evh.sch->clock + 10,4,exec_new);
   log_info("app event");
    scheduler_insert(s->evh.sch,app_start);
    log_info("insert app");
*/
}

bool 
length_check(uint32_t n)
{
   // if(n < 1240)
       // return false;
    if(n <= 66)
        return false;
    if(n > 1200)
        return false;
    return true;
}

bool 
packet_check(char * buf,uint32_t n)
{
    if(length_check(n) == false)
        return false;
    return true;
}

void
kmp_char(char * dst,char * buf,uint32_t n,char * str)
{
    // "str":"key"; return key
    //char * p = NULL;

    uint32_t key_find = 0;

    uint32_t len = strlen(str);
    for(uint32_t i = 0; i<n; ++i)
    {
        uint32_t j = 0;
        for(j = 0; i + j < n && j<strlen(str);++j)
        {
            if(buf[i + j] != str[j])
                break;
            //log_info("%c",str[j]);
        }
        if(j == strlen(str))
        {
            key_find = i + j + 3;
        }
    }

    uint32_t p_len = 0;
    for(uint32_t i = key_find; i<n; ++i)
    {
        if(buf[i] == '\"')
        {
            p_len = i - key_find;
            break;
        }
    }

    for(uint32_t i = 0; i<p_len; ++i)
    {
        dst[i] = buf[key_find + i];
    }
/*
    p = xmalloc(sizeof(char) * (p_len + 3));
    memset(p,0,p_len + 3);
    memcpy(p,(buf + key_find),p_len);
*/
    //return p;// retrun in "str": ""
}

uint32_t 
split_char(uint32_t *index,char c,char * str)
{
    uint32_t size = 0;
    for(uint32_t i = 0; str[i];++i)
    {
        if(str[i] == c)
        {
            index[size++] = i;
//            log_info("%d",i);
        }
    }
    index[size++] = strlen(str);
//    log_info("%d",strlen(str));
    return size;
}

uint8_t
to_uint8(char c)
{
    if(c <= 'f' && c >='a')
        return c - 'a' + 10;
    else
        return c - '0';
}

uint32_t
find_key_from_packet(char * buf,uint32_t n)
{
    //"DELETE" "Created"
    char del[10] = "DELETE";
    char cre[10] = "Created";
    char put[10] = "PUT";

    for(uint32_t i = 0; i<n; ++i)
    {
        uint32_t j = 0;
        for(j = 0; i+j < n && j<strlen(del); ++j)
        {
            if(buf[i + j] != del[j])
                break;
        }
        if(j >= strlen(del))
            return DELETE;
        
    }

    for(uint32_t i = 0; i<n; ++i)
    {
        uint32_t j = 0;
        for(j = 0; i + j < n && j<strlen(cre); ++j)
        {
            if(buf[i + j] != cre[j])
                break;
        }
        if(j >= strlen(cre))
            return Created;
//        log_info("the index j %d",j);
    }

    for(uint32_t i = 0; i<n; ++i)
    {
        uint32_t j = 0;
        for(j = 0; i+j < n && j<strlen(put); ++j)
        {
            if(buf[i + j] != put[j])
                break;
        }
        if(j >= strlen(put))
            return PUT;
        
    }
    return 0;
}

void
packet_to_add_event(struct sim * s,char * buf,uint32_t n)
{

    //log_info("packet add event ");

    struct topology * topo = s->evh.topo;
    struct scheduler *  sch = s->evh.sch;
    char  h_p[100];
    memset(h_p,0,sizeof(h_p));
    kmp_char(h_p,buf,n,"host_id");

    char  ip_p[100];
    memset(ip_p,0,sizeof(ip_p));
    kmp_char(ip_p,buf,n,"ip_address");

    char  mac_p[100];
    memset(mac_p,0,sizeof(mac_p));
    kmp_char(mac_p,buf,n,"mac_address");

    if(h_p == NULL || ip_p == NULL || mac_p == NULL)
    {
        log_info("non host or ip or mac");
        exit(0);
    }

    struct node * rt = rtname_to_node(topo,h_p,strlen(h_p));
    if(rt == NULL)
    {
        log_info("non host_id");
        exit(0);
    }
    //log_info("%s %s %s",h_p,ip_p,mac_p);

    uint32_t index[10];
    uint32_t ip_size = split_char(index,'.',ip_p); 
    uint32_t netmask = 0xFFFFFF00;
    uint32_t ip_address = 0;
    // char to ip;
    uint32_t kk = 0;
    for(uint32_t i = 0; i<ip_size; ++i)
    {
        uint32_t temp = 0;
        for(uint32_t j = kk; j<index[i]; ++j)
        {
            temp *= 10;
            temp += ip_p[j] - '0';
        }
        //log_info("temp:%d",temp);
        kk = index[i] + 1;
        ip_address *= 256;
        ip_address += temp;
    }
    //log_info("ip:%d",ip_address);
    //aa:aa:aa:aa:aa:aa
    uint8_t eth_addr[ETH_LEN];
    uint32_t eth_index = 0;
    //char to mac;
    for(uint32_t i = 0; i<strlen(mac_p); ++i)
    {
        if(mac_p[i] == ':')
            continue;
        else
        {
            uint8_t temp = to_uint8(mac_p[i]) * 16 + to_uint8(mac_p[i + 1]);
            ++i;
            eth_addr[eth_index++] = temp;
            //log_info("mac,%d",temp);
        }
    }

     char  device_id_p[100];
     memset(device_id_p,0,sizeof(device_id_p));
      kmp_char(device_id_p,buf,n,"device_id");
    //log_info("device_id %s",device_id_p);

    uint32_t temp = 0;
    if(ip_address % 2 == 1)
        temp = 1;

    struct sim_event_host_add* add = sim_event_host_add_new(rt,s->evh.sch->clock + 1 + temp,ip_address,netmask,eth_addr,device_id_p);
    scheduler_insert(sch,add);

    //log_info("event insert ok");
    
    //log_info("free ok");
}

void 
packet_to_del_event(struct sim * s,char * buf,uint32_t n)
{
    struct topology * topo = s->evh.topo;
    struct scheduler *  sch = s->evh.sch;
     char  device_id_p[100];
     memset(device_id_p,0,sizeof(device_id_p));
      kmp_char(device_id_p,buf,n,"device_id");

    struct sim_event_host_del * del =  sim_event_host_del_new(device_id_p,s->evh.sch->clock + 3);
    scheduler_insert(s->evh.sch,del);
    log_info("delete event");
    //free(device_id_p);
}

void
packet_to_put_event(struct sim * s,char * buf,uint32_t n)
{

}

void 
packet_to_event(void * args)
{
    struct temp * temp_t = (struct temp *)args;
    struct sim *s = temp_t->s;
    uint32_t n = temp_t->n;
    char * buf = temp_t->buf;

    // DELETE Created ;key find;
    uint32_t flag_app = find_key_from_packet(buf,n);
//    log_info("the %d flag",flag_app);
    if(flag_app == 0)
    {
        exit(0);
       // return;
    }

    switch (flag_app)
    {
    case DELETE:
        /* code */
        packet_to_del_event(s,buf,n);
        break;
    case Created:
        packet_to_add_event(s,buf,n);
        break;
    case PUT:
        packet_to_put_event(s,buf,n);
         break;
    default:
        break;
    }
    //free(buf);
    
}


void
monitor_to_packet(void * args)
{
    //struct topology * topo = s->evh.topo;
    //struct scheduler *  sch = s->evh.sch;
    struct sim * s = (struct sim *)args;
    log_info("momo");
    int n;
    int ret = 0;
    int sock;
    char buf[4096];
    struct ifreq ifreq;
    struct sockaddr_ll saddr;

    // create socket
    if((sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
        ret = errno;
        log_info("create socket fail");
        goto error_exit;
    }

    // bind veth0
    snprintf(ifreq.ifr_name, sizeof(ifreq.ifr_name), "veth0");
    if (ioctl(sock, SIOCGIFINDEX, &ifreq)) {
        ret = errno;
        log_info("bind error");
        goto error_exit;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    saddr.sll_ifindex = ifreq.ifr_ifindex;
    saddr.sll_pkttype = PACKET_HOST;

    if(bind(sock, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
        ret = errno;
        goto error_exit;
    }

    // recv data
    while(1) {
        n = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
        //log_info("%d bytes recieved" ,n);
        
        if(packet_check(buf,n) == true)
        {
            //create a thread
            char * buf_t = xmalloc(sizeof(char) * (n + 7));
            memset(buf_t,0,sizeof(char) * (n + 7));
            memcpy(buf_t,buf,n);
            struct temp  te;
            te.buf = buf_t;
            te.n = n;
            te.s = s;
            pthread_t p;
            pthread_create(&p,NULL, packet_to_event, (void*)&te);
            // thread(packet_to_event);
        }


    }
    error_exit:
    if (ret) {
        //printf("error: %s (%d)\n", strerror(ret), ret);
    }
    close(sock);
}