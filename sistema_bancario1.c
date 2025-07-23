/* TRABALHO FINAL DE PROGRAMACAO COMPUTACIONAL - 2025.1
 * EQUIPE 18: SIMULADOR DE SISTEMA BANCARIO SIMPLIFICADO
 *
 * ====MEMBROS====
 * Representante: Francisca Gessica Peres Bezerra - 579536
 * Membro 2: Neutino Vicenzo Tavares Sebastião - 584262
 * Membro 3: Matheus Pacheco Pinto - 579278
 * 
 */

// BIBLIOTECAS UTILIZADAS
// Inclusão das bibliotecas permitidas no trabalho
#include <stdio.h>  // Para funções de entrada e saída, como printf e scanf
#include <stdlib.h>  // Para alocação de memória (malloc, realloc, free) e outras utilidades
#include <string.h>  // Para manipulação de strings, como strcpy e strcmp
#include <locale.h>  // Para definir a localidade e permitir acentuação em português
#include <ctype.h>  // Para funções de caracteres, como toupper (converter para maiúsculo)

// Definimos o nome do arquivo que será usado para salvar e carregar os dados
#define NOME_ARQUIVO "banco_dados.bin"

// Definimos os tipos de transações possíveis usando uma enumeração
typedef enum {
    DEPOSITO,       
    SAQUE,          
    TRANSFERENCIA,  
    PIX             
} TipoTransacao;

// ESTRUTURAS
// Estrutura para  uma única transação financeira
typedef struct {
    TipoTransacao tipo; 
    double valor; // O valor monetário da transação
} Transacao;

// Estrutura para uma conta bancária
typedef struct {
    int numero;         
    int agencia;        
    double saldo;       
    Transacao* transacoes; // Ponteiro para um array dinâmico de transações (extrato)
    int num_transacoes; 
} Conta;

// Estrutura para um cliente do banco.
typedef struct {
    char nomeCompleto[50]; 
    char cpf[12]; // CPF com 11 dígitos + 1 para o caractere nulo ('\0')
    int idade;             
    Conta* contas; // Ponteiro para um array dinâmico de contas pertencentes ao cliente
    int num_contas;  
} Cliente;


// VARIÁVEIS GLOBAIS
// Ponteiro para o array dinâmico que armazenará todos os clientes do sistema
// Começa como NULL, indicando que o sistema está vazio
Cliente* g_clientes = NULL;

// Variável que controla o número total de clientes cadastrados
int g_num_clientes = 0;

//Declaramos antecipadamente todas as funções para que possam ser chamadas independente da ordem no arquivo
void limparBuffer();
void pausarSistema();
void salvarDados();
void carregarDados();
void liberarMemoria();
void cadastrarCliente();
void realizarTransacao();
void consultarConta();
Cliente* buscarClientePorCPF(const char* cpf);
Conta* buscarConta(Cliente* cliente, int numeroConta);
void adicionarTransacao(Conta* conta, TipoTransacao tipo, double valor);

// Declaramos a função main para o controle do fluxo principal e menu
int main() {
    // Define a localidade para "Portuguese" para exibir os acentos
    setlocale(LC_ALL, "Portuguese");

    // Tenta carregar os dados existentes do arquivo binário ao iniciar
    carregarDados();

    int opcao;
    // Loop principal do menu. Continua executando até que o usuário escolha a opção 0
    do {
        printf("\n===== SIMULADOR DE SISTEMA BANCÁRIO =====\n");
        printf("1. Cadastrar Novo Cliente\n");        //
        printf("2. Realizar Transação\n");            //
        printf("3. Consultar Saldo / Extrato\n");    //
        printf("0. Sair do Programa\n");
        printf("=========================================\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        limparBuffer(); // Limpa o buffer

        // Estrutura switch para direcionar o usuário para seu fluxo escolhido
        switch (opcao) {
            case 1:
                cadastrarCliente();
                break;
            case 2:
                realizarTransacao();
                break;
            case 3:
                consultarConta();
                break;
            case 0:
                printf("Saindo do programa. Salvando dados...\n");
                break;
            default:
                printf("Opção inválida! Tente novamente.\n");
        }

        // Pausa o sistema após cada operação (exceto ao sair) para que o usuário possa ler a saída
        if (opcao != 0) {
            pausarSistema();
        }

    } while (opcao != 0);

    // Antes de encerrar, salva todos os dados no arquivo e libera a memória alocada
    salvarDados();
    liberarMemoria();
    printf("Dados salvos com sucesso. Até logo!\n");

    return 0;
}

//FUNÇÕES UTILITÁRIAS
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Aguarda o usuário clicar 'enter' para continuar
void pausarSistema() {
    printf("\nPressione Enter para continuar...");
    limparBuffer();
}

// Para evitar vazamentos de memória (memory leaks), declaramos esta funçao 
void liberarMemoria() {
    int i, j;
    if (g_clientes != NULL) {
        //para cada cliente...
        for (i = 0; i < g_num_clientes; i++) {
            if (g_clientes[i].contas != NULL) {
                //para cada conta do cliente...
                for (j = 0; j < g_clientes[i].num_contas; j++) {
                    // libera o array de transações da conta
                    free(g_clientes[i].contas[j].transacoes);
                }
                // libera o array de contas do cliente
                free(g_clientes[i].contas);
            }
        }
        //libera o array principal de clientes
        free(g_clientes);
    }
}


// FUNÇÕES PARA SALVAR E CARREGAR DADOS NO ARQUIVO
//Salva todos os clientes, contas e transações em um arquivo binário chamado "banco_dados.bin"
void salvarDados() {
    // abre o arquivo no modo "wb" para escrita binária
    FILE* arquivo = fopen(NOME_ARQUIVO, "wb");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo para escrita");
        return;
    }

    // 1. Salva o número total de clientes 
    fwrite(&g_num_clientes, sizeof(int), 1, arquivo);

    int i, j;
    // 2. Itera sobre cada cliente para salvar seus dados
    for (i = 0; i < g_num_clientes; i++) {
        // Salva a estrutura principal do cliente (sem os ponteiros, que são salvos a seguir)
        fwrite(&g_clientes[i], sizeof(Cliente), 1, arquivo);
        
        // 3. Salva o array de contas do cliente
        if (g_clientes[i].num_contas > 0) {
            fwrite(g_clientes[i].contas, sizeof(Conta), g_clientes[i].num_contas, arquivo);
            
            // 4. Itera sobre cada conta para salvar seu extrato (array de transações)
            for (j = 0; j < g_clientes[i].num_contas; j++) {
                if (g_clientes[i].contas[j].num_transacoes > 0) {
                    fwrite(g_clientes[i].contas[j].transacoes, sizeof(Transacao), g_clientes[i].contas[j].num_transacoes, arquivo);
                }
            }
        }
    }
 
    fclose(arquivo);
}

//Esta função carrega os dados de "banco_dados.bin" para a memória 
void carregarDados() {
    // Abre o arquivo no modo "rb" para leitura binária
    FILE* arquivo = fopen(NOME_ARQUIVO, "rb");
    if (arquivo == NULL) {
        // Se o arquivo não existe (primeira execução), apenas informa e continua
        printf("Arquivo de dados nao encontrado. Iniciando com sistema vazio.\n");
        return;
    }

    // 1. Lê o número total de clientes
    fread(&g_num_clientes, sizeof(int), 1, arquivo);
    if (g_num_clientes == 0) {
        fclose(arquivo);
        return;
    }

    // 2. Aloca memória para o array de clientes
    g_clientes = (Cliente*)malloc(g_num_clientes * sizeof(Cliente));

    int i, j;
    // 3. Itera para ler os dados de cada cliente
    for (i = 0; i < g_num_clientes; i++) {
        fread(&g_clientes[i], sizeof(Cliente), 1, arquivo);
        
        // 4. Para cada cliente, aloca e lê o array de contas
        if (g_clientes[i].num_contas > 0) {
            g_clientes[i].contas = (Conta*)malloc(g_clientes[i].num_contas * sizeof(Conta));
            fread(g_clientes[i].contas, sizeof(Conta), g_clientes[i].num_contas, arquivo);

            // 5. Para cada conta, aloca e lê o array de transações
            for(j = 0; j < g_clientes[i].num_contas; j++) {
                if(g_clientes[i].contas[j].num_transacoes > 0) {
                    g_clientes[i].contas[j].transacoes = (Transacao*)malloc(g_clientes[i].contas[j].num_transacoes * sizeof(Transacao));
                    fread(g_clientes[i].contas[j].transacoes, sizeof(Transacao), g_clientes[i].contas[j].num_transacoes, arquivo);
                } else {
                    g_clientes[i].contas[j].transacoes = NULL; // Garante que contas sem transações tenham ponteiro NULL
                }
            }
        } else {
            g_clientes[i].contas = NULL; // Garante que clientes sem contas tenham ponteiro nulo
        }
    }
    fclose(arquivo);
    printf("Dados carregados com sucesso!\n");
}

// Declaramos esta função para cadastrar um novo cliente e sua primeira conta bancária
void cadastrarCliente() {
    char cpf_temp[12];
    printf("\n--- Cadastro de Novo Cliente ---\n");
    printf("Digite o CPF (apenas 11 numeros): ");
    fgets(cpf_temp, 12, stdin); 
    limparBuffer();

    //verificando se o CPF já existe 
    if (buscarClientePorCPF(cpf_temp) != NULL) {
        printf("Erro: CPF já cadastrado no sistema.\n");
        return;
    }
    
    // Aumenta o tamanho do array global de clientes usando realloc
    g_num_clientes++;
    g_clientes = (Cliente*)realloc(g_clientes, g_num_clientes * sizeof(Cliente));
    
    //Pega um ponteiro para a nova posição no array para preencher os dados
    Cliente* novoCliente = &g_clientes[g_num_clientes - 1];
    strcpy(novoCliente->cpf, cpf_temp);

    printf("Digite o nome completo (até 49 caracteres): ");
    fgets(novoCliente->nomeCompleto, 50, stdin);
    // Remove o '\n' do fgets 
    novoCliente->nomeCompleto[strcspn(novoCliente->nomeCompleto, "\n")] = 0;

    printf("Digite a idade: ");
    scanf("%d", &novoCliente->idade);
    limparBuffer();
    
    printf("\n--- Cadastro da Conta Bancária ---\n");
    // Aloca memória para a primeira conta do cliente
    novoCliente->num_contas = 1;
    novoCliente->contas = (Conta*)malloc(sizeof(Conta));
    
    printf("Digite o número da agência: ");
    scanf("%d", &novoCliente->contas[0].agencia);
    limparBuffer();
    
    printf("Digite o número da conta: ");
    scanf("%d", &novoCliente->contas[0].numero);
    limparBuffer();
    
    // Inicializa os dados da nova conta
    novoCliente->contas[0].saldo = 0.0;
    novoCliente->contas[0].num_transacoes = 0;
    novoCliente->contas[0].transacoes = NULL;
    
    printf("\nCliente '%s' cadastrado com sucesso!\n", novoCliente->nomeCompleto);
}


Cliente* buscarClientePorCPF(const char* cpf) {
    int i;
    for (i = 0; i < g_num_clientes; i++) {
        if (strcmp(g_clientes[i].cpf, cpf) == 0) {
            return &g_clientes[i]; // Retorna o endereço do cliente encontrado
        }
    }
    return NULL; 
    // Retorna um ponteiro para o cliente se encontrado ou nulo caso contrário
}

Conta* buscarConta(Cliente* cliente, int numeroConta) {
    int i;
    for (i = 0; i < cliente->num_contas; i++) {
        if (cliente->contas[i].numero == numeroConta) {
            return &cliente->contas[i]; // Retorna o endereço da conta encontrada.
        }
    }
    return NULL;
    // Retorna um ponteiro para a conta se encontrada, ou nulo caso contrário
}

// Adiciona um novo registro de transação ao extrato de uma conta
void adicionarTransacao(Conta* conta, TipoTransacao tipo, double valor) {
    //aumenta o tamanho do array de transações da conta
    conta->num_transacoes++;
    conta->transacoes = (Transacao*)realloc(conta->transacoes, conta->num_transacoes * sizeof(Transacao));
    
    //aponta para a nova posição e preenche com os dados da transação
    Transacao* novaTransacao = &conta->transacoes[conta->num_transacoes - 1];
    novaTransacao->tipo = tipo;
    novaTransacao->valor = valor;
}

// Função para realização de transações 
void realizarTransacao() {
    char cpf[12];
    int numConta;
    printf("\n--- Realizar Transação ---\n");
    printf("Digite o CPF do titular da conta: ");
    fgets(cpf, 12, stdin);
    limparBuffer();

    // Localiza o cliente e a conta onde a operação será realizada
    Cliente* cliente = buscarClientePorCPF(cpf);
    if (cliente == NULL) {
        printf("Erro: Cliente não encontrado.\n");
        return;
    }
    printf("Digite o número da conta: ");
    scanf("%d", &numConta);
    limparBuffer();
    Conta* conta = buscarConta(cliente, numConta);
    if (conta == NULL) {
        printf("Erro: Conta não encontrada para este cliente.\n");
        return;
    }

    char continuar = 'S';
    //loop que permite ao usuário fazer várias operações na mesma conta
    while (toupper(continuar) == 'S') {
        printf("\nConta de %s | Saldo Atual: R$ %.2f\n", cliente->nomeCompleto, conta->saldo);
        printf("Escolha a transação:\n1. Depósito\n2. Saque\n3. Transferência / PIX\n");
        printf("Escolha uma opção: ");
        int opcaoTransacao;
        scanf("%d", &opcaoTransacao);
        limparBuffer();

        switch (opcaoTransacao) {
            case 1: { // Depósito
                double valor;
                printf("Digite o valor do depósito: ");
                scanf("%lf", &valor);
                limparBuffer();
                if (valor > 0) {
                    conta->saldo += valor;
                    adicionarTransacao(conta, DEPOSITO, valor);
                    printf("Depósito realizado com sucesso!\n");
                } else {
                    printf("Valor inválido!\n");
                }
                break;
            }
            case 2: { // Saque
                double valor;
                printf("Digite o valor do saque: ");
                scanf("%lf", &valor);
                limparBuffer();
                if (valor > 0 && valor <= conta->saldo) {
                    conta->saldo -= valor;
                    adicionarTransacao(conta, SAQUE, valor);
                    printf("Saque realizado com sucesso!\n");
                } else {
                    printf("Valor inválido ou saldo insuficiente!\n");
                }
                break;
            }
            case 3: { // Transferência
                char cpf_dest[12];
                printf("Digite o CPF da conta de destino: ");
                fgets(cpf_dest, 12, stdin);
                limparBuffer();
                Cliente* cliente_dest = buscarClientePorCPF(cpf_dest);
                if (cliente_dest == NULL) {
                    printf("Erro: CPF de destino não encontrado!\n");
                    break;
                }
                int numConta_dest;
                printf("Digite o número da conta de destino: ");
                scanf("%d", &numConta_dest);
                limparBuffer();
                Conta* conta_dest = buscarConta(cliente_dest, numConta_dest);
                if (conta_dest == NULL) {
                    printf("Erro: Conta de destino não encontrada!\n");
                    break;
                }
                double valor;
                printf("Digite o valor da transferência: ");
                scanf("%lf", &valor);
                limparBuffer();
                if (valor > 0 && valor <= conta->saldo) {
                    conta->saldo -= valor;
                    conta_dest->saldo += valor;
                    adicionarTransacao(conta, TRANSFERENCIA, valor);
                    adicionarTransacao(conta_dest, PIX, valor);
                    printf("Transferência realizada com sucesso!\n");
                } else {
                    printf("Valor inválido ou saldo insuficiente!\n");
                }
                break;
            }
            default:
                printf("Opção inválida!\n");
        }
        //pergunta ao usuário se deseja continuar realizando operações
        printf("\nDeseja fazer nova operação nesta conta? (S/N): ");
        scanf(" %c", &continuar);
        limparBuffer();
    }
}

//Exibe o saldo e, se o usuário escolher 'S', o extrato detalhado de uma conta específica
void consultarConta() {
    char cpf[12];
    int numConta;
    printf("\n--- Consulta de Saldo / Extrato ---\n");
    printf("Digite o CPF do titular da conta: ");
    fgets(cpf, 12, stdin);
    limparBuffer();

    Cliente* cliente = buscarClientePorCPF(cpf);
    if (cliente == NULL) {
        printf("Erro: Cliente não encontrado.\n");
        return;
    }
    printf("Digite o número da conta: ");
    scanf("%d", &numConta);
    limparBuffer();
    Conta* conta = buscarConta(cliente, numConta);
    if (conta == NULL) {
        printf("Erro: Conta não encontrada.\n");
        return;
    }

    // Exibe as informações principais da conta
    printf("\n--- DADOS DA CONTA ---\n");
    printf("Titular: %s\n", cliente->nomeCompleto);
    printf("Agência: %d | Conta: %d\n", conta->agencia, conta->numero);
    printf("SALDO ATUAL: R$ %.2f\n", conta->saldo);
    
    char verExtrato;
    printf("\nDeseja ver o extrato detalhado? (S/N): ");
    scanf(" %c", &verExtrato);
    limparBuffer();

    // Se o usuário confirmar, exibe o histórico de transações
    if (toupper(verExtrato) == 'S') {
        printf("\n--- EXTRATO DETALHADO ---\n");
        if (conta->num_transacoes == 0) {
            printf("Nenhuma transação registrada.\n");
        } else {
            int i;
            // Itera sobre o array de transações e imprime cada uma
            for (i = 0; i < conta->num_transacoes; i++) {
                Transacao t = conta->transacoes[i];
                switch (t.tipo) {
                    case DEPOSITO:
                        printf("DEPÓSITO      : + R$ %.2f\n", t.valor);
                        break;
                    case SAQUE:
                        printf("SAQUE         : - R$ %.2f\n", t.valor);
                        break;
                    case TRANSFERENCIA:
                        printf("TRANSFERÊNCIA : - R$ %.2f\n", t.valor);
                        break;
                    case PIX:
                        printf("PIX RECEBIDO  : + R$ %.2f\n", t.valor);
                        break;
                }
            }
        }
    }
}
