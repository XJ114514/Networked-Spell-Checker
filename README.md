# Project Description
Design a server program that will accept the client connection and check the spelling of the word sent from the client.
# Source codes   
## server.c
It is a program for setting up server checking whether the word sent from client is spelled correctly.  
There are **5** customized options:  

    -d <"path">             : <"path"> is the path for dictionary,
                            By Default,it will be set to DEFAULT_DICTIONARY(serverFile/dictionary.txt)

    -p <"port">             : <"port"> is the port NUMBER,
                            By Default,it will be set to DEFAULT_PORT(61323)

    -c <"connectionSize">   : <"connectionSize"> is the size of connection buffer,
                            By Default,it will be set to DEFAULT_CONNECTION_SIZE(20)

    -w <"workerSize">       : <"workerSize"> is the NUMBER of worker threads that server will hold,
                            By Default,it will be set to DEFAULT_WORKTHREAD_SIZE(5)

    -s <"scheduleType">     : <"scheduleType"> is the flag about how server would select client when there are multiple client waiting
                               if flag is set to 1, server is operating with priority mode, selecting client for the highest priority
                               if flag is set to 0, server is operating with sequential mode, selecting client for the earliest arriving
                            By Default,it will be set to priority mode(1)
## client.c
It is a program for setting up the client sending word to server  
when calling as **./client <"NUM">**, client will use "**clientFile/in<"NUM">.txt**" as **input** file , "**clientFile/out<"NUM">.txt**" as **output** file.  
when calling as **./client exit**, client will send a message to the server asking the server to **terminate**.

## threadFn.c
A sourceFile containing implementation of all threads functions and thread data structure  
### workThreadFn()
Worker thread functions - process the word sent from the client:

    get the connection from connectionBuffer according to priority or sequential mode,
    checking the client-sent word's spelling according to dictionary,
    echo each result back to client
    store the log to logQueue with arrival time, completion time and priority 
### serverThreadCFn()
Server main thread function - accepting the client connection:

    prepare the server socket and bind it as the listen socket
    (loop starts)
    once accept the connection from client, it will record the arrival time and generate a random priotity
    Then store all the information with the connection to connectionBuffer
    (loop ends)
### serverThreadLogFn()
Server secondary thread funcion - output log from logQueue to OutPut file  

## connectionBuffer.c
A sourceFile containing implementation of connectionBuffer  
## dictionary.c
Simple source file containing the data structure to store dictionary
## logQueue.c
A sourceFile containing implementation of logQueue

# Compile/Executing
 - Complie \
$make server <br/>
$make client
 - Execute \
$./server -option optionArg ... (./server -p 80 ...)<br/>
$./client NUM/exit (./client 1 OR ./client exit)

# Test plan with sample data
First call **$./server** to set up the server<br/> 
Then call **$./client 1 & ./client 1 & ./client 1** to establish the connection<br/>
Lastly, call **$./client exit** to terminate the server <br/>
You will be able to see the executing result stored in **src/serverFile/log.txt** <br/>

Then change the server scheduling mode by calling **$./server -s 0** and use the same codes to test.
You will be able to see the executing result stored in **src/serverFile/log.txt** <br/>
