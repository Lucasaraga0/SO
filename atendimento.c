#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
//#include <time.h>

/*
aqui poderia criar a struct da fila, ou pelo menos declarar ela, caso seja so um vetor, 
ela tem que ter aquele formato <PID, Hora de chegada, Prioridade, Tempo para atendimento> para cada posicao
*/


void* atendente(){

}

void* recepcao(){

    /*
    aqui que devem ser criados os dois semaforos usados: sem_atend e sem_block
    */
   sem_t* sem_atend = sem_open("/sem_atend", O_CREAT, 0644, 1);   
   sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);   

    //criar processos cliente

    // se o numero de processos dados na entrada for 0, entao fica num loop infinito

    // se nao faz so um loop ate criar todos os processos 

    // ler o arquivo de demanda para ver o tempo de atendimento do cliente
    

    /*gerar prioridade
    0: prioridade alta
    1: prioridade baixa
    */
   int prioridade_cliente = rand() % 2;

    // adiciona o cara na fila com <>
}

int main(){
    pid_t pid = getpid();


    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, NULL, atendente, NULL);
    pthread_create(&thread_2, NULL, recepcao, NULL);
    
    

    return 0;
}