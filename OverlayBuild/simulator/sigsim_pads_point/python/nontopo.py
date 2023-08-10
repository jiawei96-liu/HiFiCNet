from horse import *

topo = Topology()
start_time = 1000000
end_time = 10 * start_time
sim = Sim(topo, ctrl_interval = 100000, end_time = end_time, log_level = LogLevels.LOG_INFO)
sim.start()