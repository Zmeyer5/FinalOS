//**************//
//Curent Functionality
//
//Hosts communication between client and server
//One message at a time communication(bad)
//


#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>



#define MAX 1000
#define PORT_NUM 7891


void create_server();
void *messager(void *in_arg);



int main(){
    create_server();
    return 0;
}



void create_server(){
    
    int server_fd, new_socket;
   	struct sockaddr_storage server_storage;
    socklen_t addr_size;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t thread_id;
    
    
    // Creating socket file descriptor
    //Socket is using IPv4 and TCP. Checks if NULL
    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    //Address is a struct of sockaddr_in and uses IPv4 and the localhost for IP
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT_NUM);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    
    // Forcefully attaching socket to the port PORT_NUM
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listening...\n");
    if (listen(server_fd, 3) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    int remaining_connections = 3; // TODO: change while condition to check shared memory for notification to terminate?
    while (remaining_connections > 0)  {
        addr_size = sizeof(server_storage);
        if ((new_socket = accept(server_fd, (struct sockaddr *) &server_storage, &addr_size)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if (pthread_create(&thread_id, NULL, messager, (void *)(intptr_t)new_socket) != 0) {
            perror("Unable to create thread to handle new connection");
            exit(EXIT_FAILURE);
        }
        remaining_connections--;
    }
}



void *messager(void *in_arg){
    int new_socket = (intptr_t) in_arg;
    char buffer[MAX] = {0};
    char *message = "testing, testing, 1, 2, 3..."; // Test response
    char hello[] =
    "\n   *****************************\n\
    *                           *\n\
    *       Welcome to the      *\n\
    *           server!         *\n\
    *                           *\n\
    *                           *\n\
    *****************************\n\n\n\n\n";
    
    //create shared memory str
    key_t key = ftok("shmfile",65);
    int shmid = shmget(key,1024,0666|IPC_CREAT);
    char *str = (char*) shmat(shmid,(void*)0,0);
    
    // Send server greeting message
    send(new_socket, hello, strlen(hello),0);
    
    // Receive writer message
    recv(new_socket, buffer, 1024,0);
    
    //prints shared memory as long as its not empty
    if (strlen(str) != 0)
        printf("DATA READ FROM SHARED MEMORY: %s\n", str);
    //printf("SENDING MESSAGE\n");
    sleep(2);
    
    //sends shared memory contents with client
    send(new_socket, str, strlen(str), 0);
    
    shmdt(str);
    shmctl(shmid,IPC_RMID,NULL);
    
    //printf("Other user: %s", buffer);
    
    
    close(new_socket);
    pthread_exit(NULL);
}
