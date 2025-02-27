#include "funcoes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> // Para uint32_t
#include <sys/mman.h>
#include <time.h>



SistemaArquivo sist;
FILE* disco;

#define HUGE_PAGE_SIZE (2 * 1024 * 1024) // 2 MB

// no inicio do arquivo sist estao os metadados dele, com isso eh possivel manter as informacoes do sistema de arquivos

void salvar_metadados() {
    // funcao p salvar os metadados no arquivo sist  
    fseek(disco, 0, SEEK_SET); 
    fwrite(&sist, sizeof(SistemaArquivo), 1, disco); 
}

void carregar_metadados() {
    // funcao p ler os metadados no inicio do arquivo sist
    fseek(disco, 0, SEEK_SET); 
    fread(&sist, sizeof(SistemaArquivo), 1, disco); 
}

void inicializar_disco() {
    // funcao para ser rodada no incio da main toda vez, para carregar ou criar o disco e o sistema de arquivos
    disco = fopen("Sist", "rb+"); // Tenta abrir o disco em modo leitura/escrita
    
    if (!disco) {
        // Se o disco não existe, cria um novo
        printf("Criando disco.\n");
        disco = fopen("Sist", "wb+");
        if (!disco) {
            perror("Erro ao criar o disco");
            return;
        }

        // Inicializa o sistema de arquivos
        sist.file_count = 0;
        // aq salva o espaco dos metadados
        sist.free_space = DISK_SIZE - sizeof(SistemaArquivo);
        salvar_metadados(); 

        // Preenche o restante do disco com zeros
        char zero = 0;
        for (long i = sizeof(SistemaArquivo); i < DISK_SIZE; i++) {
            fwrite(&zero, 1, 1, disco);
        }
    } else {
        carregar_metadados();
    }
    fechar_disco();
}

    // funcao p salvar os metadados e fechar o arquivo
void fechar_disco() {
    salvar_metadados(); // Salva os metadados antes de fechar o disco
    fclose(disco);
}

void criar_arquivo(const char* name, int num_numbers) {
    /*

    Cria um arquivo com nome "nome" (pode ser limitado o tamanho do nome) com uma lista aleatória de números 
    inteiros positivos de 32 bits. O argumento "tam" indica a quantidade de números. A lista pode ser guardada 
    em formato binário ou como string (lista de números legíveis separados por algum separador, como vírgula ou espaço).
    */
    
    disco = fopen("Sist", "rb+");
    carregar_metadados();

    if (sist.file_count >= MAX_FILES) {
        printf("Número máximo de arquivos atingido!\n");
        fechar_disco();
        return;
    }

    size_t file_size = num_numbers * sizeof(int);
    if (file_size > sist.free_space) {
        printf("Espaço insuficiente no disco!\n");
        fechar_disco();
        return;
    }

    // Adiciona o arquivo a tabela
    Arquivo new_file;
    strncpy(new_file.name, name, MAX_FILENAME);
    new_file.size = file_size;
    new_file.offset = DISK_SIZE - sist.free_space; // Posicao no disco
    sist.files[sist.file_count] = new_file;
    sist.file_count++;
    sist.free_space -= file_size;

    // Escreve numeros aleatorios no disco

    fseek(disco, new_file.offset, SEEK_SET);
    for (int i = 0; i < num_numbers; i++) {
        uint32_t num = rand(); 
        fwrite(&num, sizeof(uint32_t), 1, disco);
    }

    salvar_metadados(); 
    printf("Arquivo '%s' criado com %d números.\n", name, num_numbers);
    fechar_disco();
}

void apagar_arquivo(const char* name) {
    // Apaga o arquivo com o nome passado no argumento.

    disco = fopen("Sist", "rb+");
    carregar_metadados();
    
    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(sist.files[i].name, name) == 0) {
            // Libera o espaco no disco
            sist.free_space += sist.files[i].size;

            // Remove o arquivo da tabela
            for (int j = i; j < sist.file_count - 1; j++) {
                sist.files[j] = sist.files[j + 1];
            }
            sist.file_count--;

            fechar_disco();
            printf("Arquivo '%s' apagado.\n", name);
            return;
        }
    }
    
    printf("Arquivo '%s' nao encontrado.\n", name);
    fechar_disco();
}

void listar() {
    // Lista os arquivos no diretório. Deve mostrar, ao lado de cada arquivo, o seu tamanho em bytes. Ao final, deve mostrar também o espaço total
    //  do "disco" e o espaço disponível.
  
    disco = fopen("Sist", "rb+");
    carregar_metadados();
    printf("Arquivos no diretório:\n");
    for (int i = 0; i < sist.file_count; i++) {
        printf("- %s (%zu bytes)\n", sist.files[i].name, sist.files[i].size);
    }
    printf("Espaço total: %d bytes\n", DISK_SIZE);
    printf("Espaço disponível: %ld bytes\n", sist.free_space);
    fechar_disco();
}

void ler_arquivo(const char* name, int start, int end) {
    // Exibe a sublista de um arquivo com o nome passado com o argumento. O intervalo da lista é dado pelos argumentos inicio e fim.
    
    disco = fopen("Sist", "rb+");
    carregar_metadados();

    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(sist.files[i].name, name) == 0) {
            if (start < 0 || end > sist.files[i].size / sizeof(int)) {
                printf("Intervalo inválido!\n");
                fechar_disco();
                return;
            }
            // faz a busca em disco 
            fseek(disco, sist.files[i].offset + start * sizeof(int), SEEK_SET);
            for (int j = start; j <= end; j++) {
                int num;
                fread(&num, sizeof(int), 1, disco);
                printf("%d ", num);
            }
            printf("\n");
            fechar_disco();
            return;
        }
    }
    printf("Arquivo '%s' não encontrado.\n", name);
    fechar_disco();
}

void concatenar_arquivos(const char* nome1, const char* nome2) {
    /*
    Concatena dois arquivos com os nomes dados de argumento.
     O arquivo concatenado pode ter um novo nome predeterminado ou simplesmente pode assumir o nome do primeiro arquivo. 
     Os arquivos originais devem deixar de existir.
    */
    
    disco = fopen("Sist", "rb+");
    carregar_metadados();

    long int tamDados1 = 0, tamDados2 = 0;

    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(nome1, sist.files[i].name) == 0) {
            tamDados1 = sist.files[i].size / sizeof(int);
        } else if (strcmp(nome2, sist.files[i].name) == 0) {
            tamDados2 = sist.files[i].size / sizeof(int);
        }
        if (tamDados1 != 0 && tamDados2 != 0) {
            break;
        }
    }

    if (tamDados1 == 0 || tamDados2 == 0) {
        printf("Erro: Um ou ambos os arquivos não foram encontrados.\n");
        fechar_disco();
        return;
    }

    // Alocar memória dinamicamente para os dados
    int* dados1 = malloc(tamDados1 * sizeof(int));
    int* dados2 = malloc(tamDados2 * sizeof(int));

    if (dados1 == NULL || dados2 == NULL) {
        printf("Erro: Falha ao alocar memória.\n");
        free(dados1);
        free(dados2);
        fechar_disco();
        return;
    }

    // Ler os dados de nome1
    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(sist.files[i].name, nome1) == 0) {
            fseek(disco, sist.files[i].offset, SEEK_SET);
            fread(dados1, sizeof(int), tamDados1, disco);
            break;
        }
    }

    // Ler os dados de nome2
    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(sist.files[i].name, nome2) == 0) {
            fseek(disco, sist.files[i].offset, SEEK_SET);
            fread(dados2, sizeof(int), tamDados2, disco);
            break;
        }
    }

    fechar_disco();

    char novo_nome[MAX_FILENAME];
    snprintf(novo_nome, MAX_FILENAME, "%s+", nome1);
    criar_arquivo(novo_nome, tamDados1 + tamDados2);
    apagar_arquivo(nome1);
    apagar_arquivo(nome2);

    disco = fopen("Sist", "rb+");
    carregar_metadados();

    // escrever os dados no novo arquivo  
    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(sist.files[i].name, novo_nome) == 0) {
            fseek(disco, sist.files[i].offset, SEEK_SET);
            fwrite(dados1, sizeof(int), tamDados1, disco);
            fwrite(dados2, sizeof(int), tamDados2, disco); 
            break;
        }
    }

    fechar_disco();
    free(dados1);
    free(dados2);
    printf("Arquivos '%s' e '%s' concatenados com sucesso em '%s'.\n", nome1, nome2, novo_nome);
}


// Função de comparação para o qsort
int comparar(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}


void ordenar_arquivos_pequenos(const char* nome) {
    clock_t inicio = clock();
    disco = fopen("Sist", "rb+");
    carregar_metadados();

    long int tamDados = 0;
    long int offset = 0;

    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(nome, sist.files[i].name) == 0) {
            tamDados = sist.files[i].size / sizeof(int);
            offset = sist.files[i].offset;
            break;
        }
    }

    if (tamDados == 0) {
        printf("Erro: O arquivo '%s' não foi encontrado.\n", nome);
        fclose(disco);
        return;
    }

    void* huge_page = mmap(NULL, HUGE_PAGE_SIZE, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (huge_page == MAP_FAILED) {
        perror("Erro ao alocar Huge Page");
        fclose(disco);
        return;
    }

    // Calcular o número de páginas necessárias
    long int num_paginas = (tamDados * sizeof(int) / HUGE_PAGE_SIZE) + 1;

    // Processar cada página
    for (long int pagina = 0; pagina < num_paginas; pagina++) {
        // Calcular o tamanho da página atual
        long int tamanho_pagina = (pagina == num_paginas - 1)
                                  ? (tamDados * sizeof(int)) % HUGE_PAGE_SIZE
                                  : HUGE_PAGE_SIZE;
        tamanho_pagina /= sizeof(int); // Converter para número de inteiros

        // Ler a página do arquivo original
        fseek(disco, offset + pagina * HUGE_PAGE_SIZE, SEEK_SET);
        fread(huge_page, sizeof(int), tamanho_pagina, disco);

        // Ordenar a página
        qsort(huge_page, tamanho_pagina, sizeof(int), comparar);

        // Escrever a página ordenada de volta no arquivo original
        fseek(disco, offset + pagina * HUGE_PAGE_SIZE, SEEK_SET);
        fwrite(huge_page, sizeof(int), tamanho_pagina, disco);
    }

    // Liberar a Huge Page
    munmap(huge_page, HUGE_PAGE_SIZE);

    // Fechar o disco
    fclose(disco);

    clock_t fim = clock();
    double tempo_gasto = ((double)(fim - inicio)) / (CLOCKS_PER_SEC / 1000); // Tempo em ms
    printf("Arquivo '%s' ordenado com sucesso.\n", nome);
    printf("Tempo gasto: %.2f ms\n", tempo_gasto);
}

void ordenar_arquivos_grandes(const char* nome) {
    clock_t inicio = clock();
    FILE* disco = fopen("Sist", "rb+");
    carregar_metadados();

    long int tamDados = 0;
    long int offset = 0;

    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(nome, sist.files[i].name) == 0) {
            tamDados = sist.files[i].size / sizeof(int);
            offset = sist.files[i].offset;
            break;
        }
    }

    if (tamDados == 0) {
        printf("Erro: O arquivo '%s' não foi encontrado.\n", nome);
        fclose(disco);
        return;
    }

    // Alocar uma Huge Page de 2 MB
    void* huge_page = mmap(NULL, HUGE_PAGE_SIZE, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (huge_page == MAP_FAILED) {
        perror("Erro ao alocar Huge Page");
        fclose(disco);
        return;
    }

    // Calcular o número de páginas necessárias
    long int num_paginas = (tamDados * sizeof(int) + HUGE_PAGE_SIZE - 1) / HUGE_PAGE_SIZE;

    // Primeira fase: Ordenação em blocos
    for (long int pagina = 0; pagina < num_paginas; pagina++) {
        long int tamanho_pagina = (pagina == num_paginas - 1)
                                  ? (tamDados * sizeof(int)) % HUGE_PAGE_SIZE
                                  : HUGE_PAGE_SIZE;
        tamanho_pagina /= sizeof(int);

        fseek(disco, offset + pagina * HUGE_PAGE_SIZE, SEEK_SET);
        fread(huge_page, sizeof(int), tamanho_pagina, disco);

        qsort(huge_page, tamanho_pagina, sizeof(int), comparar);

        fseek(disco, offset + pagina * HUGE_PAGE_SIZE, SEEK_SET);
        fwrite(huge_page, sizeof(int), tamanho_pagina, disco);
    }

    // Segunda fase: Merge externo para ordenação global
    int* buffer = (int*)huge_page; // Reutilizando a Huge Page como buffer
    int* indices = calloc(num_paginas, sizeof(int)); // Índices para cada bloco
    int* valores = malloc(num_paginas * sizeof(int)); // Valores atuais dos blocos
    FILE* temp_disco = fopen("Sist", "rb+");

    for (long int i = 0; i < num_paginas; i++) {
        fseek(temp_disco, offset + i * HUGE_PAGE_SIZE, SEEK_SET);
        fread(&valores[i], sizeof(int), 1, temp_disco);
    }

    for (long int i = 0; i < tamDados; i++) {
        int menor = 2147483647, menor_idx = -1;
        for (long int j = 0; j < num_paginas; j++) {
            if (indices[j] * sizeof(int) < HUGE_PAGE_SIZE && valores[j] < menor) {
                menor = valores[j];
                menor_idx = j;
            }
        }

        buffer[i % (HUGE_PAGE_SIZE / sizeof(int))] = menor;
        indices[menor_idx]++;

        if (indices[menor_idx] * sizeof(int) < HUGE_PAGE_SIZE) {
            fread(&valores[menor_idx], sizeof(int), 1, temp_disco);
        }

        if (i % (HUGE_PAGE_SIZE / sizeof(int)) == 0 || i == tamDados - 1) {
            fseek(disco, offset + (i / (HUGE_PAGE_SIZE / sizeof(int))) * HUGE_PAGE_SIZE, SEEK_SET);
            fwrite(buffer, sizeof(int), (i % (HUGE_PAGE_SIZE / sizeof(int))) + 1, disco);
        }
    }

    // Limpeza
    fclose(temp_disco);
    fclose(disco);
    free(indices);
    free(valores);
    munmap(huge_page, HUGE_PAGE_SIZE);

    clock_t fim = clock();
    double tempo_gasto = ((double)(fim - inicio)) / (CLOCKS_PER_SEC / 1000);
    printf("Arquivo '%s' ordenado com sucesso.\n", nome);
    printf("Tempo gasto: %.2f ms\n", tempo_gasto);
}

void ordenar_arquivos(const char *nome){
    disco = fopen("Sist", "rb+");
    carregar_metadados();
    long int tamDados = 0;
    for (int i = 0; i < sist.file_count; i++) {
        if (strcmp(nome, sist.files[i].name) == 0) {
            tamDados = sist.files[i].size / sizeof(int);
            if (tamDados<= 2097152 ){
                fechar_disco();
                ordenar_arquivos_pequenos(nome);
                return;
            }
            else{
                fechar_disco();
                ordenar_arquivos_grandes(nome);
                return;
            }
        }
    }  
    printf("Arquivo nao encontrado\n");
    fechar_disco();
}
