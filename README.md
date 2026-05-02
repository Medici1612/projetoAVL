# Sistema de Monitoramento de Eventos Críticos (AVL)

Este projeto é uma aplicação em **C** desenvolvida para gerenciar incidentes em uma cidade inteligente. O sistema utiliza uma **Árvore AVL** (Árvore Binária de Busca Balanceada) para garantir que todas as operações de busca, inserção e remoção ocorram com eficiência máxima.

## 🚀 Funcionalidades

-   **Balanceamento Automático:** Implementação das 4 rotações clássicas (LL, RR, LR, RL).
-   **Busca Eficiente:** Localização de eventos por ID com complexidade $O(\log n)$.
-   **Consultas Avançadas:**
    -   Listagem por intervalo de severidade (1 a 5).
    -   Busca por região geográfica.
    -   Poda (pruning) por intervalo de ID.
-   **Regras de Negócio:** Sistema de segurança que impede a remoção de eventos com status "Ativo".
-   **Métricas de Performance:** Exibição de altura da árvore, fator de balanceamento médio e total de rotações realizadas.
-   **Robustez:** Tratamento de entradas inválidas e detecção de fim de arquivo (EOF).

## 🛠️ Tecnologias Utilizadas

-   Linguagem **C** (Padrão C99/C11).
-   Compilador **GCC**.
-   Estrutura de Dados: **Árvore AVL**.

## 💻 Como Compilar e Executar

1.  **Compilação:**
    Abra o terminal na pasta do projeto e execute:
    ```bash
    gcc -Wall -o sistema_avl main.c
    ```

2.  **Execução:**
    ```bash
    ./sistema_avl
    ```

## 📋 Estrutura do Código

-   `No`: Estrutura principal que contém os dados do evento e ponteiros da árvore.
-   `inserir`: Adiciona novos eventos e realiza o rebalanceamento automático.
-   `remover`: Exclui eventos (apenas se estiverem com status "Resolvido").
-   `buscarPorId`: Realiza a busca binária na árvore.
-   `exibirMetricas`: Mostra dados técnicos sobre a saúde da estrutura de dados.
