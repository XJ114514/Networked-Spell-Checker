#include "connectionBuffer.c"
#include "logQueue.c"
#include "dictionary.c"
/**
    Student Name: Xu Jiang
    A sourceFile containing implementation of all threads functions and thread data structure
*/

typedef struct ThreadData{
    ConnectionBuffer* connectionBuffer;
    LogQueue* logQueue;
    Dict* dict;
    int flagMode;
    const time_t start;
}TData;

typedef struct ThreadDataLog{
    LogQueue* logQueue;
    FILE *logPtr;
}TDataLog;

typedef struct ThreadDataC{
    ConnectionBuffer* connectionBuffer;
    int port;
    const time_t start;
}TDataC;

void addC(ConnectionBuffer* bufferObj,int socket, time_t time, int priority);
Connection* popC(ConnectionBuffer* bufferObj,Connection* connection, int flagMode);
void addLog(LogQueue *logQueue,char* word, int flag,time_t arrive, time_t complete, int priority);
Log* popLog(LogQueue* logQueue,Log* log);

/**
    worker thread functions - process the word sent from the client:
    get the connection from connectionBuffer according to priority or sequential mode,
    checking the client-sent word's spelling according to dictionary,
    echo each result back to client
    store the log to logQueue with arrival time, completion time and priority
*/
void* workThreadFn(void* TdataArg){
    TData* tData = (TData *) TdataArg;
    Connection connection;
    char word[51];
    char wordOrigin[51];
    char temp[100];

    while(1){

    //get connection from connectionBuffer(consume)
        //try to get the lock of connectionBuffer
        pthread_mutex_lock(tData->connectionBuffer->lock);
        while(tData->connectionBuffer->count == 0){
            pthread_cond_wait(tData->connectionBuffer->pop,tData->connectionBuffer->lock);
        }
        popC(tData->connectionBuffer,&connection,tData->flagMode);
        printf("workT pop, countC = [%d]\n",tData->connectionBuffer->count);
        pthread_cond_signal(tData->connectionBuffer->fill);
        //release the lock of connectionBuffer
        pthread_mutex_unlock(tData->connectionBuffer->lock);

    //process the word from connection
        do{
            //get the word sent from client
            int flagRecv = recv(connection.socket,word,51,0);
            if(flagRecv == -1){
                perror("server recv()");
                exit(1);
            }else if(flagRecv == 0){
                puts("workThread went back to get new connection");
                break;
            }
            //termination condition
            if(strcmp(word,"exit") == 0){
                //try to get the lock of logQueue
                pthread_mutex_lock(tData->logQueue->lock);
                while(tData->logQueue->count == tData->logQueue->size){
                    pthread_cond_wait(tData->logQueue->fill,tData->logQueue->lock);
                }
                addLog(tData->logQueue,word,1,0,0,0);
                printf("workT added exit, countL = [%d]\n",tData->logQueue->count);
                pthread_cond_signal(tData->logQueue->pop);
                //release the lock of connectionBuffer
                pthread_mutex_unlock(tData->logQueue->lock);
                break;
            }
            int flag = 0;
            //process the word
            strcpy(wordOrigin,word);
            for(int i = 0 ; i < strlen(word); i++){
                word[i] = toupper(word[i]);
            }
            for(int i = 0 ; i < tData->dict->size ; i++){
                if(word[0] == tData->dict->words[i].str[0]){
                    if(strcmp(word,tData->dict->words[i].str) == 0){
                        //the word is correctly spelled
                        flag = 1;
                        break;
                    }
                }
            }
            //delay for each process
            usleep(500000);
            //echo the word back to client according to flag.
            sprintf(temp,"%s - %s\n",wordOrigin,flag?"OK":"MISSPELLED");
            if(send(connection.socket,temp,strlen(temp)+1,0) == -1){
                perror("server send()");
                exit(1);
            }
    //add log to logQueue(produce)
            //try to get the lock of logQueue
            pthread_mutex_lock(tData->logQueue->lock);
            while(tData->logQueue->count == tData->logQueue->size){
                pthread_cond_wait(tData->logQueue->fill,tData->logQueue->lock);
            }
            addLog(tData->logQueue,wordOrigin,flag,connection.time,time(NULL)-tData->start,connection.priority);
            printf("workT added, countL = [%d]\n",tData->logQueue->count);
            pthread_cond_signal(tData->logQueue->pop);
            //release the lock of connectionBuffer
            pthread_mutex_unlock(tData->logQueue->lock);
        }while(strcmp(wordOrigin,"exit") != 0);
    }
}
/**
    server main thread function - accepting the client connection:
    prepare the server socket and bind it as the listen socket
    (loop starts)
    once accept the connection from client, it will record the arrival time and generate a random priotity
    Then store all the information with the connection to connectionBuffer
    (loop ends)
*/
void* serverThreadCFn(void* tDataArg){
//server socket preparation
    TDataC* tDataC = (TDataC *) tDataArg;
    struct sockaddr_in serverData = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(tDataC->port)
    };
    struct sockaddr_in client;
    int serverFd = socket(AF_INET,SOCK_STREAM,0);
    int clientFd;
    unsigned int sizeC = sizeof(struct sockaddr_in);
    if(serverFd == -1){
        perror("server socket");
        exit(1);
    }
    if(bind(serverFd,(struct sockaddr*)&serverData,sizeof(serverData)) == -1){
        puts("server bind");
        exit(1);
    }
    puts("Sever Binding finished.");
    if(listen(serverFd,3) == -1){
            perror("listen");
            exit(1);
    }
//listen connection from client
    while(1){
        //delay for each accept
        sleep(1);
        puts("ThreadC wait for connection");
        clientFd = accept(serverFd,(struct sockaddr*)&client,(socklen_t *)&sizeC);
        if(clientFd == -1){
            perror("accept");
            exit(1);
        }
        puts("ThreadC established connection");
        //generate time and priority of this connection
        time_t current = time(NULL);
        int priority = rand()%10+1;

    //add connection to connectionBuffer
        //try to get the lock of connectionBuffer
        pthread_mutex_lock(tDataC->connectionBuffer->lock);
        while(tDataC->connectionBuffer->count == tDataC->connectionBuffer->size){
            pthread_cond_wait(tDataC->connectionBuffer->fill,tDataC->connectionBuffer->lock);
        }
        addC(tDataC->connectionBuffer,clientFd,current-tDataC->start,priority);
        printf("serverThreadC added, countC = [%d]\n",tDataC->connectionBuffer->count);
        pthread_cond_signal(tDataC->connectionBuffer->pop);
        //release the lock of connectionBuffer
        pthread_mutex_unlock(tDataC->connectionBuffer->lock);
    }
}
/**
    Server secondary thread function - output log from logQueue to OutPut file
*/
void* serverThreadLogFn(void* tDataArg){
    TDataLog* tDataLog = (TDataLog *) tDataArg;
    FILE* logPtr = tDataLog->logPtr;
    Log log;
    if(logPtr == NULL){
        perror("ThreadLog fopen");
    }
    fprintf(logPtr,"%-20s   %-10s [%-6s] [%-8s] [%-8s]\n\n","Word","Spell","Arrive","Complete","Priority");
    while(1){
    //get Log from LogQueue
        //try to get the lock of logQueue
        pthread_mutex_lock(tDataLog->logQueue->lock);
        while(tDataLog->logQueue->count == 0){
            pthread_cond_wait(tDataLog->logQueue->pop,tDataLog->logQueue->lock);
        }
        popLog(tDataLog->logQueue,&log);
        printf("serverThreadL pop, countL = [%d]\n",tDataLog->logQueue->count);
        pthread_cond_signal(tDataLog->logQueue->fill);
        //release the lock of connectionBuffer
        pthread_mutex_unlock(tDataLog->logQueue->lock);
    //output log to logPtr
        if(strcmp(log.word,"exit")==0){
            //termination
            puts("serverThreadLogFn exits");
            fclose(logPtr);
            pthread_exit(0);
        }else{
            //write Log to Log.txt file
            fprintf(logPtr,"%-20s - %-10s [%-6lu] [%-8lu] [%-8d]\n",log.word,log.flag?"OK":"MISSPELLED",log.arrive,log.complete,log.priority);
            fflush(logPtr);
        }
    }


}
