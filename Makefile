# Makefile para a Calculadora Cliente-Servidor

# Compilador
CC = gcc

# Flags de compilação
# -Iinclude: Adiciona o diretório 'include' à busca por arquivos de cabeçalho
# -Wall: Habilita a maioria dos avisos do compilador
# -Wextra: Habilita avisos adicionais
# -g: Gera informações de debug
CFLAGS = -Iinclude -Wall -Wextra -g

# Diretórios
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = .

# Arquivos fonte, objetos e executáveis
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
SERVER_OBJ = $(OBJDIR)/server.o
CLIENT_OBJ = $(OBJDIR)/client.o
SERVER_EXE = $(BINDIR)/server
CLIENT_EXE = $(BINDIR)/client

# Alvo padrão: compila tudo
all: $(SERVER_EXE) $(CLIENT_EXE)

# Alvo para compilar o servidor
$(SERVER_EXE): $(SERVER_OBJ)
	@echo "Ligando o executável do servidor..."
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Servidor '$(SERVER_EXE)' compilado com sucesso."

# Alvo para compilar o cliente
$(CLIENT_EXE): $(CLIENT_OBJ)
	@echo "Ligando o executável do cliente..."
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Cliente '$(CLIENT_EXE)' compilado com sucesso."

# Regra para criar arquivos objeto a partir dos fontes .c
# $@: nome do alvo (ex: obj/server.o)
# $<: nome da primeira dependência (ex: src/server.c)
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR) # Cria o diretório de objetos se não existir
	@echo "Compilando $<..."
	$(CC) $(CFLAGS) -c -o $@ $<

# Alvo 'server' (atalho para o executável do servidor)
server: $(SERVER_EXE)

# Alvo 'client' (atalho para o executável do cliente)
client: $(CLIENT_EXE)

# Alvo para limpar os arquivos compilados
clean:
	@echo "Limpando arquivos compilados..."
	rm -rf $(OBJDIR) $(SERVER_EXE) $(CLIENT_EXE)
	@echo "Limpeza concluída."

# Declara alvos que não são nomes de arquivos
.PHONY: all server client clean
```
