#include "../header/header.h"
/**
    Student Name: Xu Jiang
    A sourceFile containing implementation of connectionBuffer
*/
typedef struct Item{
    int socket;
    time_t time;
    int priority;
} Connection;
typedef struct Buffer{
    //the maximum size of the buffer
    int size;
    Connection* buffer;
    //store 1 to that index if that index of buffer is occupied, store 0 to the index if it is not occupied.
    int* pointer;
    //current connenction count in the buffer
    int count;
    pthread_cond_t *fill;
    pthread_cond_t *pop;
    pthread_mutex_t *lock;
} ConnectionBuffer;

int firstEmpty(int* pointer, int size);
int find(Connection* buffer, int* pointer, int size,int flag);
void addC(ConnectionBuffer* bufferObj,int socket, time_t time, int priority);
Connection* popC(ConnectionBuffer* bufferObj,Connection* connection, int flagMode);

/*
    Simple internal function that will return the first index of target that has value 0(not occupied)
    - return first index of target that has value 0, return -1 if there is not 0 value
*/
int firstEmpty(int *pointer, int size){
    for(int i = 0 ; i < size; i++){
        if(pointer[i] == 0){
            return i;
        }
    }
    return -1;
}
/*
    Simple internal function that will return the index of connection stored in buffer
    in priority mode(flag = 1), return the index of connection that has highest priority
    in sequential mode (flag=0), return the index of connection that has smallest arrival time
    - return the index of connection that has highest priority (flag = 1) or smallest time (flag = 0) according to different mode
*/
int find(Connection *buffer, int *pointer, int size,int flag){
    int index = -1;
    int key = -1;
    for(int i = 0; i < size; i++){
        //if the index is occupied
        if(pointer[i] == 1){
            if(flag){
                //priority mode
                //get the largest priority
                if(key < buffer[i].priority){
                    key = buffer[i].priority;
                    index = i;
                }
            }else{
                //sequential mode
                //get the smallest time
                if(key == -1){
                    key = buffer[i].time;
                    index = i;
                }else if(key > buffer[i].time){
                    key = buffer[i].time;
                    index = i;
                }
            }
        }
    }
    return index;
}

/**
    function that will add a new Connection with file descriptor-socket, arrive time and priority to ConnectionBuffer
    - add the connection to the *connectionBuffer.
*/
void addC(ConnectionBuffer *connectionBuffer,int socket, time_t time, int priority){
    //populate connection
    Connection connection;
    connection.socket = socket;
    connection.time = time;
    connection.priority = priority;
    //add connection to connectionBuffer and update pointer and count
    int fillIndex = firstEmpty(connectionBuffer->pointer,connectionBuffer->size);
    connectionBuffer->pointer[fillIndex] = 1;
    connectionBuffer->buffer[fillIndex] = connection;
    connectionBuffer->count += 1;
}

/**
    function that will pop a new connection with file descriptor-socket, arrive time and priority from the *connectionBuffer
    - pop the first connection out to the *connection.
*/
Connection* popC(ConnectionBuffer *connectionBuffer, Connection *connection, int flagMode){
    //get the popIndex according to mode flag
    int popIndex = find(connectionBuffer->buffer,connectionBuffer->pointer,connectionBuffer->size,flagMode);
    //update pointer and count
    connectionBuffer->pointer[popIndex] = 0;
    connectionBuffer->count -= 1;
    //populate the output connection
    connection->socket = connectionBuffer->buffer[popIndex].socket;
    connection->time = connectionBuffer->buffer[popIndex].time;
    connection->priority = connectionBuffer->buffer[popIndex].priority;
    //release the space
    Connection *connectionEmpty = malloc(0);
    connectionBuffer->buffer[popIndex] = *connectionEmpty;
    return connection;
}
