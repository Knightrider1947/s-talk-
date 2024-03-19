#include <pthread.h>
#include <stdlib.h>
#include "threads_sockets.h"
#include <stdio.h>



int main(int argc, char *argv[]) {
    while (argc <0 || argc > 4 && argc != 4) {
        printf("s-talk [my port number] [remote machine name] [remote Port number ]\n");
        exit(1);
    }

    send_list = List_create(); // create a list for sending
    receive_list = List_create(); //create a list for receiving

    pthread_t thread[4];


    char* local_port = argv[1];
    s = create_socket_local_bind(local_port); // create a socket and bind local port
    if(s < 0)
    {
        perror("socket creation failed\n"); // error msg
        exit(1);
    }

    char* host = argv[2];
    char* remote_port = argv[3];

    create_remote_address(host, remote_port, &remote_address); // create remote address

    pthread_create(&thread[0], NULL, keyboard_input, NULL); // create screen output thread
    pthread_create(&thread[1], NULL, send_thread, NULL); // create send thread
    pthread_create(&thread[2], NULL, receive_thread, NULL); // create receive thread
    pthread_create(&thread[3], NULL, screen_output, NULL); // create keyboard input thread

    pthread_mutex_lock(&shutdown_mutex); // lock the mutex
    while(!shutdown_signal) // while shutdown signal is 0 it would wait for signal
    {
        pthread_cond_wait(&shutdown_cond, &shutdown_mutex); // wait for the shutdown signal
    }
    pthread_mutex_unlock(&shutdown_mutex); // unlock the mutex

    pthread_cancel(thread[0]); // cancel the keyboard thread
    pthread_cancel(thread[1]); // cancel the send thread
    pthread_cancel(thread[2]); // cancel the receive thread
    pthread_cancel(thread[3]); // cancel the output input thread
    

    
    pthread_join(thread[0], NULL); //wait for keyboard thread to finish
    pthread_join(thread[1], NULL); //wait for send to finish
    pthread_join(thread[2], NULL); //wait for receive thread to finish
    pthread_join(thread[3], NULL); //wait for output input thread to finish
    delete_sync_List(); // delete the lists and destroy the mutex and condition variables and socket close
  
    return 0;
}
