#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

sem_t* sem_prog;
sem_t* sem_block;

void ler_e_imprimir(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char *lines[10];
    int count = 0;
    size_t len = 0;
    char *buffer = NULL;

    // Lê as primeiras 10 linhas ou menos
    while (count < 10 && getline(&buffer, &len, file) != -1) {
        lines[count++] = strdup(buffer); // Armazena as linhas lidas
    }

    // Armazena as linhas restantes em memória temporária
    FILE *tempFile = tmpfile();
    if (tempFile == NULL) {
        perror("Erro ao criar arquivo temporário");
        fclose(file);
        return;
    }

    while (getline(&buffer, &len, file) != -1) {
        fputs(buffer, tempFile); // Escreve as linhas restantes no arquivo temporário
    }

    fclose(file);

    // Imprime as linhas lidas no terminal
    printf("Linhas lidas e removidas:\n");
    for (int i = 0; i < count; i++) {
        printf("%s", lines[i]); // Printar a linha lida
        free(lines[i]);        // Liberar memória alocada
    }

    // Reescreve o arquivo original com as linhas restantes
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Erro ao reabrir o arquivo para escrita");
        fclose(tempFile);
        return;
    }

    rewind(tempFile); // Retorna ao início do arquivo temporário
    while (getline(&buffer, &len, tempFile) != -1) {
        fputs(buffer, file); // Copia as linhas restantes para o arquivo original
    }

    free(buffer);
    fclose(file);
    fclose(tempFile);
}


int main(){
    
    //gera arquivo com pid e dorme
    pid_t pid = getpid();
    const char *filename = "LNG.txt"; 
    // abrir os dois semaforos fodasticos 
    printf("abrindo os semaforos\n");
    
    /*
    pelo que eu lembro a gente vai usar o sem_prog para acordar o analista entao inicialmente ele vai ta esperando a mensagem para acordar,
    essa mensagem eh a liberacao do semaforo por parte da thread 1 do atendimento
    */

    sem_t* sem_prog = sem_open("/sem_prog", O_CREAT, 0644, 1); 
    /*
    sem_block eh usado para gerenciar o acesso ao conteudo do arquivo LNG.txt
    nota: no final das contas ele eh a mesma coisa que o anterior, talvez nao precise do sem_prog, ja que o sem_block so deve ser liberado para o analista
    quando o atendimento acorda-lo. basicamente os dois sempre so vao ter os bloqueios liberados na mesma hora, o que deixa o segundo meio irrelevante 
    
    */

    sem_t* sem_block = sem_open("/sem_block", O_CREAT, 0644,1); 

    if (sem_prog == SEM_FAILED || sem_block == SEM_FAILED){
        perror("Erro ao abrir semaforo\n");
        return 1;
    }
        
        // gambiarra para testar, basicamente para conseguir sempre o acesso dos semaforos 
        if (sem_trywait(sem_block) == -1 ) {
            perror("Semáforo sem_block em uso");
            sem_post(sem_block); // Libera imediatamente
        } else {
            printf("Semáforo disponível\n");
            sem_post(sem_block); // Libera imediatamente
        }

        if (sem_trywait(sem_prog) == -1 ) {
            perror("Semáforo sem_prog em uso");
            sem_post(sem_prog); // Libera imediatamente
        
        } else {
            printf("Semáforo disponível\n");
            sem_post(sem_prog); // Libera imediatamente
        }

    while (1)
    {
        printf("Analista esta dormindo\n");
        /*
        aqui da para sintetizar bem o que eu disse antes, os dois sempre vao aguardar juntos e liberar os semaforos juntos 
        no fim das contas eles nao tem funcoes diferentes, tendo em vista que sempre serao acordados no mesmo momento pelo atendimento
        */
        sem_wait(sem_prog);
        // bloquear arquivo LNG
        sem_wait(sem_block);
                
        // ler LNG e imprimir os 10 primeiros valores
        ler_e_imprimir(filename);
        
        // desbloquear LNG

        sem_post(sem_block);
        sem_post(sem_prog);
    }   

    sem_unlink("/sem_prog");
    sem_unlink("/sem_block");
    
    return 0;
}