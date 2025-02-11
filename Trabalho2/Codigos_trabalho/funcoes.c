#include "funcoes.h"
#include <stdlib.h>
#include <stdio.h>

// funcoes para executar as ações depois de receber o que deve ser feito pelo parser

void criarArquivo(char nome[20], int tam){
/*Cria um arquivo com nome "nome" (pode ser limitado o tamanho do nome) com uma lista aleatória de números inteiros positivos de 32 bits. 
O argumento "tam" indica a quantidade de números. A lista pode ser guardada em formato binário ou como string (lista de números legíveis
separados por algum separador, como vírgula ou espaço).

 Primeira palavra = função 
 if primPalavra == "criar":
    nome = segunda palavra
    tam = terceira palavra 
    criarArquivo(nome,tam)
 else if   
*/
}

void apagarArquivo(char nome[20]){
 // Apaga o arquivo com o nome passado no argumento.
 
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

}

void concaternarArquivos(char nome1[20], char nome2[20]){
 /* Concatena dois arquivos com os nomes dados de argumento. O arquivo concatenado pode ter um novo nome predeterminado ou simplesmente 
 pode assumir o nome do primeiro arquivo. Os arquivos originais devem deixar de existir.
 */
}