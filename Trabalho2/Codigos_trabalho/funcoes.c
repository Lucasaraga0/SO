#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "funcoes.h"

// funcoes para executar as ações depois de receber o que deve ser feito pelo parser

#define TAMANHO_DISCO_MB 1024
#define CAMINHO_DISCO "Sist.img"
#define PONTO_MONTAGEM "/mnt/Sist"

void criarDisco() {
   // Criar o arquivo do disco virtual
   char comando[512];
   // criar o disco, tipo aquele da aula
   snprintf(comando, sizeof(comando), "dd if=/dev/zero of=%s bs=1M count=%d", CAMINHO_DISCO, TAMANHO_DISCO_MB);
   system(comando);
   //formatar o sistema de arquivoos
   snprintf(comando, sizeof(comando), "mkfs.ext2 %s", CAMINHO_DISCO);
   system(comando);
   // criar o diretorio para o sistema de arquivos
   snprintf(comando, sizeof(comando), "mkdir -p %s", PONTO_MONTAGEM);
   system(comando);
   // 
   snprintf(comando, sizeof(comando), "sudo mount -o loop %s %s", CAMINHO_DISCO, PONTO_MONTAGEM);
   system(comando);

   // ajustar permissões para permitir escrita pelo usuário atual, isso aqui tinha dado problema antes entao foi adicionado
   snprintf(comando, sizeof(comando), "sudo chown $USER:$USER %s", PONTO_MONTAGEM);
   system(comando);
   printf("Disco criado e montado em %s\n", PONTO_MONTAGEM);
}
   
void excluirDisco(){
   char comando[512];

   snprintf(comando, sizeof(comando), "sudo umount %s", PONTO_MONTAGEM);
   system(comando);

   snprintf(comando, sizeof(comando), "rm -f %s", CAMINHO_DISCO);
   system(comando);

   printf("Disco desmontado e excluído.\n");

}

void criarArquivo(char nome[20], int tam){
/*Cria um arquivo com nome "nome" (pode ser limitado o tamanho do nome) com uma lista aleatória de números inteiros positivos de 32 bits. 
O argumento "tam" indica a quantidade de números. A lista pode ser guardada em formato binário ou como string (lista de números legíveis
separados por algum separador, como vírgula ou espaço).
*/
   char caminho_completo[256];
   snprintf(caminho_completo, sizeof(caminho_completo), "%s/%s", PONTO_MONTAGEM, nome);

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
   printf("Arquivo %s criado!\n", nome);
}

void apagarArquivo(char nome[20]){
 // Apaga o arquivo com o nome passado no argumento.
   char caminho_completo[256];
   snprintf(caminho_completo, sizeof(caminho_completo), "%s/%s", PONTO_MONTAGEM, nome);
   if (remove(caminho_completo) == 0)
      printf("Arquivo removido\n");
   else
      printf("nao deu certo :(\n");
 
}

long pegarTamanho(const char *caminhoArquivo){
   struct stat st;
   if (stat(caminhoArquivo, &st) == 0){
      return st.st_size;
   }
   else{
      perror("Erro ao obter informações do arquivo");
      return -1;
    }
}

void listarDiretorio(){
 /* Lista os arquivos no diretório. Deve mostrar, ao lado de cada arquivo, o seu tamanho em bytes. Ao final, deve mostrar também o espaço 
 total do "disco" e o espaço disponível.
 */ 

   struct dirent *entrada;  
   DIR *diretorio = opendir(PONTO_MONTAGEM);
   long tamanhoTotal = 0;
   if (diretorio == NULL){
      perror("Error ao abrir diretorio");
      return;
   }
   while ((entrada = readdir(diretorio)) != NULL){
      if (entrada->d_name[0] == '.' || strcmp(entrada->d_name, "lost+found") == 0) {
         continue;
      }

      char caminhoCompleto[1024];

      snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s", PONTO_MONTAGEM, entrada->d_name);
      long tamanho = pegarTamanho(caminhoCompleto);
      if (tamanho != -1){
         printf("%s: %ld bytes\n", entrada->d_name, tamanho);
         tamanhoTotal += tamanho;
      }
   }
   closedir(diretorio);
   long restante = TAMANHO_DISCO_MB - tamanhoTotal;
   printf("Total ocupado: %ld\n", tamanhoTotal);
   printf("Espaco livre de disco: %ld\n", restante);  
}

void ordernarArquivo(char nome[20]){
 /* Ordena a lista no arquivo com o nome passado no argumento. O algoritmo de ordenação a ser utilizado é livre, podendo inclusive ser
  utilizado alguma implementação de biblioteca existente. Ao terminar a ordenação, deve ser exibido o tempo gasto em ms.
 */
}

void lerArquivo(char nome[20], int inicio, int fim){
//Exibe a sublista de um arquivo com o nome passado com o argumento. O intervalo da lista é dado pelos argumentos inicio e fim.
   char caminho_completo[256];
   snprintf(caminho_completo, sizeof(caminho_completo), "%s/%s", PONTO_MONTAGEM, nome);
   
   if (inicio <0 || fim < inicio){
      printf("Erro nos intervalos inseridos\n");
      return;
   }

   uint32_t *intervalo = NULL; // vetor para guardar os valores que estao dentro do intervalo pedido

   FILE *arquivo = fopen(caminho_completo, "r");
   if (arquivo == NULL) {
      perror("Erro ao ler o arquivo");
      return;
   }

   char saida[1024];
   int indiceSaida = 0;
   char caracter;
   int indiceAtual = 0;
   // int dentroIntervalo = 0;

   while ((caracter = fgetc(arquivo)) !=EOF){
      if (indiceAtual >= inicio && indiceAtual <= fim){
         saida[indiceSaida++] = caracter;
         //dentroIntervalo = 1;  
      }   
      
      if (caracter == ','){
         indiceAtual ++;
         if (indiceAtual >fim){
            break;
         }
      }
   }
   fclose(arquivo);
   if (indiceAtual < fim){
      printf("Fim maior que o tamanho do arquivo, inserir intervalo valido!\n");
      return;
   }
   saida[indiceSaida] = '\0';  // Finaliza a string corretamente
   printf("%s\n", saida);  // Exibe a saída somente no final
}

void concaternarArquivos(char nome1[20], char nome2[20]){
 /* Concatena dois arquivos com os nomes dados de argumento. O arquivo concatenado pode ter um novo nome predeterminado ou simplesmente 
 pode assumir o nome do primeiro arquivo. Os arquivos originais devem deixar de existir.
 */
   char caminho_completo1[256];
   snprintf(caminho_completo1, sizeof(caminho_completo1), "%s/%s", PONTO_MONTAGEM, nome1);
   
   char caminho_completo2[256];
   snprintf(caminho_completo2, sizeof(caminho_completo2), "%s/%s", PONTO_MONTAGEM, nome2);
   
   FILE *f1 = fopen(caminho_completo1, "r");
   FILE *f2 = fopen(caminho_completo2, "r");

   if (f1 == NULL || f2 == NULL) {
       printf("Erro ao abrir os arquivos!\n");
       return;
   }

   // Move o ponteiro do arquivo para o final pra saber o tamanho
   fseek(f1, 0, SEEK_END);
   long size1 = ftell(f1);
   rewind(f1);  

   fseek(f2, 0, SEEK_END);
   long size2 = ftell(f2);
   rewind(f2);

   // Aloca memoria suficiente pra armazenar tudo
   char *content1 = malloc(size1 + 1);
   char *content2 = malloc(size2 + 1);

   if (content1 == NULL || content2 == NULL) {
       printf("Erro ao alocar memoria!\n");
       fclose(f1);
       fclose(f2);
       return;
   }

   // Le o conteudo dos arquivos
   fread(content1, 1, size1, f1);
   fread(content2, 1, size2, f2);

   content1[size1] = '\0';  
   content2[size2] = '\0';  

   fclose(f1);
   fclose(f2);

   // Remove os dois arquivos antes de criar o novo
   remove(caminho_completo1);
   remove(caminho_completo2);

   // Cria um novo arquivo com o nome do primeiro
   f1 = fopen(caminho_completo1, "w");
   if (f1 == NULL) {
       printf("Erro ao criar o novo arquivo!\n");
       free(content1);
       free(content2);
       return;
   }

   // Escreve os conteudos concatenados
   fprintf(f1, "%s,%s", content1, content2);
   fclose(f1);

   printf("Novo arquivo '%s' criado com sucesso!\n", nome1);

   free(content1);
   free(content2);
}
