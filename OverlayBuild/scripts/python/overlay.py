import os
import warnings
warnings.filterwarnings("ignore")
import docker
import time
import pymysql
import paramiko
from optparse import OptionParser


class Overlay():
    
    def __init__(self):
        self.mysql_host = '10.10.0.11'
        self.mysql_user = 'root'
        self.mysql_password = 'sdn123456'
        self.user = 'sdn'
        self.rootuser = 'root'
        self.password = 'sdn123456'
        self.controller = '172.16.41.11'
        self.master = '172.16.41.16'
        self.workers = ['172.16.41.14', '172.16.41.15']
        self.mapnodesnum = 4
        self.ssh = paramiko.SSHClient()
        self.overlaynets = self.GetOverlayNets()
        self.overlaysubnets = self.GetOverlaySubnets()
        self.overlayrouters = self.GetOverlayRouters()
        self.underlaynodesmap = self.GetUnderlayNodesMap()
        self.overlaynodes = self.GetOverlayNodes()
        self.parseArgs()
        
    def parseArgs( self ):
        opts = OptionParser()
        opts.add_option( '--nets', action='callback', callback=self.OverlayNets)
        opts.add_option( '--subnets', action='callback', callback=self.OverlaySubnets)
        opts.add_option( '--routers', action='callback', callback=self.OverlayRouters)
        opts.add_option( '--nodes', action='callback', callback=self.OverlayNodes)
        opts.add_option( '--pingall', action='callback', callback=self.OverlayPingAll)
        opts.add_option( '--nss', action='callback', callback=self.OverlayNss)
        opts.add_option( '--routernss', action='callback', callback=self.OverlayRouterNss)
        

        self.options, self.args = opts.parse_args()

        # We don't accept extra arguments after the options
        if self.args:
            opts.print_help()
            exit()

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
    def OverlayNets(self, _option, _opt_str, _value, _parser):
        for i in self.overlaynets:
            print(i)

    def GetOverlaySubnets(self):
        sql = "select subnet.id, subnet.name, subnet.cidr, subnet.gateway_ip from neutron.subnets as subnet;"
        overlaysubnets = self.SearchInMysql(sql)
        return overlaysubnets
    def OverlaySubnets(self, _option, _opt_str, _value, _parser):
        for i in self.overlaysubnets:
            print(i)

    def GetOverlayRouters(self):
        sql = "select router.id, router.name from neutron.routers as router;"
        overlayrouters = self.SearchInMysql(sql)
        return overlayrouters
    
    def OverlayRouters(self, _option, _opt_str, _value, _parser):
        for i in self.overlayrouters:
            print(i)

    
    def GetOverlayNodes(self):
        sql = "select instance.uuid,instance.hostname,ip.ip_address \
            from nova.instances as instance, neutron.ports as port, neutron.ipallocations as ip \
            where port.id=ip.port_id and instance.uuid=port.device_id \
            order by instance.hostname;"
        overlaynodes = self.SearchInMysql(sql)
        return overlaynodes
    
    def OverlayNodes(self, _option, _opt_str, _value, _parser):
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

    def OverlayPingAll(self, _option, _opt_str, _value, _parser):
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



    def OverlayNss(self, _option, _opt_str, _value, _parser):
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
            # print('h' + str(i + 1) + " netns: ")
            output = stdout.read().decode()
            result.append('\nh' + str(i + 1) + " netns: ")
            for ns in self.overlaynodes:
                if ns['hostname'][-2:] == 'h' + str(i + 1):
                    if 'fake-' + ns['uuid'] in output:
                        # print('\t' + ns['hostname'] + ' -> ' + 'fake-' + ns['uuid'])
                        result.append('\t' + ns['hostname'] + ' -> ' + 'fake-' + ns['uuid'])
            
            # print()
            
            self.ssh.exec_command('exit')
            self.ssh.close()
        print()
        for i in result:
            print(i)

    def OverlayRouterNss(self, _option, _opt_str, _value, _parser):
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
    
    

if __name__ == '__main__':
    Overlay()
