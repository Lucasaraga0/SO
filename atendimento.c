#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

void* atendente(){

}

void* recepcao(){

}

int main(){
    pid_t pid = getpid();


    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, NULL, atendente, NULL);
    pthread_create(&thread_2, NULL, recepcao, NULL);
    
    

    return 0;
}