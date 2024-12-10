#include "fila.h"

//inicio simboliza a exata posicao do primeiro elemento
//fim simboliza a exata posicao do proximo elemento a ser adicionado (algo como proximo do ultimo elemento da fila)
void inicializarFila(FilaCliente* f) {
    f->inicio = 0;
    f->fim = 0;
}

/* vazia quando o inicio alcanca o fim, que ocorre quando o ultimo elemento da fila eh coletado, fazendo com que incremente inicio
e fique na mesma posicao de fim, que eh onde o proximo elemento deve ser adicionado */
int filaVazia(FilaCliente* f) {
    return f->inicio == f->fim;
}

/* Cheia quando o fim +  1 alcanca o inicio , que ocorre quando adicionamos um elemento de tal forma que sobre apenas um espaco vazio no vetor,
que acabamos cedendo para que essa checagem ocorra corretamente (checagem de o espaco para o proximo elemento a ser adicionado ser o mesmo
canto que o primeiro da fila esta) de tal forma que se diferencie da filaVazia*/
int filaCheia(FilaCliente* f) {
    return (f->fim + 1) % 101 == f->inicio;
}

//insercao normal, verifica se esta cheia. Caso nao esteja, insere no fim e incrementa fim
void inserir(FilaCliente* f, cliente c) {
    if (!(filaCheia(f))) {
        f->fila[f->fim] = c;
        f->fim = (f->fim + 1) % 101;
    }
}

//remocao normal, se nao tiver vazia, remove o primeiro da fila
cliente remover(FilaCliente* f) {
    if (!(filaVazia(f))) {
        cliente c = f->fila[f->inicio];
        f->inicio = (f->inicio + 1) % 101;
        return c;
    } else {
        cliente vazio = {-1, -1, -1, -1};
        return vazio;
    }
}
