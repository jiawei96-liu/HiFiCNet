## HifiCNet搭建流程

### 1.基本开发环境搭建

#### ◦ HifiCNet⾄少需要三台机器，⼀台做client节点，⼀台做master节点，⼀台做worker节点

#### ◦ 安装git python3 pip

```shell
root@client:~# apt update

root@client:~# apt install -y git python3-minimal python3-pip

root@client:~# python3 -m pip install --upgrade pip
```

#### ◦ clone HifiCNet仓库

```shell
root@client:~# cd ~

root@client:~# git clone https://github.com/J980419xq/Distrinet.git
```

#### ◦ 安装HifiCNet

```shell
root@client:~# cd ~/Distrinet

root@client:~/Distrinet# pip3 install -r requirements.txt
这个第一次可能会不行，多重复几次就好了（跟网速有关系）

root@client:~/Distrinet# python3 setup.py install
```



#### ◦ 更新PYTHONPATH，权限更改

```shell
root@client:~# export PYTHONPATH=$PYTHONPATH:mininet:

root@client:~# chmod -R 777 ~/.distrinet
```



#### ◦ 检查是否安装成功

```shell
root@client:~# cd .distrinet/

root@client:~/.distrinet# ls -al
total 20
drwxrwxrwx 2 root root 4096 Feb 6 10:45 .
drwxr-xr-x 8 ubuntu ubuntu 4096 Feb 6 10:45 ..
-rwxrwxrwx 1 root root 1144 Feb 6 10:45 conf.yml
-rwxrwxrwx 1 root root 426 Feb 6 09:54 general_purpose.json
-rwxrwxrwx 1 root root 1375 Feb 6 09:54 gros_partial.json
（文件有就可以，如果大于20没关系）
```



### 2.Client节点配置（目的是保证client可以ssh连接master和worker）

a. 假设在第⼀步后：client ip ：172.16.41.12， master ip ： 172.16.41.16， worker ip ：172.16.41.14

b. ⽣成client节点的ssh key并分别写⼊到所有的master与worker的authorized_keys中

#### ◦ client端

```shell
root@client:~# ssh-keygen
Generating public/private rsa key pair.
Enter file in which to save the key (/root/.ssh/id_rsa):
Enter passphrase (empty for no passphrase):
Enter same passphrase again:
Your identification has been saved in /root/.ssh/id_rsa
Your public key has been saved in /root/.ssh/id_rsa.pub
The key fingerprint is:
SHA256:ibxGs2DgBtqE6ESylRzgHYs6js7X8MDzMnnSKc/hjGg root@client
The key's randomart image is:
```

 ![image-20230530162526997](.\typora-user-images\image-20230530162526997.png)

```shell
root@client:~# cat .ssh/id_rsa.pub
ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQCu/bmZL3oduFLPnWGY2gt/8oaxHoS00rOOLrpn6hRB9l9do7uvzgsAKZoqWNjSt5NEwYevpmWOr+lVXekJiibOnAqW8vlvey/FhJEo7hSiomdynqrVf/gGy0zdDKZuRgdh8vEcmI4ThukMV/AbRPlwzwwrRPhw4Xfphr8+aS1np7bpnz4YxXYo0e81RUM+cgEoytgWNlhayoG6QKRTbkW3EL11RocM53Tmo+Yozo79eFKH3sEk7xmnYDdmSe777JsWT8ie+r43vq8NaowoEiU03ncIb1PNRYw7kdqUa98I6nBG4bxXllfu1TKjs5z6Doxrth1AVwXZvD+5KVfeY1JMhYYIhUbgnUHGrJv3Lc8cquXByx0Be/NT9lsYahIyMQpz6MFRf1edTRS+xM2fMIDC8SFvpjiwndu+qkJFR9FBQ34QCJLCuxaf2oWoQx7VPU1VY5qIoRm1YNASR09ZxJ6mJrsg5oOv6dIkvslKX+KIFlx7fYPPIK0TN4dvd91A+TU= root@client
```

#### ◦ master端和worker端分别执⾏（下面传递的都是上边client的id_rsa.pub部分）

```shell
root@master:~# mkdir ~/.ssh

root@master:~# echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQCu/bmZL3oduFLPnWGY2gt/8oaxHoS00rOOLrpn6hRB9l9do7uvzgsAKZoqWNjSt5NEwYevpmWOr+lVXekJiibOnAqW8vlvey/FhJEo7hSiomdynqrVf/gGy0zdDKZuRgdh8vEcmI4ThukMV/AbRPlwzwwrRPhw4Xfphr8+aS1np7bpnz4YxXYo0e81RUM+cgEoytgWNlhayoG6QKRTbkW3EL11RocM53Tmo+Yozo79eFKH3sEk7xmnYDdmSe777JsWT8ie+r43vq8NaowoEiU03ncIb1PNRYw7kdqUa98I6nBG4bxXllfu1TKjs5z6Doxrth1AVwXZvD+5KVfeY1JMhYYIhUbgnUHGrJv3Lc8cquXByx0Be/NT9lsYahIyMQpz6MFRf1edTRS+xM2fMIDC8SFvpjiwndu+qkJFR9FBQ34QCJLCuxaf2oWoQx7VPU1VY5qIoRm1YNASR09ZxJ6mJrsg5oOv6dIkvslKX+KIFlx7fYPPIK0TN4dvd91A+TU= root@client" >> ~/.ssh/authorized_keys

root@master:~# cat ~/.ssh/authorized_keys
ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQCu/bmZL3oduFLPnWGY2gt/8oaxHoS00rOOLrpn6hRB9l9do7uvzgsAKZoqWNjSt5NEwYevpmWOr+lVXekJiibOnAqW8vlvey/FhJEo7hSiomdynqrVf/gGy0zdDKZuRgdh8vEcmI4ThukMV/AbRPlwzwwrRPhw4Xfphr8+aS1np7bpnz4YxXYo0e81RUM+cgEoytgWNlhayoG6QKRTbkW3EL11RocM53Tmo+Yozo79eFKH3sEk7xmnYDdmSe777JsWT8ie+r43vq8NaowoEiU03ncIb1PNRYw7kdqUa98I6nBG4bxXllfu1TKjs5z6Doxrth1AVwXZvD+5KVfeY1JMhYYIhUbgnUHGrJv3Lc8cquXByx0Be/NT9lsYahIyMQpz6MFRf1edTRS+xM2fMIDC8SFvpjiwndu+qkJFR9FBQ34QCJLCuxaf2oWoQx7VPU1VY5qIoRm1YNASR09ZxJ6mJrsg5oOv6dIkvslKX+KIFlx7fYPPIK0TN4dvd91A+TU= root@client
```

#### ◦ 是否成功分发，client端进⾏验证Shell

#### ◦ 验证master

```shell
root@client:~# ssh root@172.16.41.16 echo connected
The authenticity of host '172.16.41.16 (172.16.41.16)' can't be established.
ED25519 key fingerprint is SHA256:/ADdFMnmD9Lyr+K6N+hSyO6aGcQZVqZyoqmMp8DSV6o.
This key is not known by any other names
Are you sure you want to continue connecting (yes/no/[fingerprint])? **yes**
Warning: Permanently added '172.16.41.16' (ED25519) to the list of known hosts.
connected

root@client:~# ssh root@172.16.41.16 echo connected
connected
```

#### ◦ 验证worker

```shell
root@client:~# ssh root@172.16.41.14 echo connected
The authenticity of host '172.16.41.14 (172.16.41.14)' can't be established.
ED25519 key fingerprint is SHA256:/ADdFMnmD9Lyr+K6N+hSyO6aGcQZVqZyoqmMp8DSV6o.
This key is not known by any other names
Are you sure you want to continue connecting (yes/no/[fingerprint])? **yes**
Warning: Permanently added '172.16.41.14' (ED25519) to the list of known hosts.
connected

root@client:~# ssh root@172.16.41.14 echo connected
connected
```



### 3.Master与Worker节点配置（目的是保证master可以ssh连接自身和worker）

#### a. 将master节点的ssh key分别写⼊到worker和⾃⼰的authorized_keys中，为ansiable的使⽤提供前提

◦ Master端

```shell
root@master:~# ssh-keygen
Generating public/private rsa key pair.
Enter file in which to save the key (/root/.ssh/id_rsa):
Enter passphrase (empty for no passphrase):
Enter same passphrase again:
Your identification has been saved in /root/.ssh/id_rsa.
Your public key has been saved in /root/.ssh/id_rsa.pub.
The key fingerprint is:
SHA256:OJrVC/C7neDVnQT4BIgDm3Gx0dUcw7HneXm8/swogkg root@master
The key's randomart image is:
```

 ![image-20230530163733995](.\typora-user-images\image-20230530163733995.png)

◦ 下面把master的id_rsa.pub写进master和worker的authorized_keys

```shell
root@master:~# cat .ssh/id_rsa.pub
ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCuq30VHZqgy5fmGnpoS7992so2wZvZhwPCOLfCmS8kWG/woFKYYRCosma41MPCtSEu1Xewy1PR5jx4um9ptF2ML2V5stThRpzJ3NAd/sTM85GQiTS6/YS3m4rdM/SA6Gn2YePZujl2DVEwh2+UPltlt4mnOfSkvxWEbtb8dtz5SgcXnD712GaRg9u5wo0sM8vtyrHogvYDXrSLcTJPBqLIzWB8bfCHDpAmzqlai7bAKCPLM4Jxn8PlqSWzn5ub/7kicCf1cnTTWmCTvvAcq3NwQ2xSTsdn7VH/3jjS1NZuyF2Palk+E/xfmPzE0/XLPLGDmDNUc9/0g6cvuRtmr/l3 root@master

root@master:~# echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCuq30VHZqgy5fmGnpoS7992so2wZvZhwPCOLfCmS8kWG/woFKYYRCosma41MPCtSEu1Xewy1PR5jx4um9ptF2ML2V5stThRpzJ3NAd/sTM85GQiTS6/YS3m4rdM/SA6Gn2YePZujl2DVEwh2+UPltlt4mnOfSkvxWEbtb8dtz5SgcXnD712GaRg9u5wo0sM8vtyrHogvYDXrSLcTJPBqLIzWB8bfCHDpAmzqlai7bAKCPLM4Jxn8PlqSWzn5ub/7kicCf1cnTTWmCTvvAcq3NwQ2xSTsdn7VH/3jjS1NZuyF2Palk+E/xfmPzE0/XLPLGDmDNUc9/0g6cvuRtmr/l3 root@master" >> ~/.ssh/authorized_keys
```

◦ 所有worker端（引号里边都是master的id_rsa.pub）

```shell
root@worker:~# echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCuq30VHZqgy5fmGnpoS7992so2wZvZhwPCOLfCmS8kWG/woFKYYRCosma41MPCtSEu1Xewy1PR5jx4um9ptF2ML2V5stThRpzJ3NAd/sTM85GQiTS6/YS3m4rdM/SA6Gn2YePZujl2DVEwh2+UPltlt4mnOfSkvxWEbtb8dtz5SgcXnD712GaRg9u5wo0sM8vtyrHogvYDXrSLcTJPBqLIzWB8bfCHDpAmzqlai7bAKCPLM4Jxn8PlqSWzn5ub/7kicCf1cnTTWmCTvvAcq3NwQ2xSTsdn7VH/3jjS1NZuyF2Palk+E/xfmPzE0/XLPLGDmDNUc9/0g6cvuRtmr/l3 root@master" >> ~/.ssh/authorized_keys
```

#### ◦ 是否成功验证，master端进⾏验证Shell

```shell
root@master:~# ssh root@localhost echo connected
The authenticity of host 'localhost (127.0.0.1)' can't be established.
ECDSA key fingerprint is SHA256:UDQV6Bz66e9dsyffZE1S9lSeZm+eBIVwucuO/97H16Q.
Are you sure you want to continue connecting (yes/no)? **yes**
Warning: Permanently added 'localhost' (ECDSA) to the list of known hosts.
**connected**

root@master:~# ssh root@172.16.41.14 echo connected
The authenticity of host '172.16.41.14 (172.16.41.14)' can't be established.
ECDSA key fingerprint is SHA256:zG5pRhNviwzpT+ruZtoWEvmwxX8/XVSXYquz3B1OudQ.
Are you sure you want to continue connecting (yes/no)? yes
Warning: Permanently added '172.16.41.14' (ECDSA) to the list of known hosts.
**connected**

root@master:~# ssh root@localhost echo connected
**connected**

root@master:~# ssh root@172.16.41.14 echo connected
**connected**
```



#### b. master端Ansible安装与配置（注意ip的修改）

```
root@master:~# apt install software-properties-common -y

root@master:~# apt install ansible -y

root@master:~# mkdir /etc/ansible

root@master:~# vim /etc/ansible/hosts
\# Here's another example of host ranges, this time there are no
\# leading 0s:
\#db-[99:101]-node.example.com
[master]
127.0.0.1 ansible_connection=local ansible_python_interpreter=/usr/bin/python3
[workers]
172.16.41.14 ansible_ssh_extra_args='-o StrictHostKeyChecking=no' ansible_python_interpreter=/usr/bin/python3

root@master:~# ansible all -m ping -u root
127.0.0.1 | SUCCESS => {
"changed": false,
"ping": "pong"
}
172.16.41.14 | SUCCESS => {
"changed": false,
"ping": "pong"
}
```



#### c. client端将distrinet搭建需要的ansible⽂件送到master端（注意加粗部分为master的ip）

```shell
root@client:~# scp ~/Distrinet/mininet/mininet/provision/playbooks/install-docker.yml root@**172.16.41.16**:

install-docker.yml 																			100% 1498 1.1MB/s 00:00

root@client:~# scp ~/Distrinet/mininet/mininet/provision/playbooks/configure-docker.yml root@192.168.29.11:

configure-docker.yml 																		100% 3431 2.7MB/s 00:00
```



#### d. 在master端执⾏ansible-playbook进⾏运⾏playbooks并进⾏验证

```shell
root@master:~# ls
configure-docker.yml install-docker.yml

root@master:~# ansible-playbook ~/install-docker.yml
这一步有可能ryu失败，直接导入文件夹里的ryu.tar
导入命令为：
docker load < ryu.tar

root@master:~# ansible-playbook ~/configure-docker.yml
这一步失败的话，直接导入文件夹里的ubuntu.tar和switch.tar
导入命令为：
docker load < ubuntu.tar
docker load < switch.tar

root@master:~# ansible all -u root -m raw -a "docker images"
```

![image-20230530170006063](.\typora-user-images\image-20230530170006063.png) 

### 4.搭建成功测试

#### a. client端的conf.yml配置

（conf.yml只⽤改这四个字段的内容，其他部分不⽤改）

```shell
root@client:~# vim .distrinet/conf.yml

root@client:~# cat .distrinet/conf.yml

ssh:
  pub_id: "client的id_rsa.pub"
  user: "root"
  client_keys: ["/root/.ssh/id_rsa"]
  bastion: "master的ip"
```



#### b. 启动⼀个最简单的实例进⾏测试

##### ◦ 先在master节点起ryu控制器

```shell
root@master:~# docker images
```

![image-20230530170911867](.\typora-user-images\image-20230530170911867.png) 

要有ryu这个镜像

```shell
root@master:~# docker run -itd --name ryu -p 6653:6653 osrg/ryu bash
```

这里要暴露端口6653

```shell
root@master:~# docker ps -a
```

![image-20230530171132825](.\typora-user-images\image-20230530171132825.png) 

会发现起来一个名叫ryu的容器

##### ◦ 下面进入容器：

```shell
root@master:~# docker exec -it ryu bash
```

![image-20230530204457990](.\typora-user-images\image-20230530204457990.png) 

##### ◦ 进入容器后，起ryu控制器：

```shell
root@d846b8cbf307:~# ryu-manager ~/ryu/ryu/app/simple_switch_13.py --verbose
```

##### ◦ 在master和worker上运行以下四行，保证主机可以转发包：（每次重启物理机都要运行一遍）

```shell
sysctl net.ipv4.conf.all.forwarding=1
iptables --policy FORWARD ACCEPT
mkdir /var/run/netns
ulimit -n 196835
```



#### 下面有可能需要修改的代码：

**1.这个ip改成master的ip**

![image-20230530205737529](.\typora-user-images\image-20230530205737529.png) 

**2.刚开始测试用箭头1，后面添加provide网络后，用箭头2**

![image-20230530205952972](.\typora-user-images\image-20230530205952972.png)

##### ◦ 在client节点起任务

```shell
root@client:~# cd ~/Distrinet/mininet

root@client:~/Distrinet/mininet$ export PYTHONPATH=$PYTHONPATH:mininet:

root@client:~/Distrinet/mininet$ python3 bin/dmn --workers="172.16.41.16,172.16.41.14,172.16.41.15" --controller=lxcremote,ip=192.168.1.1 --topo=linear,2 --docker

这里的--workers后面的ip第一个是master，后面都是worker
```

##### ◦ 进⼊SLI后执⾏pingall，显⽰以下信息说明搭建成功，可以执⾏exit退出Shell

```
*** Starting CLI:
mininet> pingall
*** Ping: testing ping reachability
h1 -> h2
h2 -> h1
*** Results: 0% dropped (2/2 received)
```

