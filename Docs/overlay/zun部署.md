# ZUN部署

系统：ubuntu22.04；以下所有组件只要涉及密码的都是sdn123456

## 安装docker

### 计算节点

选择旧版本

地址：https://docs.docker.com/engine/install/ubuntu/

```bash
sudo apt-get update
sudo apt-get install ca-certificates curl gnupg
sudo install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
sudo chmod a+r /etc/apt/keyrings/docker.gpg
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update

apt-cache madison docker-ce | awk '{ print $3 }'
VERSION_STRING=5:20.10.24~3-0~ubuntu-jammy
sudo apt-get install docker-ce=$VERSION_STRING docker-ce-cli=$VERSION_STRING containerd.io docker-buildx-plugin docker-compose-plugin
```

## 安装Kuryr_libnetwork

### 控制节点

Source the `admin` credentials to gain access to admin-only CLI commands:

```
$
. admin-openrc
```

Create the `kuryr` user

```
$
openstack user create --domain default --password-prompt kuryr
```

Add the `admin` role to the `kuryr` user:

```
$
openstack role add --project service --user kuryr admin
```

### 计算节点

确保已经安装了旧版本docker

Create kuryr user and necessary directories:

Create user:

```
#
groupadd --system kuryr
useradd --home-dir "/var/lib/kuryr" \
      --create-home \
      --system \
      --shell /bin/false \
      -g kuryr \
      kuryr
```

Create directories:

```
#
mkdir -p /etc/kuryr
chown kuryr:kuryr /etc/kuryr
```

Clone and install kuryr-libnetwork:

```
#
apt-get install python3-pip
cd /var/lib/kuryr
git clone -b master https://opendev.org/openstack/kuryr-libnetwork.git
chown -R kuryr:kuryr kuryr-libnetwork
cd kuryr-libnetwork
pip3 install -r requirements.txt
python3 setup.py install
```

如果出现报错：

```
python3 setup.py install

报错：error in setup command: Error parsing /var/lib/kuryr/kuryr-libnetwork/setup.cfg: Exception: Versioning for this project requires either an sdist tarball, or access to an upstream git repository. It's also possible that there is a mismatch between the package name in setup.cfg and the argument given to pbr.version.VersionInfo. Project name kuryr-libnetwork was given, but was not able to be found.

解决方法：
git log
git config --global --add safe.directory /var/lib/kuryr/kuryr-libnetwork
```

Generate a sample configuration file:

```
#
su -s /bin/sh -c "./tools/generate_config_file_samples.sh" kuryr
su -s /bin/sh -c "cp etc/kuryr.conf.sample \
      /etc/kuryr/kuryr.conf" kuryr
```

Edit the `/etc/kuryr/kuryr.conf`:

```
[DEFAULT]
...
bindir = /usr/local/libexec/kuryr
[neutron]
...
www_authenticate_uri = http://controller:5000
auth_url = http://controller:5000
username = kuryr
user_domain_name = default
password = sdn123456
project_name = service
project_domain_name = default
auth_type = password
```

Create an upstart config `/etc/systemd/system/kuryr-libnetwork.service`:

```
[Unit]
Description = Kuryr-libnetwork - Docker network plugin for Neutron

[Service]
ExecStart = /usr/local/bin/kuryr-server --config-file /etc/kuryr/kuryr.conf
CapabilityBoundingSet = CAP_NET_ADMIN
AmbientCapabilities = CAP_NET_ADMIN

[Install]
WantedBy = multi-user.target
```

Finalize installation

1. Enable and start the kuryr-libnetwork service:

   ```
   #
   systemctl enable kuryr-libnetwork
   systemctl start kuryr-libnetwork
   ```

2. After Kuryr starts, please restart your Docker service:

   ```
   #
   systemctl restart docker
   ```

## 安装zun

官网：https://docs.openstack.org/zun/latest/install/

### 控制节点

Use the database access client to connect to the database server as the `root` user:

```
#
mysql
MariaDB [(none)] CREATE DATABASE zun;
MariaDB [(none)]> GRANT ALL PRIVILEGES ON zun.* TO 'zun'@'localhost' \
  IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> GRANT ALL PRIVILEGES ON zun.* TO 'zun'@'%' \
  IDENTIFIED BY 'sdn123456';
MariaDB [(none)]> exit
```

Source the `admin` credentials to gain access to admin-only CLI commands:

```
$
. admin-openrc
openstack user create --domain default --password-prompt zun
openstack role add --project service --user zun admin
openstack service create --name zun \
    --description "Container Service" container
openstack endpoint create --region RegionOne \
    container public http://controller:9517/v1
openstack endpoint create --region RegionOne \
    container internal http://controller:9517/v1
openstack endpoint create --region RegionOne \
    container admin http://controller:9517/v1
```



Install and configure components

Create zun user and necessary directories:

Create user:

```
#
groupadd --system zun
useradd --home-dir "/var/lib/zun" \
      --create-home \
      --system \
      --shell /bin/false \
      -g zun \
      zun
mkdir -p /etc/zun
chown zun:zun /etc/zun
```

Install the following dependencies:

For Ubuntu, run:

```
#
apt-get install python3-pip git
```

Clone and install zun:

```
#
cd /var/lib/zun
git clone https://opendev.org/openstack/zun.git
chown -R zun:zun zun
git config --global --add safe.directory /var/lib/zun/zun
cd zun
pip3 install -r requirements.txt
python3 setup.py install
```

Generate a sample configuration file:

```
su -s /bin/sh -c "oslo-config-generator \
    --config-file etc/zun/zun-config-generator.conf" zun
su -s /bin/sh -c "cp etc/zun/zun.conf.sample \
    /etc/zun/zun.conf" zun
su -s /bin/sh -c "cp etc/zun/api-paste.ini /etc/zun" zun
```

Edit the `/etc/zun/zun.conf`:

In the `[DEFAULT]` section, configure `RabbitMQ` message queue access:

```
[DEFAULT]
...
transport_url = rabbit://openstack:sdn123456@controller

[api]
...
host_ip = 10.10.0.7
port = 9517

[database]
...
connection = mysql+pymysql://zun:sdn123456@controller/zun

[keystone_auth]
memcached_servers = controller:11211
www_authenticate_uri = http://controller:5000
project_domain_name = default
project_name = service
user_domain_name = default
password = sdn123456
username = zun
auth_url = http://controller:5000
auth_type = password
auth_version = v3
auth_protocol = http
service_token_roles_required = True
endpoint_type = internalURL

[keystone_authtoken]
...
memcached_servers = controller:11211
www_authenticate_uri = http://controller:5000
project_domain_name = default
project_name = service
user_domain_name = default
password = sdn123456
username = zun
auth_url = http://controller:5000
auth_type = password
auth_version = v3
auth_protocol = http
service_token_roles_required = True
endpoint_type = internalURL

[oslo_concurrency]
...
lock_path = /var/lib/zun/tmp

[oslo_messaging_notifications]
...
driver = messaging

[websocket_proxy]
...
wsproxy_host = 10.10.0.7
wsproxy_port = 6784
base_url = ws://controller:6784/
```

Make sure that `/etc/zun/zun.conf` still have the correct permissions. You can set the permissions again with:

```
#
chown zun:zun /etc/zun/zun.conf
```

Populate Zun database:

```
#
su -s /bin/sh -c "zun-db-manage upgrade" zun
```

Finalize installation

Create an upstart config, it could be named as `/etc/systemd/system/zun-api.service`:

```
[Unit]
Description = OpenStack Container Service API

[Service]
ExecStart = /usr/local/bin/zun-api
User = zun

[Install]
WantedBy = multi-user.target
```

Create an upstart config, it could be named as `/etc/systemd/system/zun-wsproxy.service`:

```
[Unit]
Description = OpenStack Container Service Websocket Proxy

[Service]
ExecStart = /usr/local/bin/zun-wsproxy
User = zun

[Install]
WantedBy = multi-user.target
```

Enable and start zun-api and zun-wsproxy:

```
#
systemctl enable zun-api
systemctl enable zun-wsproxy
```

```
#
systemctl start zun-api
systemctl start zun-wsproxy
```

Verify that zun-api and zun-wsproxy services are running:

```
#
systemctl status zun-api
systemctl status zun-wsproxy
```

### 计算节点

Create user:

```
#
groupadd --system zun
useradd --home-dir "/var/lib/zun" \
      --create-home \
      --system \
      --shell /bin/false \
      -g zun \
      zun
mkdir -p /etc/zun
chown zun:zun /etc/zun
mkdir -p /etc/cni/net.d
chown zun:zun /etc/cni/net.d
```

Install dependencies:

For Ubuntu, run:

```
#
apt-get install python3-pip git numactl
```

Clone and install zun:

```
#
cd /var/lib/zun
git clone https://opendev.org/openstack/zun.git
chown -R zun:zun zun
git config --global --add safe.directory /var/lib/zun/zun
cd zun
pip3 install -r requirements.txt
python3 setup.py install
```

Generate a sample configuration file:

```
#
su -s /bin/sh -c "oslo-config-generator \
    --config-file etc/zun/zun-config-generator.conf" zun
su -s /bin/sh -c "cp etc/zun/zun.conf.sample \
    /etc/zun/zun.conf" zun
su -s /bin/sh -c "cp etc/zun/rootwrap.conf \
    /etc/zun/rootwrap.conf" zun
su -s /bin/sh -c "mkdir -p /etc/zun/rootwrap.d" zun
su -s /bin/sh -c "cp etc/zun/rootwrap.d/* \
    /etc/zun/rootwrap.d/" zun
su -s /bin/sh -c "cp etc/cni/net.d/* /etc/cni/net.d/" zun
```

Configure sudoers for `zun` users:

```
#
echo "zun ALL=(root) NOPASSWD: /usr/local/bin/zun-rootwrap \
    /etc/zun/rootwrap.conf *" | sudo tee /etc/sudoers.d/zun-rootwrap
```

Edit the /etc/zun/zun.conf

```
[DEFAULT]
...
transport_url = rabbit://openstack:sdn123456@controller
state_path = /var/lib/zun

[database]
...
connection = mysql+pymysql://zun:sdn123456@controller/zun

[keystone_auth]
memcached_servers = controller:11211
www_authenticate_uri = http://controller:5000
project_domain_name = default
project_name = service
user_domain_name = default
password = sdn123456
username = zun
auth_url = http://controller:5000
auth_type = password
auth_version = v3
auth_protocol = http
service_token_roles_required = True
endpoint_type = internalURL

[keystone_authtoken]
...
memcached_servers = controller:11211
www_authenticate_uri= http://controller:5000
project_domain_name = default
project_name = service
user_domain_name = default
password = sdn123456
username = zun
auth_url = http://controller:5000
auth_type = password

[oslo_concurrency]
...
lock_path = /var/lib/zun/tmp
```

(Optional) If you want to run both containers and nova instances in this compute node, in the `[compute]` section, configure the `host_shared_with_nova`:

```
[compute]
...
host_shared_with_nova = true
```

Make sure that `/etc/zun/zun.conf` still have the correct permissions. You can set the permissions again with:

```
#
chown zun:zun /etc/zun/zun.conf
```

#### Configure Docker and Kuryr

Create the directory `/etc/systemd/system/docker.service.d`

```
#
mkdir -p /etc/systemd/system/docker.service.d
```

Create the file `/etc/systemd/system/docker.service.d/docker.conf`. Configure docker to listen to port 2375 as well as the default unix socket. Also, configure docker to use etcd3 as storage backend:

```
[Service]
ExecStart=
ExecStart=/usr/bin/dockerd --group zun -H tcp://127.0.0.1:2375 -H unix:///var/run/docker.sock --cluster-store etcd://controller:2379
```

Restart Docker:

```
#
systemctl daemon-reload
systemctl restart docker
```

Edit the Kuryr config file `/etc/kuryr/kuryr.conf`. Set `capability_scope` to `global` and `process_external_connectivity` to `False`:

```
[DEFAULT]
...
capability_scope = global
process_external_connectivity = False
```

Restart Kuryr-libnetwork:

```
#
systemctl restart kuryr-libnetwork
```

#### Configure containerd

Generate config file for containerd:

```
#
containerd config default > /etc/containerd/config.toml
```

Edit the `/etc/containerd/config.toml`. In the `[grpc]` section, configure the `gid` as the group ID of the `zun` user:

```
[grpc]
  ...
  gid = ZUN_GROUP_ID
```

用下面的命令得到的值替换ZUN_GROUP_ID

```
#
getent group zun | cut -d: -f3
```

Make sure that `/etc/containerd/config.toml` still have the correct permissions. You can set the permissions again with:

```
#
chown zun:zun /etc/containerd/config.toml
```

Restart containerd:

```
#
systemctl restart containerd
```

#### Configure CNI

Download and install the standard loopback plugin:

```
#
mkdir -p /opt/cni/bin
curl -L https://github.com/containernetworking/plugins/releases/download/v0.7.1/cni-plugins-amd64-v0.7.1.tgz \
      | tar -C /opt/cni/bin -xzvf - ./loopback
```

Install the Zun CNI plugin:

```
#
install -o zun -m 0555 -D /usr/local/bin/zun-cni /opt/cni/bin/zun-cni
```



#### Finalize installation

Create an upstart config for zun compute, it could be named as `/etc/systemd/system/zun-compute.service`:

```
[Unit]
Description = OpenStack Container Service Compute Agent

[Service]
ExecStart = /usr/local/bin/zun-compute
User = zun

[Install]
WantedBy = multi-user.target
```

Create an upstart config for zun cni daemon, it could be named as `/etc/systemd/system/zun-cni-daemon.service`:

```
[Unit]
Description = OpenStack Container Service CNI daemon

[Service]
ExecStart = /usr/local/bin/zun-cni-daemon
User = zun

[Install]
WantedBy = multi-user.target
```

Enable and start zun-compute:

```
#
systemctl enable zun-compute
systemctl start zun-compute
```

Enable and start zun-cni-daemon:

```
#
systemctl enable zun-cni-daemon
systemctl start zun-cni-daemon
```

Verify that zun-compute and zun-cni-daemon services are running:

```
#
systemctl status zun-compute
systemctl status zun-cni-daemon
```

这里如果status zun-compute出现failed的情况，检查/usr/local/bin/zun-compute的用户组和所有者是否为zun

```
#
cd /usr/local/bin
ll
如果不是zun，则解决方法为下：
chown zun:zun zun-compute
```

