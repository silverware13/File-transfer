Start by running two seperate terminals and connecting 
them to flip.engr.oregonstate.edu (I used PuTTY on port 22).

I will be using flip1 and flip2 in this example but feel free to use any flip server you wish.

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

ftclient flip1 12556 -l 27755

At this point we should see the contents of the 
servers directory on the clients terminal.

For testing we are too assume there will be a large and a small text file being transfered.
I will refrence them as "small.txt" and "large.txt" please adjust them to whatever file name you are using.

We will start by geting the "small.txt" file from the server.

ftclient flip1 12556 -g small.txt 55677

Next we will get the "large.txt" file from the server.

ftclient flip1 12556 -g large.txt 35373

and lets try getting a text file that does not exist.
We should receive an error message from the server.

ftclient flip1 12556 -g no.txt 511344

For our last test we will want to test receiving a file that
already exists at the destination.

First edit "small.txt" the file being sent from the server
to include an extra line such as "extra line!".

Now from the client request a the small.txt again.
ftclient flip1 12556 -g small.txt 54477

There should be a prompt asking if you want to overwrite the file
type n and confirm that the small.txt file has not been overwriten.
It should not have "extra line!" at the end.

Now from the client request a the small.txt again.
ftclient flip1 12556 -g small.txt 54487

This time when prompted say yes. Confirm that the small.txt file HAS
been overwriten. It should have "extra line!" at the end.

----------------------------------------------------------

At this point testing is complete. We have seen the results of
listing, getting a small file, getting a large file, trying to get
a file that does not exists, and trying to get a file that already exists.
