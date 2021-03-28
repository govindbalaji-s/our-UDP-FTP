V1_CHUNK_SIZE = 504

import threading

def sendto(fname, dest)):
    ip, port = dest

    ## read file 
    ## init sender's state

    ## handshake on udp

    ## infinite loop based on sender's state:
        ## receive on udp and call functions to update sender's state
        ## send on udp based on sender's state
    #
    #

def recv_at(src):
    ip, port = src
    #
    #

class SenderState:
    
    def __init__(self, data:bytes)
        self.data:bytes = data
        self.chunks = []
        ## TODO compute chunks
        ## for i in 
        self.unacked_chunks:set = set(range(len(self.chunks)))
        self.congestion:CongestionState = CongestionState()
        #self.flow = FlowState() TODO add flow control
        self.timers:list[threading.Timer] = [] 

    def ack_received():
        # update unacked chunks

class CongestionState:
    def __init__():
        self.cwnd:int = 0
        self.addconst:int = 1
        self.ssthresh:int = 1
        self.is_slow_start:bool = True
            
class Chunk :
    def __init__(self, payload, seq_num):
        self.payload = payload
        self.seq_num = seq_num
