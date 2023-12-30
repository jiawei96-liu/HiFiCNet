import os
import sys
import time
import pexpect

# ==================== Setting Section Begin==========================
# Base setting
# Number of nodes to be created
node_num = 1
# The starting index of the node name
node_index_start = 1
# The prefix before the node name
node_name_pre = "HifiCNet-Node-"

# Image setting
# Image type
image_type = "linux"
# Image version
image_version = "ubuntu22.04"
# Naming prefix of the created VM
image_name_pre = "HifiCNet-Node-"
# The base image on which VM creation depends
base_image_name = "HifiCNet-Node-Base.qcow2"
# The absolute addresses of the created VM and base image in the host. It is not recommended to modify them.
image_path = "/var/lib/libvirt/images/"
# Netplan file in the image
netplan_file = "01-network-manager-all.yaml"
# Username and password of the base VM image
username = "sdn"
password = "sdn123456"


# Network setting
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


def node_create():
    for i in range(node_num):
        # copy gateway node image
        image_name = image_path + image_name_pre + str(node_index_start+i) + ".qcow2"
        command_str = "cp %s%s %s" % (image_path, base_image_name, image_name)
        os.system(command_str)

        node_name = node_name_pre + str(node_index_start+i)
        # print(node_name + " Node create start")
        # gateway node create
        command_str = "virt-install --virt-type kvm --name %s --ram %s --vcpus %s "\
                      "--network bridge=%s,model=virtio " \
                      "--network bridge=%s,model=virtio " \
                      "--network bridge=%s,model=virtio " \
                      "--graphics none --noautoconsole " \
                      "--os-type=%s --os-variant=%s --disk %s,device=disk,bus=virtio --import" % \
                      (node_name, memory, vcpus, provider_network_server_bridge, manager_network_server_bridge, overlay_network_server_bridge, image_type, image_version, image_name)
        os.system(command_str)
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
        command_str = "virsh snapshot-create-as "  + node_name + " snap_init"
        os.system(command_str)
        print(node_name + " Network configure completed")


def node_initialize():
    node_create()
    time.sleep(180)
    network_configure()
    

def clean_node():
    for i in range(node_num):
        node_name = node_name_pre + str(node_index_start + i)
        image_name = image_path + image_name_pre + str(node_index_start + i) + ".qcow2"
        command_str = "virsh snapshot-list " + node_name + " | grep : |awk '{print $1}'"
        snapshot = os.popen(command_str).read()
        if snapshot != "" :
            command_str = "virsh snapshot-delete " + node_name + " " + snapshot
        os.system(command_str)
        command_str = "virsh destroy " + node_name
        os.system(command_str)
        command_str = "virsh undefine " + node_name
        os.system(command_str)
        command_str = "rm -f " + image_name
        os.system(command_str)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Parameters miss")
        exit()
    op_str = sys.argv[1]
    if op_str == "init":
        print("HifiCNet nodes initialize")
        node_initialize()
    elif op_str == "clean":
        print("HifiCNet nodes clean")
        clean_node()
    else:
        print("Parameters error")
