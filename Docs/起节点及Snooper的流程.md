### 起HifiCNet Underlay网络
1. Master及Worker节点准备docker网络
    ```
    root@master:~# netplan apply 
    root@master:~# ./prepare-docker-net.sh
    
    root@master:~# netplan apply
    root@worker1:~# ./prepare-docker-net.sh

    root@master:~# netplan apply
    root@worker2:~# ./prepare-docker-net.sh
    ```

2. Client节点请求起拓扑
   ```
    cd Distrinet/mininet
    export PYTHONPATH=mininet:$PYTHONPATH:
    python3 bin/dmn --workers="172.16.41.16,172.16.41.15,172.16.41.14" --controller=lxcremote,ip=192.168.1.1 --topo=linear,4 --docker
   ```

3. Master节点起ryu：
    ```
    root@master:/home/sdn# docker run -itd --name ryu -p 6653:6653 -i osrg/ryu  bash
    3ac0fbc3b072038a2129b89f35069b9d9a6ccc5a9db489508547e27d19c8529a

    root@master:/home/sdn# docker exec -it ryu bash

    root@3ac0fbc3b072:~# ryu-manager ryu/ryu/app/simple_switch_13.py --verbose

    ```

4. Client CLI pingall 测试
   ```
   *** Starting CLI:
    mininet>
    mininet> pingall
    *** Ping: testing ping reachability
    h1 -> h2
    h2 -> h1
    *** Results: 0% dropped (2/2 received)
    mininet>
   ```

### 构建Controller与Underlay网络的连接

1. Controller、Master节点运行
    ```
    sdn@controller:~/HiFiCNet/OverlayBuild/scripts/shell$ pwd
    /home/sdn/HiFiCNet/OverlayBuild/scripts/shell
    ```
2. 连通性测试，确保起来的vhost容器的网络都在一个L2下面

### 将vhost启动为controller的计算节点

1. 起容器中的openstack服务，进入到容器中，修改 /root/       docker_configer_container_cmd.py，
    route add default gw 192.168.1.1 
    echo "10.10.0.11  controller" >> /etc/hosts 这一行
    ```
    import subprocess

    def execShellCommand(cmd):
        print("\033[32m++ "+cmd+" ====>\033[0m")
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, executable="/bin/bash")
        ret = p.stdout.read().decode('utf-8')
        print(ret)
        return ret

    if __name__ == "__main__":
        ip10 = execShellCommand("ifconfig -a | grep 10.10. | awk '{print $2}'")[:-1]
        ip20 = execShellCommand("ifconfig -a | grep 20.20. | awk '{print $2}'")[:-1]
        ip192 = execShellCommand("ifconfig -a | grep 192.168. | awk '{print $2}'")[:-1]
        execShellCommand("route add default gw 192.168.1.1")

        execShellCommand("route add default gw 20.20.0.1")

        execShellCommand("route add default gw 10.10.0.1")

        execShellCommand('echo "10.10.0.11 controller" >> /etc/hosts')
        execShellCommand("service chrony restart")

        execShellCommand("/usr/share/openvswitch/scripts/ovs-ctl restart")
        execShellCommand("sed -i 's/my_ip = [0-9.]*/my_ip = "+ip10+"/g' /etc/nova/nova.conf")
        execShellCommand("service nova-compute restart")

        execShellCommand("sed -i 's/local_ip = [0-9.]*/local_ip = " + ip192 + "/g' /etc/neutron/plugins/ml2/linuxbridge_agent.ini")

        execShellCommand("sed -i 's/local_ip = [0-9.]*/local_ip = " + ip192 + "/g' /etc/neutron/plugins/ml2/openvswitch_agent.ini")

        execShellCommand("ifconfig eth0 0.0.0.0")

        execShellCommand("ifconfig br-ex "+ip20+"/16")
        execShellCommand("route add default gw 20.20.0.1")

        execShellCommand("service neutron-linuxbridge-agent restart")

        execShellCommand("service neutron-dhcp-agent restart")

        execShellCommand("service neutron-metadata-agent restart")

        execShellCommand("service neutron-l3-agent restart")

        execShellCommand("service neutron-openvswitch-agent restart")

        execShellCommand("service nova-compute restart")

        #execShellCommand("nohup ./nova_compute_auto_restart.sh &")

    ```
2. 执行该脚本，python3 docker_configer_container_cmd.py
3. controller root下执行su -s /bin/sh -c "nova-manage cell_v2 discover_hosts --verbose" nova，进行compute node的发现操作
4. 切换到sdn账号，执行 openstack hypervisor list，可以看到新增的节点
    ```
    sdn@controller:~$ . admin-openrc
    sdn@controller:~$ openstack hypervisor list
    +----+---------------------+-----------------+------------+-------+
    | ID | Hypervisor Hostname | Hypervisor Type | Host IP    | State |
    +----+---------------------+-----------------+------------+-------+
    |  1 | compute1            | fake            | 10.10.0.13 | down  |
    |  2 | h1                  | fake            | 10.10.0.4  | up    |
    |  3 | h2                  | fake            | 10.10.0.5  | up    |
    |  4 | h4                  | fake            | 10.10.0.7  | up    |
    |  5 | h3                  | fake            | 10.10.0.6  | up    |
    +----+---------------------+-----------------+------------+-------+
    ```

### openstack 起节点

1. 运行 ./openstack-create.sh init，起两个subnet及外部provider网络
2. 运行 ./openstack-create.sh router，起subunet间的router及添加路由规则
3. 运行 ./openstack-create.sh 8 4，起8个VM(namespace)，位于4个vhost上(docker)
4. ./openstack-clean.sh脚本为对应的相反操作，自己根据需要研究

* 注意：路由器成功创建应该有对应的qrouter namespace起来，脚本执行的时候，有可能会起不来，注意检查。由于nova的调度，router起的位置是不确定的，推荐先起一个compute node节点，起成功router之后，增量起compute node节点
    ```
    root@h3:/# ip netns ls
    fake-98ac9326-543e-47c8-bfae-6e5d8ddd899d (id: 4)
    qdhcp-28202ede-1b30-4b9d-b88f-50c53b29ea80 (id: 1)
    qrouter-d35a4f17-4aab-4083-98c9-19e179bea5ce (id: 2)
    fake-833f68ee-958e-4e39-909a-fe1c30d2ec04 (id: 3)
    root@h3:/#

    ```
### L3测试

1. controller节点添加对应的namespace subnet的路由表，可以做到controller节点ping到所有的namespace
   ```
    root@controller:/home/sdn# route add -net 1.0.0.0/8 gw 20.20.0.251
    root@controller:/home/sdn# route add -net 2.0.0.0/8 gw 20.20.0.253
    root@controller:/home/sdn# route -n
    Kernel IP routing table
    Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
    0.0.0.0         172.16.41.254   0.0.0.0         UG    101    0        0 enp1s0f1
    1.0.0.0         20.20.0.251     255.0.0.0       UG    0      0        0 enp1s0f0
    2.0.0.0         20.20.0.253     255.0.0.0       UG    0      0        0 enp1s0f0
    10.10.0.0       0.0.0.0         255.255.255.0   U     102    0        0 enp3s0
    10.10.0.0       10.10.0.1       255.255.255.0   UG    102    0        0 enp3s0
    20.20.0.0       0.0.0.0         255.255.255.0   U     100    0        0 enp1s0f0
    20.20.0.0       20.20.0.1       255.255.255.0   UG    100    0        0 enp1s0f0
    172.16.41.0     0.0.0.0         255.255.255.0   U     101    0        0 enp1s0f1
    192.168.0.0     0.0.0.0         255.255.0.0     U     0      0        0 admin-br
    192.168.0.0     0.0.0.0         255.255.0.0     U     0      0        0 admin
    root@controller:/home/sdn#

   ```


2. 默认起的路由器，由于没有添加路由规则，不支持跨subnet通信，做到节点跨subnet通信可以通过以下操作，添加路由规则
    ```
     sdn@controller:~$ openstack router list
    +--------------------------------------+------------------+--------+-------+----------------------------------+-------------+-------+
    | ID                                   | Name             | Status | State | Project                          | Distributed | HA    |
    +--------------------------------------+------------------+--------+-------+----------------------------------+-------------+-------+
    | 011943b6-84c2-445a-a608-728ed9ab6f4e | router-ext-self2 | ACTIVE | UP    | 59cb087f79154ab3b6d6015422630491 | False       | False |
    | d35a4f17-4aab-4083-98c9-19e179bea5ce | router-ext-self1 | ACTIVE | UP    | 59cb087f79154ab3b6d6015422630491 | False       | False |
    +--------------------------------------+------------------+--------+-------+----------------------------------+-------------+-------+
    sdn@controller:~$ openstack router show router-ext-self1
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | Field                   | Value                                                                                                                                                                                   |
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | admin_state_up          | UP                                                                                                                                                                                      |
    | availability_zone_hints |                                                                                                                                                                                         |
    | availability_zones      | nova                                                                                                                                                                                    |
    | created_at              | 2023-06-08T13:12:14Z                                                                                                                                                                    |
    | description             |                                                                                                                                                                                         |
    | distributed             | False                                                                                                                                                                                   |
    | external_gateway_info   | {"network_id": "b01e47bd-ec80-4ec7-8bc2-b028717f5971", "external_fixed_ips": [{"subnet_id": "5447485b-ccd2-4d76-ba4c-4bfca1df2221", "ip_address": "20.20.0.251"}], "enable_snat": true} |
    | flavor_id               | None                                                                                                                                                                                    |
    | ha                      | False                                                                                                                                                                                   |
    | id                      | d35a4f17-4aab-4083-98c9-19e179bea5ce                                                                                                                                                    |
    | interfaces_info         | [{"port_id": "f0d3b3b5-2125-4dfb-a3a9-4512f66c4436", "ip_address": "1.0.0.1", "subnet_id": "13ce93b0-ced4-4a6c-8ced-b2d68982913a"}]                                                     |
    | name                    | router-ext-self1                                                                                                                                                                        |
    | project_id              | 59cb087f79154ab3b6d6015422630491                                                                                                                                                        |
    | revision_number         | 4                                                                                                                                                                                       |
    | routes                  |                                                                                                                                                                                         |
    | status                  | ACTIVE                                                                                                                                                                                  |
    | tags                    |                                                                                                                                                                                         |
    | updated_at              | 2023-06-08T13:12:25Z                                                                                                                                                                    |
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    sdn@controller:~$ openstack router show router-ext-self2
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | Field                   | Value                                                                                                                                                                                   |
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | admin_state_up          | UP                                                                                                                                                                                      |
    | availability_zone_hints |                                                                                                                                                                                         |
    | availability_zones      | nova                                                                                                                                                                                    |
    | created_at              | 2023-06-08T13:35:52Z                                                                                                                                                                    |
    | description             |                                                                                                                                                                                         |
    | distributed             | False                                                                                                                                                                                   |
    | external_gateway_info   | {"network_id": "b01e47bd-ec80-4ec7-8bc2-b028717f5971", "external_fixed_ips": [{"subnet_id": "5447485b-ccd2-4d76-ba4c-4bfca1df2221", "ip_address": "20.20.0.253"}], "enable_snat": true} |
    | flavor_id               | None                                                                                                                                                                                    |
    | ha                      | False                                                                                                                                                                                   |
    | id                      | 011943b6-84c2-445a-a608-728ed9ab6f4e                                                                                                                                                    |
    | interfaces_info         | [{"port_id": "9dd3874a-5ab5-40c0-8277-00a1baa57388", "ip_address": "2.0.0.1", "subnet_id": "9099becc-f8c1-4d39-8ccb-e07c36ebf52f"}]                                                     |
    | name                    | router-ext-self2                                                                                                                                                                        |
    | project_id              | 59cb087f79154ab3b6d6015422630491                                                                                                                                                        |
    | revision_number         | 5                                                                                                                                                                                       |
    | routes                  |                                                                                                                                                                                         |
    | status                  | ACTIVE                                                                                                                                                                                  |
    | tags                    |                                                                                                                                                                                         |
    | updated_at              | 2023-06-08T13:41:59Z                                                                                                                                                                    |
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+



    sdn@controller:~$ openstack subnet list
    +--------------------------------------+--------------+--------------------------------------+--------------+
    | ID                                   | Name         | Network                              | Subnet       |
    +--------------------------------------+--------------+--------------------------------------+--------------+
    | 13ce93b0-ced4-4a6c-8ced-b2d68982913a | self1-subnet | ed829ed1-ec5e-49bd-9477-e01b9fc4cc9d | 1.0.0.0/8    |
    | 5447485b-ccd2-4d76-ba4c-4bfca1df2221 | ext-subnet   | b01e47bd-ec80-4ec7-8bc2-b028717f5971 | 20.20.0.0/16 |
    | 9099becc-f8c1-4d39-8ccb-e07c36ebf52f | self2-subnet | 28202ede-1b30-4b9d-b88f-50c53b29ea80 | 2.0.0.0/8    |
    +--------------------------------------+--------------+--------------------------------------+--------------+
    sdn@controller:~$ openstack router add route --route destination=1.0.0.0/8,gateway=20.20.0.251 router-ext-self2
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | Field                   | Value                                                                                                                                                                                   |
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | admin_state_up          | UP                                                                                                                                                                                      |
    | availability_zone_hints |                                                                                                                                                                                         |
    | availability_zones      | nova                                                                                                                                                                                    |
    | created_at              | 2023-06-08T13:35:52Z                                                                                                                                                                    |
    | description             |                                                                                                                                                                                         |
    | distributed             | False                                                                                                                                                                                   |
    | external_gateway_info   | {"network_id": "b01e47bd-ec80-4ec7-8bc2-b028717f5971", "external_fixed_ips": [{"subnet_id": "5447485b-ccd2-4d76-ba4c-4bfca1df2221", "ip_address": "20.20.0.253"}], "enable_snat": true} |
    | flavor_id               | None                                                                                                                                                                                    |
    | ha                      | False                                                                                                                                                                                   |
    | id                      | 011943b6-84c2-445a-a608-728ed9ab6f4e                                                                                                                                                    |
    | name                    | router-ext-self2                                                                                                                                                                        |
    | project_id              | 59cb087f79154ab3b6d6015422630491                                                                                                                                                        |
    | revision_number         | 6                                                                                                                                                                                       |
    | routes                  | destination='1.0.0.0/8', gateway='20.20.0.251'                                                                                                                                          |
    | status                  | ACTIVE                                                                                                                                                                                  |
    | tags                    |                                                                                                                                                                                         |
    | updated_at              | 2023-06-08T13:50:51Z                                                                                                                                                                    |
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    sdn@controller:~$ openstack router add route --route destination=2.0.0.0/8,gateway=20.20.0.253 router-ext-self1
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | Field                   | Value                                                                                                                                                                                   |
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    | admin_state_up          | UP                                                                                                                                                                                      |
    | availability_zone_hints |                                                                                                                                                                                         |
    | availability_zones      | nova                                                                                                                                                                                    |
    | created_at              | 2023-06-08T13:12:14Z                                                                                                                                                                    |
    | description             |                                                                                                                                                                                         |
    | distributed             | False                                                                                                                                                                                   |
    | external_gateway_info   | {"network_id": "b01e47bd-ec80-4ec7-8bc2-b028717f5971", "external_fixed_ips": [{"subnet_id": "5447485b-ccd2-4d76-ba4c-4bfca1df2221", "ip_address": "20.20.0.251"}], "enable_snat": true} |
    | flavor_id               | None                                                                                                                                                                                    |
    | ha                      | False                                                                                                                                                                                   |
    | id                      | d35a4f17-4aab-4083-98c9-19e179bea5ce                                                                                                                                                    |
    | name                    | router-ext-self1                                                                                                                                                                        |
    | project_id              | 59cb087f79154ab3b6d6015422630491                                                                                                                                                        |
    | revision_number         | 5                                                                                                                                                                                       |
    | routes                  | destination='2.0.0.0/8', gateway='20.20.0.253'                                                                                                                                          |
    | status                  | ACTIVE                                                                                                                                                                                  |
    | tags                    |                                                                                                                                                                                         |
    | updated_at              | 2023-06-08T13:51:06Z                                                                                                                                                                    |
    +-------------------------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
    sdn@controller:~$                                                                                                                                                           

    ```


