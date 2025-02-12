#include <stdlib.h>
#include <stdio.h>
#include "funcoes.h"
#include <string.h>

void parser(const char *str){
    char copiaStr[100]; // Cria uma cópia da string para não modificar a original
    strcpy(copiaStr, str);

    char *primeiraPalavra = strtok(copiaStr, " ");
    char *segundaPalavra = strtok(NULL, " ");
    char *terceiraPalavra = strtok(NULL, " ");
    char *quartaPalavra = strtok(NULL, " ");

    if (strcmp(primeiraPalavra, "criar") == 0) {
        // converter terceira palavra para int
        int tamanho = atoi(terceiraPalavra);
        printf("criar\n");
        printf("%s \n", segundaPalavra);
        printf("%s \n", terceiraPalavra);
        criarArquivo(segundaPalavra, tamanho);
        
        criarArquivo(segundaPalavra, tamanho);
    }
    else if (strcmp(primeiraPalavra, "apagar") == 0) {
        printf("apagar\n");
        printf("%s \n", segundaPalavra);
        
        apagarArquivo(segundaPalavra);
    }

    else if (strcmp(primeiraPalavra, "listar") == 0){
        printf("listar\n");
        //listarDiretorio();
    }

    else if (strcmp(primeiraPalavra, "ordenar") == 0){
        printf("ordenar\n");
        printf("%s \n", segundaPalavra);
        //ordernarArquivo(segundaPalavra);
    }

    else if (strcmp(primeiraPalavra, "ler") == 0){
        printf("ler\n");
        printf("%s \n", segundaPalavra);
        printf("%s \n", terceiraPalavra);
        printf("%s \n", quartaPalavra);
        //lerArquivo(segundaPalavra);
    }
    
    else if (strcmp(primeiraPalavra, "concatenar") == 0){
        printf("concaternar\n");
        printf("%s \n", segundaPalavra);
        printf("%s \n", terceiraPalavra);
        //ordernarArquivo(segundaPalavra);
    }

    else if (strcmp(primeiraPalavra, "exit") == 0){
        //sair do programa
    }
}

//criar exit depois

int main(){
    char str[100]; // Buffer para armazenar a string digitada pelo usuário

    printf("Digite uma string: ");
    scanf("%[^\n]", str); // Lê a string até encontrar uma nova linha

    // Chama a função para processar a string
    parser(str);

    return 0;
}