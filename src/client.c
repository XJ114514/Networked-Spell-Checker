#include "../header/header.h"
#define DEFAULT_PORT 61323;
/**
    Student Name: Xu Jiang
    It is a program for setting up the client sending word to server
    when calling as ./client <"NUM">, client will use "clientFile/in<"NUM">.txt" as input file , "clientFile/out<"NUM">.txt" as output file.
    when calling as ./client exit, client will send a message to server asking server to terminate.

*/
int main(const int argc, char* const argv[]){
    if(!(argc == 2 || argc == 3)){
        puts("Error! Please input the correct argument \n(integer as which file to RW) \n(exit to terminate the server)");
        exit(1);
    }
    int port = DEFAULT_PORT;
    if(argc == 3){
        port = atoi(argv[2]);
    }
//socket preparation
    struct sockaddr_in serverData = {
        .sin_family = AF_INET,
        //server address
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port)
    };
    int clientFd = socket(AF_INET,SOCK_STREAM,0);
    if(clientFd == -1){
        perror("client socket");
        exit(1);
    }
    if(connect(clientFd,(struct sockaddr*)&serverData,sizeof(serverData)) == -1){
        perror("client connect");
        exit(1);
    }
    puts("Established connection");
//termination condition
    if(strcmp(argv[1],"exit") == 0){
        puts("Sent exit signal to server");
        if(send(clientFd,"exit",5,0) == -1){
                perror("client send()");
                exit(1);
        }
        exit(0);
    }
    char inPath[51],outPath[51];
    FILE* inPtr;
    FILE* outPtr;
    char word[61];
    char returnV[61];
    char* flag;
    int flagRecv = 1;
//IO files
    sprintf(inPath,"clientFile/in%d.txt",atoi(argv[1]));
    sprintf(outPath,"clientFile/out%d.txt",atoi(argv[1]));
    inPtr = fopen(inPath,"r");
    if(inPtr == NULL){
        perror("inFile fopen");
        exit(1);
    }
    outPtr = fopen(outPath,"w");
    if(outPtr == NULL){
        perror("outFile fopen");
        exit(1);
    }
//Communicate with server
    while(1){
        //get the word from file
        flag = fgets(word,51,inPtr);
        word[strlen(word)-2] = '\0';
        //if EOF, break the loop
        if(flag == NULL){
            break;
        }
        //send word
        if(send(clientFd,word,strlen(word)+1,0) == -1){
            perror("client send()");
            exit(1);
        }
        //recv message
        flagRecv = recv(clientFd,returnV,400,0);
        if(flagRecv == -1){
            perror("client recv()");
            exit(1);
        }else if(flagRecv == 0){
            break;
        }
        //output the message to file
        fprintf(outPtr,"%s",returnV);
        fflush(outPtr);
    }
    puts("Finished communication");
    fclose(inPtr);
    fclose(outPtr);
    exit(0);
}
