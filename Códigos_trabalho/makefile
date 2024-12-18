# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -Wextra -g

# Executáveis
CLIENTE = cliente
ANALISTA = analista
ATENDIMENTO = atendimento

# Objetos (se necessário dividir em múltiplos arquivos no futuro)
OBJS_ATENDIMENTO = fila.o atendimento.o

# Regras principais
all: $(CLIENTE) $(ANALISTA) $(ATENDIMENTO)

$(CLIENTE): cliente.c
	$(CC) $(CFLAGS) cliente.c -o $(CLIENTE)

$(ANALISTA): analista.c
	$(CC) $(CFLAGS) analista.c -o $(ANALISTA)

$(ATENDIMENTO): fila.c atendimento.c
	$(CC) $(CFLAGS) fila.c atendimento.c -o $(ATENDIMENTO)

# Limpeza
clean:
	rm -f $(CLIENTE) $(ANALISTA) $(ATENDIMENTO) *.o

# Recompilar tudo
rebuild: clean all
