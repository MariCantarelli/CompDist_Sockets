Aluna: Marina Cantarelli Barroca
RA: 10740412

Calculadora Cliente-Servidor com Sockets em C
Este projeto implementa uma aplicação cliente-servidor simples em C utilizando sockets TCP (IPv4). O cliente envia operações matemáticas básicas para o servidor, que as processa e retorna o resultado.

Estrutura do Projeto
.
├── include
│   └── proto.h      # Cabeçalho com definições do protocolo e constantes
├── src
│   ├── client.c     # Código-fonte do cliente
│   └── server.c     # Código-fonte do servidor
├── Makefile         # Arquivo para automação da compilação
└── README.md        # Este arquivo

Funcionalidades
Servidor:

Aceita conexões TCP em uma porta configurável (padrão: 5050).

Processa uma requisição por vez (servidor iterativo).

Executa as quatro operações matemáticas básicas: ADD, SUB, MUL, DIV.

Realiza tratamento de erros para entradas inválidas e divisão por zero.

Encerra a conexão quando o cliente envia o comando QUIT.

Possui logs simples para registrar conexões, requisições e erros.

Encerra de forma limpa ao receber o sinal SIGINT (Ctrl+C).

Cliente:

Conecta a um servidor especificando endereço IP e porta.

Lê operações matemáticas do terminal (uma por linha).

Envia as operações ao servidor e exibe a resposta recebida.

Protocolo de Comunicação (Texto Simples)
O protocolo é baseado em texto e opera linha a linha.

Formato da Requisição (Cliente -> Servidor)
As requisições devem seguir o formato de prefixo: OP A B\n

OP: A operação a ser realizada. Valores possíveis: ADD, SUB, MUL, DIV.

A, B: Operandos numéricos (ponto flutuante), separados por espaço.

\n: Caractere de nova linha, que finaliza a mensagem.

Formato da Resposta (Servidor -> Cliente)
Sucesso: OK R\n

R: O resultado da operação, formatado como um número de ponto flutuante.

Erro: ERR <COD> <mensagem>\n

COD: Código de erro.

EINV: Entrada inválida (formato incorreto, operação desconhecida).

EZDV: Divisão por zero.

ESRV: Erro interno do servidor.

mensagem: Descrição breve do erro.

Comando Especial
QUIT\n: O cliente envia este comando para encerrar a conexão de forma limpa.

Como Compilar
Para compilar o projeto, basta executar o comando make na raiz do diretório.

make

O Makefile gerará dois executáveis: server e client.

Outros alvos disponíveis:

make server: Compila apenas o servidor.

make client: Compila apenas o cliente.

make clean: Remove os arquivos compilados (executáveis e objetos).

Como Executar
1. Iniciar o Servidor
Abra um terminal e execute o servidor. Você pode especificar uma porta ou usar a padrão (5050).

# Usando a porta padrão 5050
./server

# Especificando a porta 6000
./server 6000

O servidor ficará aguardando por conexões.

2. Iniciar o Cliente
Abra outro terminal para executar o cliente. Você precisa fornecer o endereço IP e a porta do servidor.

# Conectar ao servidor local (localhost) na porta 5050
./client 127.0.0.1 5050

Após a conexão, o cliente estará pronto para receber comandos.

Exemplos de Uso no Cliente
ADD 10 5
OK 15.000000

SUB 3.5 10
OK -6.500000

MUL -2.5 4
OK -10.000000

DIV 10 0
ERR EZDV divisao_por_zero

DIV 10 4
OK 2.500000

OPERACAO_INVALIDA 1 2
ERR EINV formato_requisicao_invalido

QUIT

Ao digitar QUIT, o cliente encerrará a conexão e o programa. O servidor estará pronto para aceitar uma nova conexão.

Decisões de Projeto e Limitações
Servidor Iterativo: O servidor implementado é iterativo, ou seja, atende um cliente por vez. Uma nova conexão só é aceita após o cliente atual se desconectar. Isso simplifica o código, mas não é ideal para ambientes com múltiplos usuários simultâneos. Uma melhoria seria usar fork() ou threads para tratar clientes concorrentes.

Parsing Robusto: A análise das mensagens do cliente é feita com strtok_r e strtod para garantir segurança e robustez. strtok_r é a versão reentrante de strtok, e strtod permite verificar se a conversão de string para double foi bem-sucedida.

Buffer Fixo: A comunicação utiliza um buffer de tamanho fixo (BUFFER_SIZE em proto.h). Mensagens maiores que o buffer serão truncadas. Para esta aplicação, o tamanho definido é mais do que suficiente.

Formato de Saída: O resultado é sempre formatado com 6 casas decimais (%.6f) para garantir consistência, independentemente do locale do sistema.