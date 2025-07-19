/*
 * =================================================================================
 * TRABALHO FINAL DE PROGRAMACAO COMPUTACIONAL - 2025.1
 * TEMA 18: SIMULADOR DE SISTEMA BANCARIO SIMPLIFICADO
 *
 * Este arquivo contém a implementação completa do sistema, aderindo às
 * regras da disciplina, como o uso de alocação dinâmica, ponteiros,
 * structs e manipulação de arquivos.
 * =================================================================================
 */

// --- BIBLIOTECAS PADRÃO ---
// Inclusão das bibliotecas permitidas pela especificação do trabalho.
#include <stdio.h>   // Para funções de entrada e saída, como printf e scanf.
#include <stdlib.h>  // Para alocação de memória (malloc, realloc, free) e outras utilidades.
#include <string.h>  // Para manipulação de strings, como strcpy e strcmp.
#include <locale.h>  // Para definir a localidade e permitir acentuação em português.
#include <ctype.h>   // Para funções de caracteres, como toupper (converter para maiúsculo).

// --- CONSTANTES GLOBAIS ---
// Define o nome do arquivo que será usado para salvar e carregar os dados.
#define NOME_ARQUIVO "banco_dados.bin"

// =================================================================================
// --- SEÇÃO DE ESTRUTURAS DE DADOS (STRUCTS) ---
// Modelagem dos dados do sistema conforme a especificação.
// =================================================================================

// Define os tipos de transações possíveis usando uma enumeração.
// Isso torna o código mais legível do que usar números mágicos (0, 1, 2, 3).
typedef enum {
    DEPOSITO,       //
    SAQUE,          //
    TRANSFERENCIA,  //
    PIX             //
} TipoTransacao;

// Estrutura que representa uma única transação financeira.
typedef struct {
    TipoTransacao tipo; // O tipo da transação (depósito, saque, etc.).
    double valor;       // O valor monetário da transação.
} Transacao;

// Estrutura que representa uma conta bancária.
typedef struct {
    int numero;         // Número da conta.
    int agencia;        // Número da agência.
    double saldo;       // Saldo atual da conta.
    Transacao* transacoes; // Ponteiro para um array dinâmico de transações (extrato).
    int num_transacoes; // Quantidade de transações armazenadas no extrato.
} Conta;

// Estrutura que representa um cliente do banco.
typedef struct {
    char nomeCompleto[50]; // Nome do cliente.
    char cpf[12];          // CPF com 11 dígitos + 1 para o caractere nulo ('\0').
    int idade;             // Idade do cliente.
    Conta* contas;         // Ponteiro para um array dinâmico de contas pertencentes ao cliente.
    int num_contas;        // Quantidade de contas que o cliente possui.
} Cliente;


// =================================================================================
// --- VARIÁVEIS GLOBAIS ---
// =================================================================================

// Ponteiro para o array dinâmico que armazenará todos os clientes do sistema.
// Começa como NULO, indicando que o sistema está vazio.
Cliente* g_clientes = NULL;

// Variável que controla o número total de clientes cadastrados.
int g_num_clientes = 0;


// =================================================================================
// --- PROTÓTIPOS DAS FUNÇÕES ---
// Declaração antecipada de todas as funções para que possam ser chamadas
// umas pelas outras, independentemente da ordem em que são definidas no arquivo.
// =================================================================================
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


// =================================================================================
// --- FUNÇÃO PRINCIPAL (main) ---
// Ponto de entrada do programa. Controla o fluxo principal e o menu.
// =================================================================================
int main() {
    // Define a localidade para "Portuguese" para exibir corretamente os acentos.
    setlocale(LC_ALL, "Portuguese");

    // Tenta carregar os dados existentes do arquivo binário ao iniciar.
    carregarDados();

    int opcao;
    // Loop principal do menu. Continua executando até que o usuário escolha a opção 0.
    do {
        // Exibe as opções do menu para o usuário.
        printf("\n===== SIMULADOR DE SISTEMA BANCARIO =====\n");
        printf("1. Cadastrar Novo Cliente\n");        //
        printf("2. Realizar Transacao\n");            //
        printf("3. Consultar Saldo / Extrato\n");    //
        printf("0. Sair do Programa\n");
        printf("=========================================\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        limparBuffer(); // Limpa o buffer de entrada para evitar erros em leituras futuras.

        // Estrutura switch para direcionar o fluxo do programa com base na escolha do usuário.
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
                printf("Opcao invalida! Tente novamente.\n");
        }

        // Pausa o sistema após cada operação (exceto ao sair) para que o usuário possa ler a saída.
        if (opcao != 0) {
            pausarSistema();
        }

    } while (opcao != 0);

    // Antes de encerrar, salva todos os dados no arquivo e libera a memória alocada.
    salvarDados();
    liberarMemoria();
    printf("Dados salvos com sucesso. Ate logo!\n");

    return 0; // Retorna 0 para indicar que o programa terminou com sucesso.
}


// =================================================================================
// --- SEÇÃO DE FUNÇÕES UTILITÁRIAS ---
// Funções de apoio que realizam tarefas genéricas.
// =================================================================================

// Limpa o buffer de entrada (teclado). Essencial após usar scanf para ler números,
// pois o caractere '\n' (Enter) fica no buffer e pode atrapalhar leituras de texto subsequentes.
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Pausa a execução do programa e aguarda o usuário pressionar a tecla Enter.
void pausarSistema() {
    printf("\nPressione Enter para continuar...");
    limparBuffer();
}

// Percorre todas as estruturas de dados alocadas dinamicamente e libera a memória
// correspondente para evitar vazamentos de memória (memory leaks).
void liberarMemoria() {
    int i, j;
    if (g_clientes != NULL) {
        // Para cada cliente...
        for (i = 0; i < g_num_clientes; i++) {
            if (g_clientes[i].contas != NULL) {
                // Para cada conta do cliente...
                for (j = 0; j < g_clientes[i].num_contas; j++) {
                    // Libera o array de transações da conta.
                    free(g_clientes[i].contas[j].transacoes);
                }
                // Libera o array de contas do cliente.
                free(g_clientes[i].contas);
            }
        }
        // Finalmente, libera o array principal de clientes.
        free(g_clientes);
    }
}


// =================================================================================
// --- SEÇÃO DE PERSISTÊNCIA DE DADOS (ARQUIVOS) ---
// Funções responsáveis por salvar e carregar o estado do sistema em um arquivo.
// =================================================================================

// Salva o estado atual do sistema (todos os clientes, contas e transações)
// em um arquivo binário chamado "banco_dados.bin".
void salvarDados() {
    // Abre o arquivo no modo "write binary" (escrita binária).
    FILE* arquivo = fopen(NOME_ARQUIVO, "wb");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo para escrita");
        return;
    }

    // 1. Salva o número total de clientes.
    fwrite(&g_num_clientes, sizeof(int), 1, arquivo);

    int i, j;
    // 2. Itera sobre cada cliente para salvar seus dados.
    for (i = 0; i < g_num_clientes; i++) {
        // Salva a estrutura principal do cliente (sem os ponteiros, que são salvos a seguir).
        fwrite(&g_clientes[i], sizeof(Cliente), 1, arquivo);
        
        // 3. Salva o array de contas do cliente.
        if (g_clientes[i].num_contas > 0) {
            fwrite(g_clientes[i].contas, sizeof(Conta), g_clientes[i].num_contas, arquivo);
            
            // 4. Itera sobre cada conta para salvar seu extrato (array de transações).
            for (j = 0; j < g_clientes[i].num_contas; j++) {
                if (g_clientes[i].contas[j].num_transacoes > 0) {
                    fwrite(g_clientes[i].contas[j].transacoes, sizeof(Transacao), g_clientes[i].contas[j].num_transacoes, arquivo);
                }
            }
        }
    }
    // Fecha o arquivo para garantir que todos os dados sejam gravados no disco.
    fclose(arquivo);
}

// Carrega os dados de "banco_dados.bin" para a memória, reconstruindo o
// estado do sistema de onde parou na última execução.
void carregarDados() {
    // Abre o arquivo no modo "read binary" (leitura binária).
    FILE* arquivo = fopen(NOME_ARQUIVO, "rb");
    if (arquivo == NULL) {
        // Se o arquivo não existe (primeira execução), apenas informa e continua.
        printf("Arquivo de dados nao encontrado. Iniciando com sistema vazio.\n");
        return;
    }

    // 1. Lê o número total de clientes.
    fread(&g_num_clientes, sizeof(int), 1, arquivo);
    if (g_num_clientes == 0) {
        fclose(arquivo);
        return;
    }

    // 2. Aloca memória para o array de clientes.
    g_clientes = (Cliente*)malloc(g_num_clientes * sizeof(Cliente));

    int i, j;
    // 3. Itera para ler os dados de cada cliente.
    for (i = 0; i < g_num_clientes; i++) {
        fread(&g_clientes[i], sizeof(Cliente), 1, arquivo);
        
        // 4. Para cada cliente, aloca e lê o array de contas.
        if (g_clientes[i].num_contas > 0) {
            g_clientes[i].contas = (Conta*)malloc(g_clientes[i].num_contas * sizeof(Conta));
            fread(g_clientes[i].contas, sizeof(Conta), g_clientes[i].num_contas, arquivo);

            // 5. Para cada conta, aloca e lê o array de transações.
            for(j = 0; j < g_clientes[i].num_contas; j++) {
                if(g_clientes[i].contas[j].num_transacoes > 0) {
                    g_clientes[i].contas[j].transacoes = (Transacao*)malloc(g_clientes[i].contas[j].num_transacoes * sizeof(Transacao));
                    fread(g_clientes[i].contas[j].transacoes, sizeof(Transacao), g_clientes[i].contas[j].num_transacoes, arquivo);
                } else {
                    g_clientes[i].contas[j].transacoes = NULL; // Garante que contas sem transações tenham ponteiro NULO
                }
            }
        } else {
            g_clientes[i].contas = NULL; // Garante que clientes sem contas tenham ponteiro NULO
        }
    }
    fclose(arquivo);
    printf("Dados carregados com sucesso!\n");
}


// =================================================================================
// --- SEÇÃO DE FUNCIONALIDADES PRINCIPAIS ---
// Funções que implementam os requisitos do sistema.
// =================================================================================

// Função para cadastrar um novo cliente e sua primeira conta bancária.
void cadastrarCliente() {
    char cpf_temp[12];
    printf("\n--- Cadastro de Novo Cliente ---\n");
    printf("Digite o CPF (apenas 11 numeros): ");
    fgets(cpf_temp, 12, stdin); // Leitura segura do CPF.
    limparBuffer();

    // Verifica se o CPF já existe para evitar duplicatas.
    if (buscarClientePorCPF(cpf_temp) != NULL) {
        printf("Erro: CPF ja cadastrado no sistema.\n");
        return;
    }
    
    // Aumenta o tamanho do array global de clientes usando realloc.
    g_num_clientes++;
    g_clientes = (Cliente*)realloc(g_clientes, g_num_clientes * sizeof(Cliente));
    
    // Pega um ponteiro para a nova posição no array para preencher os dados.
    Cliente* novoCliente = &g_clientes[g_num_clientes - 1];
    strcpy(novoCliente->cpf, cpf_temp);

    printf("Digite o nome completo (ate 49 caracteres): ");
    fgets(novoCliente->nomeCompleto, 50, stdin);
    // Remove o '\n' que o fgets captura.
    novoCliente->nomeCompleto[strcspn(novoCliente->nomeCompleto, "\n")] = 0;

    printf("Digite a idade: ");
    scanf("%d", &novoCliente->idade);
    limparBuffer();
    
    printf("\n--- Cadastro da Conta Bancaria ---\n");
    // Aloca memória para a primeira conta do cliente.
    novoCliente->num_contas = 1;
    novoCliente->contas = (Conta*)malloc(sizeof(Conta));
    
    printf("Digite o numero da agencia: ");
    scanf("%d", &novoCliente->contas[0].agencia);
    limparBuffer();
    
    printf("Digite o numero da conta: ");
    scanf("%d", &novoCliente->contas[0].numero);
    limparBuffer();
    
    // Inicializa os dados da nova conta.
    novoCliente->contas[0].saldo = 0.0;
    novoCliente->contas[0].num_transacoes = 0;
    novoCliente->contas[0].transacoes = NULL;
    
    printf("\nCliente '%s' cadastrado com sucesso!\n", novoCliente->nomeCompleto);
}

// Busca um cliente no array global g_clientes pelo seu CPF.
// Retorna um ponteiro para o cliente se encontrado, ou NULL caso contrário.
Cliente* buscarClientePorCPF(const char* cpf) {
    int i;
    for (i = 0; i < g_num_clientes; i++) {
        if (strcmp(g_clientes[i].cpf, cpf) == 0) {
            return &g_clientes[i]; // Retorna o endereço do cliente encontrado.
        }
    }
    return NULL; // Retorna nulo se não encontrar.
}

// Busca uma conta dentro do array de contas de um cliente específico.
// Retorna um ponteiro para a conta se encontrada, ou NULL caso contrário.
Conta* buscarConta(Cliente* cliente, int numeroConta) {
    int i;
    for (i = 0; i < cliente->num_contas; i++) {
        if (cliente->contas[i].numero == numeroConta) {
            return &cliente->contas[i]; // Retorna o endereço da conta encontrada.
        }
    }
    return NULL;
}

// Adiciona um novo registro de transação ao extrato de uma conta.
void adicionarTransacao(Conta* conta, TipoTransacao tipo, double valor) {
    // Aumenta o tamanho do array de transações da conta.
    conta->num_transacoes++;
    conta->transacoes = (Transacao*)realloc(conta->transacoes, conta->num_transacoes * sizeof(Transacao));
    
    // Aponta para a nova posição e preenche com os dados da transação.
    Transacao* novaTransacao = &conta->transacoes[conta->num_transacoes - 1];
    novaTransacao->tipo = tipo;
    novaTransacao->valor = valor;
}

// Função principal que orquestra a realização de transações (depósito, saque, etc.).
void realizarTransacao() {
    char cpf[12];
    int numConta;
    printf("\n--- Realizar Transacao ---\n");
    printf("Digite o CPF do titular da conta: ");
    fgets(cpf, 12, stdin);
    limparBuffer();

    // Localiza o cliente e a conta onde a operação será realizada.
    Cliente* cliente = buscarClientePorCPF(cpf);
    if (cliente == NULL) {
        printf("Erro: Cliente nao encontrado.\n");
        return;
    }
    printf("Digite o numero da conta: ");
    scanf("%d", &numConta);
    limparBuffer();
    Conta* conta = buscarConta(cliente, numConta);
    if (conta == NULL) {
        printf("Erro: Conta nao encontrada para este cliente.\n");
        return;
    }

    char continuar = 'S';
    // Loop que permite ao usuário fazer várias operações na mesma conta.
    while (toupper(continuar) == 'S') {
        printf("\nConta de %s | Saldo Atual: R$ %.2f\n", cliente->nomeCompleto, conta->saldo);
        printf("Escolha a transacao:\n1. Deposito\n2. Saque\n3. Transferencia / PIX\n");
        printf("Escolha uma opcao: ");
        int opcaoTransacao;
        scanf("%d", &opcaoTransacao);
        limparBuffer();

        switch (opcaoTransacao) {
            case 1: { // Depósito
                double valor;
                printf("Digite o valor do deposito: ");
                scanf("%lf", &valor);
                limparBuffer();
                if (valor > 0) {
                    conta->saldo += valor;
                    adicionarTransacao(conta, DEPOSITO, valor);
                    printf("Deposito realizado com sucesso!\n");
                } else {
                    printf("Valor invalido!\n");
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
                    printf("Valor invalido ou saldo insuficiente!\n");
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
                    printf("Erro: CPF de destino nao encontrado!\n");
                    break;
                }
                int numConta_dest;
                printf("Digite o numero da conta de destino: ");
                scanf("%d", &numConta_dest);
                limparBuffer();
                Conta* conta_dest = buscarConta(cliente_dest, numConta_dest);
                if (conta_dest == NULL) {
                    printf("Erro: Conta de destino nao encontrada!\n");
                    break;
                }
                double valor;
                printf("Digite o valor da transferencia: ");
                scanf("%lf", &valor);
                limparBuffer();
                if (valor > 0 && valor <= conta->saldo) {
                    conta->saldo -= valor;
                    conta_dest->saldo += valor;
                    adicionarTransacao(conta, TRANSFERENCIA, valor);
                    adicionarTransacao(conta_dest, PIX, valor);
                    printf("Transferencia realizada com sucesso!\n");
                } else {
                    printf("Valor invalido ou saldo insuficiente!\n");
                }
                break;
            }
            default:
                printf("Opcao invalida!\n");
        }
        // Pergunta ao usuário se deseja continuar realizando operações.
        printf("\nDeseja fazer nova operacao nesta conta? (S/N): ");
        scanf(" %c", &continuar);
        limparBuffer();
    }
}

// Exibe o saldo e, opcionalmente, o extrato detalhado de uma conta específica.
void consultarConta() {
    char cpf[12];
    int numConta;
    printf("\n--- Consulta de Saldo / Extrato ---\n");
    printf("Digite o CPF do titular da conta: ");
    fgets(cpf, 12, stdin);
    limparBuffer();

    Cliente* cliente = buscarClientePorCPF(cpf);
    if (cliente == NULL) {
        printf("Erro: Cliente nao encontrado.\n");
        return;
    }
    printf("Digite o numero da conta: ");
    scanf("%d", &numConta);
    limparBuffer();
    Conta* conta = buscarConta(cliente, numConta);
    if (conta == NULL) {
        printf("Erro: Conta nao encontrada.\n");
        return;
    }

    // Exibe as informações principais da conta.
    printf("\n--- DADOS DA CONTA ---\n");
    printf("Titular: %s\n", cliente->nomeCompleto);
    printf("Agencia: %d | Conta: %d\n", conta->agencia, conta->numero);
    printf("SALDO ATUAL: R$ %.2f\n", conta->saldo);
    
    char verExtrato;
    printf("\nDeseja ver o extrato detalhado? (S/N): ");
    scanf(" %c", &verExtrato);
    limparBuffer();

    // Se o usuário confirmar, exibe o histórico de transações.
    if (toupper(verExtrato) == 'S') {
        printf("\n--- EXTRATO DETALHADO ---\n");
        if (conta->num_transacoes == 0) {
            printf("Nenhuma transacao registrada.\n");
        } else {
            int i;
            // Itera sobre o array de transações e imprime cada uma.
            for (i = 0; i < conta->num_transacoes; i++) {
                Transacao t = conta->transacoes[i];
                switch (t.tipo) {
                    case DEPOSITO:
                        printf("DEPOSITO      : + R$ %.2f\n", t.valor);
                        break;
                    case SAQUE:
                        printf("SAQUE         : - R$ %.2f\n", t.valor);
                        break;
                    case TRANSFERENCIA:
                        printf("TRANSFERENCIA : - R$ %.2f\n", t.valor);
                        break;
                    case PIX:
                        printf("PIX RECEBIDO  : + R$ %.2f\n", t.valor);
                        break;
                }
            }
        }
    }
}
