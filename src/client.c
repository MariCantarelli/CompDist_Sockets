#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "proto.h" // Inclui definições como BUFFER_SIZE e DEFAULT_PORT

void print_usage(const char* prog_name) {
    fprintf(stderr, "Uso: %s <ip_servidor> <porta>\n", prog_name);
}

int main(int argc, char *argv[]) {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Valida os argumentos de linha de comando
    if (argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    if (server_port <= 0 || server_port > 65535) {
        fprintf(stderr, "Porta inválida: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    // 1. Cria o socket do cliente
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Falha ao criar o socket");
        return EXIT_FAILURE;
    }

    // 2. Configura o endereço do servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    // Converte o endereço IP de string para formato de rede
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Endereço IP inválido ou não suportado");
        close(client_socket);
        return EXIT_FAILURE;
    }

    // 3. Conecta ao servidor
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Falha ao conectar ao servidor");
        close(client_socket);
        return EXIT_FAILURE;
    }

    printf("Conectado ao servidor %s:%d\n", server_ip, server_port);
    printf("Digite uma operação (ex: ADD 10 5) ou 'QUIT' para sair.\n");

    // Loop para ler do stdin, enviar ao servidor e receber a resposta
    while (1) {
        printf("> ");
        fflush(stdout); // Garante que o prompt ">" seja exibido

        // Lê uma linha do usuário
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            // Ocorreu um erro ou EOF (Ctrl+D)
            break;
        }

        // Envia a mensagem para o servidor
        if (write(client_socket, buffer, strlen(buffer)) < 0) {
            perror("Falha ao enviar dados para o servidor");
            break;
        }
        
        // Remove a nova linha do buffer para a comparação com "QUIT"
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcasecmp(buffer, "QUIT") == 0) {
            printf("Encerrando a conexão.\n");
            break;
        }

        // Lê a resposta do servidor
        ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            // Usa fputs para evitar que a string de resposta seja interpretada (ex: com %)
            fputs(buffer, stdout);
        } else if (bytes_read == 0) {
            printf("Servidor fechou a conexão.\n");
            break;
        } else {
            perror("Falha ao ler dados do servidor");
            break;
        }
    }

    // 4. Fecha o socket
    close(client_socket);
    printf("Conexão fechada.\n");

    return EXIT_SUCCESS;
}

