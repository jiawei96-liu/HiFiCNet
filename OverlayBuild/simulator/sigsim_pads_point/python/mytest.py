from horse import *

#sw1 = SDNSwitch(42)
# sw2 = SDNSwitch(42)
# sw3 = SDNSwitch(42)

h1 = Host(str("h1").encode('utf-8'))
h2 = Host(str("h2").encode('utf-8'))
h3 = Host(str("h3").encode('utf-8'))
h4 = Host(str("h4").encode('utf-8'))
h5 = Host(str("h5").encode('utf-8'))
h6 = Host(str("h6").encode('utf-8'))
h7 = Host(str("h7").encode('utf-8'))
h8 = Host(str("h8").encode('utf-8'))
h9 = Host(str("h9").encode('utf-8'))

r1 = Router(str("r1").encode('utf-8'))
r2 = Router(str("r2").encode('utf-8'))
r3 = Router(str("r3").encode('utf-8'))
r4 = Router(str("r4").encode('utf-8'))
#sw1.add_port(1, "00:00:00:00:01:00")
#sw1.add_port(2, "00:00:00:00:02:00")

h1.add_port(1, "00:00:00:00:00:01", ip = "10.0.0.1", netmask = "255.255.255.0" )

#h2.add_port(2, "00:00:00:00:00:03", ip = "10.0.0.3", netmask = "255.255.255.0")
h2.add_port(1, "00:00:00:00:00:02", ip = "10.0.0.2", netmask = "255.255.255.0")
h3.add_port(1, "00:00:00:00:00:03", ip = "10.0.0.3", netmask = "255.255.255.0")
#h3.add_port(1, "00:00:00:00:00:02", ip = "10.0.0.2", netmask = "255.255.255.0")
h4.add_port(1, "00:00:00:00:00:04", ip = "10.0.0.4", netmask = "255.255.255.0" )

#h2.add_port(2, "00:00:00:00:00:03", ip = "10.0.0.3", netmask = "255.255.255.0")
h5.add_port(1, "00:00:00:00:00:05", ip = "10.0.0.5", netmask = "255.255.255.0")
h6.add_port(1, "00:00:00:00:00:06", ip = "10.0.0.6", netmask = "255.255.255.0")
h7.add_port(1, "00:00:00:00:00:07", ip = "10.0.0.7", netmask = "255.255.255.0" )

#h2.add_port(2, "00:00:00:00:00:03", ip = "10.0.0.3", netmask = "255.255.255.0")
h8.add_port(1, "00:00:00:00:00:08", ip = "10.0.0.8", netmask = "255.255.255.0")
h9.add_port(1, "00:00:00:00:00:09", ip = "10.0.0.9", netmask = "255.255.255.0")

r1.add_port(1, "00:00:00:00:00:10")
r1.add_port(2, "00:00:00:00:00:11")
r1.add_port(3, "00:00:00:00:00:12")
r1.add_port(4, "00:00:00:00:00:13")

r2.add_port(1, "00:00:00:00:00:20")
r2.add_port(2, "00:00:00:00:00:21")
r2.add_port(3, "00:00:00:00:00:22")
r2.add_port(4, "00:00:00:00:00:23")

r3.add_port(1, "00:00:00:00:00:30")
r3.add_port(2, "00:00:00:00:00:31")
r3.add_port(3, "00:00:00:00:00:32")
r3.add_port(4, "00:00:00:00:00:33")

r4.add_port(1, "00:00:00:00:00:40")
r4.add_port(2, "00:00:00:00:00:41")
r4.add_port(3, "00:00:00:00:00:42")

start_time = 1000000
end_time = 10 * start_time
#h1.ping("10.0.0.2", start_time)

#h1.ping("10.0.0.3", start_time * 2)

h1.ping("10.0.0.7", start_time)
topo = Topology()
#topo.add_node(sw1)
topo.add_node(h1)
topo.add_node(h2)
topo.add_node(h3)
topo.add_node(h4)
topo.add_node(h5)
topo.add_node(h6)
topo.add_node(h7)
topo.add_node(h8)
topo.add_node(h9)
topo.add_node(r1)
topo.add_node(r2)
topo.add_node(r3)
topo.add_node(r4)

topo.add_link(h1, r1, 1, 1)
topo.add_link(h2, r1, 1, 2)
topo.add_link(h3, r1, 1, 3)

topo.add_link(h6, r2, 1, 1)
topo.add_link(h4, r2, 1, 2)
topo.add_link(h5, r2, 1, 3)

topo.add_link(h9, r3, 1, 1)
topo.add_link(h7, r3, 1, 2)
topo.add_link(h8, r3, 1, 3)

topo.add_link(r4,r1,1,4)
topo.add_link(r4,r2,2,4)
topo.add_link(r4,r3,3,4)

#topo.add_link(h1, h2, 1, 1)
#topo.add_link(h1, h2, 1, 2)
#topo.add_link(sw1, h2, 2, 1)

#sim = Sim(topo)
sim = Sim(topo, ctrl_interval = 100000, end_time = end_time, log_level = LogLevels.LOG_INFO)
sim.start()
