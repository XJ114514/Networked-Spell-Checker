#include "../header/header.h"
/**
    Student Name: Xu Jiang
    A sourceFile containing implementation of logQueue
*/
typedef struct Log{
    char word[51];
    int flag;
    time_t arrive;
    time_t complete;
    int priority;
}Log;
typedef struct Queue{
    int size;
    Log *queue;
    int fillPtr;
    int popPtr;
    int count;
    pthread_cond_t *fill;
    pthread_cond_t *pop;
    pthread_mutex_t *lock;
}LogQueue;

void addLog(LogQueue *logQueue,char* word, int flag,time_t arrive, time_t complete, int priority);
Log* popLog(LogQueue *logQueue,Log *log);

/**
    Simple function that will add a new Log to logQueue
    - add the Log to the *logQueue.
*/
void addLog(LogQueue *logQueue,char* word, int flag,time_t arrive, time_t complete, int priority){
    Log log = {
        .flag = flag,
        .arrive = arrive,
        .complete = complete,
        .priority = priority
    };
    strcpy(log.word,word);
    logQueue->queue[logQueue->fillPtr] = log;
    logQueue->fillPtr = ((logQueue->fillPtr)+1) % logQueue->size;
    (logQueue->count)++;
}
/**
    Simple function that will pop a new Log to *log
    - pop the current first Log to *log.
*/
Log* popLog(LogQueue *logQueue,Log *log){
    //populate output log
    strcpy(log->word, logQueue->queue[logQueue->popPtr].word);
    log->flag = logQueue->queue[logQueue->popPtr].flag;
    log->arrive = logQueue->queue[logQueue->popPtr].arrive;
    log->complete = logQueue->queue[logQueue->popPtr].complete;
    log->priority = logQueue->queue[logQueue->popPtr].priority;
    //update popPtr and count
    logQueue->popPtr = ((logQueue->popPtr)+1) % logQueue->size;
    (logQueue->count)--;
    return log;
}
