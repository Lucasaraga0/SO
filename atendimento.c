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

typedef struct cliente
{
    int pid;
    int hora_chegada;
    int prioridade;
    int tempo_atendimento;
} cliente;

//inicio da sessao fila
typedef struct {
    struct cliente fila[101];
    int inicio, fim;
} FilaCliente;

//inicio simboliza a exata posicao do primeiro elemento
//fim simboliza a exata posicao do proximo elemento a ser adicionado (algo como proximo do ultimo elemento da fila)
void inicializarFila(FilaCliente* f) {
    f->inicio = 0; // representa o inicio da fila
    f->fim = 0; // representa o proximo a ser iniciado
}

/* vazia quando o inicio alcanca o fim, que ocorre quando o ultimo elemento da fila eh coletado, fazendo com que incremente inicio
e fique na mesma posicao de fim, que eh onde o proximo elemento deve ser adicionado */
int filaVazia(FilaCliente* f) {
    return f->inicio == f->fim;
}

/* vazia quando o fim +  1 alcanca o inicio , que ocorre quando adicionamos um elemento de tal forma que sobre apenas um espaco vazio no vetor,
que acabamos cedendo para que essa checagem ocorra corretamente (checagem de o espaco para o proximo elemento a ser adicionado ser o mesmo
canto que o primeiro da fila esta) de tal forma que se diferencie da filaVazia*/
int filaCheia(FilaCliente* f) {
    
    return (f->fim + 1) % 100 == f->inicio;
}

//insercao normal, verifica se esta cheia. Caso nao esteja, insere no fim e incrementa fim
void inserir(FilaCliente* f, struct cliente c) {
    if (!(filaCheia(f))) {
        f->fila[f->fim] = c;
        f->fim = (f->fim + 1) % 100;
    }
}

//remocao normal, se nao tiver vazia, remove o primeiro da fila
cliente remover(FilaCliente* f) {
    if (!(filaVazia(f))) {
        struct cliente c = f->fila[f->inicio];
        f->inicio = (f->inicio + 1) % 100;
        return c;
    }
}

//fim da sessao fila


/*criar o vetor de clientes, que vai ser a nossa fila 
a ideia da fila eh sempre aumentar o numero infinitamente, e a gente vai pegando o resto dos valores e alocando
os clientes na posicao do resto respectivo, o val_inicial == val_final, então a fila está vazia se val_final - val_inicial == 100, 
entao a fila esta cheia
*/

void* atendente(){

    /*
    
    */

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

    while (num_clientes == 0 || i < num_clientes){
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
            //entra na fila
            FILE* demanda = fopen("demanda.txt", "r");
            fscanf(demanda, "%d", &tempo_atendimento);
            fclose(demanda);
            
            // Agora tenta deletar o arquivo
            if (remove("demanda.txt") == 0) {
                printf("Arquivo 'demanda.txt' deletado com sucesso.\n");
            } else {
                printf("Erro ao deletar o arquivo 'demanda.txt'.\n");
            }

            //printf("tempo de atendimento %d\n", tempo_atendimento);

            printf("pid_2: %d \n",pid_2);
            
            //inserir_na_fila(pid_2, hora_chegada,prioridade_cliente,tempo_atendimento);
           
            printf("Cliente %d adicionado na fila\n", i);
            
            usleep(100000);
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
    
    return 0;
}