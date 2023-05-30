

## HifiCNet搭建流程

### 1.基本开发环境搭建

◦ HifiCNet⾄少需要三台机器，⼀台做client节点，⼀台做master节点，⼀台做worker节点

以下操作全在root下进行

##### ◦ 安装git python3 pip

```
Shell
1 root@client: apt update && apt install -y git python3-minimal python3-pip
2 root@client: python -m pip install --upgrade pip
```

##### ◦ clone HifiCNet仓库

```
Shell
1 root@client: cd ~
2 root@client: git clone https://github.com/J980419xq/Distrinet.git
```



##### ◦ 安装HifiCNet

```
Shell
1 root@client: cd ~/Distrinet
2 root@client: pip3 install -r requirements.txt
3 root@client: python3 setup.py install
```

##### ◦ 更新PYTHONPATH，权限更改

```
Shell
1 root@client: export PYTHONPATH=$PYTHONPATH:mininet:
2 root@client: chmod -R 777 ~/.distrinet
```

##### ◦ 检查是否安装成功

```
Shell
1 root@client: cd ~/.distrinet/
2 root@client: ls -al
3 total 20
4 drwxrwxrwx 2 root root 4096 Feb 6 10:45 .
5 drwxr-xr-x 8 ubuntu ubuntu 4096 Feb 6 10:45 ..
6 -rwxrwxrwx 1 root root 1144 Feb 6 10:45 conf.yml
7 -rwxrwxrwx 1 root root 426 Feb 6 09:54 general_purpose.json
8 -rwxrwxrwx 1 root root 1375 Feb 6 09:54 gros_partial.json
```



### 2.Client节点配置

a. 假设在第⼀步后：client ip：172.16.41.12，master ip：172.16.41.16，worker ip：172.16.41.14

b. ⽣成client节点的ssh key并分别写⼊到所有的master与worker的authorized_keys中

◦ client端Shell

```
1. root@client:~$ ssh-keygen
2. Generating public/private rsa key pair.
3. Enter file in which to save the key (/home/sdn/.ssh/id_rsa):
4. Created directory '/home/sdn/.ssh'.
5. Enter passphrase (empty for no passphrase):
6. Enter same passphrase again:
7. Your identification has been saved in /home/sdn/.ssh/id_rsa.
8. Your public key has been saved in /home/sdn/.ssh/id_rsa.pub.
9. The key fingerprint is:
10. SHA256:bCu19FteLsrbujZmVtm9H27hwqFy1hqpuAD3lsVPHRo sdn@ubuntu
11. The key's randomart image is:
12. +---[RSA 2048]----+
13. |			   |
14. |			   |
15. | 		  E .  |
16. |  		.. + . |
17. | . . So o + . |
18. | o .+o+o +....|
19. |  ..+o .== +.o|
20. |   o..oBOo*.+.|
21. |  	 o.*@B+.+.o|
22. +----[SHA256]-----+
23. root@client:~$ cat .ssh/id_rsa.pub
24. ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDPAkT7wbtpNpC33xMeZHwrcPdELDtcr9IPYZM1yGIdHhGYgK0ehE8bpY+MlHi3DTvV97n6IZZ0eacZsIhR61sqe0mtYtzSXtb+EK7IJXmCcYGDtEjqUcfCM4Cr3zvrLhxfNZEijtjiSsjZIkQhxCGklLB0hUUtwJRDVHYV123h4QbH0ZZcIsr63voxshJAgfz8sJtth7fJTeWNW5DF5kQzXQbFt8vBucjo1RU8AHnC84NN+vz+indpIebAA9QYp9a+jOCEpSDBxDeQJYGprTWxOC5LnBiJtiTHkjHZoRmIJKLsHFj2eDQS4j0utkWwTxjKxRVdR18Bf4xIgSeZ0ZHX sdn@ubuntu
```





◦ master端和worker端分别执⾏（""内部部分需要保持⼀致）

```
Shell
1.root@master: mkdir ~/.ssh
2.root@master: echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDPAkT7wbtpNpC33xMeZHwrcPdELDtcr9IPYZM1yGIdHhGYgK0ehE8bpY+MlHi3DTvV97n6IZZ0eacZsIhR61sqe0mtYtzSXtb+EK7IJXmCcYGDtEjqUcfCM4Cr3zvrLhxfNZEijtjiSsjZIkQhxCGklLB0hUUtwJRDVHYV123h4QbH0ZZcIsr63voxshJAgfz8sJtth7fJTeWNW5DF5kQzXQbFt8vBucjo1RU8AHnC84NN+vz+indpIebAA9QYp9a+jOCEpSDBxDeQJYGprTWxOC5LnBiJtiTHkjHZoRmIJKLsHFj2eDQS4j0utkWwTxjKxRVdR18Bf4xIgSeZ0ZHX sdn@ubuntu" >> ~/.ssh/authorized_keys

3.root@master: cat ~/.ssh/authorized_keys
4.ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDPAkT7wbtpNpC33xMeZHwrcPdELDtcr9IPYZM1yGIdHhGYgK0ehE8bpY+MlHi3DTvV97n6IZZ0eacZsIhR61sqe0mtYtzSXtb+EK7IJXmCcYGDtEjqUcfCM4Cr3zvrLhxfNZEijtjiSsjZIkQhxCGklLB0hUUtwJRDVHYV123h4QbH0ZZcIsr63voxshJAgfz8sJtth7fJTeWNW5DF5kQzXQbFt8vBucjo1RU8AHnC84NN+vz+indpIebAA9QYp9a+jOCEpSDBxDeQJYGprTWxOC5LnBiJtiTHkjHZoRmIJKLsHFj2eDQS4j0utkWwTxjKxRVdR18Bf4xIgSeZ0ZHX sdn@ubuntu
```



##### ◦ 是否成功分发，client端进行验证Shell

```
1 root@client:~$ ssh root@192.168.29.11 echo connected
2 The authenticity of host '192.168.29.11 (192.168.29.11)' can't be established.
3 ECDSA key fingerprint is SHA256:UDQV6Bz66e9dsyffZE1S9lSeZm+eBIVwucuO/97H16Q.
4 Are you sure you want to continue connecting (yes/no)? yes
5 Warning: Permanently added '192.168.29.11' (ECDSA) to the list of known hosts.
6 connected
7 root@client:~$ ssh -J root@192.168.29.11 root@192.168.29.12 echo connected
8 The authenticity of host '192.168.29.12 (<no hostip for proxy command>)' can't b
e established.
9 ECDSA key fingerprint is SHA256:zG5pRhNviwzpT+ruZtoWEvmwxX8/XVSXYquz3B1OudQ.
10 Are you sure you want to continue connecting (yes/no)? yes
11 Warning: Permanently added '192.168.29.12' (ECDSA) to the list of known hosts.
12 connected
13 root@client:~$ ssh root@192.168.29.11 echo connected
14 connected
15 root@client:~$ ssh -J root@192.168.29.11 root@192.168.29.12 echo connected
16 connected
```

### 3.Master与Worker节点配置

a. 将master节点的ssh key分别写⼊到worker和⾃⼰的authorized_keys中，为ansiable的使⽤提

供前提

▪ Master端

```
Shell
root@master:/home/sdn# ssh-keygen
Generating public/private rsa key pair.
Enter file in which to save the key (/root/.ssh/id_rsa):
Enter passphrase (empty for no passphrase):
Enter same passphrase again:
Your identification has been saved in /root/.ssh/id_rsa.
Your public key has been saved in /root/.ssh/id_rsa.pub.
The key fingerprint is:
SHA256:OJrVC/C7neDVnQT4BIgDm3Gx0dUcw7HneXm8/swogkg root@master
The key's randomart image is:
+---[RSA 2048]----+

|			   |
|			   |
| 		  E .  |
|  		.. + . |
| . . So o + . |
| o .+o+o +....|
|  ..+o .== +.o|
|   o..oBOo*.+.|
|  	 o.*@B+.+.o|

+----[SHA256]-----+

root@master:~# cat .ssh/id_rsa.pub
ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCuq30VHZqgy5fmGnpoS7992so2wZvZhwPCOLfCmS8kWG/woFKYYRCosma41MPCtSEu1Xewy1PR5jx4um9ptF2ML2V5stThRpzJ3NAd/sTM85GQiTS6/YS3m4rdM/SA6Gn2YePZujl2DVEwh2+UPltlt4mnOfSkvxWEbtb8dtz5SgcXnD712GaRg9u5wo0sM8vtyrHogvYDXrSLcTJPBqLIzWB8bfCHDpAmzqlai7bAKCPLM4Jxn8PlqSWzn5ub/7kicCf1cnTTWmCTvvAcq3NwQ2xSTsdn7VH/3jjS1NZuyF2Palk+E/xfmPzE0/XLPLGDmDNUc9/0g6cvuRtmr/l3 root@master
root@master:~# echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCuq30VHZqgy5fmGnpoS7992so2wZvZhwPCOLfCmS8kWG/woFKYYRCosma41MPCtSEu1Xewy1PR5jx4um9ptF2ML2V5stThRpzJ3NAd/sTM85GQiTS6/YS3m4rdM/SA6Gn2YePZujl2DVEwh2+UPltlt4mnOfSkvxWEbtb8dtz5SgcXnD712GaRg9u5wo0sM8vtyrHogvYDXrSLcTJPBqLIzWB8bfCHDpAmzqlai7bAKCPLM4Jxn8PlqSWzn5ub/7kicCf1cnTTWmCTvvAcq3NwQ2xSTsdn7VH/3jjS1NZuyF2Palk+E/xfmPzE0/XLPLGDmDNUc9/0g6cvuRtmr/l3 root@master" >> ~/.ssh/authorized_keys
```



▪ 所有worker端（绿⾊部分需要保持⼀致）

```
Shell
root@worker: echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCuq30VHZqgy5fmGnpoS7992so2wZvZhwPCOLfCmS8kWG/woFKYYRCosma41MPCtSEu1Xewy1PR5jx4um9ptF2ML2V5stThRpzJ3NAd/sTM85GQiTS6/YS3m4rdM/SA6Gn2YePZujl2DVEwh2+UPltlt4mnOfSkvxWEbtb8dtz5SgcXnD712GaRg9u5wo0sM8vtyrHogvYDXrSLcTJPBqLIzWB8bfCHDpAmzqlai7bAKCPLM4Jxn8PlqSWzn5ub/7kicCf1cnTTWmCTvvAcq3NwQ2xSTsdn7VH/3jjS1NZuyF2Palk+E/xfmPzE0/XLPLGDmDNUc9/0g6cvuRtmr/l3 root@master" >> ~/.ssh/authorized_keys
```



##### ▪ 是否成功验证，master端进⾏验证Shell

```
root@master:~# ssh root@localhost echo connected
The authenticity of host 'localhost (127.0.0.1)' can't be established.
ECDSA key fingerprint is SHA256:UDQV6Bz66e9dsyffZE1S9lSeZm+eBIVwucuO/97H16Q.
Are you sure you want to continue connecting (yes/no)? yes
Warning: Permanently added 'localhost' (ECDSA) to the list of known hosts.
connected
root@master:~# ssh root@192.168.29.12 echo connected
The authenticity of host '192.168.29.12 (192.168.29.12)' can't be established.
ECDSA key fingerprint is SHA256:zG5pRhNviwzpT+ruZtoWEvmwxX8/XVSXYquz3B1OudQ.
Are you sure you want to continue connecting (yes/no)? yes
Warning: Permanently added '192.168.29.12' (ECDSA) to the list of known hosts.
connected
root@master:~# ssh root@localhost echo connected
connected
root@master:~# ssh root@192.168.29.12 echo connected
connected
```



b. master端Ansible安装与配置（末尾增加⻩⾊部分，注意ip的修改）

```
Shell
root@master:~$ apt install software-properties-common -y
root@master:~$ apt install ansible -y
root@master:~# vim /etc/ansible/hosts
[master]
127.0.0.1 ansible_connection=local ansible_python_interpreter=/usr/bin/python3
[workers]
192.168.29.12 ansible_ssh_extra_args='-o StrictHostKeyChecking=no' ansible_python_interpreter=/usr/bin/python3

root@master:~# ansible all -m ping -u root
127.0.0.1 | SUCCESS => {
"changed": false,
"ping": "pong"
}
192.168.29.12 | SUCCESS => {
"changed": false,
"ping": "pong"
}
```



c. client端将distrinet搭建需要的ansible⽂件送到master端（注意紫⾊部分为master的ip）Shell

1 root@client:~$ scp ~/Distrinet/mininet/mininet/provision/playbooks/install-docke

r.yml root@192.168.29.11:

2 install-docker.yml 100% 1498 1.1MB/s 00:00

3 root@client:~$ scp ~/Distrinet/mininet/mininet/provision/playbooks/configure-doc

ker.yml root@192.168.29.11:

4 configure-docker.yml 100% 3431 2.7MB/s 00:00

d. 在master端执⾏ansible-playbook进⾏运⾏playbooks并进⾏验证

Shell

1 root@master:~# ls

2 configure-docker.yml install-docker.yml

3 root@master:~# ansible-playbook ~/install-docker.yml

4 root@master:~# ansible-playbook ~/configure-docker.yml

5 root@master:~# ansible all -u root -m raw -a "docker images"

6 127.0.0.1 | CHANGED | rc=0 >>

7 REPOSITORY TAG IMAGE ID CREATED SIZE

8 ubuntu latest 9c5e114f63bf 4 weeks ago 1.43G

B

9 jiawei96liu/hificnet-ubuntu openstack-v1 9c5e114f63bf 4 weeks ago 1.43G

B

10 jiawei96liu/hificnet-ubuntu v1 9c5e114f63bf 4 weeks ago 1.43G

B

11 jiawei96liu/hificnet-switch generic-v1 03a9ef2bd11f 5 months ago 426MB

12 jiawei96liu/hificnet-switch v1 03a9ef2bd11f 5 months ago 426MB

13 switch latest 03a9ef2bd11f 5 months ago 426MB

14

15 172.16.50.5 | CHANGED | rc=0 >>

16 REPOSITORY TAG IMAGE ID CREATED SIZE

17 ubuntu latest 9c5e114f63bf 4 weeks ago 1.43GB

18 switch latest 03a9ef2bd11f 5 months ago 426MB

19 Shared connection to 172.16.50.5 closed.

20

21 172.16.50.6 | CHANGED | rc=0 >>

22 REPOSITORY TAG IMAGE ID CREATED SIZE

23 ubuntu latest 9c5e114f63bf 4 weeks ago 1.43GB

24 ubuntu 22.04 2dc39ba059dc 7 weeks ago 77.8MB

25 switch latest 03a9ef2bd11f 5 months ago 426MB

26 Shared connection to 172.16.50.6 closed.

4. 搭建成功测试

a. client端的conf.yml配置（红⾊部分与之前内容⼀致，bastion为master的ip，client_keys为红

⾊部分的⽂件位置所在，conf.yml只⽤改这四个字段的内容，其他部分不⽤改）Shell

1 root@client:~$ vim .distrinet/conf.yml

2 root@client:~$ cat .distrinet/conf.yml

3 ---

4

5 ssh:

6 pub_id: "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDPAkT7wbtpNpC33xMeZHwrcPdELDtcr

9IPYZM1yGIdHhGYgK0ehE8bpY+MlHi3DTvV97n6IZZ0eacZsIhR61sqe0mtYtzSXtb+EK7IJXmCcYGDt

EjqUcfCM4Cr3zvrLhxfNZEijtjiSsjZIkQhxCGklLB0hUUtwJRDVHYV123h4QbH0ZZcIsr63voxshJAg

fz8sJtth7fJTeWNW5DF5kQzXQbFt8vBucjo1RU8AHnC84NN+vz+indpIebAA9QYp9a+jOCEpSDBxDeQJ

YGprTWxOC5LnBiJtiTHkjHZoRmIJKLsHFj2eDQS4j0utkWwTxjKxRVdR18Bf4xIgSeZ0ZHX ljw@ubun

tu"

7 user: "root"

8 client_keys: ["/home/ljw/.ssh/id_rsa"]

9 bastion: "192.168.29.11"

b. 启动⼀个最简单的实例进⾏测试

▪ 先在master节点起ryu控制器

Shell

1 root@master:~$ ryu-manager /usr/lib/python3/dist-packages/ryu/app/simple_switch_

13.py --verbose

▪ 在client节点起任务

Shell

1 root@client:~$ cd ~/Distrinet/mininet

2 root@client:~/Distrinet/mininet$ export PYTHONPATH=$PYTHONPATH:mininet:

3 root@client:~/Distrinet/mininet$ python3 bin/dmn --workers="192.168.29.11,192.16

8.29.12" --controller=lxcremote,ip=192.168.1.1 --topo=linear,2 --docker

▪ 进⼊SLI后执⾏pingall，显⽰以下信息说明搭建成功，可以执⾏exit退出Shell

1 *** Starting CLI:

2 mininet> pingall

3 *** Ping: testing ping reachability

4 h1 -> h2

5 h2 -> h1

6 *** Results: 0% dropped (2/2 received)

7 mininet>

5. 使⽤onos集群替换ryu控制器

◦ HifiCNet规模较⼤时需要使⽤onos集群替换ryu控制器，替换前关闭ryu的进程保证6653端⼝可

⽤

Shell

1 root@master:~# netstat -antlp|grep 6653

2 tcp 0 0 0.0.0.0:6653 0.0.0.0:* LISTEN

29173/python3.6

3 root@master:~# kill -9 29173

◦ 下载onos docker镜像以及atomix镜像

Shell

1 root@master:~# docker pull atomix/atomix

2 root@master:~# docker pull onosproject/onos

◦ 创建atomix docker集群

Shell

1 root@master:~# docker run -t -d --name atomix-1 atomix/atomix

2 root@master:~# docker run -t -d --name atomix-2 atomix/atomix

3 root@master:~# docker run -t -d --name atomix-3 atomix/atomix

◦ 下载onos源代码

Shell

1 root@master:~# git clone https://gerrit.onosproject.org/onos

◦ ⽣成atomix集群配置⽂件并复制到atomix容器中，重启容器Shell

1 root@master:~# export OC1=172.17.0.2 #atomix-1 ip

2 root@master:~# export OC2=172.17.0.3 #atomix-2 ip

3 root@master:~# export OC3=172.17.0.4 #atomix-3 ip

4 root@master:~# cd ~/onos

5 root@master:~#./tools/test/bin/atomix-gen-config 172.17.0.2 ~/atomix-1.conf 172.

17.0.2 172.17.0.3 172.17.0.4 #maybe need python2

6 root@master:~#./tools/test/bin/atomix-gen-config 172.17.0.3 ~/atomix-2.conf 172.

17.0.2 172.17.0.3 172.17.0.4

7 root@master:~#./tools/test/bin/atomix-gen-config 172.17.0.4 ~/atomix-3.conf 172.

17.0.2 172.17.0.3 172.17.0.4

8 root@master:~# docker cp ~/atomix-1.conf atomix-1:/opt/atomix/conf/atomix.conf

9 root@master:~# docker cp ~/atomix-2.conf atomix-2:/opt/atomix/conf/atomix.conf

10 root@master:~# docker cp ~/atomix-3.conf atomix-3:/opt/atomix/conf/atomix.conf

11 root@master:~# docker restart atomix-1

12 root@master:~# docker restart atomix-2

13 root@master:~# docker restart atomix-3

◦ 创建onos docker集群

Shell

1 root@master:~# docker run -t -d -p 6653:6653 -e ONOS_APPS="drivers,openflow-bas

e,netcfghostprovider,openflow,proxyarp,lldpprovider,fwd,optical-model,hostprovid

er,gui2" --name onos1 onosproject/onos

2 root@master:~# docker run -t -d -p 6654:6653 -e ONOS_APPS="drivers,openflow-bas

e,netcfghostprovider,openflow,proxyarp,lldpprovider,fwd,optical-model,hostprovid

er,gui2" --name onos2 onosproject/onos

3 root@master:~# docker run -t -d -p 6655:6653 -e ONOS_APPS="drivers,openflow-bas

e,netcfghostprovider,openflow,proxyarp,lldpprovider,fwd,optical-model,hostprovid

er,gui2" --name onos3 onosproject/onos

◦ ⽣成onos集群配置⽂件并复制到onos容器中，重启容器

Shell ⾃动动

6. 增量部署openstack nova节点

◦ 由于实验需要，我们实现了增量部署nova服务，逐步探究underlay的compute规模及性能瓶颈

◦ ⽣成需要启动nova服务容器的ansible配置⽂件Shell

root@node3:~/Distrinet# ./mininet/mininet/provision/playbooks/prepare-ansiable.s

h 192.168.1.5 5 #第⼀个参数为这批次容器的起始ip 第⼆个参数为容器个数

root@node3:~/Distrinet# ./mininet/mininet/provision/playbooks/prepare-ansiable.s

h 192.168.1.5 5

root@node3:~/Distrinet# cat /etc/ansible/hosts

[master]

127.0.0.1 ansible_connection=local ansible_python_interpreter=/usr/bin/python3

[workers]

192.168.1.5 ansible_ssh_extra_args='-o StrictHostKeyChecking=no' ansible_python_

interpreter=/usr/bin/python3

192.168.1.6 ansible_ssh_extra_args='-o StrictHostKeyChecking=no' ansible_python_

interpreter=/usr/bin/python3

192.168.1.7 ansible_ssh_extra_args='-o StrictHostKeyChecking=no' ansible_python_

interpreter=/usr/bin/python3

192.168.1.8 ansible_ssh_extra_args='-o StrictHostKeyChecking=no' ansible_python_

interpreter=/usr/bin/python3

192.168.1.9 ansible_ssh_extra_args='-o StrictHostKeyChecking=no' ansible_python_

interpreter=/usr/bin/python3

◦ 启动这些容器的nova服务

Shell

root@node3:~/Distrinet# ansible-playbook mininet/mininet/provision/playbooks/dep

loy_compute.yml

1

◦