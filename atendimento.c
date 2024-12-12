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
#include "fila.h"
#include <time.h>
#include<stdbool.h>

/*
Para fazer
+ corrigir medicao de tempo.
    - printar satisfifacao
    - printar o tempo de execucao total do programa (main?)
- tem um cliente a mais, quando aperta o S, ele nao vai para o LNG 
*/

bool podeparar = false;

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
    usleep(1000);

    AtendenteArgs* atend_args = (AtendenteArgs*) args;

    FilaCliente* fila = atend_args-> fila;
    int paciencia = atend_args -> paciencia;
    clock_t inicio_programa = atend_args->inicio_programa;

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
    printf("Eu atendente peguei p_diddy do analista: %d\n", pid_analista);
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

        printf("Cliente %d vai ser atendido\n", pid_cliente);

        if (kill(pid_cliente, SIGCONT) == -1) {
            perror("Erro ao enviar o sinal para o cliente\n");
            exit(EXIT_FAILURE); 
        } 
        printf("Cliente %d acordado\n", pid_cliente);
        
        int status;
        waitpid(pid_cliente, &status, 0);                

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
        printf("Cliente %d terminou de ser atendido\n", pid_cliente);
        atendidos++;
        
        // calcula satisfacao

        if ((int)tempo_decorrido - hora_chegada <= tempo_max){
            satisfeitos++;
        }

        // acorda analista (cada 10)
        if (atendidos % 10 == 0){
            if (kill(pid_analista, SIGCONT) == -1) {
                perror("Erro ao enviar o sinal para o analista\n");
                exit(EXIT_FAILURE); 
            }
            //waitpid(pid_analista, &status, WUNTRACED);
        } 
    }
    if (kill(pid_analista, SIGCONT) == -1) {
        perror("Erro ao enviar o sinal para o analista\n");
        exit(EXIT_FAILURE); 
    }

    printf("A fila esta vazia? %d\n", filaVazia(fila)); 
    printf("A\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\nA\n");
    
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
    sem_t* sem_fila = sem_open("/sem_fila", O_CREAT, 0644, 1);

    int hora_chegada;


    int i = 0;
    
    // colocar o negocio para receber a tecla s e parar a criacao de clientes quando for 0
    while ((num_clientes == 0) || (i < num_clientes)){



        //criar processos cliente
        
        pid_t pid_cliente = fork();


        i ++;

        if (pid_cliente < 0){

            printf("Não criou cliente\n");
            exit(EXIT_FAILURE);
        }

        else if(pid_cliente == 0){
           // sem_wait(sem_demanda);  // Bloqueia o acesso ao demanda.txt

            //int pid_cliente = getpid();
            // printf("pid do cliente %d: %d\n", i, pid_cliente);
            //printf("Cliente %d criado!\n", i);
            // pode ate dar um sem_wait no sem_demanda aqui, mas quando a gente vai abrir? 
            // as linhas abaixo do exec nunca sao executadas, ele precisaria abrir no cliente? 

            if (execl("./cliente", "cliente", (char *)NULL) == -1) {
                perror("Erro ao executar cliente");
                exit(EXIT_FAILURE);
            }

            //sem_post(sem_demanda); 
            
        } else {
            // Processo pai
            
            while (filaCheia(fila)){
                usleep(1);
            } 
            
            if (podeparar == true){
                break;
            }
            /*gerar prioridade
            0: prioridade alta
            1: prioridade baixa
            */
            
            //usleep(10000);
            int status;
            waitpid(pid_cliente, &status, WUNTRACED);

            int prioridade_cliente = rand() % 2;
            //entra na fila
            int tempo_atendimento;
            // ficar num laço esperando que o processo cliente durma
            
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
            printf("Voltei para o atendimento, apos fechar o demanda!\n");
            //printf("Tempo de atendimento, que eu peguei do demanda.txt: %d\n", tempo_atendimento);
            clock_t tempo_atual = clock();
            //printf("pid_cliente: %d \n",pid_cliente);
            double tempo_decorrido = (double)(tempo_atual - inicio_programa) / CLOCKS_PER_SEC;

            cliente c;
            c.hora_chegada = (int)tempo_decorrido;
            c.pid = pid_cliente;
            c.prioridade = prioridade_cliente;
            c.tempo_atendimento = tempo_atendimento;

            printf("Cliente %d vai ser adicionado na fila! \n", pid_cliente);
            if(sem_fila != SEM_FAILED) sem_wait(sem_fila);
            inserir(fila, c);
            if(sem_fila != SEM_FAILED) sem_post(sem_fila);
           
            printf("Cliente %d adicionado na fila\n", pid_cliente);

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
    
    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");
    sem_unlink("/sem_fila");

    FilaCliente fila;
    inicializarFila(&fila);

    int num_clientes = atoi(argv[1]);
    int tolerancia = atoi(argv[2]);

    clock_t inicio_programa = clock();

    printf("Numero de clientes %d\n", num_clientes);

    pthread_t thread_atendente, thread_recepcao, thread_pare;

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
    
    if (num_clientes == 0) pthread_create(&thread_pare, NULL, stop_0, NULL);
    
    pthread_join(thread_atendente, NULL);
    pthread_join(thread_recepcao,NULL);
    if (num_clientes == 0) pthread_join(thread_pare,NULL);

    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");
    sem_unlink("/sem_fila");
    clock_t final_programa = clock();

    return 0;
}