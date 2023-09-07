import m5
from m5.proxy import *
from m5.params import *
from m5.SimObject import SimObject

class SimpleMemObject(SimObject):
    type = "SimpleMemObject"
    cxx_header = "learning_gem5/part2/simple_mem_object.hh"
    cxx_class = "gem5::SimpleMemObject"
    
    inst_port = ResponsePort("CPU side port, receives requests for instructions")
    data_port = ResponsePort("CPU side port, receives requests for data")
    mem_bus_side = RequestPort("Memory side port, sends requests to the memory")
    