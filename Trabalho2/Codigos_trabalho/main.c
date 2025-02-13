#include <stdlib.h>
#include <stdio.h>
#include "funcoes.h"
#include <string.h>

int parser(const char *str){
    char copiaStr[100]; // Cria uma cópia da string para não modificar a original
    strcpy(copiaStr, str);

    char *primeiraPalavra = strtok(copiaStr, " ");
    char *segundaPalavra = strtok(NULL, " ");
    char *terceiraPalavra = strtok(NULL, " ");
    char *quartaPalavra = strtok(NULL, " ");

    if (strcmp(primeiraPalavra, "criar") == 0) {
        // converter terceira palavra para int
        int tamanho = atoi(terceiraPalavra);
        //printf("criar\n");
        //printf("%s \n", segundaPalavra);
        //printf("%s \n", terceiraPalavra);
        criarArquivo(segundaPalavra, tamanho);
    }
    else if (strcmp(primeiraPalavra, "apagar") == 0) {
        //printf("apagar\n");
        //printf("%s \n", segundaPalavra);
        
        apagarArquivo(segundaPalavra);
    }

    else if (strcmp(primeiraPalavra, "listar") == 0){
        //printf("listar\n");
        //listarDiretorio();
    }

    else if (strcmp(primeiraPalavra, "ordenar") == 0){
        //printf("ordenar\n");
        //printf("%s \n", segundaPalavra);
        //ordernarArquivo(segundaPalavra);
    }

    else if (strcmp(primeiraPalavra, "ler") == 0){
        int inicio = atoi(terceiraPalavra);
        int fim = atoi(quartaPalavra);
        lerArquivo(segundaPalavra, inicio, fim);
    }
    
    else if (strcmp(primeiraPalavra, "concatenar") == 0){
        //printf("concaternar\n");
        //printf("%s \n", segundaPalavra);
        //printf("%s \n", terceiraPalavra);
        concaternarArquivos(segundaPalavra, terceiraPalavra);
    }

    else if (strcmp(primeiraPalavra, "exit") == 0){
        return -1;
        //sair do programa
    }
    return 1;
}

//criar exit depois

int main(){
    char str[100]; // Buffer para armazenar a string digitada pelo usuário
    int p = 1;
    while (p == 1) { 
        printf("Digite um comando: ");
        if (fgets(str, sizeof(str), stdin) == NULL) {
            break; // Se der erro na leitura, sai do loop
        }

        str[strcspn(str, "\n")] = '\0'; // Remove o '\n' do final da string
        p = parser(str);
    }
    return 0;
}