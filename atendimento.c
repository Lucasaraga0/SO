#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "fila.h"
//#include <time.h>

/* Faltantes
- Problema da recepcao
- Checar se a passagem de parametro esta correta
- Checar se a declaracao de variaveis na main esta correta e completa
- Checar se o acesso as variaveis dentro das threads esta correta
*/


void* atendente(FilaCliente* fila, int paciencia){
    
    sem_t* sem_atend =  sem_open("/sem_atend", O_RDWR);   
    sem_t* sem_block = sem_open("/sem_block", O_RDWR);
    
    int satisfeitos, atendidos = 0;
    int tempo_atual;

    const char *filename = "/tmp/analista_pid.tmp";
    FILE *file = fopen(filename, "r");
    pid_t pid_analista;
    fscanf(file, "%d", &pid_analista);
    fclose(file);
    
    usleep(100);
    
    // enquanto a fila nao esta vazia
    while (filaVazia != 0){
        // acorda o cliente
        int pid_cliente, hora_chegada;
        cliente c = remover(fila);
        
        pid_cliente = c.pid;
        hora_chegada = c.hora_chegada;
        if (kill(pid_cliente, SIGCONT) == -1) {
            perror("Erro ao enviar o sinal para o cliente");
            exit(EXIT_FAILURE); 
        }
            

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
        if (tempo_atual - hora_chegada <= paciencia){
            satisfeitos++;
        }
        // acorda analista (opcional ou a cada 10)

        if (atendidos % 10 == 0){
            if (kill(pid_analista, SIGCONT) == -1) {
                perror("Erro ao enviar o sinal para o analista");
                exit(EXIT_FAILURE); 
            }
        } 
    }
}

void* recepcao(int num_clientes, int tolerancia, int pid_1){


   printf("Comecou a thread recepcao\n\n");
   sem_t* sem_atend =  sem_open("/sem_atend", O_CREAT, 0644, 1);   
   sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);   

    int inicio, fim = 0;
    
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
            //printf("pid do pai: %d\n", pid_1);
                        // se a fila nao estiver cheia 
            /*while (is_full(inicio, fim) == 0){
                usleep(100);
            } */
            
            /*gerar prioridade
            0: prioridade alta
            1: prioridade baixa
            */
            int prioridade_cliente = rand() % 2;
            wait(NULL);
            //entra na fila
            /*FILE* demanda = fopen("demanda.txt", "r");
            fscanf(demanda, "%d", &tempo_atendimento);
            fclose(demanda); */
            
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


            // Agora tenta deletar o arquivo
            if (remove("demanda.txt") == 0) {
                printf("Arquivo 'demanda.txt' deletado com sucesso.\n");
            } else {
                printf("Erro ao deletar o arquivo 'demanda.txt'.\n");
            }


            printf("pid_2: %d \n",pid_2);
            
            //inserir_na_fila(pid_2, hora_chegada,prioridade_cliente,tempo_atendimento);
           
            printf("Cliente %d adicionado na fila\n", i);
            
            //usleep(100000);
        }
            
        }
        
    sem_close(sem_block);
    sem_close(sem_atend);

    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");
    
}

int main(int argc, char *argv[]){

    /*
    0 - nome do programa
    1 - numero de clientes
    2 - tolerancia do cliente (X)
    */  

    int num_clientes = atoi(argv[1]);
    int tolerancia = atoi(argv[2]);
    
    printf("numero de clientes %d\n", num_clientes);
    
    pid_t pid = getpid();
    printf("pid do atendimento: %d\n", pid);

    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, NULL, atendente, NULL);
    pthread_create(&thread_2, NULL, recepcao(num_clientes, tolerancia, pid), NULL);

    pthread_join(thread_1, NULL);
    pthread_join(thread_2,NULL);
    return 0;
}