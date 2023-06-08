import utils
import sys
import os
import subprocess

#注意命令的空格
#全局网络创建
def openstack_network_create(name):
    network = "network_"+ name
    temp = utils.execShellCommand("openstack network list | grep "+network)
    if len(temp) == 0:
        utils.execShellCommand(
            "openstack network create --share "+network)
    return network

def openstack_subnet_create(name):
    subnet = "subnet_"+ name
    temp = utils.execShellCommand("openstack subnet list | grep "+subnet)
    if len(temp) == 0:
        utils.execShellCommand(
            "openstack subnet create --network network_" + name + " --subnet-range 1.1.1.0/24 " + subnet)
    return subnet

if __name__ == "__main__":
    if sys.argv[1] == "global_network_create":
        network = openstack_network_create(sys.argv[2])
        openstack_subnet_create(sys.argv[2])
    
