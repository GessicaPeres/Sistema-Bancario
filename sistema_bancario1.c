#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>

#define NOME_ARQUIVO "banco_dados.bin"

typedef enum { DEPOSITO, SAQUE, TRANSFERENCIA, PIX } TipoTransacao;

typedef struct {
    TipoTransacao tipo;
    double valor;
} Transacao;

typedef struct {
    int numero;
    int agencia;
    double saldo;
    Transacao* transacoes;
    int num_transacoes;
} Conta;

typedef struct {
    char nomeCompleto[50];
    char cpf[12];
    int idade;
    Conta* contas;
    int num_contas;
} Cliente;

Cliente* g_clientes = NULL;
int g_num_clientes = 0;

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

int main() {
    setlocale(LC_ALL, "Portuguese");
    carregarDados();
    int opcao;
    do {
        printf("\n===== SIMULADOR DE SISTEMA BANCARIO =====\n");
        printf("1. Cadastrar Novo Cliente\n");
        printf("2. Realizar Transacao\n");
        printf("3. Consultar Saldo / Extrato\n");
        printf("0. Sair do Programa\n");
        printf("=========================================\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        limparBuffer();
        switch (opcao) {
            case 1: cadastrarCliente(); break;
            case 2: realizarTransacao(); break;
            case 3: consultarConta(); break;
            case 0: printf("Saindo do programa. Salvando dados...\n"); break;
            default: printf("Opcao invalida! Tente novamente.\n");
        }
        if (opcao != 0) {
            pausarSistema();
        }
    } while (opcao != 0);
    salvarDados();
    liberarMemoria();
    printf("Dados salvos com sucesso. Ate logo!\n");
    return 0;
}

void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pausarSistema() {
    printf("\nPressione Enter para continuar...");
    limparBuffer();
}

void liberarMemoria() {
    int i, j;
    if (g_clientes != NULL) {
        for (i = 0; i < g_num_clientes; i++) {
            if (g_clientes[i].contas != NULL) {
                for (j = 0; j < g_clientes[i].num_contas; j++) {
                    free(g_clientes[i].contas[j].transacoes);
                }
                free(g_clientes[i].contas);
            }
        }
        free(g_clientes);
    }
}

void salvarDados() {
    FILE* arquivo = fopen(NOME_ARQUIVO, "wb");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo para escrita");
        return;
    }
    fwrite(&g_num_clientes, sizeof(int), 1, arquivo);
    int i, j;
    for (i = 0; i < g_num_clientes; i++) {
        fwrite(&g_clientes[i], sizeof(Cliente), 1, arquivo);
        if (g_clientes[i].num_contas > 0) {
            fwrite(g_clientes[i].contas, sizeof(Conta), g_clientes[i].num_contas, arquivo);
            for (j = 0; j < g_clientes[i].num_contas; j++) {
                if (g_clientes[i].contas[j].num_transacoes > 0) {
                    fwrite(g_clientes[i].contas[j].transacoes, sizeof(Transacao), g_clientes[i].contas[j].num_transacoes, arquivo);
                }
            }
        }
    }
    fclose(arquivo);
}

void carregarDados() {
    FILE* arquivo = fopen(NOME_ARQUIVO, "rb");
    if (arquivo == NULL) {
        printf("Arquivo de dados nao encontrado. Iniciando com sistema vazio.\n");
        return;
    }
    fread(&g_num_clientes, sizeof(int), 1, arquivo);
    if (g_num_clientes == 0) {
        fclose(arquivo);
        return;
    }
    g_clientes = (Cliente*)malloc(g_num_clientes * sizeof(Cliente));
    int i, j;
    for (i = 0; i < g_num_clientes; i++) {
        fread(&g_clientes[i], sizeof(Cliente), 1, arquivo);
        if (g_clientes[i].num_contas > 0) {
            g_clientes[i].contas = (Conta*)malloc(g_clientes[i].num_contas * sizeof(Conta));
            fread(g_clientes[i].contas, sizeof(Conta), g_clientes[i].num_contas, arquivo);
            for(j = 0; j < g_clientes[i].num_contas; j++) {
                if(g_clientes[i].contas[j].num_transacoes > 0) {
                    g_clientes[i].contas[j].transacoes = (Transacao*)malloc(g_clientes[i].contas[j].num_transacoes * sizeof(Transacao));
                    fread(g_clientes[i].contas[j].transacoes, sizeof(Transacao), g_clientes[i].contas[j].num_transacoes, arquivo);
                } else {
                    g_clientes[i].contas[j].transacoes = NULL;
                }
            }
        } else {
            g_clientes[i].contas = NULL;
        }
    }
    fclose(arquivo);
    printf("Dados carregados com sucesso!\n");
}

void cadastrarCliente() {
    char cpf_temp[12];
    printf("\n--- Cadastro de Novo Cliente ---\n");
    printf("Digite o CPF (apenas 11 numeros): ");
    fgets(cpf_temp, 12, stdin);
    limparBuffer();
    if (buscarClientePorCPF(cpf_temp) != NULL) {
        printf("Erro: CPF ja cadastrado no sistema.\n");
        return;
    }
    g_num_clientes++;
    g_clientes = (Cliente*)realloc(g_clientes, g_num_clientes * sizeof(Cliente));
    Cliente* novoCliente = &g_clientes[g_num_clientes - 1];
    strcpy(novoCliente->cpf, cpf_temp);
    printf("Digite o nome completo (ate 49 caracteres): ");
    fgets(novoCliente->nomeCompleto, 50, stdin);
    novoCliente->nomeCompleto[strcspn(novoCliente->nomeCompleto, "\n")] = 0;
    printf("Digite a idade: ");
    scanf("%d", &novoCliente->idade);
    limparBuffer();
    printf("\n--- Cadastro da Conta Bancaria ---\n");
    novoCliente->num_contas = 1;
    novoCliente->contas = (Conta*)malloc(sizeof(Conta));
    printf("Digite o numero da agencia: ");
    scanf("%d", &novoCliente->contas[0].agencia);
    limparBuffer();
    printf("Digite o numero da conta: ");
    scanf("%d", &novoCliente->contas[0].numero);
    limparBuffer();
    novoCliente->contas[0].saldo = 0.0;
    novoCliente->contas[0].num_transacoes = 0;
    novoCliente->contas[0].transacoes = NULL;
    printf("\nCliente '%s' cadastrado com sucesso!\n", novoCliente->nomeCompleto);
}

Cliente* buscarClientePorCPF(const char* cpf) {
    int i;
    for (i = 0; i < g_num_clientes; i++) {
        if (strcmp(g_clientes[i].cpf, cpf) == 0) {
            return &g_clientes[i];
        }
    }
    return NULL;
}

Conta* buscarConta(Cliente* cliente, int numeroConta) {
    int i;
    for (i = 0; i < cliente->num_contas; i++) {
        if (cliente->contas[i].numero == numeroConta) {
            return &cliente->contas[i];
        }
    }
    return NULL;
}

void adicionarTransacao(Conta* conta, TipoTransacao tipo, double valor) {
    conta->num_transacoes++;
    conta->transacoes = (Transacao*)realloc(conta->transacoes, conta->num_transacoes * sizeof(Transacao));
    Transacao* novaTransacao = &conta->transacoes[conta->num_transacoes - 1];
    novaTransacao->tipo = tipo;
    novaTransacao->valor = valor;
}

void realizarTransacao() {
    char cpf[12];
    int numConta;
    printf("\n--- Realizar Transacao ---\n");
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
        printf("Erro: Conta nao encontrada para este cliente.\n");
        return;
    }
    char continuar = 'S';
    while (toupper(continuar) == 'S') {
        printf("\nConta de %s | Saldo Atual: R$ %.2f\n", cliente->nomeCompleto, conta->saldo);
        printf("Escolha a transacao:\n1. Deposito\n2. Saque\n3. Transferencia / PIX\n");
        printf("Escolha uma opcao: ");
        int opcaoTransacao;
        scanf("%d", &opcaoTransacao);
        limparBuffer();
        switch (opcaoTransacao) {
            case 1: {
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
            case 2: {
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
            case 3: {
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
        printf("\nDeseja fazer nova operacao nesta conta? (S/N): ");
        scanf(" %c", &continuar);
        limparBuffer();
    }
}

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
    printf("\n--- DADOS DA CONTA ---\n");
    printf("Titular: %s\n", cliente->nomeCompleto);
    printf("Agencia: %d | Conta: %d\n", conta->agencia, conta->numero);
    printf("SALDO ATUAL: R$ %.2f\n", conta->saldo);
    char verExtrato;
    printf("\nDeseja ver o extrato detalhado? (S/N): ");
    scanf(" %c", &verExtrato);
    limparBuffer();
    if (toupper(verExtrato) == 'S') {
        printf("\n--- EXTRATO DETALHADO ---\n");
        if (conta->num_transacoes == 0) {
            printf("Nenhuma transacao registrada.\n");
        } else {
            int i;
            for (i = 0; i < conta->num_transacoes; i++) {
                Transacao t = conta->transacoes[i];
                switch (t.tipo) {
                    case DEPOSITO: printf("DEPOSITO      : + R$ %.2f\n", t.valor); break;
                    case SAQUE: printf("SAQUE         : - R$ %.2f\n", t.valor); break;
                    case TRANSFERENCIA: printf("TRANSFERENCIA : - R$ %.2f\n", t.valor); break;
                    case PIX: printf("PIX RECEBIDO  : + R$ %.2f\n", t.valor); break;
                }
            }
        }
    }
}
