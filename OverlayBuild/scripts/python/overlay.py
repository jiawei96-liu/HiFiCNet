import os
import warnings
warnings.filterwarnings("ignore")
import docker
import time
import pymysql
import paramiko
from optparse import OptionParser
import cmd
import subprocess
import sys



class OverlayCmd(cmd.Cmd):
    prompt = '-OverLay> '  # 设置命令行提示符
    def __init__(self):
        super(OverlayCmd, self).__init__()
        self.mysql_host = '172.16.50.7'
        self.mysql_user = 'root'
        self.mysql_password = 'sdn123456'
        self.user = 'sdn'
        self.rootuser = 'root'
        self.password = 'sdn123456'
        self.controller = '172.16.50.7'
        self.master = '172.16.50.6'
        self.workers = []
        self.hosts = self.workers[:]
        self.hosts.append(self.controller)
        self.hosts.append(self.master)
        self.mapnodesnum = 4
        self.ssh = paramiko.SSHClient()
        self.overlaynets = self.GetOverlayNets()
        self.overlaysubnets = self.GetOverlaySubnets()
        self.overlayrouters = self.GetOverlayRouters()
        self.underlaynodesmap = self.GetUnderlayNodesMap()
        self.overlaynodes = self.GetOverlayNodes()
    
    def do_refresh(self, arg):
        self.overlaynets = self.GetOverlayNets()
        self.overlaysubnets = self.GetOverlaySubnets()
        self.overlayrouters = self.GetOverlayRouters()
        self.underlaynodesmap = self.GetUnderlayNodesMap()
        self.overlaynodes = self.GetOverlayNodes()

    def do_help(self, arg):
        command = ['init', 'clean', 'networks', 'subnets', 'routers', 'nodes', 'nss(namespaces)', 'rnss(routernamespaces)', 'pingall']
        for i in command:
            print("\t" + i)

    def do_exit(self, arg):
        return True

    def do_init(self, arg):
                # 自动添加主机密钥
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        print("---------Overlay Config vxadmin---------")
        self.ssh.connect(hostname=self.controller, port=22, username='root', password='sdn123456')
        _, stdout, _ = self.ssh.exec_command("ip link | grep -E 'admin' | tr -d ':' | tr '@' ' ' | awk '{print $2}'")
        if stdout.read().decode() == "":
            self.ssh.exec_command("brctl addbr admin-br")
            self.ssh.exec_command("ifconfig admin-br 192.168.1.10/16")
            self.ssh.exec_command("tunctl -t admin")
            self.ssh.exec_command("brctl addif admin-br admin")
            self.ssh.exec_command("ifconfig admin 192.168.1.11/16")
            self.ssh.exec_command("ip link set admin-br up")
            self.ssh.exec_command("ip link set admin up")
            self.ssh.exec_command("ip link add vxadmin type vxlan id 00 remote 172.16.50.6 local 172.16.50.7 dstport 4789")
            self.ssh.exec_command("ip link set up vxadmin")
            self.ssh.exec_command("brctl addif admin-br vxadmin")
            self.ssh.exec_command("ip link set up admin-br")
            self.ssh.exec_command('exit')
            self.ssh.close()
            self.ssh.connect(hostname=self.master, port=22, username='root', password='sdn123456')
            self.ssh.exec_command("ip link add vxadmin type vxlan id 00 remote 172.16.50.7 local 172.16.50.6 dstport 4789")
            self.ssh.exec_command("ip link set up vxadmin")
            self.ssh.exec_command("brctl addif admin-br vxadmin")
            self.ssh.exec_command("ip link set up admin-br")
            self.ssh.exec_command('exit')
            self.ssh.close()
            time.sleep(2)
        print("successfully!")
        print("---------Overlay Config vxdata---------")
        self.ssh.connect(hostname=self.master, port=22, username='root', password='sdn123456')
        _, stdout, _ = self.ssh.exec_command("ip link | grep -E 'data' | tr -d ':' | tr '@' ' ' | awk '{print $2}'")
        if stdout.read().decode() == "":
            self.ssh.exec_command("ip link add vxdata type vxlan id 1 remote 172.16.50.7 local 172.16.50.6 dstport 4788")
            self.ssh.exec_command("ip link set up vxdata")
            self.ssh.exec_command("brctl addif intf6 vxdata")
            self.ssh.exec_command("ip link set up intf6")
            self.ssh.exec_command('exit')
            self.ssh.close()
            self.ssh.connect(hostname=self.controller, port=22, username='root', password='sdn123456')
            self.ssh.exec_command("ip link add vxdata type vxlan id 1 remote 172.16.50.6 local 172.16.50.7 dstport 4788")
            self.ssh.exec_command("brctl addbr data-br")
            self.ssh.exec_command("ifconfig data-br 10.10.0.7/16")
            self.ssh.exec_command("ip addr flush eno4")
            self.ssh.exec_command("brctl addif data-br eno4")
            self.ssh.exec_command("tunctl -t data")
            self.ssh.exec_command("brctl addif data-br data")
            self.ssh.exec_command("ifconfig data 10.10.0.2/16")
            self.ssh.exec_command("ip link set data up")
            self.ssh.exec_command("ip link set up data-br")
            self.ssh.exec_command("ip link set up vxdata")
            self.ssh.exec_command("brctl addif data-br vxdata")
            self.ssh.exec_command('exit')
            self.ssh.close()
            time.sleep(2)
        print("successfully!")
        print("---------Overlay Config Compute Node---------")
        hosts = self.workers[:]
        hosts.append(self.master)
        for host in hosts:
            self.ssh.connect(hostname=host, port=22, username='root', password='sdn123456')
            # 执行命令
            _, stdout, _ = self.ssh.exec_command("docker ps -a | grep -E 'ubuntu' | awk '{print $1}'")
            compute_nodes = []
            for node in stdout:
                compute_nodes.append(node.replace("\n", ""))
            print(compute_nodes)
            for node in compute_nodes:
                print("docker exec " + node + " python3 /root/docker_configer_container_cmd.py")
                self.ssh.exec_command("docker exec " + node + " python3 /root/docker_configer_container_cmd.py &")
            self.ssh.exec_command('exit')
            self.ssh.close()
        time.sleep(2)
        print("successfully!")

    def do_clean(self, arg):
        pass

    def add_command_arg(self, command, argname):
        arg = argname.replace('_', '-')
        return command + ' ' + arg


    def add_command_value_arg(self, command, argname, argvalue):
        if argvalue:
            arg = argname.replace('_', '-')
            return command + ' --' + arg + ' ' + argvalue
        else:
            print('your ' + argname + ' need a value')
            return




    def add_network(self, networkname=None, share=False, external=False, ppn=None, pnt=None):
        # ppn: provider-physical-network
        # pnt: provider-network-type
        command = 'openstack network create '
        
        if not networkname:
            print('Your network needs a name')
            return
        else:
            command += networkname
        if share:
            command += ' --share'
        if external:
            command += ' --external'
        if ppn:
            command += ' --provider-physical-network ' + ppn
        if pnt:
            command += ' --provider-network-type ' + pnt
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.ssh.connect(hostname=self.controller, port=22, username=self.user, password=self.password)
        _, stdout, _ = self.ssh.exec_command(command)
        print(stdout.read().decode())

    def add_network(self, networkname=None, subnetname=None, dns_nameserver=None, gateway=None, subnet_range=None):
        # ppn: provider-physical-network
        # pnt: provider-network-type
        command = 'openstack subnet create '
        if not networkname:
            print('Your subnet needs a base network')
            return
        else:
            command += '--network ' + networkname
        if dns_nameserver:
            command += ' --dns-nameserver '+dns_nameserver
        if gateway:
            command += ' --gateway '+gateway
        if subnet_range:
            command += ' --subnet-range '+subnet_range
        if not subnetname:
            print('Your subnet needs a subnet name')
            return
        else:
            command += ' subnetname'
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.ssh.connect(hostname=self.controller, port=22, username=self.user, password=self.password)
        _, stdout, _ = self.ssh.exec_command(command)
        print(stdout.read().decode())
    
    def add_security_group(self, sgname):
        command = 'openstack security group create '



    def SearchInMysql(self, sql):
        conn = pymysql.connect(host=self.mysql_host, port=3306, user=self.mysql_user, password=self.mysql_password,
                            cursorclass=pymysql.cursors.DictCursor)
        result = None
        with conn.cursor() as cursor:
            cursor.execute(sql)
            result = cursor.fetchall()
        conn.close()
        return result

    def GetOverlayNets(self):
        sql = "select network.id, network.name from neutron.networks as network;"
        overlaynets = self.SearchInMysql(sql)
        return overlaynets
    def do_networks(self, arg):
        for i in self.overlaynets:
            print(i)

    def GetOverlaySubnets(self):
        sql = "select subnet.id, subnet.name, subnet.cidr, subnet.gateway_ip from neutron.subnets as subnet;"
        overlaysubnets = self.SearchInMysql(sql)
        return overlaysubnets
    def do_subnets(self, arg):
        for i in self.overlaysubnets:
            print(i)

    def GetOverlayRouters(self):
        sql = "select router.id, router.name from neutron.routers as router;"
        overlayrouters = self.SearchInMysql(sql)
        return overlayrouters
    
    def do_routers(self, arg):
        for i in self.overlayrouters:
            print(i)

    
    def GetOverlayNodes(self):
        sql = "select instance.uuid,instance.hostname,ip.ip_address \
            from nova.instances as instance, neutron.ports as port, neutron.ipallocations as ip \
            where port.id=ip.port_id and instance.uuid=port.device_id \
            order by instance.hostname;"
        overlaynodes = self.SearchInMysql(sql)
        return overlaynodes
    
    def do_nodes(self, arg):
        for i in self.overlaynodes:
            print(i)
    
    def GetAvgPingTimeDelay(self, ping_info):
        lines = ping_info.split("\n")
        # 删除第一行ping头信息
        del lines[0]
        delays = []
        for line in lines:
            if "time=" in line:
                delay = line.split("time=")[1].split(" ")[0]
                delays.append(float(delay))
        # 去掉第一个ping时延
        del delays[0]
        return sum(delays)/len(delays)

    def do_pingall(self, arg):
        # 自动添加主机密钥
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        notconnected=[]
        pingtime=[]
        for ns in self.overlaynodes:
            self.ssh.connect(hostname=self.underlaynodesmap[ns['hostname'][-2:]], port=22, username='root', password='sdn123456')
            
            bar_i=1
            for ns_others in self.overlaynodes:
                if ns_others['uuid'] != ns['uuid']:
                    _, tap_down, _ = self.ssh.exec_command('ip netns exec fake-'+ns_others['uuid']+' ip a | grep noqueue | grep DOWN')
                    if not tap_down:
                        print(ns_others['hostname']+' is down!')
                        continue
                    _, stdout, _ = self.ssh.exec_command('docker exec '+ns['hostname'][-2:]+' ip netns exec fake-'+ns['uuid']+' ping -c 2 '+ns_others['ip_address'])
                    output = stdout.read().decode()
                    
                    if '100% packet loss' in output:
                        notconnected.append(ns_others['hostname'])
                    else:
                        avgtimedelay = self.GetAvgPingTimeDelay(output)
                        pingtime.append(avgtimedelay)

                    # 进度条
                    percent = bar_i / (len(self.overlaynodes)-1) * 100
                    bar = '#' * int(percent / 2)
                    print(f'\r[{bar:<50}] {percent:.1f}%', end='')
                    bar_i+=1
            print()
            if len(notconnected):
                for notconnectedns in notconnected:
                    print(ns['hostname']+' and '+notconnectedns+' are not connected!')
            else:
                print('\t'+ns['hostname']+' and others are all connected!')
                print('\tAvg Time Delay = ' + f"{sum(pingtime)/len(pingtime):.4f}" + ' ms')
            pingtime=[]
            notconnected=[]
            self.ssh.exec_command('exit')
            self.ssh.close()



    def do_namespaces(self, arg):
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        result = []
        bar_i = 1
        hosts = self.workers[:]
        hosts.append(self.master)
        for host in hosts:
            self.ssh.connect(hostname=host, port=22, username='root', password='sdn123456')
            # 执行命令
            _, stdout, _ = self.ssh.exec_command("docker ps -a | grep -E 'ubuntu' | awk '{print $1}'")
            compute_nodes = []
            for node in stdout:
                compute_nodes.append(node.replace("\n", ""))
            for node in compute_nodes:
                _, stdout, _ = self.ssh.exec_command('docker exec ' + node + ' ip netns ls')
                for fake in stdout:
                    print(fake)
            self.ssh.exec_command('exit')
            self.ssh.close()

        # for i in range(self.mapnodesnum):
        #     # 进度条
        #     percent = bar_i / (self.mapnodesnum) * 100
        #     bar = '#' * int(percent / 2)
        #     print(f'\r[{bar:<50}] {percent:.1f}%', end='')
        #     bar_i+=1
        #     self.ssh.connect(hostname=self.underlaynodesmap['h' + str(i + 1)], port=22, username='root', password='sdn123456')

        #     _, stdout, _ = self.ssh.exec_command('docker exec h' + str(i + 1) + ' ip netns ls')
        #     # print('h' + str(i + 1) + " netns: ")
        #     output = stdout.read().decode()
        #     result.append('\nh' + str(i + 1) + " netns: ")
        #     for ns in self.overlaynodes:
        #         if ns['hostname'][-2:] == 'h' + str(i + 1):
        #             if 'fake-' + ns['uuid'] in output:
        #                 # print('\t' + ns['hostname'] + ' -> ' + 'fake-' + ns['uuid'])
        #                 result.append('\t' + ns['hostname'] + ' -> ' + 'fake-' + ns['uuid'])
            
        #     # print()

    def do_routernamespaces(self, arg):
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        result = []
        bar_i = 1
        for i in range(self.mapnodesnum):
            # 进度条
            percent = bar_i / (self.mapnodesnum) * 100
            bar = '#' * int(percent / 2)
            print(f'\r[{bar:<50}] {percent:.1f}%', end='')
            bar_i+=1
            self.ssh.connect(hostname=self.underlaynodesmap['h' + str(i + 1)], port=22, username='root', password='sdn123456')

            _, stdout, _ = self.ssh.exec_command('docker exec h' + str(i + 1) + ' ip netns ls')
            output = stdout.read().decode()
            for router in self.overlayrouters:
                if 'qrouter-' + router['id'] in output:
                    result.append(router['name'] + ' -> ' + 'h' + str(i + 1) + ' -> ' + 'qrouter-' + router['id'])
            self.ssh.exec_command('exit')
            self.ssh.close()
        print()
        for i in result:
            print(i)

    def GetUnderlayNodesMap(self):
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        underlaynodesmap={}
        for worker in self.workers:
            self.ssh.connect(hostname=worker, port=22, username='root', password='sdn123456')
            _, stdout, _ = self.ssh.exec_command('docker ps -a | grep ubuntu | awk \'{print"", $10}\'')
            overlaynodes = stdout.read().decode().split(' ')
            del overlaynodes[0]
            overlaynodes = ''.join(overlaynodes).split('\n')
            del overlaynodes[-1]
            for node in overlaynodes:
                underlaynodesmap[node] = worker
        return underlaynodesmap
    do_nss = do_namespaces
    do_rnss = do_routernamespaces
    

# if __name__ == '__main__':
#     Overlay()
