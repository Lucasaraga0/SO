#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
//#include <time.h>

/*
struct de cliente

int pid;
int hora_chegada;
int prioridade;
int tempo_atendimento;
*/

struct cliente
{
    int pid;
    int hora_chegada;
    int prioridade;
    int tempo_atendimento;
};


/*criar o vetor de clientes, que vai ser a nossa fila 
a ideia da fila eh sempre aumentar o numero infinitamente, e a gente vai pegando o resto dos valores e alocando
os clientes na posicao do resto respectivo, o val_inicial == val_final, então a fila está vazia se val_final - val_inicial == 100, 
entao a fila esta cheia
*/


int is_empty(){

}

int is_full(){

}




void inserir_na_fila(int pid, int hora_chegada, int prioridade, int tempo_atendimento){
    
}

void remover_da_fila(){

}

void* atendente(){

    /*
    
    */

}

void* recepcao(int num_clientes, int tolerancia, int pid_1){


   printf("Comecou a thread recepcao\n\n");
   sem_t* sem_atend =  sem_open("/sem_atend", O_CREAT, 0644, 1);   
   sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);   


    pid_t pid_2;
    int i = 0;

    while (num_clientes == 0 || i < num_clientes){
        //criar processos cliente
        
        pid_2 = fork();
        i ++;
        printf("pid_2: %d\n", pid_2);

        if (pid_2 < 0){
            printf("Não criou cliente\n");
            exit(EXIT_FAILURE);
        }

        else if(pid_2 == 0){

            printf("pid do cliente: %d\n", pid_2);
            printf("Cliente %d criado!\n", i);

            if (execl("./cliente", "cliente", (char *)NULL) == -1) {
                perror("Erro ao executar cliente");
                exit(EXIT_FAILURE);
            }

            printf("Cliente %d adicionado na fila\n", i);
            

        } else {
            // Processo pai
            printf("pid do pai: %d\n", pid_1);
            usleep(100000);
        }

            
        }

    // ler o arquivo de demanda para ver o tempo de atendimento do cliente    

    /*gerar prioridade
    0: prioridade alta
    1: prioridade baixa
    */
    int prioridade_cliente = rand() % 2;

    // adiciona o cara na fila com <pid, hora de chegada, prioridade_cliente, tempo_para atendimento>

    /*
    fila[0].pid = 1;    fila[0].hora_chegada = 1200;    fila[0].prioridade = 5;    fila[0].tempo_atendimento = 15;
    */
        
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
    /*
    aqui poderia criar a struct da fila, ou pelo menos declarar ela, caso seja so um vetor, 
    ela tem que ter aquele formato <PID, Hora de chegada, Prioridade, Tempo para atendimento> para cada posicao
    */

    struct cliente fila[100];

    pthread_t thread_1, thread_2;
    pthread_create(&thread_1, NULL, atendente, NULL);
    pthread_create(&thread_2, NULL, recepcao(num_clientes, tolerancia, pid), NULL);
    
    return 0;
}