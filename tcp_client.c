#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>
#include<string.h>
#include<arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define MAX  15
#define PORT_NUM 7891


void delay();
void *run_client(void * in_arg);
void create_threads(const char * ip_addr);
void delete_thread(pthread_t threadArray[3]);

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Usage: /{filename} {server_ip}\n");
        return -1;
    }
    char * ip_addr = argv[1];
    
    create_threads(ip_addr);
    return 0;
}



void create_threads(const char * ip_addr){
    //Initializing threads
    pthread_t threadArray[3];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    //Creating three writer threads
    int i;
    for(i = 0; i < 3; i++){
        if (pthread_create(&threadArray[i], NULL, run_client, (void *)ip_addr) != 0) {
            perror("Unable to create thread to handle new connection");
            exit(EXIT_FAILURE);
        }
    }
    delete_thread(threadArray);
}

void delete_thread(pthread_t threadArray[3]){
    int i;
    for(i = 0; i < 3; i++){
        pthread_join(threadArray[i],NULL);
    }
}


void delay(int number_of_seconds){
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
    
    // Stroing start time
    clock_t start_time = clock();
    
    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds);
}



void *run_client(void * in_arg){
    const char * ip_addr  = (const char *)in_arg;
    char received[MAX], to_send[MAX], username[MAX];
    int net_socket, conn_stat, valread;
    struct sockaddr_in server_address;
    socklen_t address_size;
    
    //FILE * fp = fopen("mydata.txt", "r");
    
    //creates shared memory
    key_t key = ftok("shmfile",65);
    int shmid = shmget(key,1024,0666|IPC_CREAT);
    char *str = (char*) shmat(shmid,(void*)0,0);
    int i = 0;
    
    //reads in message of max length 15 from a txt file
    FILE *ptrFile;
    char word[15];
    char *filename = "message.txt";
    ptrFile = fopen(filename,"r");
    if(ptrFile == NULL){
        return 1;
    }
    fgets(word,15,ptrFile);
    
    
    //places string from file in shared memory
    while (word[i]!='\0')
    {
        str[i] = word[i];
        i++;
    }
    printf("Shared memory created.\n");
    fclose(ptrFile);
    shmdt(str);
    
    if((net_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Socket Failed");
        exit(EXIT_FAILURE);
    }
    
    //Server address is IPv4 and port is PORT_NUM
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT_NUM);
    server_address.sin_addr.s_addr = inet_addr(ip_addr);
    
    // address_size = sizeof server_address;
    if(connect(net_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1){
        printf("There was an error connecting to the socket\n");
        printf("ERRNO %s\n",strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    
    // TODO - Shared Buffer needs to be created
    memset(to_send, 0, sizeof(to_send));
    // fgets(shared_buffer, MAX, fp);
    
    
    //Sending message to server
    if(send(net_socket, to_send, strlen(to_send), 0) < 0) {
        printf("Error: send() failed\n");
        exit(EXIT_FAILURE);
    }
    
    // Receive reply and terminate
    if(recv(net_socket, received, sizeof(received), 0) < 0) {
        printf("Error: recv() failed\n");
        exit(EXIT_FAILURE);
    }
    printf("Server: %s\n", received);
    
    return 1;
}
