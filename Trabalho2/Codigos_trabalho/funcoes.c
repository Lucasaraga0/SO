#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "funcoes.h"

#define TAMANHO_DISCO_MB 1024
#define TAMANHO_BLOCO 4096
#define TOTAL_BLOCOS (TAMANHO_DISCO_MB * 1024 * 1024 / TAMANHO_BLOCO)
#define CAMINHO_DISCO "Sist.img"
#define MAX_ARQUIVOS 128

typedef struct {
   uint32_t total_blocos;
   uint32_t tamanho_bloco;
   uint8_t bitmap[TOTAL_BLOCOS / 8];  // Bitmap de blocos livres
   uint32_t tabela_alocacao[TOTAL_BLOCOS];  // Tabela de encadeamento de blocos
} Superbloco;


typedef struct {
   char nome[32];
   uint32_t bloco_inicial;
   uint32_t tamanho;  // Tamanho real do arquivo (em bytes)
   uint8_t usado;
} Diretorio;

Diretorio diretorios[MAX_ARQUIVOS];

void inicializarSistemaArquivos() {
    FILE *disco = fopen(CAMINHO_DISCO, "rb+");
    if (!disco) {
        perror("Erro ao abrir o disco virtual");
        exit(1);
    }

    Superbloco sb;
    memset(&sb, 0, sizeof(Superbloco));
    sb.total_blocos = TOTAL_BLOCOS;
    sb.tamanho_bloco = TAMANHO_BLOCO;
    
    // Marcar o primeiro bloco como ocupado para o superbloco
    sb.bitmap[0] = 0x01;
    
    // Inicializar a Tabela de Alocação de Blocos (TAB)
    for (int i = 0; i < TOTAL_BLOCOS; i++) {
        sb.tabela_alocacao[i] = 0xFFFFFFFF;  // Nenhum bloco encadeado no início
    }

    // Inicializar estrutura de diretórios
    memset(diretorios, 0, sizeof(diretorios));
    strcpy(diretorios[0].nome, "/");
    diretorios[0].bloco_inicial = 1;
    diretorios[0].usado = 1;

    // Escrever o superbloco e diretórios no disco
    fseek(disco, 0, SEEK_SET);
    fwrite(&sb, sizeof(Superbloco), 1, disco);
    fwrite(diretorios, sizeof(diretorios), 1, disco);
    fflush(disco);
    fclose(disco);

    printf("Sistema de arquivos inicializado em %s\n", CAMINHO_DISCO);
}

void criarDisco() {
   char comando[512];

   // Criar o arquivo do disco virtual
   snprintf(comando, sizeof(comando), "dd if=/dev/zero of=%s bs=1M count=%d", CAMINHO_DISCO, TAMANHO_DISCO_MB);
   system(comando);

   // Ajustar permissões para garantir que o usuário atual tenha permissão de escrita
   snprintf(comando, sizeof(comando), "sudo chown $USER:$USER %s", CAMINHO_DISCO);
   system(comando);

   snprintf(comando, sizeof(comando), "chmod 666 %s", CAMINHO_DISCO);  // Permitir leitura e escrita para o usuário atual
   system(comando);

   // Inicializar sistema de arquivos próprio
   inicializarSistemaArquivos();
   
   printf("Disco criado e sistema de arquivos inicializado.\n");
}

void excluirDisco(){
   char comando[512];
   snprintf(comando, sizeof(comando), "rm -f %s", CAMINHO_DISCO);
   system(comando);
   printf("Disco desmontado e excluído.\n");

}


void criarArquivo(char nome[20], int tam){
   FILE *disco = fopen(CAMINHO_DISCO, "rb+");
   if (!disco){
      perror("Erro ao abrir o disco virtual\n");
      return;
   }

   Superbloco sb;
   fseek(disco, 0, SEEK_SET);
   fread(&sb, sizeof(Superbloco), 1, disco);

   //checar se ja tem um cara com esse nome no diretorio
   for (int i = 0; i < MAX_ARQUIVOS; i++) {
      if (diretorios[i].usado && strcmp(diretorios[i].nome, nome) == 0) {
          printf("Erro: Já existe um arquivo com o nome '%s'.\n", nome);
          fclose(disco);
          return;
      }
  }  

   // cada bloco tem 4KB, entao p saber quanto um bloco ocupa precisa calcular antes
   int blocos_necessarios = (tam*sizeof(uint32_t) + TAMANHO_BLOCO - 1)/TAMANHO_BLOCO;

   // encontrar onde tem bloco livre 
   int blocos_arquivo[blocos_necessarios];
   int blocos_encontrados = 0;
   for (int i =0; i <TOTAL_BLOCOS && blocos_encontrados < blocos_necessarios; i++){
      if (!(sb.bitmap[i/8]  & (1 <<(i%8)))){
         blocos_arquivo[blocos_encontrados++] = i;
         sb.bitmap[i/8] |= (1<< (i%8)); //marcar no bitmap que o bloco ta ocupado 
      }
   }

   if (blocos_encontrados<blocos_necessarios){
      printf("Sem espaco.\n");
      fclose(disco);
      return;
   }

   for(int i =0; i <blocos_necessarios-1;i++){
      sb.tabela_alocacao[blocos_arquivo[i]] = blocos_arquivo[i+1];
   }
   sb.tabela_alocacao[blocos_arquivo[blocos_necessarios-1]] = 0xFFFFFFFF;

   fseek(disco, sizeof(Superbloco), SEEK_SET);
   fread(diretorios, sizeof(diretorios), 1, disco);

   int indice_dir = -1;
   for (int i = 0; i < MAX_ARQUIVOS; i++) {
       if (!diretorios[i].usado) {  // Encontrar espaço livre
           indice_dir = i;
           break;
       }
   }

   if (indice_dir == -1) {
       printf("Erro: Número máximo de arquivos atingido.\n");
       fclose(disco);
       return;
   }
   
   strcpy(diretorios[indice_dir].nome, nome);
   diretorios[indice_dir].bloco_inicial = blocos_arquivo[0];
   diretorios[indice_dir].tamanho = tam*sizeof(uint32_t);
   diretorios[indice_dir].usado = 1;

   // Escrever números aleatórios nos blocos alocados
   srandom(time(NULL));
   int total_numeros = tam; // Escrever exatamente 'tam' números aleatórios
   for (int i = 0; i < blocos_necessarios; i++) {
      fseek(disco, blocos_arquivo[i] * TAMANHO_BLOCO, SEEK_SET);
      for (int j = 0; j < TAMANHO_BLOCO / sizeof(uint32_t) && total_numeros > 0; j++) {
         uint32_t num = (uint32_t)random();
         printf("%u, ", num);
         fwrite(&num, sizeof(uint32_t), 1, disco);
         total_numeros--;
         if (total_numeros == 0) {
            printf("\n");
            break;
         }
      }
      if (total_numeros == 0) {
         printf("\n");
         break;
      }
   }

   for (int i = 0; i < TOTAL_BLOCOS; i++) {
      if (sb.tabela_alocacao[i] != 0xFFFFFFFF) {
          printf("Bloco %d -> %d\n", i, sb.tabela_alocacao[i]);
      }
  }
  
   fseek(disco,0,SEEK_SET);
   fwrite(&sb, sizeof(Superbloco), 1, disco);
   fwrite(diretorios, sizeof(diretorios), 1, disco);

   fclose(disco);
   printf("Arquivo %s criado com sucesso!\n", nome);
}

void apagarArquivo(char nome[20]){
   FILE *disco = fopen(CAMINHO_DISCO, "rb+");
   if (!disco){
      perror("Erro ao abrir o disco virtual\n");
      return;
   }

   Superbloco sb;
   fseek(disco, 0, SEEK_SET);
   fread(&sb, sizeof(Superbloco), 1, disco);

   fseek(disco, sizeof(Superbloco), SEEK_SET);
   fread(diretorios, sizeof(diretorios), 1, disco);

   // encontrar o arquivo no diretorio 

   int indice_dir = -1;
   for (int i =0; i<MAX_ARQUIVOS; i++){
      if (diretorios[i].usado && strcmp(diretorios[i].nome, nome) == 0){
         indice_dir = i;
         break;
      }
   }

   if (indice_dir == -1){
      printf("Arquivo %s nao encontrado.\n", nome);
      fclose(disco);
      return;
   }
   
   uint32_t bloco_atual = diretorios[indice_dir].bloco_inicial;
   
   while (bloco_atual != 0xFFFFFFFF) {
      // Liberar o bloco no bitmap
      sb.bitmap[bloco_atual / 8] &= ~(1 << (bloco_atual % 8));

      // Passar para o próximo bloco
      uint32_t proximo_bloco = sb.tabela_alocacao[bloco_atual];
      bloco_atual = proximo_bloco;
  }
   
   diretorios[indice_dir].usado = 0;
   // Atualizar superbloco e diretórios no disco
   fseek(disco, 0, SEEK_SET);
   fwrite(&sb, sizeof(Superbloco), 1, disco);
   fwrite(diretorios, sizeof(diretorios), 1, disco);

   fclose(disco);
   printf("Arquivo %s apagado com sucesso!\n", nome);   
}

void listarDiretorio(){

   FILE *disco = fopen(CAMINHO_DISCO, "rb+");
   if (!disco){
      perror("Erro ao abrir o disco virtual\n");
      return;
   }

   Superbloco sb;
   fseek(disco, 0, SEEK_SET);
   fread(&sb, sizeof(Superbloco), 1, disco);

   fseek(disco, sizeof(Superbloco), SEEK_SET);
   fread(diretorios, sizeof(diretorios), 1, disco);

   long tamanhoTotal = 0;

   for (int i=0; i < MAX_ARQUIVOS; i ++){
      if (diretorios[i].usado && strcmp(diretorios[i].nome, "/") != 0){
         uint32_t bloco_atual = diretorios[i].bloco_inicial;
         long tamanhoArquivo = 0;
      
      while (bloco_atual != 0xFFFFFFFF){
         tamanhoArquivo += sb.tamanho_bloco;
         uint32_t proximoBloco = sb.tabela_alocacao[bloco_atual];
         bloco_atual = proximoBloco;
      }
      printf("%s: %ld bytes\n", diretorios[i].nome, tamanhoArquivo);
      tamanhoTotal += tamanhoArquivo;
      }
   }

   long restante = TAMANHO_DISCO_MB*1024 - tamanhoTotal;
   printf("Total ocupado: %ld bytes\n", tamanhoTotal);
   printf("Espaco livre de disco: %ld MB\n", restante/1024);  
   fclose(disco);
}

void ordernarArquivo(char nome[20]){
 /* Ordena a lista no arquivo com o nome passado no argumento. O algoritmo de ordenação a ser utilizado é livre, podendo inclusive ser
  utilizado alguma implementação de biblioteca existente. Ao terminar a ordenação, deve ser exibido o tempo gasto em ms.
 */
}

void lerArquivo(char nome[20], int inicio, int fim){
//Exibe a sublista de um arquivo com o nome passado com o argumento. O intervalo da lista é dado pelos argumentos inicio e fim.
   
FILE *disco = fopen(CAMINHO_DISCO, "rb+");
   if (!disco) {
      perror("Erro ao abrir o disco virtual");
      return;
   }

   Superbloco sb;
   fseek(disco, 0, SEEK_SET);
   fread(&sb, sizeof(Superbloco), 1, disco);

   fseek(disco, sizeof(Superbloco), SEEK_SET);
   fread(diretorios, sizeof(diretorios), 1, disco);
   
   int indice_dir = -1;
   for (int i = 0; i < MAX_ARQUIVOS; i++) {
      if (diretorios[i].usado && strcmp(diretorios[i].nome, nome) == 0) {
         indice_dir = i;
         break;
      }
   }

   if (indice_dir == -1) {
      printf("Arquivo não encontrado.\n");
      fclose(disco);
      return;
   }

   // Obter o número de blocos necessários
   int bloco_atual = diretorios[indice_dir].bloco_inicial;
   int tamanho_arquivo = diretorios[indice_dir].tamanho;
   int blocos_necessarios = (tamanho_arquivo + TAMANHO_BLOCO - 1) / TAMANHO_BLOCO;

   if (inicio < 0 || fim >= tamanho_arquivo / sizeof(uint32_t) || inicio > fim) {
      printf("Erro nos intervalos inseridos\n");
      fclose(disco);
      return;
   }

   uint32_t buffer[TAMANHO_BLOCO/sizeof(uint32_t)];
   int numValLidos = 0;
   int indInicial = inicio/(TAMANHO_BLOCO/sizeof(uint32_t));
   int intFinal = fim/(TAMANHO_BLOCO/sizeof(uint32_t));

   while (bloco_atual != 0xFFFFFFFF){
      printf("Lendo bloco %d...\n", bloco_atual);
      fseek(disco, bloco_atual*TAMANHO_BLOCO, SEEK_SET);
      fread(buffer, sizeof(uint32_t), TAMANHO_BLOCO/sizeof(uint32_t), disco);

      printf("Valores lidos do bloco %d: ", bloco_atual);
      for (int i = 0; i < TAMANHO_BLOCO/sizeof(uint32_t); i++){
         if (numValLidos >= inicio && numValLidos <= fim){
            printf("%u, ", buffer[i]);
         }         
         numValLidos++;
         if (numValLidos >fim){
            printf("\n");
            fclose(disco);
            return;
         }
      }

      bloco_atual = sb.tabela_alocacao[bloco_atual];
   }
   fclose(disco);

}

void concaternarArquivos(char nome1[20], char nome2[20]){
 /* Concatena dois arquivos com os nomes dados de argumento. O arquivo concatenado pode ter um novo nome predeterminado ou simplesmente 
 pode assumir o nome do primeiro arquivo. Os arquivos originais devem deixar de existir.
 */
   char caminho_completo1[256];
   snprintf(caminho_completo1, sizeof(caminho_completo1), "%s/%s", CAMINHO_DISCO, nome1);
   
   char caminho_completo2[256];
   snprintf(caminho_completo2, sizeof(caminho_completo2), "%s/%s", CAMINHO_DISCO, nome2);
   
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
