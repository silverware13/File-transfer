Start by running two seperate terminals and connecting 
them to flip.engr.oregonstate.edu (I used PuTTY on port 22).

Terminal #1:
Begin by compiling ftserver with the following command:
gcc -o ftserver ftserver.c

Terminal #2:
We will start the server on port 12556 (or use any port you like):
chmod +x ./chatserve
./chatserve 12556
Next we select our handle, we are going to use term2:
term2

Terminal #1:
We will now start the client using local host and the same port:
./chatclient localhost 12556
Next we select our handle, we are going to use term1:
term1
At this point we should see a prompt with our handle, let us enter a message:
Hello server

Terminal #2:
We should see the message we sent from term1 and a new prompt, let us respond:
Hey client

Terminal #1:
At this point we want to end the connection so we can type "\quit":
\quit

At this point terminal #1 should have finished running chatclient and terminal #2 should have gone back to awaiting a new connection. We have now successfully used the chat client and server.
