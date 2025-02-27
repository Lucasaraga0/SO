#ifndef SISTEMA_ARQUIVOS_H
#define SISTEMA_ARQUIVOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Constantes
#define DISK_SIZE 1073741824 // 1 GB
#define MAX_FILENAME 32
#define MAX_FILES 1000
#define HUGE_PAGE_SIZE (2 * 1024 * 1024) // 2 MB
#define TEMP_FILE_PREFIX "temp_"
#define UINT32_MAX 4294967295 // Valor máximo de uint32_t

// Estruturas
typedef struct {
    char name[MAX_FILENAME]; // Nome do arquivo
    size_t size;             // Tamanho do arquivo em bytes
    long offset;             // Posição do arquivo no "disco"
} Arquivo;

typedef struct {
    Arquivo files[MAX_FILES]; // Tabela de arquivos
    int file_count;             // Número de arquivos
    long free_space;            // Espaço livre no "disco"
} SistemaArquivo;

// Variáveis globais
extern SistemaArquivo sist;
extern FILE* disco;

// Protótipos das funções
void inicializar_disco();
void fechar_disco(); 
void criar_arquivo(const char* name, int num_numbers); 
void apagar_arquivo(const char* name); 
void listar(); 
void ler_arquivo(const char* name, int start, int end); 
void concatenar_arquivos(const char* nome1, const char* nome2); 
void ordenar_arquivos(const char* nome);

#endif // SISTEMA_ARQUIVOS_H