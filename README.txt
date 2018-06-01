Start by running two seperate terminals and connecting 
them to flip.engr.oregonstate.edu (I used PuTTY on port 22).

For this assignment it is assumed that the server and client are 
in two different folders, but are both on the same server (such as flip1). 
If this is not the case you may have to change some of the commands to reflect 
the correct server address.

---------------------------------------------------------
TERMINAL #1:

Begin by compiling ftserver with the following command:
gcc -o ftserver ftserver.c

We will start the server on port 12556 (or use any port you like):

./ftserver 12556

---------------------------------------------------------
Terminal #2:

Next we want to send a request to see the contents of the servers directory: 

chmod +x ./ftclient

ftclient localhost 12556 -l 27755

At this point we should see the contents of the 
servers directory on the clients terminal.

For testing we are too assume there will be a large and a small text file being transfered.
I will refrence them as "small.txt" and "large.txt" please adjust them to whatever file name you are using.

We will start by geting the "small.txt" file from the server.

ftclient localhost 12556 -g small.txt 55677

Next we will get the "large.txt" file from the server.

ftclient localhost 12556 -g large.txt 35373

and last lets try getting a text file that does not exist.

ftclient localhost 12556 -g no.txt 511344
----------------------------------------------------------

At this point testing is complete. We have seen the results of
listing, getting a small file, getting a large file, and trying to get
a file that does not exists.
