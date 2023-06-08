#!/usr/bin/python3
# -*- coding:utf-8 -*-

import sys
import os
import subprocess
from typing import NewType
import utils

# 映射到三个系统
# sysId = {"computer11": 0, "computer12": 0, "computer13": 0, "computer14": 0,
#          "computer15": 1, "computer16": 1, "computer21": 1, "computer22": 1,
#          "computer23": 2, "computer24": 2, "computer25": 2, "computer26": 2}

#   映射到物理主机
zone = ["nova:computer11:computer11", "nova:computer12:computer12", "nova:computer13:computer13", "nova:computer14:computer14",
        "nova:computer15:computer15", "nova:computer16:computer16", "nova:computer21:computer21", "nova:computer22:computer22",
        "nova:computer23:computer23", "nova:computer24:computer24", "nova:computer25:computer25", "nova:sdn20:sdn20"]


def raw_str_parse(raw_str):
    lt = raw_str.split("], [")
    lt[0] = lt[0].split("[[")[-1].strip()
    lt[-1] = lt[-1].split("]]")[0].strip()
    # print(lt)
    ret = []
    for i in range(len(lt)):
        temp = lt[i].split("),")
        temp[0] = temp[0].split('(')[-1]
        temp[0] = temp[0].split(',')
        temp[1] = temp[1].split(',')
        for j in range(len(temp[0])):
            temp[0][j] = temp[0][j].strip()
        for j in range(len(temp[1])):
            temp[1][j] = temp[1][j].strip()

        ret.append(temp)
    # print(ret)
    return ret


def openstack_flavor_create(cpu, ram, bandwidth):
    flavor = "cpu_"+cpu+"_ram_"+ram+"_bandwidth_"+bandwidth
    temp = utils.execShellCommand("openstack flavor list | grep "+flavor)
    if len(temp) == 0:
        utils.execShellCommand(
            "openstack flavor create --vcpus "+cpu+" --ram "+ram+" --disk 10 "+flavor)
    return flavor

# 全局网络创建
# def openstack_network_create(name):
#     network = "network_"+ name
#     temp = utils.execShellCommand("openstack network list | grep "+network)
#     if len(temp) == 0:
#         utils.execShellCommand(
#             "openstack network create --share "+network)
#     return network

# def openstack_subnet_create(network,name):
#     subnet = "subnet_"+ name
#     temp = utils.execShellCommand("openstack network list | grep "+subnet)
#     if len(temp) == 0:
#         utils.execShellCommand(
#             "openstack subnet create --network " + network + " --subnet-range 1.1.1.0/24 " + subnet)
#     return subnet


def openstack_create_instance(info, my_id):
    flavor = openstack_flavor_create(info[0][0], info[0][1], info[0][2])
    cmd = "openstack server create --flavor "+flavor+" --image ubuntu_sdnlab "
    cmd += "--nic net-id=network_ljw --key-name ljwkey --user-data config/ubuntu.config --availability-zone " + \
        zone[int(info[1][0])] + " "+my_id
    utils.execShellCommand(cmd)


def openstack_delete_instance():
    ret = utils.execShellCommand(
        "openstack server list |grep ubuntu").split("\n")[0:-1]
    for i in range(len(ret)):
        id = ret[i].split("|")[1].strip()
        utils.execShellCommand("openstack server delete "+id)


if __name__ == "__main__":

    print("******************************************************************")
    print("***                 project: P"+sys.argv[2])
    print("******************************************************************")
    # computer_name = utils.execShellCommand("uname -n").strip()
    if sys.argv[1] == "create":
        raw_str = utils.read_file_as_str(sys.argv[3])
        pd_list = raw_str_parse(raw_str)
        instance_id = 0
        for i in range(len(pd_list)):
            info = pd_list[i]
            if sys.argv[2] == info[1][2]:
                # 租户ID,实例ID,sysID,zoneID,vCPU,RAM,带宽
                # info[0][0] : vcpu                  #info[0][1] : RAM               #info[0][2] : bandwidth
                # info[1][0] : zoneID                #info[1][1] : sysID             #info[1][2] : 租户ID
                # 如果是-1，部署在最后一个物理机上，整个数据没有将所有资源占满，一般最后一个PM的资源是最空的
                if info[1][0] == -1:
                    info[1][0] = len(zone)-1
                my_id = "P"+str(sys.argv[2])+"_I"+str(instance_id)+"_S"+str(info[1][1])+"_Z"+str(
                        info[1][0])+"_C"+str(info[0][0])+"_R"+str(info[0][1])+"_B"+str(info[0][2])
                openstack_create_instance(info, my_id)
                instance_id += 1
    elif sys.argv[1] == "delete":
        openstack_delete_instance()

