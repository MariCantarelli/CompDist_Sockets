#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>

#include "proto.h" // Inclui definições como BUFFER_SIZE e DEFAULT_PORT

// Variável global para controlar o loop principal do servidor
volatile sig_atomic_t server_running = 1;

// Protótipos de funções locais
void handle_client(int client_socket);
void sigint_handler(int sig);
void send_error(int client_socket, const char* code, const char* message);


void sigint_handler(int sig) {
    (void)sig; // Evita aviso de "unused parameter"
    printf("\nRecebido sinal de interrupção. Encerrando o servidor...\n");
    server_running = 0;
}

void send_error(int client_socket, const char* code, const char* message) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "ERR %s %s\n", code, message);
    write(client_socket, buffer, strlen(buffer));
}


void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Loop para ler as mensagens do cliente
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';

        // Remove nova linha, se houver, para processamento consistente
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        
        char *carriage_return = strchr(buffer, '\r');
        if (carriage_return) *carriage_return = '\0';

        printf("Recebido: '%s'\n", buffer);

        // Verifica se o cliente quer sair
        if (strcasecmp(buffer, "QUIT") == 0) {
            printf("Cliente solicitou desconexão.\n");
            break;
        }

        // Faz uma cópia do buffer para usar com strtok_r
        char buffer_copy[BUFFER_SIZE];
        strncpy(buffer_copy, buffer, sizeof(buffer_copy));
        buffer_copy[sizeof(buffer_copy) - 1] = '\0';

        char *saveptr;
        char *op = strtok_r(buffer_copy, " ", &saveptr);
        char *arg1_str = strtok_r(NULL, " ", &saveptr);
        char *arg2_str = strtok_r(NULL, " ", &saveptr);

        // Valida o formato da requisição
        if (!op || !arg1_str || !arg2_str) {
            send_error(client_socket, "EINV", "formato_requisicao_invalido");
            continue;
        }
        
        char *endptr1, *endptr2;
        double val1 = strtod(arg1_str, &endptr1);
        double val2 = strtod(arg2_str, &endptr2);

        // Valida se os operandos são números válidos
        if (*endptr1 != '\0' || *endptr2 != '\0') {
            send_error(client_socket, "EINV", "operando_nao_numerico");
            continue;
        }

        double result = 0.0;
        int error = 0;

        // Executa a operação
        if (strcasecmp(op, "ADD") == 0) {
            result = val1 + val2;
        } else if (strcasecmp(op, "SUB") == 0) {
            result = val1 - val2;
        } else if (strcasecmp(op, "MUL") == 0) {
            result = val1 * val2;
        } else if (strcasecmp(op, "DIV") == 0) {
            if (val2 == 0.0) {
                send_error(client_socket, "EZDV", "divisao_por_zero");
                error = 1;
            } else {
                result = val1 / val2;
            }
        } else {
            send_error(client_socket, "EINV", "operacao_desconhecida");
            error = 1;
        }
        
        // Se não houve erro, envia o resultado
        if (!error) {
            char response[BUFFER_SIZE];
            // Formata com 6 casas decimais para consistência
            snprintf(response, sizeof(response), "OK %.6f\n", result);
            write(client_socket, response, strlen(response));
        }
    }

    if (bytes_read < 0) {
        perror("Erro ao ler do socket do cliente");
    }
    
    printf("Fechando conexão do cliente.\n");
    close(client_socket);
}


int main(int argc, char *argv[]) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int port = DEFAULT_PORT;

    // Permite alterar a porta via argumento de linha de comando
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Porta inválida: %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    // Configura o manipulador de sinal para SIGINT
    signal(SIGINT, sigint_handler);

    // 1. Cria o socket do servidor
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Falha ao criar o socket");
        return EXIT_FAILURE;
    }
    printf("Socket do servidor criado com sucesso.\n");

    // Permite reutilizar o endereço para evitar erro "Address already in use"
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Falha ao configurar SO_REUSEADDR");
        close(server_socket);
        return EXIT_FAILURE;
    }

    // 2. Configura o endereço do servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Aceita conexões de qualquer IP
    server_addr.sin_port = htons(port);

    // 3. Associa o socket ao endereço (bind)
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Falha no bind");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("Bind na porta %d realizado com sucesso.\n", port);

    // 4. Coloca o servidor para escutar por conexões
    if (listen(server_socket, 5) < 0) {
        perror("Falha no listen");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("Servidor escutando na porta %d...\n", port);

    // 5. Loop principal para aceitar conexões
    while (server_running) {
        printf("Aguardando por uma nova conexão...\n");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (client_socket < 0) {
            // Se accept foi interrompido por um sinal (como SIGINT), não é um erro fatal
            if (errno == EINTR) {
                continue;
            }
            perror("Falha no accept");
            continue;
        }

        // Obtém o IP do cliente para log
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("Conexão aceita de %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        // Lida com o cliente
        handle_client(client_socket);
    }
    
    // Fecha o socket de escuta ao sair do loop
    printf("Fechando o socket do servidor.\n");
    close(server_socket);

    return EXIT_SUCCESS;
}

