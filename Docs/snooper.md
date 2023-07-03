1. 给容器ovs创建镜像端口及将端口数据挂载传输到外部的linux bridge上面

```
root@worker1:/home/sdn# ip link add veth-bs-h4 type veth peer name veth-h4-bs
root@worker1:/home/sdn# ip link set veth-bs-h4 up
root@worker1:/home/sdn# brctl addif br-snooper veth-bs-h4
root@worker1:/home/sdn# docker ps -a
CONTAINER ID   IMAGE     COMMAND   CREATED      STATUS      PORTS     NAMES
1b1d0488e645   ubuntu    "bash"    5 days ago   Up 5 days             h4
d0d79d26c21d   switch    "bash"    5 days ago   Up 5 days             s4
e89d00cc90cf   switch    "bash"    5 days ago   Up 5 days             s2
0148716c3f95   ubuntu    "bash"    5 days ago   Up 5 days             h2
root@worker1:/home/sdn# docker inspect -f '{{.State.Pid}}' 1b1d0488e645
255065
root@worker1:/home/sdn# ip link set veth-h4-bs netns 255065
root@worker1:/home/sdn#
root@worker1:/home/sdn# docker exec -it h4 bash
```

```
root@h4:/# ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
2: ovs-system: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether 16:d2:b8:a7:e7:29 brd ff:ff:ff:ff:ff:ff
3: br-ex: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UNKNOWN group default qlen 1000
    link/ether f6:0e:58:9f:17:46 brd ff:ff:ff:ff:ff:ff
    inet 20.20.0.7/16 brd 20.20.255.255 scope global br-ex
       valid_lft forever preferred_lft forever
4: br-tun: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether 16:07:a4:cf:e2:49 brd ff:ff:ff:ff:ff:ff
5: br-int: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether ae:cf:7d:46:63:41 brd ff:ff:ff:ff:ff:ff
6: vxlan-1: <BROADCAST,MULTICAST> mtu 1450 qdisc noqueue state DOWN group default qlen 1000
    link/ether 6a:de:3d:fe:db:48 brd ff:ff:ff:ff:ff:ff
8: vxlan_sys_4789: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 65000 qdisc noqueue master ovs-system state UNKNOWN group default qlen 1000
    link/ether de:8d:b2:9e:61:c6 brd ff:ff:ff:ff:ff:ff
307: eth0@if308: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master ovs-system state UP group default
    link/ether 02:42:14:14:00:07 brd ff:ff:ff:ff:ff:ff link-netnsid 0
316: admin@if315: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default qlen 1000
    link/ether ea:d1:23:dd:13:52 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 192.168.1.7/16 brd 192.168.255.255 scope global admin
       valid_lft forever preferred_lft forever
331: h4-eth0@if330: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc htb state UP group default qlen 1000
    link/ether 1a:fd:12:70:4d:86 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.10.0.7/16 brd 10.10.255.255 scope global h4-eth0
       valid_lft forever preferred_lft forever
366: veth-h4-bs@if367: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN group default qlen 1000
    link/ether 7a:e6:eb:ea:7f:6e brd ff:ff:ff:ff:ff:ff link-netnsid 0
root@h4:/# ip link set veth-h4-bs up
root@h4:/# ovs-vsctl show
481e162e-a8c9-4676-9db0-57798006848d
    Manager "ptcp:6640:127.0.0.1"
        is_connected: true
    Bridge br-int
        Controller "tcp:127.0.0.1:6633"
            is_connected: true
        fail_mode: secure
        datapath_type: system
        Port patch-tun
            Interface patch-tun
                type: patch
                options: {peer=patch-int}
        Port tap6ee17364-43
            tag: 1
            Interface tap6ee17364-43
                type: internal
        Port tap8a7b2b3d-19
            tag: 1
            Interface tap8a7b2b3d-19
                type: internal
        Port int-br-ex
            Interface int-br-ex
                type: patch
                options: {peer=phy-br-ex}
        Port tapd6def79b-25
            tag: 1
            Interface tapd6def79b-25
                type: internal
        Port br-int
            Interface br-int
                type: internal
    Bridge br-ex
        Controller "tcp:127.0.0.1:6633"
            is_connected: true
        fail_mode: secure
        datapath_type: system
        Port phy-br-ex
            Interface phy-br-ex
                type: patch
                options: {peer=int-br-ex}
        Port eth0
            Interface eth0
        Port br-ex
            Interface br-ex
                type: internal
    Bridge br-tun
        Controller "tcp:127.0.0.1:6633"
            is_connected: true
        fail_mode: secure
        datapath_type: system
        Port vxlan-c0a80105
            Interface vxlan-c0a80105
                type: vxlan
                options: {df_default="true", egress_pkt_mark="0", in_key=flow, local_ip="192.168.1.7", out_key=flow, remote_ip="192.168.1.5"}
        Port br-tun
            Interface br-tun
                type: internal
        Port vxlan-c0a80106
            Interface vxlan-c0a80106
                type: vxlan
                options: {df_default="true", egress_pkt_mark="0", in_key=flow, local_ip="192.168.1.7", out_key=flow, remote_ip="192.168.1.6"}
        Port patch-int
            Interface patch-int
                type: patch
                options: {peer=patch-tun}
    ovs_version: "2.17.2"
root@h4:/#

root@h4:/# ovs-vsctl add-port br-tun veth-h4-bs

root@h4:/# ovs-vsctl -- set Bridge br-tun mirrors=@m -- --id=@veth-h4-bs get Port veth-h4-bs -- --id=@patch-int get Port patch-int -- --id=@m create Mirror name=mymirror select-dst-port=@patch-int select-src-port=@patch-int output-port=@veth-h4-bs select_all=1
dae1e910-9fdf-40fd-aad4-fe1fc51e06c5
root@h4:/#
root@h4:/# ovs-vsctl list mirror
_uuid               : dae1e910-9fdf-40fd-aad4-fe1fc51e06c5
external_ids        : {}
name                : mymirror
output_port         : 972e67dd-4397-497f-bf2d-74d9b550545a
output_vlan         : []
select_all          : true
select_dst_port     : [f5cc3e6b-d1d7-418b-a2d1-91bd26ba8270]
select_src_port     : [f5cc3e6b-d1d7-418b-a2d1-91bd26ba8270]
select_vlan         : []
snaplen             : []
statistics          : {tx_bytes=0, tx_packets=0}
root@h4:/#
```

2. 宿主机创建linux bridge，并且使用vxlan拉通为一个L2的网络

```
root@worker2:/home/sdn# brctl addbr br-snooper
root@worker2:/home/sdn# ip link set br-snooper up
root@worker2:/home/sdn# ip link add vx_snooper type vxlan id 10 remote 10.10.0.14 local 10.10.0.15 dstport 4789
root@worker2:/home/sdn# ip link set up vx_snooper
root@worker2:/home/sdn# brctl addif br-snooper vx_snooper

root@worker1:/home/sdn# ip link add vx_snooper type vxlan id 10 remote 10.10.0.15 local 10.10.0.14 dstport 4789
root@worker1:/home/sdn# ip link set up vx_snooper
root@worker1:/home/sdn# brctl addif br-snooper vx_snooper
```

3. 创建tap-snooper，挂载在桥上，并插入到snooper容器中

```
root@worker1:/home/sdn# tunctl -t tap-snooper
Set 'tap-snooper' persistent and owned by uid 0
root@worker1:/home/sdn#
root@worker1:/home/sdn# ip link list | grep tap-
375: tap-snooper: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT group default qlen 1000
root@worker1:/home/sdn# ip link set tap-snooper up
root@worker1:/home/sdn# docker ps -a
CONTAINER ID   IMAGE     COMMAND   CREATED       STATUS       PORTS     NAMES
510e1caed213   ubuntu    "bash"    5 days ago    Up 5 days              snooper
1b1d0488e645   ubuntu    "bash"    13 days ago   Up 13 days             h4
d0d79d26c21d   switch    "bash"    13 days ago   Up 13 days             s4
e89d00cc90cf   switch    "bash"    13 days ago   Up 13 days             s2
0148716c3f95   ubuntu    "bash"    13 days ago   Up 13 days             h2
root@worker1:/home/sdn# brctl show
bridge name     bridge id               STP enabled     interfaces
admin-br                8000.6a017aa04c75       no              intf2
                                                        intf4
                                                        intf6
                                                        intf8
                                                        vx_21
br-c7ceabdd2921         8000.0242d9095130       no              enp1s0f0
                                                        veth079560f
                                                        vetha53eacb
br-snooper              8000.aa4b1e476bd0       no              veth-bs-h2
                                                        veth-bs-h4
                                                        veth-bs-sn
                                                        vx_snooper
docker0         8000.0242467b67f3       no              veth4298786
intf14          8000.fe1deb6b4c32       no              intf13
                                                        vintf14
                                                        vx_01
intf16          8000.a6fa1c0e985d       no              intf15
                                                        vintf16
intf22          8000.6a1d936b5309       no              intf21
                                                        vintf22
intf24          8000.ca1480508a44       no              intf23
                                                        vintf24
intf26          8000.022a1680daef       no              intf25
                                                        vx_26
intf32          8000.32929a48f878       no              intf31
                                                        vx_27
intf34          8000.be8600bac0bc       no              intf33
                                                        vx_28
root@worker1:/home/sdn#
root@worker1:/home/sdn# brctl addif br-snooper tap-snooper
root@worker1:/home/sdn# docker inspect -f '{{.State.Pid}}' snooper
2473120



```