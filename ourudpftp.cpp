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
#include <fstream>
#include <cassert>
#include <set>
#include <fstream>

using namespace std;

const int V1_MAX_MSG_SIZE = 512;
const size_t V1_CHUNK_SIZE = 504;
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

pair<string, int> std_recvfrom(int sd, vector<unsigned char> &msg) {
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len;
    int sz;
    if((sz = recvfrom(sd, msg.data(), msg.size(), 0, (struct sockaddr*)&peer_addr, &peer_addr_len)) < 0){
        if(errno == EAGAIN or errno == EWOULDBLOCK){
            cout << errno << '\n';
            return {TIMEOUT_IP, 0};
        }
    }
    msg.resize(sz);

    pair<string, int> ret;
    char *ch = inet_ntoa(peer_addr.sin_addr);
    return {string(ch), ntohs(peer_addr.sin_port)};
}

void std_sendto(int sd, vector<unsigned char> &msg, pair<string, int> dest) {
    struct sockaddr_in peer_addr;
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(dest.second);
    peer_addr.sin_addr.s_addr = inet_addr(dest.first.c_str());

    if(sendto(sd, msg.data(), msg.size(), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0){}
}

class CongestionState{
    int cwnd = 0,
        addconst = 1,
        ssthresh = 1;
    bool is_slow_start = true;
};

struct Chunk{
    vector<unsigned char> payload;
    uint32_t seq_num;

    Chunk(vector<unsigned char> &pload, uint32_t sn)
        : payload(pload), seq_num(sn) {}
};

struct Metadata {
    uint32_t numchunks;
    string filename;

    Metadata() {}
    Metadata(uint32_t nch, string fn) : numchunks(nch), filename(fn) {}

    Metadata(vector<unsigned char> data) {
        numchunks = 0;
        for(int i = 3; i >= 0; i--)
            numchunks += uint32_t(data[3-i]) << (8*i);
        
        filename = string(data.begin()+4, data.end());
    }

    vector<unsigned char> to_bytes() {
        vector<unsigned char> data;
        for(int i = 3; i >= 0; i--)
            data.push_back((numchunks >> (8*i)) & 255u);
        
        data.insert(data.end(), filename.begin(), filename.end());
        return data;
    }
};

uint32_t add_carry(uint16_t a_, uint16_t b_) {
    uint32_t a = a_, b = b_;
    uint32_t c = a + b;
    return ((c & 0xffffu) + (c >> 16u)); // wrap around carry bit
}
/* Compute checksum for count bytes starting at addr, using one's complement of one's complement sum*/
static uint32_t compute_checksum(uint32_t *addr, uint32_t count) {
  uint64_t sum = 0;
  while (count > 1) {
    sum += * addr++;
    count -= 2;
  }
  //if any bytes left, pad the bytes and add
  if(count > 0) {
    sum += ((*addr)&htons(0xFF00u));
  }
  //Fold sum to 16 bits: add carrier to result
  while (sum>>16) {
      sum = (sum & 0xffffu) + (sum >> 16);
  }
  //one's complement
  sum = ~sum;
  return ((uint16_t)sum);
}
uint32_t calc_checksum(vector<unsigned char> msg2) {
    // if(msg.size() % 2 == 1)
    //     msg.push_back(0);

    // uint32_t *buf = new uint32_t[msg.size()/2];
    // for(int i = 0; i < msg.size(); i+=2)
    //     buf[i/2] = (uint32_t(msg[i]) << 8) + uint32_t(msg[i+1]);
    // uint32_t ans = compute_checksum(buf, msg.size());
    // delete buf;
    // return ans;
    // vector<uint16_t> msg2;
    // for(int i = 0; i < msg.size(); i++)
    //     msg2.push_back((static_cast<unsigned unsigned char>(msg[i])));
    cout << '@' << msg2.size() <<'@';
    for(uint16_t m: msg2) cout << (m) << '@';
    cout << '\n';
    uint32_t s = 0;
    if( msg2.size() %2 == 1)
        msg2.push_back(0);
    for(unsigned long i = 0; i < msg2.size(); i += 2) {
        uint16_t next_word = ((uint16_t)(msg2[i])<<8u) + (uint16_t)(msg2[i+1]);
        // cout << "&&" << next_word << ' ';
        s = add_carry(s, next_word);
        // cout << "^^" << s << ' ';
    }
    // cout <<  '\n';
    // cout << '%' << s << '\n';
    uint16_t ret = (uint16_t)(~s & 0xffffu);
    // cout << "$$" << ret << '\n';
    // unsigned char *buf = new unsigned char[msg.size()];
    // for(int i = 0; i < msg.size(); i++)
    //     buf[i] = msg[i];
    // auto ans = ip_checksum(buf, msg.size());
    // delete buf;
    // return ans;
    // return ip_checksum(msg.data(), msg.size());
    return ret;
}


// uint16_t calc_checksum(vector<unsigned char> data)
// {
//     unsigned char *buffer = data.data();
//     int size = data.size();
//     unsigned long cksum=0;
//     while(size >1)
//     {
//         cksum+=*buffer++;
//         size -=sizeof(uint16_t);
//     }
//     if(size)
//         cksum += *(unsigned char*)buffer;

//     cksum = (cksum >> 16) + (cksum & 0xffff);
//     cksum += (cksum >>16);
//     return (uint16_t)(~cksum);
// }

struct Packet {
    uint16_t version = 1, type_;
    uint32_t seqnum;
    uint16_t checksum = 0;
    uint16_t payload_length;
	//int version=1, type_, seqnum, payload_length=0, checksum=0;
	vector<unsigned char> payload;
	
public:
	Packet(int ctype_, uint32_t cseqnum, vector<unsigned char> cpayload) {  // Creating packet at sender
		type_ = ctype_;
		seqnum = cseqnum;
		payload = cpayload;
		payload_length = cpayload.size();
		checksum = calc_checksum(to_bytes());
        assert(verify_checksum());
        // cout << "Calculated checksum = " << checksum << '\n';
	}

	Packet(vector<unsigned char> data) {  // Creating packet at receiver
		version = (uint32_t)data[0] >> 4;
		type_ = ((uint32_t)data[0] >> 2) & 3;
		payload_length = (((uint32_t)data[0] & 3) << 8) + (uint32_t)data[1];
		checksum = ((uint32_t)data[2] << 8) + (uint32_t)data[3];
		seqnum = ((uint32_t)data[4] << 24) + ((uint32_t)data[5] << 16) + ((uint32_t)data[6] << 8) + (uint32_t)data[7];
		payload = vector<unsigned char>(data.begin() + 8, data.end());
	}

	vector<unsigned char> to_bytes() {
		vector<unsigned char> data;
		data.push_back((unsigned char)((version << 4) + (type_ << 2) + (payload_length >> 8)));  // byte 1
		data.push_back((unsigned char)(payload_length & 255));  // byte 2
		data.push_back((unsigned char)(checksum >> 8));  // byte 3
		data.push_back((unsigned char)(checksum & 255));  // byte 4
		for(int i = 3; i >= 0; i--) // bytes 5-8
			data.push_back((unsigned char)((seqnum >> (i*8)) & 255));
		data.insert(data.end(), payload.begin(), payload.end());
		return data;
	}

    bool verify_checksum() {
        cout << "Checksum = " << calc_checksum(to_bytes()) << '\n';
        return calc_checksum(to_bytes()) == 0;
    }
};

// Ignore this attempt to port
class Sender{
    pair<string, int> myaddr, dest;
    std::string fname;
    vector<unsigned char> data;
    vector<Chunk> chunks;
    vector<bool> unacked_chunks;
    CongestionState cstate;
    int timeoutval = 200000;
    //timers if reqd

public:
    Sender(pair<string, int> myaddr_, pair<string, int> dest_, string fname_)
    : myaddr(myaddr_), dest(dest_), fname(fname_) {
        ifstream infile(fname, ios_base::binary);
        auto temp = vector<char>(istreambuf_iterator<char>(infile), istreambuf_iterator<char>());
        for(auto &uc : temp)
            data.push_back(static_cast<unsigned char>(uc));
        populate_chunks();
        unacked_chunks.resize(chunks.size());
        cstate = CongestionState();
        timeoutval = 200000;
        //timers = 
    }

    void populate_chunks(){
        chunks.clear();
        uint32_t seqnum = 0;
        for(uint64_t ptr = 0; ptr < data.size();) {
            long till = min(ptr + V1_CHUNK_SIZE, data.size());
            vector<unsigned char> cdata(data.begin() + ptr, data.begin() + till);
            chunks.push_back(Chunk(cdata, seqnum));
            seqnum++;
            ptr = till;
        }
    }

    void do_handshake() {
        cout << "Metadata:" << chunks.size() << ' ' << fname << '\n';
        auto mdata = Metadata(chunks.size(), fname);
        auto pload = mdata.to_bytes();
        cout << ";;" << calc_checksum(pload);

        auto mdpkt = Packet(V1_TYPE_MDATA, 0, pload);

        auto sock = setup_std_sock(myaddr, timeoutval);

        while(true) {
            auto bytes = mdpkt.to_bytes();
            std_sendto(sock, bytes, dest);
            vector<unsigned char> msg(512);
            auto src = std_recvfrom(sock, msg);
            if(src.first == TIMEOUT_IP) {
                cout << "Timedout\n";
                continue;
            }
            cout << "Received some handshake.\n";
            cout << '#' << msg.size() <<'#';
            for(uint16_t m: msg) cout << uint16_t(static_cast<unsigned char>(m)) << '#';
            cout << '\n';
            auto ackpkt = Packet(msg);
            cout << ackpkt.verify_checksum() << '\n';
            if(ackpkt.verify_checksum() and ackpkt.type_ == V1_TYPE_MDATA_ACK)
                break;
        }
        cout << "Handshake over\n";
        close(sock);
    }

    void listen_for_acks(int sock) {
        while(true) {
            cout << "Remaining: " << count(unacked_chunks.begin(), unacked_chunks.end(), true) << '\n';
            vector<unsigned char> msg(512);
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

        thread thread_ACK(&Sender::listen_for_acks, this, sock);
        while(count(unacked_chunks.begin(), unacked_chunks.end(), true) > 0) {
            for(uint32_t seq_num = 0; seq_num < unacked_chunks.size(); seq_num++) {
                if(unacked_chunks[seq_num]) {
                    auto utp_pkt = Packet(1, seq_num, chunks[seq_num].payload);
                    auto bytes = utp_pkt.to_bytes();
                    std_sendto(sock, bytes, dest);
                    // timeout?
                }
            }
        }
        thread_ACK.join();
        close(sock);
    }
};

class Receiver{
	pair<string, int> myaddr, src;
	vector<Chunk> chunks;
    Metadata mdata;
	set<long> pending_chunks;
	int count = 0;

public:
	 Receiver(pair<string,int>myaddr_){
	 	myaddr = myaddr_;
	 }

	 void do_handshake(){
       auto sock = setup_std_sock(myaddr);

       while(true){
       	vector<unsigned char> msg(512);
       	src = std_recvfrom(sock, msg);
        cout << "Got some handshake\n";  
       	Packet hdrpkt(msg);
       	if(hdrpkt.verify_checksum() and hdrpkt.type_ == V1_TYPE_MDATA)
       	{
            cout << "Verfied handshake\n";
                    cout << ";;" << calc_checksum(hdrpkt.payload);

            mdata = Metadata(hdrpkt.payload);
            cout << "Metadata:" << mdata.numchunks << ' ' << mdata.filename << '\n';
            for(int i=0;i<mdata.numchunks;i++)
        	{
        		pending_chunks.insert(i);
        	}
       		vector<unsigned char> temp;
       		auto ackpacket = Packet(V1_TYPE_MDATA_ACK,hdrpkt.seqnum,temp);
            auto bytes = ackpacket.to_bytes();
       		std_sendto(sock, bytes, src);
            cout << "Sent ack\n";
       		break;
       	}
       }
       cout << "Handshake over\n";
        close(sock);
	 }

	 void receive_data(){
	 	auto sock = setup_std_sock(myaddr);
	 	while(!pending_chunks.empty())
	 	{
          vector<unsigned char>msg(512);
          auto src = std_recvfrom(sock,msg);

          auto pkt = Packet(msg);
          if(pkt.verify_checksum() and pkt.type_ == V1_TYPE_MDATA)
          {
            cout << "gggg\n";
          	vector<unsigned char>temp;
          	auto ackpt = Packet(V1_TYPE_MDATA_ACK, pkt.seqnum,temp);
            auto bytes = ackpt.to_bytes();
          	std_sendto(sock, bytes, src);
          }
          else if(pkt.verify_checksum() and pkt.type_ == V1_TYPE_DATA)
          {
          	auto seqnum = pkt.seqnum;
          	chunks[seqnum] = Chunk(pkt.payload, seqnum);
          	vector<unsigned char>temp;
          	auto ackpt = Packet(V1_TYPE_DATA_ACK,seqnum,temp);
          	auto bytes = ackpt.to_bytes();
          	std_sendto(sock, bytes, src);
          	pending_chunks.erase(seqnum);
          }
	 	}
        close(sock);
	 }

     void write_chunks(){
         ofstream file;
         file.open("r" + mdata.filename, ios::binary | ios::out);
         for(auto i: chunks)
         {
             vector<char> temp;
             for(auto &uc : i.payload)
                temp.push_back(static_cast<char>(uc));
             file.write(temp.data(), i.payload.size());
         }
         file.close();
     }
};

void ourudpftp_sendto(string fname, pair<string, int> myaddr, pair<string, int> dest) {
    auto sender = Sender(myaddr, dest, fname);
    sender.do_handshake();
    sender.send_data();
    cout << "Done sending\n";
}
    
void ourudpftp_recv_at(pair<string, int> myaddr) {
    auto receiver = Receiver(myaddr);
    receiver.do_handshake();
    receiver.receive_data();
    cout << "Done receiving\n";
    receiver.write_chunks();
}
