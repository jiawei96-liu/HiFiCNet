### KVM安装-ubuntu 22.04



1. 使用以下命令更新系统：

​		`sudo apt update`

2. 检查系统与KVM的兼容性, 在安装KVM之前，确保KVM与你的系统兼容。否则，你将不断遇到错误，KVM也无法正确安装。要检查两者之间的兼容性，运行此命令:  `egrep -c '(vmx|svm)' /proc/cpuinfo`   如果得到的输出大于0，那么意味着KVM与系统兼容，可以安装。

3. 安装必要的KVM依赖项：

​		`sudo apt install qemu-kvm libvirt-daemon-system virtinst virt-manager libvirt-clients bridge-utils`

4. 启用虚拟化所需的服务并检查它们的状态，成功安装必要的依赖项后，让我们启用需要进行虚拟化的服务。

   启用livirtd：

​		`sudo systemctl enable libvirtd`

​		启动libvirtd：

​		`sudo systemctl start libvirtd`

​		验证libvirtd的状态。如果运行正常，将在输出中看到'active'。

​		`sudo systemctl status libvirtd`

5. 将用户添加到KVM和Libvirt组，以便启动KVM并避免遇到问题：

​		`sudo usermod -aG kvm $USER`
​		现在使用此命令将用户添加到libvirt组：

​		`sudo usermod -aG libvirt $USER`

6. 为虚拟机（VMs）创建一个桥接网络, 这可以通过创建一个netplan配置文件来完成：

​		`sudo vim /etc/netplan/01-network-manager-all.yaml`
​		现在在01-netcfg.yaml文件中添加以下配置：

```network:
    ethernets:
        eth0:
            dhcp4: false
            dhcp6: false
    bridges:
        br0:
            interfaces: [eth0]
            dhcp4: false
            addresses: [192.168.1.10/24]
            macaddress: 01:26:3b:4b:1d:43
            routes:
                - to: default
                  via: 192.168.1.2
                  metric: 100
            nameservers:
                addresses: [8.8.8.8,144.144.144.144]
            parameters:
                stp: false
            dhcp6: flse
    version: 2
```

7. 使用Netplan应用更改并查看更改， 要应用配置，运行以下命令：

​		`sudo netplan apply`
​	8. 使用ip addr命令这样查看更改：

​		`ip addr show`



## QCOW2镜像制作-ubuntu 22.04

为了方便的VM创建，可以利用KVM，将常见的iso镜像制作为qcow2镜像。

1. 下载ubuntu 22.04 LST的ISO镜像，并放置在正确的路径下。

   https://releases.ubuntu.com/

   `mv ubuntu-22.04.1-desktop-amd64.iso  /var/lib/libvirt/images/`

2. 启动KVM的GUI，远程的话可以利用MobaXterm等工具启动。

​		`virt-manager`

3. 基于ISO镜像，利用virt-manager创建VM。

4. 创建完毕后，在/var/lib/libvirt/images/路径下可以看到对应的qcow2镜像

5. 下面开始虚拟机的初步定制，重启虚拟机在Software&Updates中替换apt源

6. 虚拟机中执行 `apt update & apt upgrade`

7. 解决virsh console方式无法连接终端的问题，virt-manager进入终端进行配置，不同的系统配置方法如下：

   ```
   （1）Ubuntu 22.04
   # vim /boot/grub/grub.cfg
   在第一个“linux   /boot/vmlinuz-6.2.0-39-generic”（大约在159行）的同一行后额外添加：
   consoleblank=0 console=tty1 console=ttyS0,115200n8
   # reboot 重启
   
   （2）Ubuntu 18.04
   # vim /etc/securetty
   在最后一行添加 ttyS0
   # vim /boot/grub/grub.cfg
   在第一个“linux   /boot/vmlinuz-4.15.0-140-generic”（大约在143行）的同一行后额外额外添加：
   consoleblank=0 console=tty1 console=ttyS0,115200n8
   # reboot 重启
   
   （3）Ubuntu 20.04
   # vim /boot/grub/grub.cfg
   在第一个“linux   /vmlinuz-5.4.0-70-generic”（大约在168行）的同一行后额外添加：
   consoleblank=0 console=tty1 console=ttyS0,115200n8
   # reboot 重启
   ```

8. 安装常见的工具 `apt install vim && apt install net-tools && apt install openssh-server`  

9. VM shut down

10. 镜像体积压缩：

    `virt-sparsify -x --compress --convert qcow2 ubuntu22.04-base.qcow2 ubuntu-server.qcow2`

11. /var/lib/libvirt/images/ubuntu-server.qcow2即为制作好的VM镜像

## 宿主机网络设置

1. 宿主机上创建VM依赖的网络环境

```
brctl addbr br-50
brctl addif br-50 eno1
ip addr flush dev eno1
ip link set eno1 mtu 9000
ifconfig br-50 172.16.50.4 netmask 255.255.255.0 broadcast 172.16.50.255 mtu 9000 up
route add default gw 172.16.50.254 dev br-50

brctl addbr br-10
brctl addif br-10 eno4
ip addr flush dev eno4
ip link set eno4 mtu 9000
ifconfig br-10 10.10.0.4 netmask 255.255.255.0 broadcast 10.10.0.255 mtu 9000 up

brctl addbr br-20
brctl addif br-20 eno3
ip addr flush dev eno3
ip link set eno3 mtu 9000
ifconfig br-20 20.20.0.4 netmask 255.255.255.0 broadcast 20.20.0.255 mtu 9000 up

brctl show
```

## VM的创建，连接与删除

1. 运行脚本，构建宿主机网络环境

```
python3 hificnet-vm-node-manage.py prepare
```

2. 运行脚本，批量起虚拟机，注意设置合适的参数

```
python3 hificnet-vm-node-manage.py create
```

3. 可以使用virsh console或者ssh远程连接虚拟机

4. 运行脚本，批量删除虚拟机

```
python3 hificnet-vm-node-manage.py clean
```

## KVM常用操作

1. 镜像

```
（1）创建镜像文件
 	qemu-img create -f qcow2 /var/lib/libvirt/images/ubuntu.qcow2 10G
（2）查看镜像文件信息
	qemu-img info ubuntu.qcow2
（3）镜像格式转换（qemu-img）
	qemu-img convert -O qcow2 /tmp/ubuntu.raw /tmp/ubuntu.qcow2
	qemu-img convert -O raw /tmp/ubuntu.qcow2 /tmp/ubuntu.raw
（4）镜像格式转换（virt-sparsify）
	virt-sparsify --compress --convert qcow2 ubuntu.img ubuntu.qcow2
（5）镜像扩容
	qemu-img resize ubuntu.qcow2 +1G  （似乎没用）
	qemu-img create -f qcow2 ubuntu-miza-big.qcow2 40G
	virt-resize --expand /dev/sda2 ubuntu-miza.qcow2 ubuntu-miza-big.qcow2
（6）镜像压缩
	virt-sparsify -x --compress --convert qcow2 ubuntu.qcow2 ubuntu-server.qcow2
（7） 导入镜像
	virt-install --virt-type kvm --name node --ram 8192 --vcpus 4 --import --disk ubuntu-server.qcow2 --network network=default --graphics vnc,listen=0.0.0.0 --noautoconsole --os-type=linux --os-variant=ubuntu18.04 
```
2.  kvm virsh常用命令

```
virsh list
virsh console instance-00000008
其他可以查看 https://www.cnblogs.com/g2thend/p/12976252.html
```

3. kvm 快照管理
```
root@server4:~# virsh list
 Id   Name              State
---------------------------------
 14   HifiCNet-node-1   running
 15   HifiCNet-node-2   running
 16   HifiCNet-node-3   running


root@server4:~# virsh snapshot-create-as HifiCNet-node-1 snap-init
Domain snapshot snap-init created
root@server4:~# virsh snapshot-create-as HifiCNet-node-2 snap-init
Domain snapshot snap-init created
root@server4:~# virsh snapshot-create-as HifiCNet-node-3 snap-init
Domain snapshot snap-init created

root@server4:~# virsh snapshot-list HifiCNet-node-1
 Name        Creation Time               State
--------------------------------------------------
 snap-init   2023-12-23 22:40:24 +0800   running

恢复快照
恢复快照虚拟机必须关机
虚拟机关机：virsh shutdown HifiCNet-node-1
检查虚拟机是否已经关机：virsh domstate HifiCNet-node-1
快照恢复：virsh snapshot-revert HifiCNet-node-1 snap-init

```