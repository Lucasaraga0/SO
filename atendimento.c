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

typedef struct {
    FilaCliente* fila;
    int paciencia;
    clock_t inicio_programa;
} AtendenteArgs;

typedef struct {
    FilaCliente* fila;
    int num_clientes;
    pid_t pid_atendente;
    clock_t inicio_programa;
} RecepcaoArgs;


void* atendente(void* args){
    AtendenteArgs* atend_args = (AtendenteArgs*) args;

    FilaCliente* fila = atend_args-> fila;
    int paciencia = atend_args -> paciencia;
    clock_t inicio_programa = atend_args->inicio_programa;

    printf("Comecou a thread atendente");
    
    sem_t* sem_atend =  sem_open("/sem_atend", O_RDWR);   
    sem_t* sem_block = sem_open("/sem_block", O_RDWR);
    
    int satisfeitos = 0, atendidos = 0;

    
    const char *filename = "/tmp/analista_pid.tmp";
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo analista_pid.tmp");
        pthread_exit(NULL);
    }
    pid_t pid_analista;
    fscanf(file, "%d", &pid_analista);
    printf("Eu atendente peguei p_diddy do analista: %d\n", pid_analista);
    fclose(file);
    
    //se no comeco ainda tiver vazia ao chegar aqui
    //na nossa logica, so estara vazia novamente apos terminar programa
    while(filaVazia(fila)){
    }

    
    // enquanto a fila nao esta vazia
    while (!filaVazia(fila)){
        // acorda o cliente
        int pid_cliente, hora_chegada, prioridade;
        cliente c = remover(fila);
        
        pid_cliente = c.pid;
        hora_chegada = c.hora_chegada;
        prioridade = c.prioridade;
        
        if (kill(pid_cliente, SIGCONT) == -1) {
            perror("Erro ao enviar o sinal para o cliente");
            exit(EXIT_FAILURE); 
        }
        
        clock_t tempo_atual = clock();
        
        // checar as medidas de tempo para nao dar problema
        double tempo_decorrido = (tempo_atual - inicio_programa)/CLOCKS_PER_SEC; 
        int tempo_max;
        
        if (prioridade == 1){
            //caso der ruim, ver unidades de paciencia e hora_chegada
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
        fclose(LNG);
        // abre sem_block
        sem_post(sem_block);
        atendidos++;
        
        // calcula satisfacao
            // (tempo atual - hora de chegada) <= paciência
        if ((int)tempo_decorrido - hora_chegada <= tempo_max){
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

void* recepcao(void* args){

    RecepcaoArgs* rec_args = (RecepcaoArgs*) args;
    FilaCliente* fila = rec_args->fila;
    int num_clientes = rec_args->num_clientes;
    pid_t pid_atendente = rec_args->pid_atendente;
    clock_t inicio_programa = rec_args->inicio_programa;

    printf("Comecou a thread recepcao\n");
    sem_t* sem_atend =  sem_open("/sem_atend", O_CREAT, 0644, 1);   
    sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);   

    
    int hora_chegada;


    int i = 0;
    
    while ((num_clientes == 0) || (i < num_clientes)){
        //criar processos cliente
        
        pid_t pid_cliente = fork();
        i ++;

        if (pid_cliente < 0){
            printf("Não criou cliente\n");
            exit(EXIT_FAILURE);
        }

        else if(pid_cliente == 0){

            //int pid_cliente = getpid();
            // printf("pid do cliente %d: %d\n", i, pid_cliente);
            //printf("Cliente %d criado!\n", i);

            if (execl("./cliente", "cliente", (char *)NULL) == -1) {
                perror("Erro ao executar cliente");
                exit(EXIT_FAILURE);
            }
        } else {
            // Processo pai

            while (filaCheia(fila)){
                usleep(1);
            } 
            
            /*gerar prioridade
            0: prioridade alta
            1: prioridade baixa
            */
            int prioridade_cliente = rand() % 2;
            //entra na fila
            int tempo_atendimento;
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
            
            clock_t tempo_atual = clock();
            //printf("pid_cliente: %d \n",pid_cliente);
            double tempo_decorrido = (double)(tempo_atual - inicio_programa) / CLOCKS_PER_SEC;

            cliente c;
            c.hora_chegada = (int)tempo_decorrido;
            c.pid = pid_cliente;
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

    clock_t inicio_programa = clock();

    printf("Numero de clientes %d\n", num_clientes);

    pthread_t thread_atendente, thread_recepcao;

    AtendenteArgs atend_args;
    atend_args.fila = &fila;
    atend_args.inicio_programa = inicio_programa;
    atend_args.paciencia = tolerancia;
    
    RecepcaoArgs recep_args;
    recep_args.fila = &fila;
    recep_args.inicio_programa = inicio_programa;
    recep_args.num_clientes = num_clientes;
    recep_args.pid_atendente = getpid();

    pthread_create(&thread_atendente, NULL, atendente, &atend_args);
    pthread_create(&thread_recepcao, NULL, recepcao, &recep_args);

    pthread_join(thread_atendente, NULL);
    pthread_join(thread_recepcao,NULL);

    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");

    return 0;
}