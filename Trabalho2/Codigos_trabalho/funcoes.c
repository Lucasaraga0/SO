#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include "funcoes.h"

// funcoes para executar as ações depois de receber o que deve ser feito pelo parser

//const char *caminho_virtual = "mnt/Sis";
const char *caminho_virtual = "teste";

void criarArquivo(char nome[20], int tam){
/*Cria um arquivo com nome "nome" (pode ser limitado o tamanho do nome) com uma lista aleatória de números inteiros positivos de 32 bits. 
O argumento "tam" indica a quantidade de números. A lista pode ser guardada em formato binário ou como string (lista de números legíveis
separados por algum separador, como vírgula ou espaço).
*/
   char caminho_completo[256];
   snprintf(caminho_completo, sizeof(caminho_completo), "%s/%s", caminho_virtual, nome);

   FILE *arquivoNovo = fopen(caminho_completo, "w");
   if (arquivoNovo == NULL) {
      perror("Erro ao criar o arquivo");
   }

   srandom(time(NULL));

   // gerar e escrever números aleatórios no arquivo
   for (int i = 0; i < tam; i++) {
      uint32_t num = ((uint32_t)random() << 16) | (random() & 0xFFFF);
      fprintf(arquivoNovo, "%u%s", num, (i < tam - 1) ? "," : "");  
   }
 
   fclose(arquivoNovo);
   
}

void apagarArquivo(char nome[20]){
 // Apaga o arquivo com o nome passado no argumento.
   char caminho_completo[256];
   snprintf(caminho_completo, sizeof(caminho_completo), "%s/%s", caminho_virtual, nome);
   if (remove(caminho_completo) == 0)
      printf("Arquivo removido\n");
   else
      printf("nao deu certo :(\n");
 
}

void listarDiretorio(){
 /* Lista os arquivos no diretório. Deve mostrar, ao lado de cada arquivo, o seu tamanho em bytes. Ao final, deve mostrar também o espaço 
 total do "disco" e o espaço disponível.
 */ 
}

void ordernarArquivo(char nome[20]){
 /* Ordena a lista no arquivo com o nome passado no argumento. O algoritmo de ordenação a ser utilizado é livre, podendo inclusive ser
  utilizado alguma implementação de biblioteca existente. Ao terminar a ordenação, deve ser exibido o tempo gasto em ms.
 */
}

void lerArquivo(char nome[20], int inicio, int fim){
//Exibe a sublista de um arquivo com o nome passado com o argumento. O intervalo da lista é dado pelos argumentos inicio e fim.
   char caminho_completo[256];
   snprintf(caminho_completo, sizeof(caminho_completo), "%s/%s", caminho_virtual, nome);
   
}

void concaternarArquivos(char nome1[20], char nome2[20]){
 /* Concatena dois arquivos com os nomes dados de argumento. O arquivo concatenado pode ter um novo nome predeterminado ou simplesmente 
 pode assumir o nome do primeiro arquivo. Os arquivos originais devem deixar de existir.
 */
}