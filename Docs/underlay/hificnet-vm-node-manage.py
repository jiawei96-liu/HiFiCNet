import sys
import time
import pexpect
import subprocess

# ==================== Setting Section Begin==========================
# Base setting
# Number of nodes to be created
node_num = 1
# The starting index of the node name
node_index_start = 1
# The prefix before the node name
node_name_pre = "HifiCNet-Node-VM"

# Image setting
# Image type
image_type = "ubuntu22.04"
# Naming prefix of the created VM
image_name_pre = "HifiCNet-Node-VM"
# The base image on which VM creation depends
base_image_name = "HifiCNet-Node-VM-Base.qcow2"
# The absolute addresses of the created VM and base image in the host. It is not recommended to modify them.
image_path = "/var/lib/libvirt/images/"
# Netplan file in the image
netplan_file = "01-network-manager-all.yaml"
# Username and password of the base VM image
username = "sdn"
password = "sdn123456"


# Network setting, 需要和server_network_prepare保持一致
# The starting IP of the node
node_ip_start = 151
# Provider network used in openstack
provider_network_ip_seg = "172.16.50."
provider_network_gateway_ip = "172.16.50.254"
provider_network_nic = "enp1s0"
# The bridge connecting the network in the host server
provider_network_server_bridge = "br-50"

# Manager network used in openstack
manager_network_ip_seg = "10.10.0."
manager_network_nic = "enp2s0"
manager_network_server_bridge = "br-10"

# Overlay network used in openstack
overlay_network_ip_seg = "20.20.0."
overlay_network_nic = "enp3s0"
overlay_network_server_bridge = "br-20"

# Flavor setting
vcpus = "2"
memory = "8192"





# ==================== Setting Section End==========================

# 注意，execShellCommand在宿主机上执行命令，VM内执行命令请使用process.sendline
def execShellCommand(cmd):
    print("\033[32m++ "+cmd+" ====>\033[0m")
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, executable="/bin/bash")
    ret = p.stdout.read().decode('utf-8')
    print(ret)
    return ret


def server_network_prepare():
    # provider_network
    command_str = "brctl addbr br-50"
    execShellCommand(command_str)
    command_str = "brctl addif br-50 eno1"
    execShellCommand(command_str)
    command_str = "ip addr flush dev eno1"
    execShellCommand(command_str)
    command_str = "ip link set eno1 mtu 9000"
    execShellCommand(command_str)
    command_str = "ifconfig br-50 172.16.50.4 netmask 255.255.255.0 broadcast 172.16.50.255 mtu 9000 up"
    execShellCommand(command_str)
    command_str = "route add default gw 172.16.50.254 dev br-50"
    execShellCommand(command_str)
    # manager_network
    command_str = "brctl addbr br-10"
    execShellCommand(command_str)
    command_str = "brctl addif br-10 eno4"
    execShellCommand(command_str)
    command_str = "ip addr flush dev eno4"
    execShellCommand(command_str)
    command_str = "ip link set eno4 mtu 9000"
    execShellCommand(command_str)
    command_str = "ifconfig br-10 10.10.0.4 netmask 255.255.255.0 broadcast 10.10.0.255 mtu 9000 up"
    execShellCommand(command_str)
    # manager_network
    command_str = "brctl addbr br-20"
    execShellCommand(command_str)
    command_str = "brctl addif br-20 eno3"
    execShellCommand(command_str)
    command_str = "ip addr flush dev eno3"
    execShellCommand(command_str)
    command_str = "ip link set eno3 mtu 9000"
    execShellCommand(command_str)
    command_str = "ifconfig br-20 20.20.0.4 netmask 255.255.255.0 broadcast 20.20.0.255 mtu 9000 up"
    execShellCommand(command_str)
    # check
    command_str = "brctl show"
    execShellCommand(command_str)

    

def node_create():
    for i in range(node_num):
        # copy image
        image_name = image_path + image_name_pre + str(node_index_start+i) + ".qcow2"
        command_str = "cp %s%s %s" % (image_path, base_image_name, image_name)
        execShellCommand(command_str)

        node_name = node_name_pre + str(node_index_start+i)
        # print(node_name + " Node create start")
        # node create
        command_str = "virt-install --virt-type kvm --name %s --ram %s --vcpus %s "\
                      "--network bridge=%s,model=virtio " \
                      "--network bridge=%s,model=virtio " \
                      "--network bridge=%s,model=virtio " \
                      "--network network=default,model=virtio " \
                      "--graphics none --noautoconsole " \
                      "--os-variant=%s --disk %s,device=disk,bus=virtio --import" % \
                      (node_name, memory, vcpus, provider_network_server_bridge, manager_network_server_bridge, overlay_network_server_bridge, image_type, image_name)
        execShellCommand(command_str)
        print(node_name + " Node create completed")


def network_configure():
    for i in range(node_num):
        node_name = node_name_pre + str(node_index_start+i)
        provider_network_ip = provider_network_ip_seg+ str(node_ip_start + i) + "/24"
        manager_network_ip = manager_network_ip_seg + str(node_ip_start+i) + "/24"
        overlay_network_ip = overlay_network_ip_seg + str(node_ip_start+i) + "/24"
        print("\n", node_name + " Network configure start")

        process_program = "virsh console " + node_name
        process = pexpect.spawn(process_program, timeout=20)
        time.sleep(2)
        # Need to be adjusted according to the actual image login interface
        process.expect("\)\r\n")
        process.sendline("")
        index = process.expect(["login:", "#", "\$"])
        if index == 0:
            # first login
            process.sendline(username)
            process.expect("Password:")
            process.sendline(password)
            process.expect("\$")
            process.sendline("")
            process.expect("\$")
            process.sendline("sudo su")
            process.expect("%s:" % username)
            process.sendline(password)
            process.expect("#")
            process.sendline("")
        elif index == 1:
            # login as root
            process.sendline("")
        else:
            # login as sdn
            process.sendline("")
            process.expect("\$")
            process.sendline("sudo su")
            process.expect("%s:" % username)
            process.sendline(password)
            process.expect("#")
            process.sendline("")
        process.expect("#")
        print("Login success")

        process.sendline("cd /etc/netplan")
        process.expect("#")
        process.sendline("cp %s %s.back" % (netplan_file, netplan_file))
        process.expect("#")

        net_message = "network:\n"\
                      + "    ethernets:\n"\
                      + "        " + provider_network_nic + ":\n"\
                      + "            dhcp4: false\n"\
                      + "            addresses:\n"\
                      + "              - " + provider_network_ip + "\n"\
                      + "            gateway4: " + provider_network_gateway_ip + "\n"\
                      + "        " + manager_network_nic + ":\n"\
                      + "            dhcp4: false\n"\
                      + "            addresses:\n"\
                      + "              - " + manager_network_ip + "\n"\
                      + "        " + overlay_network_nic + ":\n"\
                      + "            dhcp4: false\n"\
                      + "            addresses:\n"\
                      + "              - " + overlay_network_ip + "\n"\
                      + "    version: 2"
        echo_command = "echo \'%s\' > netconfiguration.yaml" % net_message
        process.sendline(echo_command)
        process.expect("#")
        process.sendline("rm -f %s" % netplan_file)
        process.expect("#")
        process.sendline("netplan apply")
        process.expect("#")
        print("netplan apply success")
        command_str = "sed -i 's/ubuntu22/"+ node_name + "/g' /etc/hostname"
        execShellCommand(command_str)
        command_str = "virsh snapshot-create-as "  + node_name + " snap_init"
        execShellCommand(command_str)
        print(node_name + " Network configure completed")
        command_str = "reboot"
        process.sendline(command_str)
        


def node_initialize():
    node_create()
    time.sleep(180)
    network_configure()
    

def clean_node():
    for i in range(node_num):
        node_name = node_name_pre + str(node_index_start + i)
        image_name = image_path + image_name_pre + str(node_index_start + i) + ".qcow2"
        command_str = "virsh snapshot-list " + node_name + " | grep : |awk '{print $1}'"
        snapshot = execShellCommand(command_str)
        if snapshot != "" :
            command_str = "virsh snapshot-delete " + node_name + " " + snapshot
        execShellCommand(command_str)
        command_str = "virsh destroy " + node_name
        execShellCommand(command_str)
        command_str = "virsh undefine " + node_name
        execShellCommand(command_str)
        command_str = "rm -f " + image_name
        execShellCommand(command_str)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Parameters miss")
        exit()
    op_str = sys.argv[1]
    if op_str == "create":
        print("HifiCNet nodes initialize")
        node_initialize()
    elif op_str == "clean":
        print("HifiCNet nodes clean")
        clean_node()
    elif op_str == "prepare":
        print("HifiCNet server network prepare")
        server_network_prepare()
    else:
        print("Parameters error")
