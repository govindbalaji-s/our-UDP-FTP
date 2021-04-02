#include <iostream>
#include <string>
#include <utility>

using namespace std;
void ourudpftp_recv_at(pair<string, int>);

int main(){

	string serverip ;	
	int serverport ;	

	cout << "Enter IP of server: ";
	cin >> serverip;
	cout << "Enter PORT of server: ";
	cin >> serverport;
	cout << "Waiting for client ...\n";
    while(1){
        ourudpftp_recv_at({serverip, serverport});
		cout << "Waiting for next client ... \n";
    }

	return 0;
}