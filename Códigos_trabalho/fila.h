#ifndef FILA_H
#define FILA_H

#include <stdio.h>

typedef struct cliente {
    int pid;
    int hora_chegada;
    int prioridade;
    int tempo_atendimento;
} cliente;

typedef struct {
    cliente fila[101];
    int inicio;
    int fim;
} FilaCliente;


void inicializarFila(FilaCliente* f);
int filaVazia(FilaCliente* f);
int filaCheia(FilaCliente* f);
void inserir(FilaCliente* f, cliente c);
cliente remover(FilaCliente* f);

#endif