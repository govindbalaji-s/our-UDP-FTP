import socket

print("Enter IP of server:")
serverip= input()	# IP address

print("Enter PORT of server:")
serverport= int(input())	# port


while True:
    ourudpftp.recv_at((serverip, serverport))
