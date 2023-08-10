/* Copyright (c) 2016-2017, Eder Leao Fernandes
 * All rights reserved.
 *
 * The contents of this file are subject to the license defined in
 * file 'doc/LICENSE'.
 *
 *
 * Author: Eder Leao Fernandes <e.leao@qmul.ac.uk>
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H 1

#include "datapath.h"
#include "host.h"
#include "router.h"
#include "lib/util.h"

/* Total number of possible datapaths
*  UINT32_MAX == 2 ^ 16 datapaths 
*/
#define MAX_DPS UINT16_MAX 

struct topology;

struct topology* topology_new(void);

struct node * rtname_to_node(struct topology *t,char * name,uint32_t name_size);

struct topology * topo_test(struct topology *topo);
void  topology_remove_node(struct topology *topo,uint64_t uuid);
void topology_add_datapath(struct topology *topo, struct datapath *dp);
void topology_add_host(struct topology *topo, struct host *h);
void topology_add_router(struct topology *topo, struct router *r);

void topology_add_link(struct topology *t, uint64_t uuidA, 
                       uint64_t uuidB, uint32_t portA, uint32_t portB,
                       uint32_t bw, uint32_t latency, bool directed);

bool topology_next_hop(const struct topology *topo, const uint64_t orig_uuid, const uint32_t orig_port, uint64_t *dst_uuid, uint32_t *dst_port, uint32_t *latency);

void topology_destroy(struct topology *topo);

struct node* topology_node(const struct topology *topo, uint64_t uuid);

struct datapath* topology_datapath_by_dpid(const struct topology *topo,
                                             uint64_t dp_id);

struct topology* from_json(char *json_file);
uint64_t device_id_to_host_uuid(struct topology *topo,char * device_id);

/* Get struct members */
uint32_t topology_dps_num(const struct topology *topo);

uint32_t topology_links_num(const struct topology *topo);

uint32_t topology_hosts_num(const struct topology *topo);

struct node* topology_nodes(const struct topology *topo);

void topology_link_del(struct topology *topo,struct node * n,uint32_t port_id);

struct node* uuid_to_node(struct topology *t,uint64_t uuid);

#endif /* TOPOLOGY_H */