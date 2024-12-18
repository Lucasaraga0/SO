#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include<stdbool.h>
#include "fila.h"

/*
Para fazer

*/

bool podeparar = false;

typedef struct {
    FilaCliente* fila;
    int paciencia;
    struct timespec inicio_programa;
;
} AtendenteArgs;

typedef struct {
    FilaCliente* fila;
    int num_clientes;
    pid_t pid_atendente;
    struct timespec inicio_programa;
} RecepcaoArgs;


void* atendente(void* args){
    usleep(1000);

    AtendenteArgs* atend_args = (AtendenteArgs*) args;

    FilaCliente* fila = atend_args-> fila;
    int paciencia = atend_args -> paciencia;
    struct timespec inicio_programa = atend_args->inicio_programa;
    int caras_pacientes = 0, caras_impacientes = 0;
    printf("Comecou a thread atendente\n");
    
    sem_t* sem_atend =  sem_open("/sem_atend", O_RDWR);   
    sem_t* sem_block = sem_open("/sem_block", O_RDWR);
    sem_t* sem_fila = sem_open("/sem_fila", O_RDWR);
    
    int satisfeitos = 0, atendidos = 0;

    
    const char *filename = "/tmp/analista_pid.tmp";
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo analista_pid.tmp\n");
        pthread_exit(NULL);
    }
    pid_t pid_analista;
    fscanf(file, "%d", &pid_analista);
    //printf("Eu atendente peguei p_diddy do analista: %d\n", pid_analista);
    fclose(file);
    
    //se no comeco ainda tiver vazia ao chegar aqui
    //na nossa logica, so estara vazia novamente apos terminar programa
    
    while(filaVazia(fila)){
        usleep(10);
    }

    // enquanto a fila nao esta vazia
    while ((!filaVazia(fila)) || !(podeparar)){

        // se a fila estiver vazia, mas ainda estao sendo criados novos clientes         
        while(filaVazia(fila) && !(podeparar)){
            usleep(10);
        }

        if(filaVazia(fila) && podeparar) break;

        //usleep(100);
        // acorda o cliente
        int pid_cliente, hora_chegada, prioridade;
        if(sem_fila != SEM_FAILED) sem_wait(sem_fila);
        cliente c = remover(fila);
        if(sem_fila != SEM_FAILED) sem_post(sem_fila);
        pid_cliente = c.pid;
        hora_chegada = c.hora_chegada;
        prioridade = c.prioridade;

        //printf("Cliente %d vai ser atendido\n", pid_cliente);

        if (kill(pid_cliente, SIGCONT) == -1) {
            perror("Erro ao enviar o sinal para o cliente\n");
            exit(EXIT_FAILURE); 
        } 
        //printf("Cliente %d acordado\n", pid_cliente);
        
        int status;
        waitpid(pid_cliente, &status, 0);               

        //printf("Cliente %d terminou de ser atendido\n", pid_cliente);

        //clock_t tempo_atual = clock();
        struct timespec tempo_atual;
        clock_gettime(CLOCK_REALTIME, &tempo_atual);

        // checar as medidas de tempo para nao dar problema
        // double aux1 = (double)tempo_atual/CLOCKS_PER_SEC;
        /*
        aqui eh a conversao para microsegundos, ele pega o tempo em segundos e multiplica por 10e6
        e soma com o (tempo em nanosegundos multiplicado por 10e3) 
        */
        double tempo_decorrido = (tempo_atual.tv_sec - inicio_programa.tv_sec) *1e6 + 
                      (tempo_atual.tv_nsec - inicio_programa.tv_nsec) /1e3;

        //printf("\nO tempo atual eh  %f\n\n", tempo_decorrido);
        
        //double tempo_decorrido = ((double)tempo_atual - inicio_programa)/CLOCKS_PER_SEC; 
        
        // tempo decorrido esta medido em microsegundos

        //printf("\nO tempo decorrido foi %f\n\n", tempo_decorrido);
        int tempo_max;

        if (prioridade == 1){
            //caso der ruim, ver unidades de paciencia e hora_chegada
            tempo_max = paciencia/2;
            caras_impacientes ++;
        }
        else{
            tempo_max = paciencia;
            caras_pacientes++;
        }
        
        // espera sem_atend abrir
        if(sem_atend != SEM_FAILED) sem_wait(sem_atend);
        if(sem_atend != SEM_FAILED) sem_post(sem_atend);
        // espera sem_block abrir
        if(sem_block != SEM_FAILED) sem_wait(sem_block);
        // escreve pid do cliente em LNG
        FILE* LNG = fopen("LNG.txt", "a");
        fprintf(LNG, "%d\n", pid_cliente);
        fclose(LNG);
        // abre sem_block

        sem_post(sem_block);
        atendidos++;
        
        // calcula satisfacao

        int aux = (int)tempo_decorrido - (hora_chegada);
        if ((int)tempo_decorrido - (hora_chegada) <= tempo_max){
            //printf("O cliente foi satisfeito pois o tempo maximo era %d, e o tempo que ele ficou na fila foi %d\n", tempo_max,aux);
            //printf("O tempo decorrido foi %f e a hora de chegada foi %d\n", tempo_decorrido, hora_chegada);
            satisfeitos++;
        }
        /*
        else{
            printf("O cliente nao foi satisfeito pois o tempo maximo era %d e o tempo que ele ficou na fila foi %d\n", tempo_max, aux);
        }
        */
        
        // acorda analista (cada 10)
        if (atendidos % 10 == 0){
            if (kill(pid_analista, SIGCONT) == -1) {
                perror("Erro ao enviar o sinal para o analista\n");
                exit(EXIT_FAILURE); 
            }
        
        } 
    }
    if (kill(pid_analista, SIGCONT) == -1) {
        perror("Erro ao enviar o sinal para o analista\n");
        exit(EXIT_FAILURE); 
    }


    int status;

    //waitpid(pid_analista, &status, WUNTRACED);
    double taxa = ((double)satisfeitos/atendidos) * 100;
    printf("O numero de atendidos foi %d e o numero de satisfeitos foi %d\n", atendidos, satisfeitos);
    printf("Foram %d clientes impacientes, e %d clientes pacientes\n", caras_impacientes, caras_pacientes);
    printf("A taxa de satisfacao total foi igual a %f%%\n", taxa);
}

void* recepcao(void* args){

    RecepcaoArgs* rec_args = (RecepcaoArgs*) args;
    FilaCliente* fila = rec_args->fila;
    int num_clientes = rec_args->num_clientes;
    pid_t pid_atendente = rec_args->pid_atendente;
    struct timespec inicio_programa = rec_args->inicio_programa;
    

    printf("Comecou a thread recepcao\n");
    sem_t* sem_atend =  sem_open("/sem_atend", O_CREAT, 0644, 1);   
    sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);   
    sem_t* sem_fila = sem_open("/sem_fila", O_CREAT, 0644, 1);

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
            // processo filho vira um processo cliente
            if (execl("./cliente", "cliente", (char *)NULL) == -1) {
                perror("Erro ao executar cliente");
                exit(EXIT_FAILURE);
            }
            
        } else {
            // Processo pai
            
            while (filaCheia(fila)){
                usleep(1);
            } 
            
            if (podeparar == true){
                break;
            }
            
            /*gerar prioridade
            0: prioridade baixa
            1: prioridade alta
            */
            

            int status;
            waitpid(pid_cliente, &status, WUNTRACED);

            int prioridade_cliente = rand() % 2;
            //entra na fila
            int tempo_atendimento;
            
            FILE* demanda = fopen("demanda.txt", "r");
            if (demanda == NULL) {
                perror("Erro ao abrir o arquivo 'demanda.txt'\n");
                exit(EXIT_FAILURE);
            }
            

            if (fscanf(demanda, "%d", &tempo_atendimento) != 1) {
                fprintf(stderr, "Erro ao ler o tempo de atendimento do arquivo 'demanda.txt'\n");
                fclose(demanda);
                exit(EXIT_FAILURE);
            }
            fclose(demanda);

            //printf("Voltei para o atendimento, apos fechar o demanda!\n");
            //printf("Tempo de atendimento, que eu peguei do demanda.txt: %d\n", tempo_atendimento);

            struct timespec tempo_atual;
            clock_gettime(CLOCK_REALTIME, &tempo_atual);
            //printf("pid_cliente: %d \n",pid_cliente);
            double tempo_decorrido = (tempo_atual.tv_sec - inicio_programa.tv_sec)* 1e6 +
                                     (tempo_atual.tv_nsec - inicio_programa.tv_nsec) / 1e3;
            //printf("O tempo na recepcao eh %f \n", tempo_decorrido);
            //tempo_decorrido = tempo_decorrido * 1000000;
            cliente c;
            c.hora_chegada = (int)tempo_decorrido;
            c.pid = pid_cliente;
            c.prioridade = prioridade_cliente;
            c.tempo_atendimento = tempo_atendimento;

            //printf("Cliente %d vai ser adicionado na fila! \n", pid_cliente);
            if(sem_fila != SEM_FAILED) sem_wait(sem_fila);
            inserir(fila, c);
            if(sem_fila != SEM_FAILED) sem_post(sem_fila);
           
            //printf("Cliente %d adicionado na fila\n", pid_cliente);
        }
        }
        podeparar = true;
}

void* stop_0(void* arg)
{
	while(getchar() != 's');
    podeparar = true;
}

int main(int argc, char *argv[]){

    /*
    0 - nome do programa
    1 - numero de clientes
    2 - tolerancia do cliente (X)
    */  
    srand( (unsigned)time(NULL));
    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");
    sem_unlink("/sem_fila");

    FilaCliente fila;
    inicializarFila(&fila);

    int num_clientes = atoi(argv[1]);
    if (num_clientes<0) return 0;
    // a tolerancia esta medida em milisegundos
    int paciencia = atoi(argv[2]);
    if (num_clientes == 0) printf("O numero de clientes é indefinido\n"); 
    else printf("Numero de clientes %d\n", num_clientes);
    printf("A paciencia maxima dos clientes eh %d e a minima eh %d\n", paciencia, paciencia/2);
    //clock_t inicio_programa = clock();
    struct timespec inicio_programa;
    clock_gettime(CLOCK_REALTIME, &inicio_programa);



    pthread_t thread_atendente, thread_recepcao, thread_pare;

    AtendenteArgs atend_args;
    atend_args.fila = &fila;
    atend_args.inicio_programa = inicio_programa;
    atend_args.paciencia = paciencia;
    
    RecepcaoArgs recep_args;
    recep_args.fila = &fila;
    recep_args.inicio_programa = inicio_programa;
    recep_args.num_clientes = num_clientes;
    recep_args.pid_atendente = getpid();

    pthread_create(&thread_atendente, NULL, atendente, &atend_args);
    pthread_create(&thread_recepcao, NULL, recepcao, &recep_args);
    
    if (num_clientes == 0) pthread_create(&thread_pare, NULL, stop_0, NULL);
    
    pthread_join(thread_atendente, NULL);
    pthread_join(thread_recepcao,NULL);
    if (num_clientes == 0) pthread_join(thread_pare,NULL);

    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");
    sem_unlink("/sem_fila");

    //clock_t final_programa = clock();
    //double tempo_total = ((double)final_programa - inicio_programa)/CLOCKS_PER_SEC;
    //tempo_total = tempo_total * 1000; //para virar ms
    struct timespec final_programa;
    clock_gettime(CLOCK_REALTIME, &final_programa);
    double tempo_total = ((final_programa.tv_sec - inicio_programa.tv_sec) +
                         (final_programa.tv_nsec - inicio_programa.tv_nsec) / 1e9);

    printf("O tempo de execucao total foi igual a %f s \n", tempo_total*2);
    return 0;
}
