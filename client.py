import socket
import sys
import ipaddress


print("Enter IP of server:")
ip= input()	# IP address

print("Enter PORT of server:")
port= int(input())	# port

try:
	while True:
		print(">", sep='')
		c= input() # input commands
		if 'put' == c.split()[0]:																				
				fname= c.split()[1]	# file to download
                ourudpftp.sendto(fname, (ip, port))
				print("Sent successfully")

		elif c== 'exit': # close connection
			print("Exiting..")
			sys.exit() # exit
		
		else:
			if result == "Invalid command":	
				print(result)
			else: # result from running linux shell command
				print(result)
				print("Command run successfully")
except Exception as e:
    print(traceback.format_exc())