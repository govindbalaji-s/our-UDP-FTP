V1_MAX_MSG_SIZE = 512
V1_CHUNK_SIZE = 504
V1_TYPE_MDATA = 0
V1_TYPE_DATA = 1
V1_TYPE_MDATA_ACK = 2
V1_TYPE_DATA_ACK = 3

import threading
import math
import socket

def sendto(fname, dest):
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

class Sender:
    
    def __init__(self, myaddr:(str, int), dest:(str, int), fname:str):
        self.myaddr = myaddr
        self.dest:(str, int) = dest
        self.fname:str = fname
        with open(fname, 'rb') as file:
            self.data:bytes = file.read()
        self.populate_chunks()
        self.unacked_chunks:set = set(range(len(self.chunks)))
        self.congestion:CongestionState = CongestionState()
        #self.flow = FlowState() TODO add flow control
        self.timeoutval = 0.2
        self.timers:list[threading.Timer] = [] 

    def do_handshake(self):
        mdata = Metadata(len(self.chunks), self.fname)
        pload = mdata.to_bytes()
        mdpkt = Packet.fromVals(V1_TYPE_MDATA, 0, pload)

        sock = socket.socket(socket.AF_INET, sock.SOCK_DGRAM)
        sock.bind(self.myaddr)
        sock.settimeout(self.timeoutval) # Initial value 0.2ms

        while True:
            try:
                sock.sendto(mdpkt.to_bytes(), self.dest)
                data, src = sock.recvfrom(512)
                ackpkt = Packet.fromBytes(data)
                if ackpkt.verify_checksum() and ackpkt.type == V1_TYPE_MDATA_ACK:
                    break

            except socket.timeout:
                continue

    def send_data(self):    #func to start sending data, and recieving ACKs for sent data.

        def listen_for_acks():
            while True:
                data, src = sock.recvfrom(512)
                ackpkt = Packet.fromBytes(data)
                ##verify ack packet appriately
                if ackpkt.verify_checksum() and ackpkt.type == V1_TYPE_MDATA_ACK:
                    ##lock here?
                    self.unacked_chunks.discard(ackpkt.seqnum)  ##if already acked, then does nothing
                else:
                    pass
                    ##Do nothing??
                if len(self.unacked_chunks) == 0:
                    break

        thread_ACK = threading.Thread(target=listen_for_acks)
        thread_ACK.start()

        for seq_num in self.unacked_chunks: ##How do we put a lock here? (:
            utp_pkt = Packet(type_ = 1 , seqnum = seq_num, payload = self.chunks[seq_num], checksum = 0)
            utp_pkt.calc_paylen()
            test_msg = utp_pkt.to_bytes()   ##test_msg has checksum = 0, and is entirely in bytes
            utp_pkt.calc_checksum()         ##Checksum computed and updated

            sock = socket.socket(socket.AF_INET, sock.SOCK_DGRAM)
            sock.bind(self.myaddr)
            sock.sendto(utp_pkt.to_bytes(), self.dest)
            ##Ig some sort of timeout here before sending again?

        thread_ACK.join()


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


    def ack_received(self):
        pass
        # update unacked chunks

class CongestionState:
    def __init__(self):
        self.cwnd:int = 0
        self.addconst:int = 1
        self.ssthresh:int = 1
        self.is_slow_start:bool = True
    

class Chunk :
    def __init__(self, payload, seq_num:int):
        self.payload = payload
        self.seq_num = seq_num

def write_chunks(chunks_list, filename: str):
    f = open(filename, "wb")
    for chunk in chunks_list:
        data = chunk.payload
        f.write(data)
    f.close()
        
class Metadata :
    def __init__(self,number,name):
        self.numchunks = number
        self.filename = name
    
    @classmethod
    def fromBytes(cls,data):
        numchunks = int(data[0:4])
        filename = str(data[4:504])
        return cls(numchunks, filename)

    def tobytes(self) -> bytes:
        return self.numchunks.to_bytes(4, 'big') + self.filename.to_bytes(500, 'big')

class Receiver:
    def __init__(self, myaddr:(str, int)):
        self.metadata = None
        self.myaddr = myaddr
        self.chunks = []  
        self.pending_chunks:set = set(range(len(self.chunks)))

    def do_handshake(self):
        sock = socket.socket(socket.AF_INET, sock.SOCK_DGRAM)
        sock.bind(self.myaddr)

        while True:
            data, self.src = sock.recvfrom(512)
            Headerpkt = Packet.fromBytes(data)
            if Headerpkt.verify_checksum() and Headerpkt.type == V1_TYPE_MDATA:
                ackpt = Packet.fromVals(V1_TYPE_MDATA_ACK,Headerpkt.seqnum,int(0).to_bytes(0,'big'))
                sck.sendto(ackpt.to_bytes(),self.src)
                break

        self.metadata = Metadata.fromBytes(Headerpkt.payload)
        self.chunks = [None for _ in range(self.metadata.numchunks)]  # Initialize list of chunks
        self.pending_chunks = set(range(self.metadata.numchunks))  # Initialize list of pending chunks

    def receive_data(self):
        sock = socket.socket(socket.AF_INET, sock.SOCK_DGRAM)
        sock.bind(self.myaddr)

        while len(pending_chunks) > 0:  # While chunks are still pending
            data, self.src = sock.recvfrom(512)
            pkt = Packet.fromBytes(data)

            if pkt.verify_checksum() and pkt.type_ == V1_TYPE_MDATA:  # If metadata, send mdack
                ackpt = Packet.fromVals(V1_TYPE_MDATA_ACK, pkt.seqnum, int(0).to_bytes(0,'big'))
                sck.sendto(ackpt.to_bytes(),self.src)

            elif pkt.verify_checksum() and pkt.type_ == V1_TYPE_DATA:  # If chunk, store chunk and update pending chunks
                seqnum = pkt.seqnum
                self.chunks[seqnum] = Chunk(pkt.payload, seqnum)
                self.pending_chunks.remove(seqnum)


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
        first = (self.version << 28) + (self.type << 26) + (self.payload_length << 16) + self.checksum

        return first.to_bytes(4, 'big') + self.seqnum.to_bytes(4, 'big') + self.payload

    def calc_paylen(self):
        self.payload_length = len(self.payload)

    def calc_checksum(self, msg:bytes):
        #self.checksum = 0
        ## Calculate checksum
        def add_carry(a, b):
            c = a + b
            return (c & 0xffff) + (c >> 16) #carry bit wrapped around
        s = 0
        for i in range(0, len(msg), 2):
            next_word = (msg[i]<<8) + msg[i+1]
            s = add_carry(s, next_word)
        self.checksum =  ~s & 0xffff
		
# def calc_checksum(self, msg:bytes):
# 	#self.checksum = 0
# 	## Calculate checksum
# 	def add_carry(a, b):
# 	    c = a + b
# 	    return (c & 0xffff) + (c >> 16) #carry bit wrapped around
# 	s = 0
# 	for i in range(0, len(msg), 2):
# 	    next_word = (msg[i]<<8) + msg[i+1]
# 	    s = add_carry(s, next_word)
# 	self.checksum =  ~s & 0xffff

    def verify_checksum(self):
        pass
        ##
