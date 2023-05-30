## Openstack Yoga 配置说明

Ubuntu版本：**22.04**

**#**代表在root下，**$**代表在user下

注意配置中网卡名和ip地址要与下面表格中一一对应

### 网络配置

**controller**

|          | 覆盖网络   | 提供者网络    | 管理网络   |
| -------- | ---------- | ------------- | ---------- |
| 控制节点 | 20.20.0.11 | 172.16.41.11  | 10.10.0.11 |
| 网卡     | enp1s0f0   | enp1s0f1      | enp3s0     |
| 网关     | 20.20.0.1  | 172.16.41.254 | 10.10.0.1  |

**compute**

|          | 覆盖网络   | 提供者网络    | 管理网络   |
| -------- | ---------- | ------------- | ---------- |
| 控制节点 | 20.20.0.13 | 172.16.41.13  | 10.10.0.13 |
| 网卡     | enp1s0f1   | enp1s0f0      | enp2s0     |
| 网关     | 20.20.0.1  | 172.16.41.254 | 10.10.0.1  |



#### 编辑控制节点、计算节点的/etc/hosts为：

```
127.0.0.1    localhost
10.10.0.11 controller
```

 

### 配置NTP（网络时间协议）：

#### 控制节点 

```
apt install chrony

#编辑/etc/chrony/chrony.conf，添加以下内容
server ntp.aliyun.com iburst
allow 10.10.0.0/24

#重启 NTP 服务
service chrony restart
apt install python3-openstackclient
```

#### 计算节点

```
apt install chrony

#编辑/etc/chrony/chrony.conf，添加以下内容
server controller iburst

#重启 NTP 服务
service chrony restart
apt install python3-openstackclient
apt install nova-compute
```



### 配置SQL 数据库：

#### 控制节点

```
apt install mariadb-server python3-pymysql

#创建和编辑/etc/mysql/mariadb.conf.d/99-openstack.cnf
[mysqld]
bind-address = 10.10.0.11
default-storage-engine = innodb
innodb_file_per_table = on
max_connections = 4096
collation-server = utf8_general_ci
character-set-server = utf8

#完成安装
service mysql restart
mysql_secure_installation
```



### 配置消息队列：

#### 控制节点

```
apt install rabbitmq-server
rabbitmqctl add_user openstack sdn123456
rabbitmqctl set_permissions openstack ".*" ".*" ".*"
```



### 配置内存缓存

#### 控制节点

```
apt install memcached python3-memcache

#编辑/etc/memcached.conf，修改：
-l 10.10.0.11

#重启 Memcached 服务
service memcached restart
```



### 配置Etcd

#### 控制节点

```
apt install etcd

#编辑/etc/default/etcd，添加：
ETCD_NAME="controller"
ETCD_DATA_DIR="/var/lib/etcd"
ETCD_INITIAL_CLUSTER_STATE="new"
ETCD_INITIAL_CLUSTER_TOKEN="etcd-cluster-01"
ETCD_INITIAL_CLUSTER="controller=http://10.10.0.11:2380"
ETCD_INITIAL_ADVERTISE_PEER_URLS="http://10.10.0.11:2380"
ETCD_ADVERTISE_CLIENT_URLS="http://10.10.0.11:2379"
ETCD_LISTEN_PEER_URLS="http://0.0.0.0:2380"
ETCD_LISTEN_CLIENT_URLS="http://10.10.0.11:2379"

#启用并重启 etcd 服务
systemctl enable etcd
systemctl restart etcd
```



### Keystone

#### 控制节点

```
#配置数据库
mysql
MariaDB [(none)]> CREATE DATABASE keystone;
MariaDB [(none)]> GRANT ALL PRIVILEGES ON keystone.* TO 'keystone'@'localhost' \
IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON keystone.* TO 'keystone'@'%' \
IDENTIFIED BY 'sdn123456';
exit

#安装和配置组件
apt install keystone

#编辑/etc/keystone/keystone.conf
[database]
connection = mysql+pymysql://keystone:sdn123456@controller/keystone
[token]
provider = fernet

#填充身份服务数据库
su -s /bin/sh -c "keystone-manage db_sync" keystone

#初始化 Fernet 密钥存储库
keystone-manage fernet_setup --keystone-user keystone --keystone-group keystone
keystone-manage credential_setup --keystone-user keystone --keystone-group keystone

#引导身份服务
keystone-manage bootstrap --bootstrap-password sdn123456 \
 --bootstrap-admin-url http://controller:5000/v3/ \
 --bootstrap-internal-url http://controller:5000/v3/ \
 --bootstrap-public-url http://controller:5000/v3/ \
 --bootstrap-region-id RegionOne

#编辑/etc/apache2/apache2.conf，添加
ServerName controller

#重启 Apache 服务
service apache2 restart

$设置临时环境变量
export OS_USERNAME=admin
export OS_PASSWORD=sdn123456
export OS_PROJECT_NAME=admin
export OS_USER_DOMAIN_NAME=Default
export OS_PROJECT_DOMAIN_NAME=Default
export OS_AUTH_URL=http://controller:5000/v3
export OS_IDENTITY_API_VERSION=3

$创建域、项目、用户和角色
openstack domain create --description "An Example Domain" example
openstack project create --domain default \
 --description "Service Project" service
openstack project create --domain default \
 --description "Demo Project" myproject
openstack user create --domain default \
 --password-prompt myuser
openstack role create myrole
openstack role add --project myproject --user myuser myrole

unset OS_AUTH_URL OS_PASSWORD
openstack --os-auth-url http://controller:5000/v3 \
 --os-project-domain-name Default --os-user-domain-name Default \
 --os-project-name admin --os-username admin token issue
openstack --os-auth-url http://controller:5000/v3 \
 --os-project-domain-name Default --os-user-domain-name Default \
 --os-project-name myproject --os-username myuser token issue
 
$创建脚本
$编辑admin-openrc
export OS_PROJECT_DOMAIN_NAME=Default
export OS_USER_DOMAIN_NAME=Default
export OS_PROJECT_NAME=admin
export OS_USERNAME=admin
export OS_PASSWORD=sdn123456
export OS_AUTH_URL=http://controller:5000/v3
export OS_IDENTITY_API_VERSION=3
export OS_IMAGE_API_VERSION=2

$编辑demo-openrc
export OS_PROJECT_DOMAIN_NAME=Default
export OS_USER_DOMAIN_NAME=Default
export OS_PROJECT_NAME=myproject
export OS_USERNAME=myuser
export OS_PASSWORD=sdn123456
export OS_AUTH_URL=http://controller:5000/v3
export OS_IDENTITY_API_VERSION=3
export OS_IMAGE_API_VERSION=2
```

 

### Glance

#### 控制节点

```
#配置数据库
mysql
MariaDB [(none)]> CREATE DATABASE glance;
MariaDB [(none)]> GRANT ALL PRIVILEGES ON glance.* TO 'glance'@'localhost' \
 IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON glance.* TO 'glance'@'%' \
 IDENTIFIED BY 'sdn123456';
exit

$创建服务
. admin-openrc
openstack user create --domain default --password-prompt glance
openstack role add --project service --user glance admin
openstack service create --name glance \
 --description "OpenStack Image" image
openstack endpoint create --region RegionOne \
 image public http://controller:9292
openstack endpoint create --region RegionOne \
 image internal http://controller:9292
openstack endpoint create --region RegionOne \
 image admin http://controller:9292
openstack role add --user MY_SERVICE --user-domain Default --system all reader

#
apt install glance

#编辑/etc/glance/glance-api.conf
[database]
connection = mysql+pymysql://glance:sdn123456@controller/glance

[keystone_authtoken]
www_authenticate_uri = http://controller:5000
auth_url = http://controller:5000
memcached_servers = controller:11211
auth_type = password
project_domain_name = Default
user_domain_name = Default
project_name = service
username = glance
password = sdn123456

[paste_deploy]
flavor = keystone

[glance_store]
stores = file,http
default_store = file
filesystem_store_datadir = /var/lib/glance/images/

[oslo_limit]
auth_url = http://controller:5000
auth_type = password
user_domain_id = default
username = MY_SERVICE
system_scope = all
password = MY_PASSWORD
endpoint_id = ENDPOINT_ID
region_name = RegionOne

[DEFAULT]
use_keystone_quotas = True

#填充图像服务数据库
su -s /bin/sh -c "glance-manage db_sync" glance
service glance-api restart

$
. admin-openrc
wget http://download.cirros-cloud.net/0.4.0/cirros-0.4.0-x86_64-disk.img
glance image-create --name "cirros" \
 --file cirros-0.4.0-x86_64-disk.img \
 --disk-format qcow2 --container-format bare \
 --visibility=public
```

 

### Placement

#### 控制节点

```
# 配置数据库
mysql
MariaDB [(none)]> CREATE DATABASE placement;
MariaDB [(none)]> GRANT ALL PRIVILEGES ON placement.* TO 'placement'@'localhost' \
 IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON placement.* TO 'placement'@'%' \
 IDENTIFIED BY 'sdn123456';
exit

$
. admin-openrc
openstack user create --domain default --password-prompt placement
openstack role add --project service --user placement admin
openstack service create --name placement \
 --description "Placement API" placement
openstack endpoint create --region RegionOne \
 placement public http://controller:8778
openstack endpoint create --region RegionOne \
 placement internal http://controller:8778
openstack endpoint create --region RegionOne \
 placement admin http://controller:8778

#
apt install placement-api

#编辑/etc/placement/placement.conf
[placement_database]
connection = mysql+pymysql://placement:sdn123456@controller/placement

[api]
auth_strategy = keystone

[keystone_authtoken]
auth_url = http://controller:5000/v3
memcached_servers = controller:11211
auth_type = password
project_domain_name = Default
user_domain_name = Default
project_name = service
username = placement
password = sdn123456

#
su -s /bin/sh -c "placement-manage db sync" placement
service apache2 restart
pip3 install osc-placement
```

 

### Nova

#### 控制节点

```
# 配置数据库
mysql
MariaDB [(none)]> CREATE DATABASE nova_api;
MariaDB [(none)]> CREATE DATABASE nova;
MariaDB [(none)]> CREATE DATABASE nova_cell0;
MariaDB [(none)]> GRANT ALL PRIVILEGES ON nova_api.* TO 'nova'@'localhost' \
 IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON nova_api.* TO 'nova'@'%' \
 IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON nova.* TO 'nova'@'localhost' \
 IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON nova.* TO 'nova'@'%' \
 IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON nova_cell0.* TO 'nova'@'localhost' \
 IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON nova_cell0.* TO 'nova'@'%' \
 IDENTIFIED BY 'sdn123456';
exit

$
. admin-openrc
openstack user create --domain default --password-prompt nova
openstack role add --project service --user nova admin
openstack service create --name nova \
 --description "OpenStack Compute" compute
openstack endpoint create --region RegionOne \
 compute public http://controller:8774/v2.1
openstack endpoint create --region RegionOne \
 compute internal http://controller:8774/v2.1
openstack endpoint create --region RegionOne \
 compute admin http://controller:8774/v2.1

#
apt install nova-api nova-conductor nova-novncproxy nova-scheduler

#编辑/etc/nova/nova.conf
[api_database]
connection = mysql+pymysql://nova:sdn123456@controller/nova_api

[database]
connection = mysql+pymysql://nova:sdn123456@controller/nova

[api]
auth_strategy = keystone

[keystone_authtoken]
www_authenticate_uri = http://controller:5000/
auth_url = http://controller:5000/
memcached_servers = controller:11211
auth_type = password
project_domain_name = Default
user_domain_name = Default
project_name = service
username = nova
password = sdn123456

[DEFAULT]
#删除log_dir
my_ip = 10.10.0.11

[scheduler]
discover_hosts_in_cells_interval = 300

[vnc]
enabled = true
server_listen = $my_ip
server_proxyclient_address = $my_ip

[glance]
api_servers = http://controller:9292

[oslo_concurrency]
lock_path = /var/lib/nova/tmp

[placement]
region_name = RegionOne
project_domain_name = Default
project_name = service
auth_type = password
user_domain_name = Default
auth_url = http://controller:5000/v3
username = placement
password = sdn123456

#
su -s /bin/sh -c "nova-manage api_db sync" nova
su -s /bin/sh -c "nova-manage cell_v2 map_cell0" nova
su -s /bin/sh -c "nova-manage cell_v2 create_cell --name=cell1 --verbose" nova
su -s /bin/sh -c "nova-manage db sync" nova
service nova-api restart
service nova-scheduler restart
service nova-conductor restart
service nova-novncproxy restart
```

#### 计算节点

```
#
apt install nova-compute

#编辑/etc/nova/nova.conf
[DEFAULT]
transport_url = rabbit://openstack:sdn123456@controller
my_ip = 10.10.0.13

[api]
auth_strategy = keystone

[keystone_authtoken]
www_authenticate_uri = http://controller:5000/
auth_url = http://controller:5000/
memcached_servers = controller:11211
auth_type = password
project_domain_name = Default
user_domain_name = Default
project_name = service
username = nova
password = sdn123456

[vnc]
enabled = true
server_listen = 0.0.0.0
server_proxyclient_address = $my_ip
novncproxy_base_url = http://controller:6080/vnc_auto.html

[glance]
api_servers = http://controller:9292

[oslo_concurrency]
lock_path = /var/lib/nova/tmp

[placement]
region_name = RegionOne
project_domain_name = Default
project_name = service
auth_type = password
user_domain_name = Default
auth_url = http://controller:5000/v3
username = placement
password = sdn123456

#
service nova-compute restart
```

```
$以下操作在控制节点上进行
. admin-openrc
openstack compute service list --service nova-compute

#
su -s /bin/sh -c "nova-manage cell_v2 discover_hosts --verbose" nova
```



### Neutron

#### 控制节点

```
# 配置数据库
mysql -u root -p
MariaDB [(none)] CREATE DATABASE neutron;
MariaDB [(none)]> GRANT ALL PRIVILEGES ON neutron.* TO 'neutron'@'localhost' \
 IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON neutron.* TO 'neutron'@'%' \
 IDENTIFIED BY 'sdn123456';
exit

$
. admin-openrc
openstack user create --domain default --password-prompt neutron
openstack role add --project service --user neutron admin
openstack service create --name neutron \
 --description "OpenStack Networking" network
openstack endpoint create --region RegionOne \
 network public http://controller:9696
openstack endpoint create --region RegionOne \
 network internal http://controller:9696
openstack endpoint create --region RegionOne \
 network admin http://controller:9696

#
apt install neutron-server neutron-plugin-ml2 \
 neutron-linuxbridge-agent neutron-l3-agent neutron-dhcp-agent \
 neutron-metadata-agent

#编辑/etc/neutron/neutron.conf
[DEFAULT]
core_plugin = ml2
service_plugins = router
allow_overlapping_ips = true
transport_url = rabbit://openstack:sdn123456@controller
auth_strategy = keystone
notify_nova_on_port_status_changes = true
notify_nova_on_port_data_changes = true

[database]
connection = mysql+pymysql://neutron:sdn123456@controller/neutron

[keystone_authtoken]
www_authenticate_uri = http://controller:5000
auth_url = http://controller:5000
memcached_servers = controller:11211
auth_type = password
project_domain_name = default
user_domain_name = default
project_name = service
username = neutron
password = sdn123456

[nova]
auth_url = http://controller:5000
auth_type = password
project_domain_name = default
user_domain_name = default
region_name = RegionOne
project_name = service
username = nova
password = sdn123456

[oslo_concurrency]
lock_path = /var/lib/neutron/tmp

#编辑/etc/neutron/plugins/ml2/ml2_conf.ini
[ml2]
type_drivers = flat,vlan,vxlan
tenant_network_types = vxlan
mechanism_drivers = linuxbridge,l2population
extension_drivers = port_security

[ml2_type_flat]
flat_networks = provider

[ml2_type_vxlan]
vni_ranges = 1:1000

[securitygroup]
enable_ipset = true

#编辑/etc/neutron/plugins/ml2/linuxbridge_agent.ini
[linux_bridge]
physical_interface_mappings = provider:enp1s0f1

[vxlan]
enable_vxlan = true
local_ip = 20.20.0.11
l2_population = true

[securitygroup]
enable_security_group = true
firewall_driver = neutron.agent.linux.iptables_firewall.IptablesFirewallDriver

#编辑/etc/neutron/l3_agent.ini
[DEFAULT]
interface_driver = linuxbridge

#编辑/etc/neutron/dhcp_agent.ini
[DEFAULT]
interface_driver = linuxbridge
dhcp_driver = neutron.agent.linux.dhcp.Dnsmasq
enable_isolated_metadata = true

#编辑/etc/neutron/metadata_agent.ini
[DEFAULT]
nova_metadata_host = controller
metadata_proxy_shared_secret = sdn123456

#编辑/etc/nova/nova.conf
[neutron]
auth_url = http://controller:5000
auth_type = password
project_domain_name = default
user_domain_name = default
region_name = RegionOne
project_name = service
username = neutron
password = sdn123456
service_metadata_proxy = true
metadata_proxy_shared_secret = sdn123456

#
su -s /bin/sh -c "neutron-db-manage --config-file /etc/neutron/neutron.conf \
 --config-file /etc/neutron/plugins/ml2/ml2_conf.ini upgrade head" neutron
service nova-api restart
service neutron-server restart
service neutron-linuxbridge-agent restart
service neutron-dhcp-agent restart
service neutron-metadata-agent restart
service neutron-l3-agent restart
```

#### 计算节点

```
#
apt install neutron-linuxbridge-agent

#编辑/etc/neutron/neutron.conf
[database]
#注释掉connection

[DEFAULT]
transport_url = rabbit://openstack:sdn123456@controller
auth_strategy = keystone

[keystone_authtoken]
www_authenticate_uri = http://controller:5000
auth_url = http://controller:5000
memcached_servers = controller:11211
auth_type = password
project_domain_name = default
user_domain_name = default
project_name = service
username = neutron
password = sdn123456

[oslo_concurrency]
lock_path = /var/lib/neutron/tmp

#编辑/etc/neutron/plugins/ml2/linuxbridge_agent.ini
[linux_bridge]
physical_interface_mappings = provider:enp1s0f0

[vxlan]
enable_vxlan = true
local_ip = 20.20.0.13
l2_population = true

[securitygroup]
enable_security_group = true
firewall_driver = neutron.agent.linux.iptables_firewall.IptablesFirewallDriver

#编辑/etc/nova/nova.conf
[neutron]
auth_url = http://controller:5000
auth_type = password
project_domain_name = default
user_domain_name = default
region_name = RegionOne
project_name = service
username = neutron
password = sdn123456

#
service nova-compute restart
service neutron-linuxbridge-agent restart
```

 

### 创建虚拟网络

#### 控制节点

```
$
. admin-openrc
openstack network create  --share --external \
 --provider-physical-network provider \
 --provider-network-type flat provider
openstack subnet create --network provider \
 --allocation-pool start=172.16.41.200,end=172.16.41.230 \
 --dns-nameserver 8.8.8.8 --gateway 172.16.41.254 \
 --subnet-range 172.16.41.0/24 provider

. demo-openrc
openstack network create selfservice
openstack subnet create --network selfservice \
 --dns-nameserver 8.8.8.8 --gateway 30.0.0.1 \
 --subnet-range 30.0.0.0/8 selfservice
openstack router create router
openstack router add subnet router selfservice
openstack router set router --external-gateway provider

. admin-openrc
openstack flavor create --id 0 --vcpus 1 --ram 64 --disk 1 m1.nano

. demo-openrc
ssh-keygen -q -N ""
openstack keypair create --public-key mykey.pub mykey
openstack security group rule create --proto icmp default
openstack security group rule create --proto tcp --dst-port 22 default

$启动实例
. demo-openrc
openstack server create --flavor m1.nano --image cirros \
 --nic net-id=3a654808-87d4-41a2-a096-bac9c12b177c  --security-group default \
 --key-name mykey selfservice-instance
openstack server create --flavor m1.nano --image cirros \
 --nic net-id=ca7e29c0-360e-47e9-8450-3fe0b5ca5772  --security-group default \
 --key-name mykey provider-instance

$检查实例的状态
openstack server list
```

 

### OVS-Selfservice

#### 控制节点

```
#
systemctl disable neutron-linuxbridge-agent
systemctl stop neutron-linuxbridge-agent
apt install neutron-openvswitch-agent

#/etc/neutron/plugins/ml2/ml2_conf.ini
[ml2]
type_drivers = flat,vlan,vxlan
tenant_network_types = vxlan
mechanism_drivers = openvswitch,l2population
extension_drivers = port_security

[ml2_type_vxlan]
vni_ranges = 1:1000

[securitygroup]
enable_ipset = true
service neutron-server restart
service neutron-openvswitch-agent restart
```

#### 计算节点

```
apt install neutron-openvswitch-agent
apt install neutron-dhcp-agent
apt install neutron-l3-agent

#修改/etc/neutron/plugins/ml2/ml2_conf.ini
[DEFAULT]

[ml2]
type_drivers = flat,vlan,vxlan
tenant_network_types = vxlan
mechanism_drivers = openvswitch,l2population
extension_drivers = port_security

[ml2_type_flat]
flat_networks = provider

[ml2_type_vxlan]
vni_ranges = 1:1000

[securitygroup]
enable_ipset = true

#修改 /etc/neutron/l3_agent.ini
[DEFAULT]
interface_driver = neutron.agent.linux.interface.OVSInterfaceDriver
external_network_bridge = br-ex

#修改/etc/neutron/dhcp_agent.ini
[DEFAULT]
interface_driver=neutron.agent.linux.interface.OVSInterfaceDriver
dhcp_driver = neutron.agent.linux.dhcp.Dnsmasq
enable_isolated_metadata = true

#修改/etc/neutron/plugins/ml2/openvswitch_agent.ini
[ovs]
integration_bridge = br-int
tunnel_bridge = br-tun
local_ip = ~~172.16.41.6~~20.20.0.13
#bridge_mappings = br-ex
bridge_mappings = provider:br-ex
#bridge_mappings = 

[agent]
tunnel_types = vxlan,gre
l2_population = true
arp_responder = true

[securitygroup]
enable_security_group = true
firewall_driver = neutron.agent.linux.iptables_firewall.OVSHybridIptablesFirewallDriver

#
service neutron-openvswitch-agent restart
ovs-vsctl add-br br-ex
ovs-vsctl add-port br-ex enp1s0f0
ifconfig enp1s0f0 0.0.0.0 
ifconfig br-ex 172.16.41.13/24
route add default gw 172.16.41.254

service neutron-dhcp-agent restart
service neutron-metadata-agent restart
service neutron-l3-agent restart
service neutron-openvswitch-agent restart
service nova-compute restart
```

```
$controller上创建实例
. demo-openrc
openstack network create selfservice2
openstack subnet create --network selfservice2 \
 --dns-nameserver 8.8.8.8 --gateway 10.10.3.1 \
 --subnet-range 10.10.3.0/24 selfservice2
openstack router create router2
openstack router add subnet router selfservice2
openstack router set router2 --external-gateway provider
openstack server create --flavor m1.nano --image cirros \
 --nic net-id=abae106a-46ad-46b6-ba8a-6fd3176728b8  --security-group d051406e-6083-45ef-a6d6-fc5e3c016dc8  \
  selfservice-instance-ovs1
openstack server create --flavor m1.nano --image cirros  --nic net-id=09db42e4-548d-472e-8d2c-642b6c696c38  --availability-zone nova:compute-node:compute-node selfservice-instance-ovs1
openstack server create --flavor m1.nano --image cirros  --nic net-id=4bd3c4c1-10a4-4638-9c02-b7fa4f82dd44  --availability-zone nova:compute-node-2:compute-node-2 selfservice-instance-ovs2

$检查实例的状态
openstack server list
openstack security group rule create --proto icmp bfeda05d-6da5-443b-ae2a-12998aaac7ed
openstack security group rule create --proto tcp --dst-port 22 bfeda05d-6da5-443b-ae2a-12998aaac7ed
 
```





```
$在controller上删除网络
1.列出所有路由设备，获得要删除的路由器id
$ neutron router-list
2.根据路由器id，删除其网关端口
$ neutron router-gateway-clear <router_id>
3.根据路由设备id，列出其其他端口
$ neutron router-port-list <router_id>
4.根据路由器id与subnet_id删除路由器其他端口
$ neutron router-interface-delete <router-id> <subnet-id>
5.最后删除路由
$ neutron router-delete <router-id>
6.列出所有使用网络的端口
$ neutron port-list
7.删除端口
$ neutron port-delete <port_id>
8.删除子网,列出所有子网，获得要删除子网的subnet_id
$ neutron subnet-list
9.根据获得的网络id，删除子网
$ neutron subnet-delete <subnet-id>
10.根据网络id，删除网络
$ openstack network list
$ openstack network delete <net-id>
```

