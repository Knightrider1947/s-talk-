#ifndef threads_sockets_h
#define threads_sockets_h
#include <pthread.h>    

#include "list.h"

#define BUFFER 256

extern struct sockaddr_in remote_address;
extern int s;
extern List *send_list, *receive_list;
extern pthread_cond_t shutdown_cond; // using Brian's video method
extern pthread_mutex_t shutdown_mutex; // using Brian's video method
extern int shutdown_signal;


int create_socket_local_bind(const char* port);
void create_remote_address(const char* host, const char* port, struct sockaddr_in* remote_address);
void* send_thread(void*);
void* receive_thread(void*);
void* keyboard_input(void*);
void* screen_output(void*);
void* delete_sync_List(void);

#endif
