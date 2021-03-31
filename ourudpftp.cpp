#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <algorithm>
#include <thread>

using namespace std;

const int V1_MAX_MSG_SIZE = 512;
const int V1_CHUNK_SIZE = 504;
const int V1_TYPE_MDATA = 0;
const int V1_TYPE_DATA = 1;
const int V1_TYPE_MDATA_ACK = 2;
const int V1_TYPE_DATA_ACK = 3;
const string TIMEOUT_IP = "TIMEOUT";

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

pair<string, int> std_recvfrom(int sd, vector<char> &msg) {
    struct sockaddr_in peer_addr;
    size_t peer_addr_len;
    int sz;
    if((sz = recvfrom(sd, msg.data(), msg.size(), 0, (struct sockaddr*)&peer_addr, &peer_addr_len)) < 0){
        if(errno == EAGAIN or errno == EWOULDBLOCK){
            return {TIMEOUT_IP, 0};
        }
    }
    msg.resize(sz);

    pair<string, int> ret;
    char *ch;
    inet_aton(ch, &peer_addr.sin_addr);
    return {string(ch), ntohs(peer_addr.sin_port)};
}

void std_sendto(int sd, vector<char> &msg, pair<string, int> dest) {
    struct sockaddr_in peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(dest.second);
    peer_addr.sin_addr.s_addr = inet_addr(dest.first.c_str());

    if(sendto(sd, msg.data(), msg.size(), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0){}
}

void ourudpftp_sendto(string fname, pair<string, int> myaddr, pair<string, int> dest) {
    sender = Sender(myaddr, dest, fname);
    sender.do_handshake();
    sender.send_data();
    cout << "Done sending\n";
}
    
void ourudpftp_recv_at(pair<string, int> myaddr) {
    receiver = Receiver(myaddr)
    receiver.do_handshake()
    receiver.receive_data()
    cout << "Done receiving\n";
    receiver.write_chunks()
}

// Ignore this attempt to port
class Sender{
    pair<string, int> myaddr, dest;
    std::string fname;
    vector<char> data;
    vector<Chunk> chunks;
    vector<bool> unacked_chunks;
    CongestionState cstate;
    int timeoutval = 200;
    //timers if reqd

public:
    Sender(pair<string, int> myaddr_, pair<string, int> dest_, string fname_)
    : myaddr(myaddr_), dest(dest_), fname(fname_) {
        ifstream infile(fname, ios_base::binary);
        data = vector<char>(istreambuf_iterator<char>(infile), istreambuf_iterator<char>());
        populate_chunks();
        unacked_chunks.resize(chunks.size());
        congestion = CongestionState();
        timeoutval = 200;
        //timers = 
    }

    void populate_chunks(){
        chunks.clear();
        int seqnum = 0;
        for(long ptr = 0; ptr < data.size();) {
            long till = min(ptr + V1_CHUNK_SIZE, data.size());
            vector<char> cdata(data.begin() + ptr, data.begin() + till);
            chunks.push_back(Chunk(cdata, seqnum));
            seqnum++;
            ptr = till;
        }
    }

    void do_handshake() {
        auto mdata = Metadata(chunks.size(), fname);
        auto pload = mdata.to_bytes();
        mdpkt = Packet(V1_TYPE_MDATA, 0, pload);

        auto sock = setup_std_sock(myaddr, timeoutval);

        while(true) {
            std_sendto(sock, mdpkt.to_bytes(), dest);
            vector<char> msg(512);
            auto src = std_recvfrom(sock, msg);
            if(src.first == TIMEOUT_IP)
                continue;
            cout << "Received some handshake.\n";
            auto ackpkt = Packet(msg);
            if(ackpkt.verify_checksum() and ackpkt.type_ == V1_TYPE_MDATA_ACK)
                break;
        }
        cout << "Handshake over\n";
        close(sock);
    }

    void listen_for_acks(int sock) {
        while(true) {
            cout << "Remaining: " << count(unacked_chunks.begin(), unacked_chunks.end(), true) << '\n';
            vector<char> msg(512);
            std_recvfrom(sock, msg);
            auto ackpkt = Packet(msg);
            if(ackpkt.verify_checksum() and ackpkt.type_ == V1_TYPE_DATA_ACK) {
                unacked_chunks[ackpkt.seqnum] = 0;
                if(count(unacked_chunks.begin(), unacked_chunks.end(), true) == 0)
                    break;
            }
        }
    }

    void send_data() {
        auto sock = setup_std_sock(dest);

        thread thread_ACK(listen_for_acks, sock);
        while(count(unacked_chunks.begin(), unacked_chunks.end(), true) > 0) {
            for(long i = 0; i < unacked_chunks.size(); i++) {
                if(unacked_chunks[i]) {
                    auto utp_pkt = Packet(1, seq_num, chunks[seq_num].payload);
                    std_sendto(sock, utp_pkt.to_bytes(), dest);
                    // timeout?
                }

            }
        }
        thread_ACK.join();
        close(sock);
    }
};

class CongestionState{
    int cwnd = 0,
        addconst = 1,
        ssthresh = 1;
    bool is_slow_start = true;
};

struct Chunk{
    vector<char> payload;
    long seq_num;

    Chunk(vector<char> &pload, long sn)
        : payload(pload), seq_num(sn) {}
};
        
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

class Packet {
	int version=1, type_, seqnum, payload_length=0, checksum=0;
	vector<char> payload;

	Packet(int ctype_, int cseqnum, vector<char> cpayload) {  // Creating packet at sender
		type_ = ctype_;
		seqnum = cseqnum;
		payload = cpayload;
		payload_length = cpayload.size();
		checksum = calc_checksum(cpayload);
	}

	Packet(vector<char> data) {  // Creating packet at receiver
		version = (int)data[0] >> 4;
		type_ = ((int)data[0] >> 2) & 3;
		payload_length = (((int)data[0] & 3) << 8) + (int)data[1];
		checksum = ((int)data[2] << 8) + (int)data[3];
		seqnum = ((int)data[4] << 24) + ((int)data[5] << 16) + ((int)data[6] << 8) + (int)data[7];
		payload = vector<char>(data.begin() + 8, data.end());
	}

	vector<char> to_bytes() {
		vector<char> data;
		data.push_back((char)((version << 4) + (type << 2) + (payload_length >> 8)));  // byte 1
		data.push_back((char)(payload_length & 255));  // byte 2
		data.push_back((char)(checksum >> 8));  // byte 3
		data.push_back((char)(checksum & 255));  // byte 4
		for(int i = 3; i >= 0; i--) // bytes 5-8
			data.push_back((char)((seqnum >> (i*8)) & 255));
		data.insert(data.end(), payload.begin(), payload.end());
		return data;
	}
}

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
