#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;
void ourudpftp_sendto(string, pair<string, int>, pair<string, int>);

vector<string> split(string str){
    vector<string> res; 
	string word = "";
    for (auto x : str) {
        if (x == ' '){
            res.push_back(word);
			word = "";
        }
        else {
            word = word + x;
        }
    }
    if(word != "") res.push_back(word);
	if(res.empty())
		res.push_back(str);

    return res;
}

int main(){

	string myip, ip ;	//'ip' is server ip
	int myport, port ;	//'port' is server port

	cout << "Enter IP of myself: ";
	cin >> myip;

	cout << "Enter Port of myself: ";
	cin >> port;

	cout << "Enter IP of server: ";
	cin >> ip;

	cout << "Enter PORT of server: ";
	cin >> port;
	
    while(1){
        cout << "Syntax: put filename\n> ";
        //print(">", sep='') -> what is this for??
        string inp;
        getline(cin, inp);
        //cin << inp	// input commands
		vector<string> out = split(inp);
        // for(auto &s : out) cerr << s << '#'; cerr << '\n';
        if ("put" == out[0]){                                                                                
                string fname = out[1];
                ourudpftp_sendto(fname, {myip, myport}, {ip, port});
                cout << "Sent successfully\n";
                return 0;
		}
        else if (out[0] == "exit"){  // close connection
            cout << "Exiting..\n";
            return 0;
		}
	}
	return 0;
}


