#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "fila.h"

// Estruturas para passar parâmetros às threads
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

void* atendente(void* args) {
    AtendenteArgs* atend_args = (AtendenteArgs*) args;
    FilaCliente* fila = atend_args->fila;
    int paciencia = atend_args->paciencia;
    clock_t inicio_programa = atend_args->inicio_programa;

    printf("Começou a thread atendente\n");

    sem_t* sem_atend = sem_open("/sem_atend", O_RDWR);
    sem_t* sem_block = sem_open("/sem_block", O_RDWR);

    int satisfeitos = 0, atendidos = 0, prioridade = 0;

    const char* filename = "/tmp/analista_pid.tmp";
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo analista_pid.tmp");
        pthread_exit(NULL);
    }
    pid_t pid_analista;
    fscanf(file, "%d", &pid_analista);
    fclose(file);

    usleep(100);

    while (!filaVazia(fila)) {
        cliente c = remover(fila);

        int pid_cliente = c.pid;
        int hora_chegada = c.hora_chegada;
        prioridade = c.prioridade;

        if (kill(pid_cliente, SIGCONT) == -1) {
            perror("Erro ao enviar sinal para o cliente");
            pthread_exit(NULL);
        }

        clock_t tempo_atual = clock();
        double tempo_decorrido = (double)(tempo_atual - inicio_programa) / CLOCKS_PER_SEC;

        int tempo_max = (prioridade == 1) ? paciencia / 2 + hora_chegada : paciencia + hora_chegada;

        usleep(5);

        if (sem_atend != SEM_FAILED) sem_wait(sem_atend);
        if (sem_block != SEM_FAILED) sem_wait(sem_block);

        FILE* LNG = fopen("LNG.txt", "w");
        if (LNG) {
            fprintf(LNG, "\n%d\n", pid_cliente);
            fclose(LNG);
        }

        sem_post(sem_block);
        atendidos++;

        if ((int)tempo_decorrido - hora_chegada <= tempo_max) {
            satisfeitos++;
        }

        if (atendidos % 10 == 0) {
            if (kill(pid_analista, SIGCONT) == -1) {
                perror("Erro ao enviar sinal para o analista");
                pthread_exit(NULL);
            }
        }
    }

    pthread_exit(NULL);
}

void* recepcao(void* args) {
    RecepcaoArgs* rec_args = (RecepcaoArgs*) args;
    FilaCliente* fila = rec_args->fila;
    int num_clientes = rec_args->num_clientes;
    pid_t pid_atendente = rec_args->pid_atendente;
    clock_t inicio_programa = rec_args->inicio_programa;

    printf("Começou a thread recepção\n");

    sem_t* sem_atend = sem_open("/sem_atend", O_CREAT, 0644, 1);
    sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644, 1);

    int i = 0;

    while (num_clientes == 0 || i < num_clientes) {
        pid_t pid_cliente = fork();
        i++;

        if (pid_cliente < 0) {
            perror("Erro ao criar cliente");
            pthread_exit(NULL);
        } else if (pid_cliente == 0) {
            if (execl("./cliente", "cliente", (char*)NULL) == -1) {
                perror("Erro ao executar cliente");
                exit(EXIT_FAILURE);
            }
        } else {
            while (filaCheia(fila)) {
                usleep(1);
            }

            int prioridade_cliente = rand() % 2;

            clock_t tempo_atual = clock();
            double tempo_decorrido = (double)(tempo_atual - inicio_programa) / CLOCKS_PER_SEC;

            cliente c = { .hora_chegada = (int)tempo_decorrido, .pid = pid_cliente, .prioridade = prioridade_cliente };
            inserir_na_fila(fila, c);

            printf("Cliente %d adicionado na fila\n", i);
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <numero_clientes> <tolerancia>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FilaCliente fila;
    inicializarFila(&fila);

    int num_clientes = atoi(argv[1]);
    int tolerancia = atoi(argv[2]);

    clock_t inicio_programa = clock();

    pthread_t thread_atendente, thread_recepcao;

    AtendenteArgs atend_args = { .fila = &fila, .paciencia = tolerancia, .inicio_programa = inicio_programa };
    RecepcaoArgs rec_args = { .fila = &fila, .num_clientes = num_clientes, .pid_atendente = getpid(), .inicio_programa = inicio_programa };

    pthread_create(&thread_atendente, NULL, atendente, &atend_args);
    pthread_create(&thread_recepcao, NULL, recepcao, &rec_args);

    pthread_join(thread_atendente, NULL);
    pthread_join(thread_recepcao, NULL);

    sem_unlink("/sem_atend");
    sem_unlink("/sem_block");

    return 0;
}
