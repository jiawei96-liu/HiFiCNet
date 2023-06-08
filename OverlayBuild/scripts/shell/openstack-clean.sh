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
	echo "Usage: 	./openstack-clean.sh server 	## delete all servers"
	echo "	./openstack-clean.sh router	## delete all routers"
	echo "	./openstack-clean.sh network 	## delete all networks"
	echo "	./openstack-clean.sh all    	## delete all all"
elif [[ $1 == "server" ]];then
	#server_id=`openstack server list | grep "h1" | awk '{print $2}'`
	server_id=`openstack server list | grep "cirros" | awk '{print $2}'`
	#server_id=`openstack server list | grep ERROR | awk '{print $2}'`
	#echo "server_id = ${server_id}"

	if [[ -z  ${server_id} ]];then
		sleep 1
		#echo "Server Clean: No Server!"
	else
		shX openstack server delete ${server_id}
		#echo "Server Clean: Done!"
	fi
# 只做了路由器绑定一个内部网络端口的场景的代码
elif [[ $1 == "router" ]];then
	router_id=`openstack router list | grep router | awk '{print $2}'`
	#echo "router_id = ${router_id}"
	
	if [[ -z  ${router_id} ]];then
		sleep 1
		#echo "Router Clean: No Router!"
	else
		array=(`echo $router_id | tr '\n' ' '` )

        	for var in ${array[@]}
        	do
                	port_id=`openstack router show ${var} | grep interfaces_info | awk '{print $5}' | sed 's/,//g' |sed 's/"//g'`
			#echo "port_id = ${port_id}"

			if [[ ${port_id} == "|" ]];then
				sleep 1
				#echo "Router Port Clean: No Port!"
			else
				shX openstack router remove port ${var} ${port_id}
			fi
			shX openstack router delete ${var}
			#echo "Router Clean: Done!"
        	done
	fi
elif [[ $1 == "network" ]];then
	./openstack-clean.sh router
	subnet_id=`openstack subnet list | grep / | awk '{print $2}'`
	if [[ -z  ${subnet_id} ]];then
                sleep 1
        else
		array=(`echo $subnet_id | tr '\n' ' '` )

                for var in ${array[@]}
                do
                        shX openstack subnet delete ${var}
                done
	fi
	network_id=`openstack network list | grep "|" | grep -v "ID" | awk '{print $2}'`
        if [[ -z  ${network_id} ]];then
                sleep 1
        else
                array=(`echo $network_id | tr '\n' ' '` )

                for var in ${array[@]}
                do
                        shX openstack network delete ${var}
                done
        fi

elif [[ $1 == "all" ]];then
	./openstack-clean.sh server
	./openstack-clean.sh router
	./openstack-clean.sh network
	shX openstack security group delete sg-admin-1
fi
