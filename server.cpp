#include <iostream>
#include <string>
#include <utility>

using namespace std;
void ourudpftp_recv_at(pair<string, int>);

int main(){

	string serverip ;	
	int serverport ;	

	cout << "Enter IP of server:\n";
	cin >> serverip;
	cout << "Enter PORT of server:\n";
	cin >> serverport;
	
    while(1){
        ourudpftp_recv_at({serverip, serverport});
    }

	return 0;
}