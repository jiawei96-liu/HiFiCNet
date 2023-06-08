role="controller"


if [[ $role == "controller" ]];then
    #vx_00
    flag=`ifconfig | grep admin-br | wc -l`
    if [[ $flag == 1 ]];then 
        ifconfig admin-br down &&\
        brctl delbr admin-br
    fi
    flag=`ifconfig | grep admin | wc -l`
    if [[ $flag == 1 ]];then 
        ifconfig admin down &&\
        tunctl -d admin
    fi
    flag=`ifconfig | grep vx_00 | wc -l`
    if [[ $flag == 1 ]];then 
        ip link delete vx_00
    fi
    sleep 2

    brctl addbr admin-br &&\
    ifconfig admin-br 192.168.1.10/16 &&\
    tunctl -t admin  &&\
    brctl addif admin-br admin &&\
    ifconfig admin 192.168.1.11/16 &&\
    ip link set admin-br up &&\
    ip link set admin up &&\
    ip link add vx_00 type vxlan id 00 remote 172.16.41.16 local 172.16.41.11 dstport 4789 &&\
    ip link set up vx_00 &&\
    brctl addif admin-br vx_00 &&\
    ip link set up admin-br
elif [[ $role == "master" ]];then
    # vx_00
    flag=`ifconfig | grep vx_00 | wc -l`
    if [[ $flag == 1 ]];then 
        ip link delete vx_00
    fi
    sleep 3


    ip link add vx_00 type vxlan id 00 remote 172.16.41.11 local 172.16.41.16 dstport 4789 &&\
    ip link set up vx_00 &&\
    brctl addif admin-br vx_00 &&\
    ip link set up admin-br

    # vx_01
    flag=`ifconfig | grep data-br | wc -l`
    if [[ $flag == 1 ]];then 
        ifconfig data-br down  &&\
        brctl delbr data-br
    fi
    flag=`ifconfig | grep data | wc -l`
    if [[ $flag == 1 ]];then 
        ifconfig data down  &&\
        tunctl -d data
    fi
    flag=`ifconfig | grep vx_01 | wc -l`
    if [[ $flag == 1 ]];then 
        ip link delete vx_01
    fi
    sleep 5

    brctl addbr data-br &&\
    ifconfig data-br 10.10.0.2/16 &&\
    ip addr flush enp3s0 &&\
    brctl addif data-br enp3s0 &&\
    tunctl -t data  &&\
    brctl addif data-br data &&\
    ifconfig data 10.10.0.16/16 &&\
    ip link set data-br up &&\
    ip link set data up &&\
    ip link add vx_01 type vxlan id 01 remote 172.16.41.14 local 172.16.41.16 dstport 4788 &&\
    ip link set up vx_01 &&\
    brctl addif data-br vx_01 &&\
    ip link set up data-br

elif [[ $role == "worker" ]];then
    flag=`ifconfig | grep vx_01 | wc -l`
    if [[ $flag == 1 ]];then 
        ip link delete vx_01
    fi
    sleep 3

    ip link add vx_01 type vxlan id 01 remote 172.16.41.16 local 172.16.41.14 dstport 4788 &&\
    ip link set up vx_01 &&\
    brctl addif intf14 vx_01 &&\
    ip link set up intf14
fi

