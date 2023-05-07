# P2P File Sharing system

Implemneted the p2p file sharing network in C++

Instructions to run the code:

1) make 
to compile the code

2) ./server
to run the central tracking server

3) In a different terminal window, run './client' to run the first client/peer

4) In another different terminal window, run './client' to run the second client/peer

In the client side:

1) first, press 0 to handshake 

The other features don't work without handshake
so, always send the handshake first

2) press 1 for registering a file you want to share

3) press 3 to view all the names of files along with their file ids which are present in the network to download

4) press 2 to request downlaod and then enter the file id number to downlaod a file

5) Don't download from the same client which was used to register the file. Download from a different client.

Initiate multiple terminal sessions and register the same file from all of them to create a multiple peers with the same file.

Then try downloading the file from a different client where we did not register the file.

**Always send the handshake first**

