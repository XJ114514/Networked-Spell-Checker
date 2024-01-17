#include "../header/header.h"
#include "threadFn.c"

#define DEFAULT_DICTIONARY "serverFile/dictionary.txt";
#define DEFAULT_PORT 61323;
#define DEFAULT_CONNECTION_SIZE 20;
#define DEFAULT_WORKTHREAD_SIZE 5;
#define DEFAULT_SCHEDULE_TYPE 1;
/**
    Student Name: Xu Jiang
    It is a program for setting up server checking whether the word sent from client is spelled correctly.
    There are 5 customized options:
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

*/


void* workThreadFn(void* TdataArg);
void* serverThreadCFn(void* tDataArg);
void* serverThreadLogFn(void* tDataArg);
int main(const int argc, char * argv[]){
//process cmd line
    char options[5] = "dpcws";
    char option = -1;
    char *dictionaryPath = DEFAULT_DICTIONARY;
    int portN = DEFAULT_PORT;
    int connectionSize = DEFAULT_CONNECTION_SIZE;
    int workTSize = DEFAULT_WORKTHREAD_SIZE;
    int scheduleF = DEFAULT_SCHEDULE_TYPE;
    do{
        option = getopt(argc,argv,options);
        if(option == '?'){
            perror("getopt");
            exit(1);
        }
        switch(option){
            case 'd':
                dictionaryPath = argv[optind];
                break;
            case 'p':
                portN = atoi(argv[optind]);
                break;
            case 'c':
                connectionSize = atoi(argv[optind]);
                break;
            case 'w':
                workTSize = atoi(argv[optind]);
                break;
            case 's':
                scheduleF = atoi(argv[optind]);
        }
    }while(option != -1);

    pthread_t threads[workTSize+2];
    time_t start = time(NULL);
    srand(start);
//dictionary data initializing
    int sizeDic = 110000;
    int singleWordLength = 51;
    char bufferDic[singleWordLength];
    char* flagEOF;
    Word words[sizeDic];
    FILE* dictionaryPtr;
//LogQueue data initializing
    FILE *logPtr = fopen("serverFile/log.txt","w");
    if(logPtr == NULL){
        perror("fopen");
        exit(1);
    }
    int sizeLog = 20;
    Log queue[sizeLog];
    //cvs and lock for LogQueue
    pthread_cond_t fillcv1 = PTHREAD_COND_INITIALIZER;
    pthread_cond_t popcv1 = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t bufferLock1 = PTHREAD_MUTEX_INITIALIZER;
//ConnectionBuffer data initializing
    Connection connections[connectionSize];
    int pointerC[connectionSize];
    //initialize pointerC to none is occupied
    for(int i = 0; i < connectionSize; i++){
        pointerC[i]= 0;
    }
    //cvs and lock for ConnectionBuffer
    pthread_cond_t fillcv2 = PTHREAD_COND_INITIALIZER;
    pthread_cond_t popcv2 = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t bufferLock2 = PTHREAD_MUTEX_INITIALIZER;

//process dictionary
    Dict dict = {
        .words = words,
        .size = 0
    };
    dictionaryPtr = fopen(dictionaryPath,"r");
    if(dictionaryPtr == NULL){
        perror("fopen");
        exit(1);
    }
    while(1){
        flagEOF = fgets(bufferDic,singleWordLength,dictionaryPtr);
        if(flagEOF == NULL){
            break;
        }
        bufferDic[strlen(bufferDic)-1] = '\0';
        for(int i = 0 ; i< strlen(bufferDic) ; i++){
            bufferDic[i] = toupper(bufferDic[i]);
        }
        strcpy(dict.words[(dict.size)++].str,bufferDic);
    }
//create log entry queue
    LogQueue logQueue = {
        .size = sizeLog,
        .queue = queue,
        .fillPtr = 0,
        .popPtr = 0,
        .count = 0,
        .fill = &fillcv1,
        .pop = &popcv1,
        .lock = &bufferLock1
    };
//Create connection buffer
    ConnectionBuffer connectionBuffer = {
        .size = connectionSize,
        .buffer = connections,
        .pointer = pointerC,
        .count = 0,
        .fill = &fillcv2,
        .pop = &popcv2,
        .lock = &bufferLock2
    };

//create worker thread pool
    TData tData = {
            .connectionBuffer = &connectionBuffer,
            .logQueue = &logQueue,
            .dict = &dict,
            .flagMode = scheduleF,
            .start = start
    };
    for(int i = 0; i< workTSize; i++){
        pthread_create(&threads[i],NULL,workThreadFn,(void *)&tData);
    }
//accept connection thread
    TDataC tDatac = {
        .connectionBuffer = &connectionBuffer,
        .port = portN,
        .start = start
    };
    pthread_create(&threads[workTSize],NULL,serverThreadCFn,(void *)&tDatac);

//consume logQueue thread
    TDataLog tDataLog = {
        .logQueue = &logQueue,
        .logPtr = logPtr
    };
    pthread_create(&threads[workTSize+1],NULL,serverThreadLogFn,(void *)&tDataLog);

    //wait for the log thread exit
    pthread_join(threads[workTSize+1],NULL);
    puts("sever exits");
    exit(0);
}
