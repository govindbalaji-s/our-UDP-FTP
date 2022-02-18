V1_MAX_MSG_SIZE = 512
V1_CHUNK_SIZE = 504
V1_TYPE_MDATA = 0
V1_TYPE_DATA = 1
V1_TYPE_MDATA_ACK = 2
V1_TYPE_DATA_ACK = 3

import threading
import socket
import numpy as np

def sendto(fname, myaddr, dest):
    sender = Sender(myaddr, dest, fname)
    sender.do_handshake()
    sender.send_data()
    print("Done sending")
    ## read file 
    ## init sender's state

    ## handshake on udp

    ## infinite loop based on sender's state:
        ## receive on udp and call functions to update sender's state
        ## send on udp based on sender's state
    #
    #

def recv_at(myaddr):

    receiver = Receiver(myaddr)
    receiver.do_handshake()
    receiver.receive_data()
    
    print("Done receiving")
    receiver.write_chunks()

    #
    #

class Sender:
    
    def __init__(self, myaddr:(str, int), dest:(str, int), fname:str):
        self.myaddr = myaddr
        self.dest = dest
        self.fname = fname
        with open(fname, 'rb') as file:
            self.data = file.read()
        self.populate_chunks()
        #self.unacked_chunks = set(range(len(self.chunks)))
        self.unacked_chunks = np.ones(len(self.chunks))

        self.congestion = CongestionState()
        #self.flow = FlowState() TODO add flow control
        self.timeoutval = 0.2
        self.timers = [] 

    def do_handshake(self):
        mdata = Metadata(len(self.chunks), self.fname)
        pload = mdata.to_bytes()
        mdpkt = Packet.fromVals(V1_TYPE_MDATA, 0, pload)

        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(self.myaddr)
        sock.settimeout(self.timeoutval) # Initial value 0.2ms

        while True:
            try:
                sock.sendto(mdpkt.to_bytes(), self.dest)
                data, src = sock.recvfrom(512)
                print("recvd some handshake")
                ackpkt = Packet.fromBytes(data)
                if ackpkt.verify_checksum() and ackpkt.type_ == V1_TYPE_MDATA_ACK:
                    break

            except socket.timeout:
                continue
            
        print("Handshake over")
        sock.close()

    def send_data(self):    #func to start sending data, and recieving ACKs for sent data.

        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(self.myaddr)

        def listen_for_acks():
            while True:
                #print("Remaining: " + (str(len(self.unacked_chunks)))
                #print("Remaining: " + str(np.sum(self.unacked_chunks)))
                data, src = sock.recvfrom(512)
                ackpkt = Packet.fromBytes(data)
                ##verify ack packet appriately
                if ackpkt.verify_checksum() and ackpkt.type_ == V1_TYPE_DATA_ACK:
                    ##lock here?
                    #self.unacked_chunks.discard(ackpkt.seqnum)  ##if already acked, then does nothing
                    self.unacked_chunks[ackpkt.seqnum] = 0
                else:
                    pass
                    ##Do nothing??
                #if len(self.unacked_chunks) == 0:
                if np.sum(self.unacked_chunks) == 0:
                    break

        thread_ACK = threading.Thread(target=listen_for_acks)
        thread_ACK.start()

        # unacked_iter = iter(self.unacked_chunks)
        # while len(self.unacked_chunks) > 0:
        # ##How do we put a lock here? (:
        #     try:
        #         ----------
        #     except StopIteration:
        #         unacked_iter = iter(self.unacked_chunks)
        while np.sum(self.unacked_chunks) > 0:
            for seq_num in range(len(self.unacked_chunks)):
                if self.unacked_chunks[seq_num]:
                    utp_pkt = Packet.fromVals(type_ = 1 , seqnum = seq_num, payload = self.chunks[seq_num].payload) #fromVals takes care of checksumcalculation            utp_pkt.calc_paylen()
                    ##Checksum computed and updated

                    sock.sendto(utp_pkt.to_bytes(), self.dest)
                    ##Ig some sort of timeout here before sending again?

        thread_ACK.join()
        sock.close()


    # Breaks self.data into self.chunks
    def populate_chunks(self) -> None:
        self.chunks = []
        seqnum = 0
        ptr = 0
        while ptr < len(self.data):
            till = min(ptr + V1_CHUNK_SIZE, len(self.data))
            self.chunks.append(Chunk(self.data[ptr:till], seqnum))
            seqnum += 1
            ptr = till


    def ack_received(self):
        pass
        # update unacked chunks

class CongestionState:
    def __init__(self):
        self.cwnd = 0
        self.addconst = 1
        self.ssthresh = 1
        self.is_slow_start = True
    

class Chunk :
    def __init__(self, payload, seq_num:int):
        self.payload = payload
        self.seq_num = seq_num



    vector<char> to_bytes() {
        vector<char> data;
        for(int i = 3; i >= 0; i--)
            data.push_back((numchunks >> (8*i)) & 255);
        
class Metadata :
    def __init__(self,number,name):
        self.numchunks = number
        self.filename = name
    
    @classmethod
    def fromBytes(cls,data):
        numchunks = int.from_bytes(data[0:4], 'big')
        filename = data[4:504].decode('utf-8')
        return cls(numchunks, filename)

    def to_bytes(self) -> bytes:
        return self.numchunks.to_bytes(4, 'big') + self.filename.encode('utf-8')

class Receiver:
    def __init__(self, myaddr:(str, int)):
        self.metadata = None
        self.myaddr = myaddr
        self.chunks = []  
        self.pending_chunks = set(range(len(self.chunks)))

    def do_handshake(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(self.myaddr)

        while True:
            data, self.src = sock.recvfrom(512)
            print("recvd some handshake")
            Headerpkt = Packet.fromBytes(data)
            if Headerpkt.verify_checksum() and Headerpkt.type_ == V1_TYPE_MDATA:
                ackpt = Packet.fromVals(V1_TYPE_MDATA_ACK,Headerpkt.seqnum,int(0).to_bytes(0,'big'))
                sock.sendto(ackpt.to_bytes(),self.src)
                break

        self.metadata = Metadata.fromBytes(Headerpkt.payload)
        self.chunks = [None for _ in range(self.metadata.numchunks)]  # Initialize list of chunks
        self.pending_chunks = set(range(self.metadata.numchunks))  # Initialize list of pending chunks
        print("Handshake done")
        sock.close()
    
    def receive_data(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(self.myaddr)

        while len(self.pending_chunks) > 0:  # While chunks are still pending
           # print("Remaining: " + str(len(self.pending_chunks)))

            data, self.src = sock.recvfrom(512)
            pkt = Packet.fromBytes(data)

            if pkt.verify_checksum() and pkt.type_ == V1_TYPE_MDATA:  # If metadata, send mdack
                ackpt = Packet.fromVals(V1_TYPE_MDATA_ACK, pkt.seqnum, int(0).to_bytes(0,'big'))
                sock.sendto(ackpt.to_bytes(),self.src)

            elif pkt.verify_checksum() and pkt.type_ == V1_TYPE_DATA:  # If chunk, store chunk and update pending chunks
                seqnum = pkt.seqnum
                self.chunks[seqnum] = Chunk(pkt.payload, seqnum)
                ackpt = Packet.fromVals(V1_TYPE_DATA_ACK, seqnum, int(0).to_bytes(0,'big'))
                sock.sendto(ackpt.to_bytes(),self.src)
                self.pending_chunks.discard(seqnum)
        sock.close()

    def write_chunks(self):
        f = open('r'+self.metadata.filename, "wb")
        for chunk in self.chunks:
            data = chunk.payload
            f.write(data)
        f.close()

class Packet :
    def __init__(self, type_:int, seqnum:int, payload, payload_length:int=0, checksum:int=0):
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
        test_msg = pkt.to_bytes()
        pkt.checksum = calc_checksum(test_msg)
        return pkt

    @classmethod
    def fromBytes(cls, data):  ## Creating packet at Receiver
        version = data[0] >> 4
        type_ = (data[0] >> 2) & 3
        payload_length = ((data[0] & 3) << 8) + data[1]
        checksum = int.from_bytes(data[2:4], 'big')
        seqnum = int.from_bytes(data[4:8], 'big')
        payload = data[8:]
        return cls(type_, seqnum, payload, payload_length, checksum)

    def to_bytes(self):
        # 4 bits version, 2 bits type, 10 bits payload length
        ## Error handle version, type etc ranges TODO
        first = (self.version << 28) + (self.type_ << 26) + (self.payload_length << 16) + self.checksum

        return first.to_bytes(4, 'big') + self.seqnum.to_bytes(4, 'big') + self.payload

    def calc_paylen(self):
        self.payload_length = len(self.payload)

    def verify_checksum(self):
        return calc_checksum(self.to_bytes()) == 0
        ##

def calc_checksum(msg:bytes):
    #self.checksum = 0
    ## Calculate checksum
    def add_carry(a, b):
        c = a + b
        return (c & 0xffff) + (c >> 16) #carry bit wrapped around
    s = 0
    if len(msg)%2 == 1:
        msg = msg + int(0).to_bytes(1, 'big')
    for i in range(0, len(msg), 2):
        next_word = (msg[i]<<8) + msg[i+1]
        s = add_carry(s, next_word)
    return ~s & 0xffff

