#include <inttypes.h>
#include <stddef.h>

// Max size of keyword
#define MAX_KEY_LEN 10

#define NO_KEY "NoKey"
#define DPS_KEY "dps"
#define LINKS_KEY "links"
#define SWITCHX "switchX"
#define SWITCHY "switchY"
#define PORTX "portX" 
#define PORTY "portY"
#define DELAY "delay"
#define BW    "bw"

#define ROUTER_KEY "router"
#define ROUTERX "routerX"
#define ROUTERY "routerY"

#define DP_LIMIT UINT16_MAX
#define LINK_LIMIT 10000
#define HOST_MAX 128
#define ROUTER_MAX 128
#define LINK_MAX 1024
#define PORT_MAX 16
#define NAME_MAX 16

struct link_spec {
    uint64_t switchX;
    uint64_t switchY;
    uint32_t portX;
    uint32_t portY;
    uint32_t delay;
    uint32_t bw;
};

struct link_rt{
    uint64_t rt_x;
    uint64_t rt_y;
    uint32_t portX;
    uint32_t portY;
    uint32_t delay;
    uint32_t bw;
};
/*
struct port_json
{
    uint32_t port_id;
    uint8_t mac_addr[6];
};

struct router_json
{
    //struct port_json  p_json[PORT_MAX];
    uint32_t num_port;
    char name[NAME_MAX];
};

struct host_json
{
    char name[NAME_MAX];
    struct port_json port;
    uint32_t ipv4;
    uint32_t netmask;

};
*/

struct parsed_topology {
    /*
    uint64_t dps[DP_LIMIT];
    size_t ndps;
    struct link_spec links[LINK_LIMIT];
    size_t nlinks;
    */
    size_t num_router;
    uint64_t rt[ROUTER_MAX];
    struct link_rt links_rt[LINK_LIMIT];
    size_t num_links_rt;
    /*
    size_t num_host;
    struct host_json h[HOST_MAX];
    size_t num_link_rt;
    struct link_uuid link_rt[LINK_MAX];
    */
};

enum json_error {
    INVALID_JSON = 1,
    INVALID_KEYWORD = 2,
    INVALID_LINK_DP = 3
};

void parse_topology(char *json, size_t s, struct parsed_topology *ptopo);