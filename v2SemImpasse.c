// SIMULAÇÃO DE CRESCIMENTO DE COLÔNIAS DE BACTÉRIAS (SEM IMPASSE)

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>

#define NUM_COLONIAS 7              // número de colônias (threads)
#define NUM_RECURSO_NUTRIENTES 3    // número de nutrientes (primeiro recurso)
#define NUM_RECURSO_ESPACO 3        // número de espaço (segundo recurso)

pthread_mutex_t recursoNutrientes[NUM_RECURSO_NUTRIENTES];  // array de mutexes para nutrientes (primeiro recurso)
pthread_mutex_t recursoEspaco[NUM_RECURSO_ESPACO];          //array de mutexes para espaço (segundo recurso)
int recursosAdquiridos[NUM_COLONIAS] = {0};                 // array para monitorar os recursos adquiridos de cada colônia

// estrutura que armazena os argumentos para cada colônia
typedef struct {
    int id;     // identificador da colônia (thread)
    int P0;     // população inicial da colônia
    float r;    // taxa de crescimento da colônia
    int t;      // tempo de simulação
} colonia;

// prototipação
void* executarColonia(void*); 
int adquirirRecurso(pthread_mutex_t[], int, char*, int);
float calcularCrescimento(int, float, int);
void* monitorarImpasses();

int main() {
    pthread_t colonias[NUM_COLONIAS];       // array de threads representando as colônias
    colonia colonias_args[NUM_COLONIAS];    // array de argumentos para as colônias
    pthread_t monitor_thread;               // thread para monitoramento de impasses (deadlocks)

    // inicializa os mutexes de recursos
    for (int i = 0; i < NUM_RECURSO_NUTRIENTES; i++) {
        pthread_mutex_init(&recursoNutrientes[i], NULL);    // inicializa o mutex do recurso nutrientes
    }
    for (int i = 0; i < NUM_RECURSO_ESPACO; i++) {
        pthread_mutex_init(&recursoEspaco[i], NULL);        // inicializa o mutex do recurso espaço
    }

    // cria as threads para cada colônia e passa seus argumentos
    for (int i = 0; i < NUM_COLONIAS; i++) {
        colonias_args[i].id = i;            // identificador da colônia (thread)
        
        // preenche os argumentos da colônia estaticamente
        colonias_args[i].P0 = 10;           // população inicial da colônia (pode ser alterada)
        colonias_args[i].r = 0.2;           // taxa de crescimento da colônia (pode ser alterada)
        colonias_args[i].t = 5;             // tempo total de simulação (pode ser alterado)

        // solicita ao usuário preencher os argumentos da colônia
        // printf("\nCOLÔNIA %d:\n", i);
        // printf("\tInsira sua população inicial: ");
        // scanf("%d", &colonias_args[i].P0);
        // printf("\tInsira sua taxa de crescimento: ");
        // scanf("%f", &colonias_args[i].r);
        // printf("\tInsira o tempo para simulação: ");
        // scanf("%d", &colonias_args[i].t);
    }

    printf("\nEXECUTANDO...\n\n");

    // cria as threads das colônias e inicia suas execuções 
    //(precisa ser feito separado caso o usuário preencha os argumentos da colônia)
    for (int i = 0; i < NUM_COLONIAS; i++) {
        pthread_create(&colonias[i], NULL, executarColonia, (void *)&colonias_args[i]);   
    }

    // cria a thread de monitoramento para deadlock
    pthread_create(&monitor_thread, NULL, monitorarImpasses, NULL);

    // aguarda todas as threads terminarem
    for (int i = 0; i < NUM_COLONIAS; i++) {
        pthread_join(colonias[i], NULL);    // espera a thread correspondente terminar
    }

    // destrói os mutexes após o uso 
    for (int i = 0; i < NUM_RECURSO_NUTRIENTES; i++) {
        pthread_mutex_destroy(&recursoNutrientes[i]);    // destrói o mutex do recurso nutrientes
    }
    for (int i = 0; i < NUM_RECURSO_ESPACO; i++) {
        pthread_mutex_destroy(&recursoEspaco[i]);        // destrói o mutex do recurso espaço
    }

    pthread_cancel(monitor_thread);     // cancela a thread de monitoramento quando as colônias terminam
    pthread_join(monitor_thread, NULL);
    
    return 0;
}

// função executada para cada colônia (thread)
void* executarColonia(void* colonia_args) {
    colonia* ca = (colonia*)colonia_args;   // converte o argumento para a estrutura de colônia
    int id = ca->id;                        // armazena o id da colônia
    int recursoNutrientes_index = -1;       // índice do recurso de nutrientes
    int recursoEspaco_index = -1;           // índice do recurso de espaço

    // colônia tenta adquirir os recursos em MESMA ORDEM (evitando impasses)
    printf("Colônia %d tentando obter recurso de nutrientes primeiro\n", id);      
    recursoNutrientes_index = adquirirRecurso(recursoNutrientes, NUM_RECURSO_NUTRIENTES, "Nutrientes", id);     // trava o mutex do recurso de nutrientes (obtém o recurso de nutrientes)
    sleep(1);                                       // simula o tempo de processamento ou espera

    printf("Colônia %d tentando obter recurso de espaço\n", id);
    recursoEspaco_index = adquirirRecurso(recursoEspaco, NUM_RECURSO_ESPACO, "Espaço", id);                     // trava o mutex do recurso de espaço (obtém o recurso de espaço)
    
    // simula o crescimento da colônia ao longo do tempo
    for (int ta = 0; ta <= ca->t; ta++) {
        float populacao = calcularCrescimento(ca->P0, ca->r, ta);                 // calcula a população no tempo t
        printf("\t\t\t\t\t\t\tColônia %d, Tempo %d: População %.2f\n", id, ta, populacao);     // exibe a população da colônia
        sleep(1);                                                               // simula o tempo de processamento (espera 1 segundo)
    }

    // libera os recursos de nutrientes e espaço após o uso
    pthread_mutex_unlock(&recursoNutrientes[recursoNutrientes_index]);      // destrava o mutex do recurso de nutrientes (libera o recurso de nutrientes)
    recursosAdquiridos[id]--;                                               // marca que um recurso foi liberado
    printf("Colônia %d liberou o recurso Nutrientes[%d]\n", id, recursoNutrientes_index);

    pthread_mutex_unlock(&recursoEspaco[recursoEspaco_index]);              // destrava o mutex do recurso de espaço (libera o recurso de espaço)
    recursosAdquiridos[id]--;                                               // marca que outro recurso foi liberado
    printf("Colônia %d liberou o recurso Espaço[%d]\n", id, recursoEspaco_index);

    pthread_exit(NULL);     // termina a execução da thread
}

// função para adquirir um recurso (mutex) 
int adquirirRecurso(pthread_mutex_t recurso[], int num_recursos, char* tipo_recurso, int id) {
    int index = -1;     // variável para armazenar o índice do vetor do recurso

    // loop para tentar obter o recurso, só é quebrado quando o obtém (por isso o programa ocupa muita CPU)
    while (index == -1) {
        // percorre todos os índices do recurso tentando obtê-lo (caso esteja disponível)
        for (int i = 0; i < num_recursos; i++) { 
            if (pthread_mutex_trylock(&recurso[i]) == 0) {  // tenta obter o recurso (mutex)
                index = i;
                printf("Colônia %d obteve o recurso %s[%d]\n", id, tipo_recurso, i);
                recursosAdquiridos[id]++;       // marca que um recurso foi adquirido
                return index;                   // retorna o índice do recurso obtido
            }
        }
    }

    return -1;  // retorna -1 caso não consiga obter o recurso (não deve ocorrer)
}

// função que calcula o crescimento exponencial da colônia
float calcularCrescimento(int P0, float r, int t) {
    return P0 * exp(r * t);     // fórmula do crescimento exponencial -> P(t) = P0 * eˆ(r * t)
}

// função para monitorar impasses (deadlocks)
void* monitorarImpasses() {
    // loop infinito, para monitorar durante toda a exeução do programa
    while (1) {
        sleep(5);          // faz uma pausa para permitir que as colônias tentem obter recursos
        
        int semCrescimentoGeral = 1;    // variável para identificar se nenhuma colônia cresceu por um tempo 
        
        // verifica o progresso de todas as colônias
        for (int i = 0; i < NUM_COLONIAS; i++) {
            if (recursosAdquiridos[i] == 2) {   // verifica se a colônia adquiriu todos os recursos
                semCrescimentoGeral = 0;        // se adquiriu, marca que houve crescimento
            } 
        }

        sleep(5); 
        
        // após um tempo, executa a mesma verificação para garantir que, se houver um impasse, seja verdadeiro
        for (int i = 0; i < NUM_COLONIAS; i++) {
            if (recursosAdquiridos[i] == 2) {
                semCrescimentoGeral = 0;   
            } 
        }

        // se nenhuma colônia cresceu por um tempo, exibe uma mensagem de possível deadlock
        if (semCrescimentoGeral) {
            printf("* POSSÍVEL IMPASSE DETECTADO! Nenhuma colônia teve crescimento por um tempo\n");
        } 
        
    }
    pthread_exit(NULL);     // finaliza a thread de monitoramento (como loop infinito, nunca é executado)
}