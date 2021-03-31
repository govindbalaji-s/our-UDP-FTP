#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string>
#include <utility>

using namespace std;

const int V1_MAX_MSG_SIZE = 512;
const int V1_CHUNK_SIZE = 504;
const int V1_TYPE_MDATA = 0;
const int V1_TYPE_DATA = 1;
const int V1_TYPE_MDATA_ACK = 2;
const int V1_TYPE_DATA_ACK = 3;

extern int errno;

int setup_std_sock(pair<string, int> myaddr, long timeout=0) {
    int sd;
    struct sockaddr_in my_addr;

    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sd < 0){}

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(myaddr.second);
    my_addr.sin_addr.s_addr = inet_addr(myaddr.first.c_str());

    if(timeout != 0) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = timeout;
        if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {perror("Error");}
    }

    if(bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) {}

    return sd;
}

pair<string, int> std_recvfrom(int sd, char *msg, size_t len) {
    struct sockaddr_in peer_addr;
    size_t peer_addr_len;
    if(recvfrom(sd, msg, len, 0, (struct sockaddr*)&peer_addr, &peer_addr_len) < 0){
        if(errno == EAGAIN or errno == EWOULDBLOCK){
            return {"TIMEOUT", 0};
        }
    }
    pair<string, int> ret;
    char *ch;
    inet_aton(ch, &peer_addr.sin_addr);
    return {string(ch), ntohs(peer_addr.sin_port)};
}

void std_sendto(int sd, char *msg, size_t len, pair<string, int> dest) {
    struct sockaddr_in peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(dest.second);
    peer_addr.sin_addr.s_addr = inet_addr(dest.first.c_str());

    if(sendto(sd, msg, len, 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0){}
}

// Ignore this attempt to port
class Sender{
    sockaddr_in myaddr, dest;
    std::string fname;
    vector<Chunk> chunks;
    unordered_set<int> unacked_chunks;
    CongestionState cstate;
    float timeoutval = 0.2;


};
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

    def do_handshake():
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

class Receiverstate :
    def __init__(self, metadata):
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

	def calc_paylen(self):
		self.payload_length = len(self.payload)

	#def calc_checksum(self):
		#self.checksum = 0
		## Calculate checksum
		
def calc_checksum(msg:bytes):

    #msg in bytes
    def add_carry(a, b):
        c = a + b
        return (c & 0xffff) + (c >> 16) #carry bit wrapped around

    s = add_carry(msg[0],msg[1])
    for i in range(2, len(msg)):
        next_word = msg[i]
        s = add_carry(s, next_word)
    return ~s & 0xffff

    def verify_checksum():
        ##

