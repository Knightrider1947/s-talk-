#include "threads_sockets.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int shutdown_signal = 0;

struct sockaddr_in remote_address;
int s; // socket descriptor
List *send_list, *receive_list;


static pthread_mutex_t send_mutex = PTHREAD_MUTEX_INITIALIZER; // using Brian's video method initializing
static pthread_mutex_t receive_mutex = PTHREAD_MUTEX_INITIALIZER; // using Brian's video method initializing
static pthread_cond_t send_cond = PTHREAD_COND_INITIALIZER; // using Brian's video method initializing
static pthread_cond_t receive_cond = PTHREAD_COND_INITIALIZER; // using Brian's video method initializing
pthread_cond_t shutdown_cond = PTHREAD_COND_INITIALIZER; // using Brian's video method of initializing
pthread_mutex_t shutdown_mutex = PTHREAD_MUTEX_INITIALIZER; // using Brian's video method of initializing


// using extern to directly use them in main.c
int create_socket_local_bind(const char* port)
{
    //char buffer[BUFFER];
    struct sockaddr_in my_address;

    // Set up your address structure my_address using exactly in Brian, beej's book along with research
    memset(&my_address, 0, sizeof(my_address)); // Clear the structure Brian's video
    my_address.sin_family = AF_INET; // IPv4    
    my_address.sin_addr.s_addr = INADDR_ANY;  // Accept from any IP address 
    my_address.sin_port = htons(atoi(port)); // Convert the port number to network byte order


 // using guide for below 
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket failed to create\n");
        exit(1);
    }
    int confirm_bind; // for testing if bind was successful
   
    confirm_bind = bind(s, (struct sockaddr *)&my_address, sizeof(my_address)); // Bind the socket test if bind was successful

    if (confirm_bind < 0) {
        perror("failed to bind\n");
        exit(1);
    }


    return s; // return the socket
}

void create_remote_address(const char* host, const char* port, struct sockaddr_in* remote_address)
{
    
    //en.wikipedia.org/wiki/getaddrinfo 
    /*   /getaddrinfo()
    getaddrinfo() converts human-readable text strings representing 
    hostnames or IP addresses into a dynamically 
    allocated linked list of struct addrinfo structures. 
    struct addrinfo {
    int       ai_flags;
    int       ai_family;
    int       ai_socktype;
    int       ai_protocol;
    socklen_t ai_addrlen;
    struct    sockaddr* ai_addr;
    char*     ai_canonname;       canonical name 
    struct    addrinfo* ai_next;  this struct can form a linked list 
};
    
    The function prototype for this function is specified as follows:
    
    
    int getaddrinfo(const char* hostname,
                const char* service,
                const struct addrinfo* hints,
                struct addrinfo** res);*/

    struct addrinfo hints, *res; // hints is a pointer to a struct addrinfo
    memset(&hints, 0, sizeof(struct addrinfo)); // Clear structure Brian's video
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    int j; //for testing if getaddrinfo was successful
    j = getaddrinfo(host, port, &hints, &res);
    if (j != 0) 
    {
        fprintf(stderr, "getaddrinfo failed%s\n", gai_strerror(j)); // was using print but then found this 
        exit(1);                                                // provide error details    
    }
   *remote_address = *(struct sockaddr_in *)res->ai_addr; // set remote address to the address of the remote machine
    freeaddrinfo(res); // free memory allocated by getaddrinfo

}


void* send_thread(void*)
 {
    char* message; // pointer to a char
   
    while (1) {
        pthread_mutex_lock(&send_mutex); // lock send mutex
        while (List_count(send_list) == 0) { // if send list is empty it would wait for 
            pthread_cond_wait(&send_cond, &send_mutex); //  wait for msg to be added to the send list
        }
        message = (char*) List_first(send_list); // going in as FIFO
        List_remove(send_list); // remove message from the list
        pthread_mutex_unlock(&send_mutex); // unlock send mutex
        ssize_t msg_size = sendto(s, message, strlen(message), 0, (struct sockaddr*)&remote_address, sizeof(remote_address));
        if(msg_size < 0)
        {
            printf("failed to send msg\n"); // print error message
            free(message);
        }
        free(message);
    }

    return NULL;
 }

void* receive_thread(void*) {
   
    char buffer[BUFFER]; 

    while (1) {
        ssize_t msg_size = recvfrom(s, buffer, sizeof(buffer), 0, NULL, NULL);// receive the message
        if(buffer[0] == '!' && buffer[1] =='\n' ) // if the message is ! and enter
        {
            pthread_mutex_lock(&shutdown_mutex); // lock the shutdown mutex
            shutdown_signal = 1; // set the shutdown signal to 1
            pthread_cond_signal(&shutdown_cond); // signal the shutdown condition
            pthread_mutex_unlock(&shutdown_mutex); // unlock the shutdown mutex
            break;
        }
        if (msg_size > 0) { // if message size is greater than 0
            buffer[msg_size] = '\0'; // set last character to null
            pthread_mutex_lock(&receive_mutex); // lock receive mutex
            List_append(receive_list, strdup(buffer)); //add msg to the receive list
            pthread_cond_signal(&receive_cond); // signal receive condition that a message has been received 
            pthread_mutex_unlock(&receive_mutex); // unlock receive mutex
        }
        else if(msg_size < 0)
        {
            printf("Failed to receive msg\n"); // print error message
        }
    }
    
    return NULL;
}

void* keyboard_input(void*) {

    char buffer[BUFFER];
    while (fgets(buffer, sizeof(buffer), stdin)) { // get the input from the user
        if (buffer[0] == '!' && buffer[1] == '\n') { // condition of ! entering and press enter 
            pthread_mutex_lock(&send_mutex); // lock send mutex
            List_append(send_list, strdup(buffer)); // add the message to the send list
            pthread_cond_signal(&send_cond); // signal send condition
            pthread_mutex_unlock(&send_mutex); // unlock send mutex




            pthread_mutex_lock(&shutdown_mutex); // lock shutdown mutex
            shutdown_signal = 1; // set shutdown signal to 1 in main.c
            pthread_cond_signal(&shutdown_cond); // signal shutdown condition
            pthread_mutex_unlock(&shutdown_mutex); // unlock shutdown mutex
            break;
        } else {
            pthread_mutex_lock(&send_mutex); // lock send mutex
            List_append(send_list, strdup(buffer)); // add the message to the send list
            pthread_cond_signal(&send_cond); // signal send condition
            pthread_mutex_unlock(&send_mutex); // unlock send mutex
        }
    }
    return NULL;
}

void* screen_output(void*) { // output message to the screen thread
    
    
    
    
    char* message; // pointer to char
    while (1) {
        pthread_mutex_lock(&receive_mutex); // lock receive mutex
        while (List_count(receive_list) == 0) { // if receive list is empty it would wait
            pthread_cond_wait(&receive_cond, &receive_mutex); // signal & mutex
        }
        message = (char*) List_first(receive_list); // going out as FIFO
        List_remove(receive_list); // pointer set to current first item remove from list
        pthread_mutex_unlock(&receive_mutex); // unlock receive mutex
        fputs(message, stdout); // print to screen using fputs as requested in details on discussion 
        free(message); // free memory
    }
    return NULL;
    
}

void* delete_sync_List(void) {
    //clean up all here
    List_free(send_list, free);
    List_free(receive_list, free);
    pthread_cond_destroy(&send_cond);
    pthread_cond_destroy(&receive_cond);
    pthread_mutex_destroy(&send_mutex);
    pthread_mutex_destroy(&receive_mutex);
    pthread_cond_destroy(&shutdown_cond);
    pthread_mutex_destroy(&shutdown_mutex);
    close(s);
    return NULL;
}


