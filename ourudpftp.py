V1_CHUNK_SIZE = 504

import threading
import math
from BitVector import BitVector

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
    
    def __init__(self, data:bytes):
        self.data:bytes = data
        self.populate_chunks()
        self.unacked_chunks:set = set(range(len(self.chunks)))
        self.congestion:CongestionState = CongestionState()
        #self.flow = FlowState() TODO add flow control
        self.timers:list[threading.Timer] = [] 

    # Breaks self.data into self.chunks
    def populate_chunks(self) -> None:
        self.chunks = []
        seqnum = 0
        ptr = 0
        while ptr < len(self.data):
            till = math.min(ptr + V1_CHUNK_SIZE, len(self.data))
            self.chunks.append(Chunk(data[ptr:till], seqnum))
            seqnum += 1
            ptr = till


    def ack_received():
        # update unacked chunks

class CongestionState:
    def __init__():
        self.cwnd:int = 0
        self.addconst:int = 1
        self.ssthresh:int = 1
        self.is_slow_start:bool = True
            
class Chunk :
    def __init__(self, payload:bytes, seq_num:int):
        self.payload = payload
        self.seq_num = seq_num

def write_chunks(chunks_list: list[Chunk], filename: str):
    f = open(filename, "wb")
    for chunk in chunks_list:
        data = chunk.payload
        f.write(data)
    f.close()
        
class Metadata :
    def __init__(self,number,name):
        self.noofchunks = number
        self.filename = name

class Receiverstate :
    def __init__(self,metadata):
        self.metadata = metadata
        self.chunks = []  
        self.pending_chunks:set = set(range(len(self.chunks)))
        self.temp_filepath

class Packet :
	def __init__(self, type:int, seqnum:int, payload, payload_length:int=None, checksum:int=None):
		self.version = 1
		self.type_ = type_
		self.payload_length = payload_length
		self.checksum = checksum
		self.seqnum = seqnum
		self.payload = payload

	@classmethod
	def fromVals(cls, type_:int, seqnum:int, payload):  ## Creating Packet at Sender
		pkt = cls(type_=type_, seqnum=seqnum, payload=payload)
		pkt.calc_paylen()
		pkt.calc_checksum()
		return pkt

	@classmethod
	def fromBytes(cls, data):  ## Creating packet at Receiver
		version = data[0] >> 4
		type_ = (data[0] >> 2) & 3
		payload_length = ((data[0] & 3) << 8) + data[1]
		checksum = int(data[2:4])
		seqnum = int(data[4:8])
		payload = data[8:]
		return cls(type_, seqnum, payload, payload_length, checksum)

	def to_bytes(self):
		# 4 bits version, 2 bits type, 10 bits payload length
		## Error handle version, type etc ranges TODO
		first = (self.version << 28) + (self.type << 26) + 
			(self.payload_length << 16) + self.checksum

		return first.to_bytes(4, 'big') + self.seqnum.to_bytes(4, 'big') + self.payload

	def calc_paylen():
		self.payload_length = len(self.payload)

	def calc_checksum():
		self.checksum = 0
		## Calculate checksum

