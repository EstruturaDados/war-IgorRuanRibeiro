#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

// Estruturas de dados
typedef struct {
    char nome[30];   
    char cor[10];    
    int tropas;      
} Territorio;

typedef struct {
    char nome[30];
    char cor[10];
    char* missao;    // Alocada dinamicamente
    int territorios_conquistados;
} Jogador;

// ==================== FUNÇÕES UTILITÁRIAS ====================

// Limpa buffer de entrada para evitar problemas com scanf/fgets
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Valida se string contém apenas letras e espaços
int validarString(const char *str, size_t tamanho_maximo) {
    if (str == NULL) return 0;
    
    size_t len = strlen(str);
    if (len == 0 || len > tamanho_maximo) return 0;
    
    for (size_t i = 0; i < len; i++) {
        unsigned char ch = str[i];
        if (!isalpha(ch) && ch != ' ') return 0;
    }
    return 1;
}

// Lê string com tratamento de erros e limite de tamanho
int lerStringSegura(char *buffer, size_t tamanho, const char *prompt) {
    if (buffer == NULL || tamanho < 2) return 0;
    
    while (1) {
        printf("%s", prompt);
        
        if (fgets(buffer, tamanho, stdin) == NULL) {
            printf("Erro na leitura. Tente novamente.\n");
            limparBuffer();
            continue;
        }
        
        // Remove newline se existir
        buffer[strcspn(buffer, "\n")] = '\0';
        
        if (strlen(buffer) == 0) {
            printf("A entrada nao pode ser vazia. Tente novamente.\n");
            continue;
        }
        
        return 1;
    }
}

// Lê e valida número inteiro dentro de um intervalo
int lerInteiro(const char* prompt, int min, int max) {
    char input[50];
    long valor;
    char *endptr;
    
    while (1) {
        printf("%s", prompt);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Erro na leitura.\n");
            continue;
        }
        
        // Remove newline se existir
        input[strcspn(input, "\n")] = '\0';
        
        // Converte string para long
        valor = strtol(input, &endptr, 10);
        
        // Verifica se a conversão foi bem-sucedida
        if (endptr == input || *endptr != '\0') {
            printf("Por favor, digite um numero valido.\n");
            continue;
        }
        
        // Verifica overflow
        if (valor < INT_MIN || valor > INT_MAX) {
            printf("Numero fora dos limites.\n");
            continue;
        }
        
        // Verifica se está dentro dos limites
        if (valor >= min && valor <= max) {
            return (int)valor;
        }
        
        printf("Digite um numero entre %d e %d.\n", min, max);
    }
}

// ==================== GESTÃO DE TERRITÓRIOS ====================

// Aloca array de territórios
Territorio* criarTerritorios(int quantidade) {
    if (quantidade <= 0 || quantidade > 100) return NULL;
    return (Territorio*)calloc(quantidade, sizeof(Territorio));
}

// Libera array de territórios
void liberarTerritorios(Territorio* territorios) {
    free(territorios);
}

// Remove espaços de uma string
void removerEspacos(char* str) {
    if (!str) return;
    
    char *src = str, *dst = str;
    while (*src) {
        if (*src != ' ') *dst++ = *src;
        src++;
    }
    *dst = '\0';
}

// Cadastra dados de um território
void cadastrarTerritorio(Territorio* territorio, int indice) {
    if (!territorio) return;
    
    printf("\n--- Territorio %d ---\n", indice + 1);
    
    // Nome do território
    while (1) {
        if (!lerStringSegura(territorio->nome, sizeof(territorio->nome), 
                           "Nome do territorio: ")) continue;
        if (!validarString(territorio->nome, sizeof(territorio->nome) - 1)) {
            printf("Use apenas letras e espacos.\n");
            continue;
        }
        break;
    }
    
    // Cor do exército
    while (1) {
        if (!lerStringSegura(territorio->cor, sizeof(territorio->cor), 
                           "Cor do exercito: ")) continue;
        
        removerEspacos(territorio->cor);
        
        if (!validarString(territorio->cor, sizeof(territorio->cor) - 1)) {
            printf("Use apenas letras.\n");
            continue;
        }
        break;
    }
    
    // Quantidade de tropas
    territorio->tropas = lerInteiro("Quantidade de tropas: ", 1, 10000);
}

// Exibe informações de um território
void exibirTerritorio(const Territorio* territorio, int indice) {
    if (!territorio) return;
    printf("%2d. %-20s %-12s %4d tropas\n", 
           indice + 1, territorio->nome, territorio->cor, territorio->tropas);
}

// Exibe todos os territórios
void exibirTodosTerritorios(const Territorio* territorios, int quantidade) {
    if (!territorios || quantidade <= 0) return;
    
    printf("\n=== TERRITORIOS CADASTRADOS ===\n");
    printf("#  Nome                 Cor         Tropas\n");
    printf("--------------------------------------------\n");
    for (int i = 0; i < quantidade; i++) {
        exibirTerritorio(&territorios[i], i);
    }
}

// ==================== SISTEMA DE MISSÕES ====================

// Missões pré-definidas disponíveis
const char* missoesPredefinidas[] = {
    "Conquistar 3 territorios consecutivos",
    "Eliminar todas as tropas da cor vermelha", 
    "Controlar pelo menos 5 territorios",
    "Conquistar todos os territorios de uma cor",
    "Manter 10 tropas em um territorio"
};
const int totalMissoes = 5;

// Atribui missão aleatória ao jogador
int atribuirMissao(Jogador* jogador) {
    if (!jogador) return 0;
    
    int indice = rand() % totalMissoes;
    size_t tamanho = strlen(missoesPredefinidas[indice]) + 1;
    
    jogador->missao = (char*)malloc(tamanho);
    if (!jogador->missao) return 0;
    
    strncpy(jogador->missao, missoesPredefinidas[indice], tamanho);
    jogador->missao[tamanho - 1] = '\0';
    return 1;
}

// Exibe missão do jogador
void exibirMissao(const Jogador* jogador) {
    if (jogador && jogador->missao) {
        printf("MISSAO: %s\n", jogador->missao);
    }
}

// Verifica missão: 3 territórios consecutivos
int verificarMissaoConsecutivos(const Territorio* territorios, int quantidade, const Jogador* jogador) {
    if (!territorios || !jogador || quantidade < 3) return 0;
    
    for (int i = 0; i <= quantidade - 3; i++) {
        if (strcmp(territorios[i].cor, jogador->cor) == 0 &&
            strcmp(territorios[i+1].cor, jogador->cor) == 0 &&
            strcmp(territorios[i+2].cor, jogador->cor) == 0) {
            return 1;
        }
    }
    return 0;
}

// Verifica missão: eliminar cor vermelha
int verificarMissaoEliminarCor(const Territorio* territorios, int quantidade, const char* corAlvo) {
    if (!territorios || !corAlvo) return 0;
    
    for (int i = 0; i < quantidade; i++) {
        if (strcmp(territorios[i].cor, corAlvo) == 0) return 0;
    }
    return 1;
}

// Verifica missão: controlar 5 territórios
int verificarMissaoControlarTerritorios(const Territorio* territorios, int quantidade, const Jogador* jogador) {
    if (!territorios || !jogador) return 0;
    
    int count = 0;
    for (int i = 0; i < quantidade; i++) {
        if (strcmp(territorios[i].cor, jogador->cor) == 0) count++;
    }
    return count >= 5;
}

// Verifica missão: manter 10 tropas em um território
int verificarMissaoTropas(const Territorio* territorios, int quantidade, const Jogador* jogador) {
    if (!territorios || !jogador) return 0;
    
    for (int i = 0; i < quantidade; i++) {
        if (strcmp(territorios[i].cor, jogador->cor) == 0 && territorios[i].tropas >= 10) 
            return 1;
    }
    return 0;
}

// Verifica se missão foi cumprida
int verificarMissaoCumprida(const char* missao, const Territorio* territorios, int quantidade, const Jogador* jogador) {
    if (!missao || !territorios || !jogador || quantidade <= 0) return 0;
    
    if (strstr(missao, "3 territorios consecutivos")) 
        return verificarMissaoConsecutivos(territorios, quantidade, jogador);
    
    if (strstr(missao, "cor vermelha")) 
        return verificarMissaoEliminarCor(territorios, quantidade, "vermelho");
    
    if (strstr(missao, "5 territorios")) 
        return verificarMissaoControlarTerritorios(territorios, quantidade, jogador);
    
    if (strstr(missao, "10 tropas")) 
        return verificarMissaoTropas(territorios, quantidade, jogador);
    
    return 0;
}

// ==================== GESTÃO DE JOGADORES ====================

// Aloca array de jogadores
Jogador* criarJogadores(int quantidade) {
    if (quantidade <= 0 || quantidade > 10) return NULL;
    
    Jogador* jogadores = (Jogador*)calloc(quantidade, sizeof(Jogador));
    if (!jogadores) return NULL;
    
    // Inicializa missões como NULL
    for (int i = 0; i < quantidade; i++) {
        jogadores[i].missao = NULL;
    }
    
    return jogadores;
}

// Libera array de jogadores e suas missões
void liberarJogadores(Jogador* jogadores, int quantidade) {
    if (!jogadores) return;
    
    for (int i = 0; i < quantidade; i++) {
        free(jogadores[i].missao);
    }
    free(jogadores);
}

// Cadastra dados de um jogador
int cadastrarJogador(Jogador* jogador, int indice) {
    if (!jogador) return 0;
    
    printf("\n--- Jogador %d ---\n", indice + 1);
    
    // Nome do jogador
    if (!lerStringSegura(jogador->nome, sizeof(jogador->nome), "Nome do jogador: ")) {
        return 0;
    }
    
    // Cor do jogador
    while (1) {
        if (!lerStringSegura(jogador->cor, sizeof(jogador->cor), "Cor do jogador: ")) {
            return 0;
        }
        
        removerEspacos(jogador->cor);
        
        if (validarString(jogador->cor, sizeof(jogador->cor) - 1)) {
            break;
        }
        printf("Use apenas letras.\n");
    }
    
    jogador->territorios_conquistados = 0;
    
    // Atribui missão
    if (!atribuirMissao(jogador)) {
        return 0;
    }
    
    return 1;
}

// Exibe informações do jogador
void exibirJogador(const Jogador* jogador, int indice) {
    if (!jogador) return;
    printf("Jogador %d: %s (%s)\n", indice + 1, jogador->nome, jogador->cor);
    exibirMissao(jogador);
}

// ==================== SISTEMA DE ATAQUES ====================

// Valida se ataque pode ser realizado
int validarAtaque(const Territorio* atacante, const Territorio* defensor, const char* corJogador) {
    if (!atacante || !defensor || !corJogador) return 0;
    
    if (atacante->tropas <= 1) {
        printf("Atacante precisa de pelo menos 2 tropas!\n");
        return 0;
    }
    
    if (strcmp(atacante->cor, defensor->cor) == 0) {
        printf("Nao pode atacar territorio da mesma cor!\n");
        return 0;
    }
    
    if (strcmp(atacante->cor, corJogador) != 0) {
        printf("So pode atacar com seus proprios territorios!\n");
        return 0;
    }
    
    return 1;
}

// Simula rolagem de dado (1-6)
int simularDado() {
    return (rand() % 6) + 1;
}

// Processa vitória do atacante
void processarVitoriaAtacante(Territorio* atacante, Territorio* defensor) {
    if (!atacante || !defensor) return;
    
    int tropas_conquistadas = defensor->tropas / 2;
    if (tropas_conquistadas < 1) tropas_conquistadas = 1;
    
    printf("VITORIA! %s conquista %s\n", atacante->nome, defensor->nome);
    
    // Defensor muda para cor do atacante
    strncpy(defensor->cor, atacante->cor, sizeof(defensor->cor) - 1);
    defensor->cor[sizeof(defensor->cor) - 1] = '\0';
    
    defensor->tropas = tropas_conquistadas;
    atacante->tropas--;
    printf("Atacante perde 1 tropa. Restantes: %d\n", atacante->tropas);
}

// Processa vitória do defensor
void processarVitoriaDefensor(Territorio* atacante, Territorio* defensor) {
    if (!atacante || !defensor) return;
    
    printf("DEFESA BEM-SUCEDIDA! %s defendeu seu territorio\n", defensor->nome);
    atacante->tropas--;
    printf("Atacante perde 1 tropa. Restantes: %d\n", atacante->tropas);
}

// Processa empate
void processarEmpate(Territorio* atacante, Territorio* defensor) {
    if (!atacante || !defensor) return;
    
    printf("EMPATE! Ambos perdem tropas\n");
    atacante->tropas--;
    defensor->tropas--;
    printf("Atacante: %d tropas | Defensor: %d tropas\n", atacante->tropas, defensor->tropas);
}

// Executa ataque entre dois territórios
void executarAtaque(Territorio* atacante, Territorio* defensor) {
    if (!atacante || !defensor) return;
    
    printf("\n%s (%s) ataca %s (%s)\n", 
           atacante->nome, atacante->cor, defensor->nome, defensor->cor);
    
    int dadoAtaque = simularDado();
    int dadoDefesa = simularDado();
    
    printf("Dados: Atacante=%d | Defensor=%d\n", dadoAtaque, dadoDefesa);
    
    if (dadoAtaque > dadoDefesa) {
        processarVitoriaAtacante(atacante, defensor);
    } else if (dadoDefesa > dadoAtaque) {
        processarVitoriaDefensor(atacante, defensor);
    } else {
        processarEmpate(atacante, defensor);
    }
}

// ==================== CONTROLE PRINCIPAL DO JOGO ====================

// Estrutura que representa o estado do jogo
typedef struct {
    Territorio* territorios;
    Jogador* jogadores;
    int quantidade_territorios;
    int quantidade_jogadores;
    int turno_atual;
    int jogador_vez;
} Jogo;

// Cria nova instância do jogo
Jogo* criarJogo() {
    return (Jogo*)calloc(1, sizeof(Jogo));
}

// Inicializa jogo com territórios e jogadores
int inicializarJogo(Jogo* jogo) {
    if (!jogo) return 0;
    
    printf("=== CONFIGURACAO DO JOGO ===\n");
    
    jogo->quantidade_territorios = lerInteiro("Quantidade de territorios (1-50): ", 1, 50);
    jogo->quantidade_jogadores = lerInteiro("Quantidade de jogadores (1-10): ", 1, 10);
    
    jogo->territorios = criarTerritorios(jogo->quantidade_territorios);
    jogo->jogadores = criarJogadores(jogo->quantidade_jogadores);
    
    // Verifica se alocações foram bem-sucedidas
    if (!jogo->territorios || !jogo->jogadores) {
        liberarTerritorios(jogo->territorios);
        liberarJogadores(jogo->jogadores, jogo->quantidade_jogadores);
        return 0;
    }
    
    // Cadastra territórios
    printf("\n=== CADASTRO DE TERRITORIOS ===\n");
    for (int i = 0; i < jogo->quantidade_territorios; i++) {
        cadastrarTerritorio(&jogo->territorios[i], i);
    }
    
    // Cadastra jogadores
    printf("\n=== CADASTRO DE JOGADORES ===\n");
    for (int i = 0; i < jogo->quantidade_jogadores; i++) {
        if (!cadastrarJogador(&jogo->jogadores[i], i)) {
            return 0;
        }
        printf("---\n");
    }
    
    jogo->turno_atual = 1;
    jogo->jogador_vez = 0;
    return 1;
}

// Exibe menu de ataque para o turno atual
void exibirMenuAtaque(const Jogo* jogo) {
    if (!jogo) return;
    
    printf("\n=== TURNO %d - Jogador %s ===\n", 
           jogo->turno_atual, jogo->jogadores[jogo->jogador_vez].nome);
    exibirTodosTerritorios(jogo->territorios, jogo->quantidade_territorios);
    printf("\n--- ATAQUE ---\n");
}

// Verifica se algum jogador venceu
int verificarVitoria(Jogo* jogo) {
    if (!jogo || !jogo->jogadores || !jogo->territorios) return -1;
    
    for (int i = 0; i < jogo->quantidade_jogadores; i++) {
        if (jogo->jogadores[i].missao && 
            verificarMissaoCumprida(jogo->jogadores[i].missao, 
                                  jogo->territorios, 
                                  jogo->quantidade_territorios, 
                                  &jogo->jogadores[i])) {
            printf("\n*** VITORIA! ***\n");
            printf("%s cumpriu a missao: %s\n", 
                   jogo->jogadores[i].nome, jogo->jogadores[i].missao);
            return i;
        }
    }
    return -1;
}

// Executa um turno completo
void executarTurno(Jogo* jogo) {
    if (!jogo) return;
    
    exibirMenuAtaque(jogo);
    
    // Seleção de territórios
    int idxAtacante = lerInteiro("Selecione territorio ATACANTE: ", 1, jogo->quantidade_territorios) - 1;
    int idxDefensor = lerInteiro("Selecione territorio DEFENSOR: ", 1, jogo->quantidade_territorios) - 1;
    
    if (idxAtacante == idxDefensor) {
        printf("Nao pode atacar o proprio territorio!\n");
        return;
    }
    
    // Verifica índices válidos
    if (idxAtacante < 0 || idxAtacante >= jogo->quantidade_territorios ||
        idxDefensor < 0 || idxDefensor >= jogo->quantidade_territorios) {
        printf("Indices de territorio invalidos.\n");
        return;
    }
    
    Territorio* atacante = &jogo->territorios[idxAtacante];
    Territorio* defensor = &jogo->territorios[idxDefensor];
    
    if (!validarAtaque(atacante, defensor, jogo->jogadores[jogo->jogador_vez].cor)) {
        printf("Ataque invalido!\n");
        return;
    }
    
    executarAtaque(atacante, defensor);
    
    // Passa para próximo jogador
    jogo->jogador_vez = (jogo->jogador_vez + 1) % jogo->quantidade_jogadores;
    if (jogo->jogador_vez == 0) {
        jogo->turno_atual++;
    }
}

// Libera toda memória alocada
void liberarJogo(Jogo* jogo) {
    if (!jogo) return;
    
    liberarTerritorios(jogo->territorios);
    liberarJogadores(jogo->jogadores, jogo->quantidade_jogadores);
    free(jogo);
}

// ==================== PROGRAMA PRINCIPAL ====================

int main() {
    srand((unsigned int)time(NULL));
    
    printf("=== JOGO DE ESTRATEGIA ===\n");
    
    Jogo* jogo = criarJogo();
    if (!jogo) return 1;
    
    if (!inicializarJogo(jogo)) {
        liberarJogo(jogo);
        return 1;
    }
    
    // Loop principal do jogo
    char continuar;
    do {
        executarTurno(jogo);
        
        if (verificarVitoria(jogo) != -1) break;
        
        printf("\nContinuar jogando? (s/n): ");
        scanf(" %c", &continuar);
        limparBuffer();
        
    } while (continuar == 's' || continuar == 'S');
    
    liberarJogo(jogo);
    printf("\nFim do jogo! Obrigado por jogar!\n");
    
    return 0;
}