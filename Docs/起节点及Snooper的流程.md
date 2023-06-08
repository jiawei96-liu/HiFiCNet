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

### 将vhost启动为controller的计算节点