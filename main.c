#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#define MAX_REGIAO   64
#define MAX_TIPO_STR 32

typedef enum {

    ACIDENTE_TRANSITO   = 1,
    FALHA_SEMAFORO      = 2,
    INTERRUPCAO_ENERGIA = 3,
    ALAGAMENTO          = 4,
    INCENDIO            = 5

} TipoEvento;

typedef enum {

    ATIVO    = 0,
    RESOLVIDO = 1

} StatusEvento;

typedef struct {

    int dia, mes, ano;
    int hora, minuto, segundo;

} DataHora;

typedef struct No {

    int          id;
    TipoEvento   tipo;
    int          severidade;
    DataHora     timestamp;
    char         regiao[MAX_REGIAO];
    StatusEvento status;
    int          altura;
    struct No   *esq;
    struct No   *dir;

} No;

static long g_rotacoes = 0;

/* ---- Auxiliares AVL ---- */

static int altura(No *n)           { return n ? n->altura : -1; }

static int maximo(int a, int b)    { return a > b ? a : b; }

static void atualizarAltura(No *n) {

    if (n) n->altura = 1 + maximo(altura(n->esq), altura(n->dir));

}

static int fatorBalanceamento(No *n) {

    return n ? altura(n->esq) - altura(n->dir) : 0;

}

/* ---- Rotações ---- */

static No *rotacaoDireita(No *y) {

    No *x = y->esq, *T2 = x->dir;

    x->dir = y; y->esq = T2;

    atualizarAltura(y); atualizarAltura(x);

    g_rotacoes++;

    return x;

}

static No *rotacaoEsquerda(No *x) {

    No *y = x->dir, *T2 = y->esq;

    y->esq = x; x->dir = T2;

    atualizarAltura(x); atualizarAltura(y);

    g_rotacoes++;

    return y;

}

static No *rotacaoEsquerdaDireita(No *z) {

    z->esq = rotacaoEsquerda(z->esq);

    return rotacaoDireita(z);

}

static No *rotacaoDireitaEsquerda(No *z) {

    z->dir = rotacaoDireita(z->dir);

    return rotacaoEsquerda(z);

}

static No *rebalancear(No *n) {

    atualizarAltura(n);

    int fb = fatorBalanceamento(n);

    if (fb > 1)  return fatorBalanceamento(n->esq) >= 0 ? rotacaoDireita(n)       : rotacaoEsquerdaDireita(n);

    if (fb < -1) return fatorBalanceamento(n->dir) <= 0 ? rotacaoEsquerda(n)      : rotacaoDireitaEsquerda(n);

    return n;

}

/* ---- Inserção ---- */

static No *criarNo(int id, TipoEvento tipo, int sev, DataHora ts, const char *reg) {

    No *n = (No *)malloc(sizeof(No));

    if (!n) { fprintf(stderr, "Sem memoria!\n"); exit(EXIT_FAILURE); }

    n->id = id; n->tipo = tipo; n->severidade = sev; n->timestamp = ts;

    strncpy(n->regiao, reg, MAX_REGIAO - 1); n->regiao[MAX_REGIAO - 1] = '\0';

    n->status = ATIVO; n->altura = 0; n->esq = n->dir = NULL;

    return n;

}

static No *inserir(No *raiz, int id, TipoEvento tipo, int sev, DataHora ts, const char *reg, int *ok) {

    if (!raiz) { *ok = 1; return criarNo(id, tipo, sev, ts, reg); }

    if      (id < raiz->id) raiz->esq = inserir(raiz->esq, id, tipo, sev, ts, reg, ok);

    else if (id > raiz->id) raiz->dir = inserir(raiz->dir, id, tipo, sev, ts, reg, ok);

    else { *ok = 0; return raiz; }

    return rebalancear(raiz);

}

/* ---- Remoção ---- */

static No *minNo(No *n) { while (n->esq) n = n->esq; return n; }

static No *removerInterno(No *raiz, int id, int *res, int forcar) {

    if (!raiz) { if (!forcar) *res = -1; return NULL; }

    if      (id < raiz->id) raiz->esq = removerInterno(raiz->esq, id, res, forcar);

    else if (id > raiz->id) raiz->dir = removerInterno(raiz->dir, id, res, forcar);

    else {

        if (!forcar && raiz->status != RESOLVIDO) { *res = 0; return raiz; }

        if (!forcar) *res = 1;

        if (!raiz->esq || !raiz->dir) {

            No *tmp = raiz->esq ? raiz->esq : raiz->dir;

            free(raiz); return tmp;

        }

        No *suc = minNo(raiz->dir);

        raiz->id = suc->id; raiz->tipo = suc->tipo;

        raiz->severidade = suc->severidade; raiz->timestamp = suc->timestamp;

        strncpy(raiz->regiao, suc->regiao, MAX_REGIAO - 1);

        raiz->regiao[MAX_REGIAO - 1] = '\0'; raiz->status = suc->status;

        raiz->dir = removerInterno(raiz->dir, suc->id, res, 1);

    }

    return rebalancear(raiz);

}

static No *remover(No *raiz, int id, int *res) { return removerInterno(raiz, id, res, 0); }



/* ---- Busca e impressão ---- */

static No *buscarPorId(No *raiz, int id) {

    if (!raiz) return NULL;

    if (id == raiz->id) return raiz;

    return id < raiz->id ? buscarPorId(raiz->esq, id) : buscarPorId(raiz->dir, id);

}

static const char *tipoStr(TipoEvento t) {

    switch (t) {

        case ACIDENTE_TRANSITO:   return "Acidente de Transito";

        case FALHA_SEMAFORO:      return "Falha em Semaforo";

        case INTERRUPCAO_ENERGIA: return "Interrupcao de Energia";

        case ALAGAMENTO:          return "Alagamento";

        case INCENDIO:            return "Incendio";

        default:                  return "Desconhecido";

    }
}

static const char *statusStr(StatusEvento s) { return s == ATIVO ? "Ativo" : "Resolvido"; }

static void imprimirNo(const No *n) {

    printf("  ID: %-6d | Tipo: %-22s | Sev: %d | Status: %-9s\n",

           n->id, tipoStr(n->tipo), n->severidade, statusStr(n->status));

    printf("            Regiao: %-20s | Data: %02d/%02d/%04d %02d:%02d:%02d\n",

           n->regiao, n->timestamp.dia, n->timestamp.mes, n->timestamp.ano,

           n->timestamp.hora, n->timestamp.minuto, n->timestamp.segundo);

}

/* ---- Consultas avançadas ---- */

static void listarPorSeveridade(No *r, int mn, int mx, int *c) {

    if (!r) return;

    listarPorSeveridade(r->esq, mn, mx, c);

    if (r->status == ATIVO && r->severidade >= mn && r->severidade <= mx) { imprimirNo(r); (*c)++; }

    listarPorSeveridade(r->dir, mn, mx, c);

}

static void listarPorRegiao(No *r, const char *reg, int *c) {

    if (!r) return;

    listarPorRegiao(r->esq, reg, c);

    if (r->status == ATIVO && strcasecmp(r->regiao, reg) == 0) { imprimirNo(r); (*c)++; }

    listarPorRegiao(r->dir, reg, c);

}

/* Poda eficiente por intervalo de ID */

static void listarPorIntervaloId(No *r, int mn, int mx, int *c) {

    if (!r) return;

    if (r->id > mn) listarPorIntervaloId(r->esq, mn, mx, c);

    if (r->id >= mn && r->id <= mx) { imprimirNo(r); (*c)++; }

    if (r->id < mx) listarPorIntervaloId(r->dir, mn, mx, c);

}

static void emOrdem(No *r) {

    if (!r) return;

    emOrdem(r->esq); imprimirNo(r); emOrdem(r->dir);

}

/* ---- Métricas ---- */

static int contarNos(No *r)    { return r ? 1 + contarNos(r->esq) + contarNos(r->dir) : 0; }

static int contarAtivos(No *r) { return r ? (r->status == ATIVO ? 1 : 0) + contarAtivos(r->esq) + contarAtivos(r->dir) : 0; }

static void somarFB(No *r, double *s, int *c) {

    if (!r) return;

    *s += abs(fatorBalanceamento(r)); (*c)++;

    somarFB(r->esq, s, c); somarFB(r->dir, s, c);

}

static void exibirMetricas(No *raiz) {

    int total = contarNos(raiz), ativos = contarAtivos(raiz);

    double soma = 0.0; int cnt = 0;

    somarFB(raiz, &soma, &cnt);

    printf("\n  +----- Metricas da Arvore AVL -----+\n");
    printf("  | Altura total          : %d\n",   altura(raiz));
    printf("  | Total de nos          : %d\n",   total);
    printf("  | Eventos ativos        : %d\n",   ativos);
    printf("  | Eventos resolvidos    : %d\n",   total - ativos);
    printf("  | FB medio (|FB|)       : %.3f\n", cnt > 0 ? soma / cnt : 0.0);
    printf("  | Rotacoes realizadas   : %ld\n",  g_rotacoes);
    printf("  +----------------------------------+\n\n");

}

static void liberarArvore(No *r) {

    if (!r) return;

    liberarArvore(r->esq); liberarArvore(r->dir); free(r);

}

/* ---- Entrada segura ----

   Retorna:
             1 = leu número OK

             0 = entrada inválida (não numérica)

            -1 = EOF (stdin fechou) → sinaliza saída do loop */

static int lerInteiro(const char *msg, int *val) {

    printf("%s", msg);

    char buf[64];

    if (!fgets(buf, sizeof(buf), stdin)) return -1; /* EOF */

    char *end;

    long v = strtol(buf, &end, 10);

    if (end == buf || (*end != '\n' && *end != '\0')) return 0;

    *val = (int)v;

    return 1;

}

static void lerString(const char *msg, char *dest, int maxLen) {

    printf("%s", msg);

    if (fgets(dest, maxLen, stdin)) dest[strcspn(dest, "\n")] = '\0';

}

static DataHora lerDataHora(void) {

    DataHora dt = {0};

    printf("    Data (DD MM AAAA HH MM SS): ");

    char buf[128];

    if (fgets(buf, sizeof(buf), stdin)) {

        if (sscanf(buf, "%d %d %d %d %d %d",

                   &dt.dia, &dt.mes, &dt.ano,

                   &dt.hora, &dt.minuto, &dt.segundo) != 6)

            printf("    [Aviso] Formato invalido. Campos faltantes serao zerados.\n");

    }

    return dt;

}

/* ---- Submenus ---- */

static No *menuCadastro(No *raiz) {

    printf("\n=== CADASTRAR NOVO EVENTO ===\n");

    int id, r;

    r = lerInteiro("  ID do evento  : ", &id);

    if (r <= 0 || id <= 0) { printf("  ID invalido.\n"); return raiz; }

    printf("  Tipos: 1-Acidente Transito  2-Falha Semaforo  3-Interrupcao Energia\n");
    printf("         4-Alagamento         5-Incendio\n");

    int tipoInt;

    r = lerInteiro("  Tipo (1-5)    : ", &tipoInt);

    if (r <= 0 || tipoInt < 1 || tipoInt > 5) { printf("  Tipo invalido.\n"); return raiz; }

    int sev;

    r = lerInteiro("  Severidade (1-5): ", &sev);

    if (r <= 0 || sev < 1 || sev > 5) { printf("  Severidade invalida.\n"); return raiz; }

    DataHora ts = lerDataHora();

    char regiao[MAX_REGIAO];

    lerString("  Regiao        : ", regiao, MAX_REGIAO);

    if (strlen(regiao) == 0) { printf("  Regiao invalida.\n"); return raiz; }

    int ok = 0;

    raiz = inserir(raiz, id, (TipoEvento)tipoInt, sev, ts, regiao, &ok);

    printf(ok ? "  [OK] Evento #%d cadastrado.\n" : "  [ERRO] ID #%d ja existe.\n", id);

    return raiz;

}

static No *menuRemocao(No *raiz) {

    printf("\n=== REMOVER EVENTO ===\n");

    int id, r;

    r = lerInteiro("  ID do evento a remover: ", &id);

    if (r <= 0) { printf("  Entrada invalida.\n"); return raiz; }

    int resultado = 0;

    raiz = remover(raiz, id, &resultado);

    switch (resultado) {

        case  1: printf("  [OK] Evento #%d removido.\n", id); break;
        case  0: printf("  [ERRO] Evento #%d esta ATIVO. Resolva antes de remover.\n", id); break;
        case -1: printf("  [ERRO] Evento #%d nao encontrado.\n", id); break;

    }

    return raiz;

}

static void menuBuscaPorId(No *raiz) {

    printf("\n=== BUSCAR EVENTO POR ID ===\n");

    int id, r;

    r = lerInteiro("  ID: ", &id);

    if (r <= 0) { printf("  Entrada invalida.\n"); return; }

    No *n = buscarPorId(raiz, id);

    if (n) { printf("\n  Evento encontrado:\n"); imprimirNo(n); }

    else   printf("  Evento #%d nao encontrado.\n", id);

}

static void menuConsultas(No *raiz) {

    int op, r;

    do {

        printf("\n+--- CONSULTAS AVANCADAS ---+\n");
        printf("| 1. Por intervalo de severidade |\n");
        printf("| 2. Por regiao (em-ordem)       |\n");
        printf("| 3. Por intervalo de ID         |\n");
        printf("| 4. Listar todos os eventos     |\n");
        printf("| 0. Voltar                      |\n");
        printf("+--------------------------------+\n");

        r = lerInteiro("Opcao: ", &op);

        if (r == -1) break; /* EOF: sai do submenu */

        if (r == 0) { op = -1; printf("  Opcao invalida.\n"); continue; }

        switch (op) {

            case 1: {

                int minS, maxS;

                if (lerInteiro("  Severidade minima (1-5): ", &minS) <= 0 ||

                    lerInteiro("  Severidade maxima (1-5): ", &maxS) <= 0) { printf("  Entrada invalida.\n"); break; }

                int c = 0;

                printf("\n  Eventos ATIVOS com severidade entre %d e %d:\n", minS, maxS);

                listarPorSeveridade(raiz, minS, maxS, &c);

                printf("  Total encontrado: %d\n", c);

                break;

            }

            case 2: {

                char reg[MAX_REGIAO];

                lerString("  Regiao: ", reg, MAX_REGIAO);

                int c = 0;

                printf("\n  Eventos ATIVOS na regiao \"%s\" (ordenados por ID):\n", reg);

                listarPorRegiao(raiz, reg, &c);

                printf("  Total encontrado: %d\n", c);

                break;

            }

            case 3: {

                int idMin, idMax;

                if (lerInteiro("  ID minimo: ", &idMin) <= 0 ||

                    lerInteiro("  ID maximo: ", &idMax) <= 0) { printf("  Entrada invalida.\n"); break; }

                int c = 0;

                printf("\n  Eventos com ID entre %d e %d:\n", idMin, idMax);

                listarPorIntervaloId(raiz, idMin, idMax, &c);

                printf("  Total encontrado: %d\n", c);

                break;

            }

            case 4:

                printf("\n  Todos os eventos (em-ordem por ID):\n");

                if (!raiz) printf("  Arvore vazia.\n");

                else emOrdem(raiz);

                break;

            case 0: break;

            default: printf("  Opcao invalida.\n");

        }
    } while (op != 0);

}

static void menuAtualizacoes(No *raiz) {

    int op, r;

    do {

        printf("\n+--- ATUALIZACOES ---+\n");
        printf("| 1. Alterar status (Ativo -> Resolvido) |\n");
        printf("| 2. Atualizar severidade                |\n");
        printf("| 0. Voltar                              |\n");
        printf("+----------------------------------------+\n");

        r = lerInteiro("Opcao: ", &op);

        if (r == -1) break; /* EOF */

        if (r == 0) { op = -1; printf("  Opcao invalida.\n"); continue; }

        switch (op) {

            case 1: {

                int id;

                if (lerInteiro("  ID do evento: ", &id) <= 0) { printf("  Entrada invalida.\n"); break; }

                No *n = buscarPorId(raiz, id);

                if (!n)                      printf("  Evento #%d nao encontrado.\n", id);

                else if (n->status == RESOLVIDO) printf("  Evento #%d ja foi resolvido.\n", id);

                else { n->status = RESOLVIDO; printf("  [OK] Evento #%d marcado como Resolvido.\n", id); }

                break;
            }

            case 2: {

                int id, novaS;

                if (lerInteiro("  ID do evento: ", &id) <= 0) { printf("  Entrada invalida.\n"); break; }

                No *n = buscarPorId(raiz, id);

                if (!n)                      printf("  Evento #%d nao encontrado.\n", id);

                else if (n->status == RESOLVIDO) printf("  Evento #%d ja resolvido, nao e possivel alterar severidade.\n", id);

                else {
                    if (lerInteiro("  Nova severidade (1-5): ", &novaS) <= 0 || novaS < 1 || novaS > 5) {
                        printf("  Severidade invalida.\n"); break;
                    }

                    n->severidade = novaS;
                    printf("  [OK] Severidade do evento #%d atualizada para %d.\n", id, novaS);
                }
                break;
            }
            case 0: break;
            default: printf("  Opcao invalida.\n");
        }
    } while (op != 0);
}

/* ---- Main ---- */

int main(void) {
    No *raiz = NULL;
    int opcao, r;

    printf("\n##############################################\n");
    printf("##  SISTEMA DE EVENTOS CRITICOS - CIDADE    ##\n");
    printf("##            INTELIGENTE  |  AVL           ##\n");
    printf("##############################################\n");

    do {

        printf("\n+========== MENU PRINCIPAL ==========+\n");
        printf("| 1. Cadastrar evento                  |\n");
        printf("| 2. Remover evento (so resolvidos)    |\n");
        printf("| 3. Buscar evento por ID              |\n");
        printf("| 4. Consultas avancadas (Relatorios)  |\n");
        printf("| 5. Atualizar status ou severidade    |\n");
        printf("| 6. Ver Metricas da AVL               |\n");
        printf("| 0. Sair do sistema                   |\n");
        printf("+======================================+\n");

        r = lerInteiro("Opcao: ", &opcao);

        if (r == -1) { /* EOF: encerra limpo */
            printf("\n  EOF detectado. Encerrando.\n");
            break;
        }

        if (r == 0) {
            printf("  Opcao invalida. Digite um numero.\n");
            opcao = -1;
            continue;
        }

        switch (opcao) {
            case 1: raiz = menuCadastro(raiz);    break;
            case 2: raiz = menuRemocao(raiz);     break;
            case 3: menuBuscaPorId(raiz);          break;
            case 4: menuConsultas(raiz);           break;
            case 5: menuAtualizacoes(raiz);        break;
            case 6: exibirMetricas(raiz);          break;
            case 0: printf("\n  Desligando o sistema. Ate a proxima!\n\n"); break;
            default: printf("  Opcao invalida. Tente novamente.\n");
        }

    } while (opcao != 0);

    liberarArvore(raiz);

    return 0;

}