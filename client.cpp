#include<iostream>
#include<string>
#include<vector>
#include "ourudpftp.h"

using namespace std;

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
	if(res.empty())
		res.push_back(str);

    return res;
}

int main(){

	string myip, ip ;	//'ip' is server ip
	int myport, port ;	//'port' is server port

	cout << "Enter IP of myself:\n";
	cin >> myip;

	cout << "Enter Port of myself:\n";
	cin >> port;

	cout << "Enter IP of server:\n";
	cin >> ip;

	cout << "Enter PORT of server:\n";
	cin >> port;
	
    while(1){
        //print(">", sep='') -> what is this for??
        cin << inp	// input commands
		vector<string> out = split(inp)
        if ("put" == out[0]){                                                                                
                string fname = out[1];
                ourudpftp_sendto(fname, {myip, myport}, {ip, port});
                cout << "Sent successfully\n";
		}
        else if (out[0] == "exit"){  // close connection
            cout << "Exiting..\n";
            return 0;
		}
	}
	return 0;
}


