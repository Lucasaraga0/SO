#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "fila.h"
#include <time.h>


void* atendente(FilaCliente* fila, int paciencia){
    
    printf("Comecou a thread atendente");
    
    sem_t* sem_atend =  sem_open("/sem_atend", O_RDWR);   
    sem_t* sem_block = sem_open("/sem_block", O_RDWR);
    
    int satisfeitos = 0, atendidos = 0, prioridade = 0;
    int tempo_atual; // pegar o tempo de execucao do processo 
    
    const char *filename = "/tmp/analista_pid.tmp";
    FILE *file = fopen(filename, "r");
    pid_t pid_analista;
    fscanf(file, "%d", &pid_analista);
    printf("Eu atendente peguei p_diddy do analista: %d\n", pid_analista);
    fclose(file);
    
    
    usleep(100);
    
    // enquanto a fila nao esta vazia
    while (filaVazia != 0){
        // acorda o cliente
        int pid_cliente, hora_chegada;
        cliente c = remover(fila);
        
        pid_cliente = c.pid;
        hora_chegada = c.hora_chegada;
        prioridade = c.prioridade;
        
        if (kill(pid_cliente, SIGCONT) == -1) {
            perror("Erro ao enviar o sinal para o cliente");
            exit(EXIT_FAILURE); 
        }
          
        int tempo_max;
        if (prioridade == 1){
            tempo_max = paciencia/2 + hora_chegada;
        }
        else{
            tempo_max = paciencia + hora_chegada;
        }

        usleep(5);
        // espera sem_atend abrir
        if(sem_atend != SEM_FAILED) sem_wait(sem_atend);
        // espera sem_block abrir
        if(sem_block != SEM_FAILED) sem_wait(sem_block);
        // fecha semaforo sem_block
        if(sem_block != SEM_FAILED) sem_close(sem_block);
        // escreve pid do cliente em LNG
        FILE* LNG = fopen("LNG.txt", "w");
        fprintf("\n%d\n", pid_cliente);
        // abre sem_block
        sem_post(sem_block);
        atendidos++;
        
        // calcula satisfacao
            // (tempo atual - hora de chegada) <= paciência
        if (tempo_atual - hora_chegada <= tempo_max){
            satisfeitos++;
        }

        // acorda analista (cada 10)
        if (atendidos % 10 == 0){
            if (kill(pid_analista, SIGCONT) == -1) {
                perror("Erro ao enviar o sinal para o analista");
                exit(EXIT_FAILURE); 
            }
        } 
    }
}

void* recepcao(FilaCliente* fila, int num_clientes, int pid_1){

   printf("Comecou a thread recepcao\n");
   sem_t* sem_atend =  sem_open("/sem_atend", O_CREAT, 0644, 1);   
   sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);   

    int inicio = 0 , fim = 0;
    
    int hora_chegada;
    int tempo_atendimento;

    pid_t pid_2;
    int i = 0;
    
    while ((num_clientes == 0) || (i < num_clientes)){
        //criar processos cliente
        
        pid_2 = fork();
        i ++;

        if (pid_2 < 0){
            printf("Não criou cliente\n");
            exit(EXIT_FAILURE);
        }

        else if(pid_2 == 0){

            int pid_cliente = getpid();
            printf("pid do cliente %d: %d\n", i, pid_cliente);
            //printf("Cliente %d criado!\n", i);

            if (execl("./cliente", "cliente", (char *)NULL) == -1) {
                perror("Erro ao executar cliente");
                exit(EXIT_FAILURE);
            }
        } else {
            // Processo pai

            while (filaCheia(fila) == 0){
                usleep(1);
            } 
            
            /*gerar prioridade
            0: prioridade alta
            1: prioridade baixa
            */
            int prioridade_cliente = rand() % 2;
            //entra na fila
            
            FILE* demanda = fopen("demanda.txt", "r");
            if (demanda == NULL) {
                perror("Erro ao abrir o arquivo 'demanda.txt'");
                exit(EXIT_FAILURE);
            }
            if (fscanf(demanda, "%d", &tempo_atendimento) != 1) {
                fprintf(stderr, "Erro ao ler o tempo de atendimento do arquivo 'demanda.txt'\n");
                fclose(demanda);
                exit(EXIT_FAILURE);
            }
            fclose(demanda);

            printf("pid_2: %d \n",pid_2);
            tempo_atendimento; //pegar tempo decorrido ate agora, para colocar na fila 

            cliente c;
            c.hora_chegada = hora_chegada;
            c.pid = pid_2;
            c.prioridade = prioridade_cliente;
            c.tempo_atendimento = tempo_atendimento;

            inserir_na_fila(&fila, c);
           
            printf("Cliente %d adicionado na fila\n", i);

        }
            
        }
        

    
}

int main(int argc, char *argv[]){

    /*
    0 - nome do programa
    1 - numero de clientes
    2 - tolerancia do cliente (X)
    */  
    FilaCliente fila;
    inicializarFila(&fila);

    int num_clientes = atoi(argv[1]);
    int tolerancia = atoi(argv[2]);
    
    printf("numero de clientes %d\n", num_clientes);

    pid_t pid = getpid();
    printf("pid do atendimento: %d\n", pid);

    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, NULL, atendente, NULL);
    pthread_create(&thread_2, NULL, recepcao(&fila, num_clientes, pid), NULL);

    pthread_join(thread_1, NULL);
    pthread_join(thread_2,NULL);

    // sem_close(sem_block);
    // sem_close(sem_atend);

    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");

    return 0;
}