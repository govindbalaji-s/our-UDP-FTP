===============================================================================
------------------------------- Our UDP FTP -----------------------------------
-------------------------------------------------------------------------------

Our UDP FTP is a miniature FTP that supports only the following operation:
    - Send a file from client to server.

-------------------------------------------------------------------------------
Building:
On the project's directory, do:
    $ make
-------------------------------------------------------------------------------
Running the server:
Do:
    $ ./server
Enter the IP address and port(new port to be opened) of the server machine.
Follow the self explanatory interactive messages.

Look at serverinput.txt for sample input to the server.
-------------------------------------------------------------------------------
Running the client:
Do:
    $ ./client
Enter the IP address and port(new port to be opened) of the client, and the
same IP address and port of the server as prompted.

Then, enter one command of the form "put {filename}". For example:
    > put CS3543_100MB

The file transfer progress and completion will be displayed.
Note that on the receiver's end the filename will be original file name 
prefixed with an 'r'. This helps to distinguish between original file, in case
you run both server and client on same directory across localhost.


Look at clientinput.txt for sample input to the client.
-------------------------------------------------------------------------------