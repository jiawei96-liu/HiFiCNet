#!/bin/bash
export OS_PROJECT_DOMAIN_NAME=Default
export OS_USER_DOMAIN_NAME=Default
export OS_PROJECT_NAME=admin
export OS_USERNAME=admin
export OS_PASSWORD=sdn123456
export OS_AUTH_URL=http://controller:5000/v3
export OS_IDENTITY_API_VERSION=3
export OS_IMAGE_API_VERSION=2

function shX(){
	CMD=$*
    	echo -e "\e[32m[$(date +"%F %T")] " EXEC: ${CMD} " \e[0m"
    	${CMD}
}


if [[ -z $1 ]];then
        echo "Usage:    ./openstack-server-create.sh {server-num} {host-num}"
elif [[ $1 == "init" ]];then        
	./openstack-clean.sh all
	shX openstack network create  --share --external --provider-physical-network provider --provider-network-type flat ext-network
	shX openstack subnet create --network ext-network --allocation-pool start=20.20.0.50,end=20.20.0.250  --dns-nameserver 114.114.114.114 --gateway 20.20.0.1 --subnet-range 20.20.0.0/16 ext-subnet
	shX openstack security group create sg-admin-1
	shX openstack security group rule create --protocol icmp sg-admin-1
	shX openstack security group rule create --protocol tcp sg-admin-1
	shX openstack security group rule create --protocol udp sg-admin-1
	shX openstack network create self1-network
	shX openstack network create self2-network
	shX openstack subnet create   --network self1-network   --dns-nameserver 114.114.114.114 --gateway 1.0.0.1   --subnet-range 1.0.0.0/8 self1-subnet
	shX openstack subnet create --network self2-network   --dns-nameserver 114.114.114.114 --gateway 2.0.0.1   --subnet-range 2.0.0.0/8 self2-subnet
	shX openstack subnet list
	shX openstack quota set --instances 1000000 --cores 1000000  --routers 1000000  --ram 1000000  --ports 1000000 admin
elif [[ $1 == "router" ]];then
	shX openstack router create router-ext-self1
	shX openstack router create router-ext-self2
	shX openstack router set --external-gateway ext-network --fixed-ip subnet=ext-subnet,ip-address=20.20.0.251 router-ext-self1
	shX openstack router set --external-gateway ext-network --fixed-ip subnet=ext-subnet,ip-address=20.20.0.252 router-ext-self2
	shX openstack router add subnet router-ext-self1 self1-subnet
	shX openstack router add subnet router-ext-self2 self2-subnet
else	
	#openstack server create --flavor m1.nano --image cirros   --nic net-id=self-network-1  --security-group sg-admin-1 --availability-zone nova:cn2:cn2  ljw-net1	
	let azbase=0
	let idbase=0
	#let time_single=0.5
	#let time_group=10
	for  ((idx=0;idx<$1;idx++))
	do
		CMD_BASE="openstack server create --flavor m1.nano --image cirros --security-group sg-admin-1 --nic net-id="
    		let az=${idx}%$2+1+${azbase}
		let id=${idx}+1+${idbase}
		if [[ ${idx}%2 -eq 0 ]];then
			CMD_BASE="${CMD_BASE}self1-network --availability-zone nova:h${az}:h${az} VM${id}-net1-h${az}"
		else
			CMD_BASE="${CMD_BASE}self2-network --availability-zone nova:h${az}:h${az} VM${id}-net2-h${az}"
		fi
		shX ${CMD_BASE} &
		sleep 1
		let flag=${idx}%30
		if [[ flag == 29 ]];then
			sleep 15
		fi
	done
	#shX openstack server list
fi
