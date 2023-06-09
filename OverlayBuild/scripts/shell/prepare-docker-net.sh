# ip addr flush eno3
# #docker network create  --subnet 20.20.0.0/16 --gateway 20.20.0.4 network20
# brctl addif br-a149c07e9972 eno3
# ping 20.20.0.1 -c 2 -I br-a149c07e9972
# docker network rm network10
# #ip addr flush eno4
# #docker network create  --subnet 10.10.0.0/16 --gateway 10.10.0.4 network10
# #brctl addif br-e91b254f36d6 eno4
# #ping 10.10.0.1 -c 2 -I br-e91b254f36d6
# sysctl net.ipv4.conf.all.forwarding=1 
# iptables --policy FORWARD ACCEPT 
# mkdir /var/run/netns


ip addr flush $(ip addr | awk '/inet 20\./ {print $NF}')
# 要改一下网关gateway
docker network create  --subnet 20.20.0.0/16 --gateway 20.20.0.6 network20
brctl addif $(ip addr | awk '/inet 20\./ {print $NF}') eno3
ping 20.20.0.1 -c 2 -I $(ip addr | awk '/inet 20\./ {print $NF}')

sysctl net.ipv4.conf.all.forwarding=1
iptables --policy FORWARD ACCEPT
ulimit -n 196835

mkdir /var/run/netns