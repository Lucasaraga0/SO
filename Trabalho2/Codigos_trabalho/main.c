#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "funcoes.h"


int parser(const char *str){
    char copiaStr[100];
    strcpy(copiaStr, str);

    char *primeiraPalavra = strtok(copiaStr, " ");
    char *segundaPalavra = strtok(NULL, " ");
    char *terceiraPalavra = strtok(NULL, " ");
    char *quartaPalavra = strtok(NULL, " ");

    if (strcmp(primeiraPalavra, "criar") == 0) {
        // converter terceira palavra para int
        int tamanho = atoi(terceiraPalavra);

        criar_arquivo(segundaPalavra, tamanho);
    }
    else if (strcmp(primeiraPalavra, "montar") == 0){
        inicializar_disco("Sist");
    }
    else if (strcmp(primeiraPalavra, "apagar") == 0) {
        
        apagar_arquivo(segundaPalavra);
    }

    else if (strcmp(primeiraPalavra, "listar") == 0){
        listar();
    }

    else if (strcmp(primeiraPalavra, "ordenar") == 0){
        //printf("ordenar\n");
        //printf("%s \n", segundaPalavra);
        ordenar_arquivos(segundaPalavra);
    }

    else if (strcmp(primeiraPalavra, "ler") == 0){
        int inicio = atoi(terceiraPalavra);
        int fim = atoi(quartaPalavra);
        ler_arquivo(segundaPalavra, inicio, fim);
    }
    
    else if (strcmp(primeiraPalavra, "concatenar") == 0){
        concatenar_arquivos(segundaPalavra, terceiraPalavra);
    }

    else if (strcmp(primeiraPalavra, "sair") == 0){
        return -1;
        //sair do programa
    }
    return 1;
}

//criar exit depois

int main(){
    inicializar_disco();
    //criarDisco();
    char str[100]; // Buffer para armazenar a string digitada pelo usuÃ¡rio
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