#!/usr/bin/python3
# -*- coding:utf-8 -*-

import sys
import os
import subprocess
from typing import NewType
import utils
import csv

if __name__ == "__main__":
    temp = utils.execShellCommand("openstack server list | grep -v +")
    temp = temp.split("\n")
    node_list=[]
    for _temp in temp:
        _temp = _temp.strip().split("|")[1:-1]
        node_list.append(_temp)
    for i in range(0,len(node_list)):
        for j in range(0,len(node_list[i])):
            node_list[i][j]=node_list[i][j].strip()

    
    node_list=node_list[0:-1]
    # print(node_list)
    for i in range(0,len(node_list)):
        if i == 0:
            node_list[i].append("Host")
        else:
            cmd = "openstack server show "+node_list[i][0]+ " | grep hypervisor_hostname"
            temp = utils.execShellCommand(cmd).split(" ")[3].strip()
            node_list[i].append(temp)

    with open('simulation-node-info-data.csv', 'w') as f:
        writer = csv.writer(f)
        for temp in node_list:
            writer.writerow(temp)