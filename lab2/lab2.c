#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "timer.h"

// Vetores que representarao as matrizes
int *aMatriz, *bMatriz, *sequencialResultadoMatriz, *concorrenteResultadoMatriz;

// Numero de threads e dimensao das matrizes
int numeroThreads, dimensao;

// Funcao que gera matrizes aleatorias
void criaMatriz (int *matriz) {
    for (int linhaContador = 0; linhaContador < dimensao; linhaContador++)
        for (int colunaContador = 0; colunaContador < dimensao; colunaContador++)
            matriz[ linhaContador * dimensao + colunaContador ] = rand();
}

// DEBUG - Imprime matriz
void imprimeMatriz (int *matriz) {
    for (int linhaContador = 0; linhaContador < dimensao; linhaContador++) {
        for (int colunaContador = 0; colunaContador < dimensao; colunaContador++) {
            printf("%d ", matriz[ linhaContador * dimensao + colunaContador ] );
        }
        printf("\n");
    }
}

// Compara resultados dos dois algoritmos
void confereResultado ( ) {
    // Variavel que indica se a solucao esta correta
    int erro = 0;
    int linhaContador, colunaContador;

    for (linhaContador = 0; linhaContador < dimensao; linhaContador++) {
        for (colunaContador = 0; colunaContador < dimensao; colunaContador++) {
            // Se encontra posicoes diferentes nas matrizes, seta variavel erro e quebra for aninhado
            if ( sequencialResultadoMatriz[ linhaContador * dimensao + colunaContador ] != concorrenteResultadoMatriz[ linhaContador * dimensao + colunaContador ] )
            erro = 1;
            break;
        }

        // Se erro != 0, imprime mensagem de erro e sai do loop
        if (erro) {
            printf("Resultado incorreto: linha %d, coluna %d", linhaContador, colunaContador);
            break;
        }
    }

    if (!erro)
        printf("\nResultado correto!\n");
}

// Algoritmo sequencial de multiplicacao de matrizes
void multSequencial ( ) {
    int auxiliar = 0; // Variavel auxiliar da multiplicacao

    for (int linhaContador = 0; linhaContador < dimensao; linhaContador++) {
        //printf("\n"); // DEBUG
        for (int colunaContador = 0; colunaContador < dimensao; colunaContador++) {
            //printf("LINHA %d COLUNA %d\n", linhaContador, colunaContador);// DEBUG
            for (int contadorAuxiliar = 0; contadorAuxiliar < dimensao; contadorAuxiliar++) {
                // Calcula valor da celula
                auxiliar += aMatriz[ linhaContador * dimensao + contadorAuxiliar ] * bMatriz[ contadorAuxiliar * dimensao + colunaContador ];
            }

            // Celula recebe valor e zera variavel auxiliar
            sequencialResultadoMatriz[ linhaContador * dimensao + colunaContador ] = auxiliar;
            auxiliar = 0;
        }
    }
}

// Algoritmo concorrente de multiplicacao de matrizes
void * multConcorrente (void *tID) {
    int id = * (int *) tID; // ID da thread, utilizada para alternar as linhas
    //printf("\nThread %d\n", id); // DEBUG
    int auxiliar = 0; // Variavel auxiliar da multiplicacao

    for (int linhaContador = id; linhaContador < dimensao; linhaContador += numeroThreads) {
        for (int colunaContador = 0; colunaContador < dimensao; colunaContador++) {
            for (int contadorAuxiliar = 0; contadorAuxiliar < dimensao; contadorAuxiliar++) {
                // Calcula valor da celula
                auxiliar += aMatriz[ linhaContador * dimensao + contadorAuxiliar] * bMatriz[ contadorAuxiliar * dimensao + colunaContador ];
            }

            // Celula recebe valor e zera variavel auxiliar
            concorrenteResultadoMatriz[ linhaContador * dimensao + colunaContador ] = auxiliar;
            auxiliar = 0;
        }
    }

    // Libera ponteiro do argumento de ID da thread e encerra thread
    free(tID);
    pthread_exit(NULL);
}

int main (int argc, char const *argv[]) {

    // Confere argumentos de linha de comando
    if ( argc < 3) {
        printf("Execucao do programa: %s <numero de threads> <dimensao da matriz>\n\n", argv[0]);
        exit(-1);
    }

    numeroThreads = atoi(argv[1]);
    dimensao = atoi(argv[2]);

    // Alocacao de memoria para as matrizes
    aMatriz = malloc (sizeof(int *) * dimensao * dimensao);
    if (aMatriz == NULL) {
        printf("ERRO: malloc(aMatriz)\n");
        exit(-1);
    }

    bMatriz = malloc (sizeof(int *) * dimensao * dimensao);
    if (bMatriz == NULL) {
        printf("ERRO: malloc(bMatriz)\n");
        exit(-1);
    }

    sequencialResultadoMatriz = malloc (sizeof(int *) * dimensao * dimensao);
    if (sequencialResultadoMatriz == NULL) {
        printf("ERRO: malloc(sequencialResultadoMatriz)\n");
        exit(-1);
    }

    concorrenteResultadoMatriz = malloc (sizeof(int *) * dimensao * dimensao);
    if (concorrenteResultadoMatriz == NULL) {
        printf("ERRO: malloc(concorrenteResultadoMatriz)\n");
        exit(-1);
    }

    // Declaracao e alocacao de memoria para identificadores das threads no sistema
    pthread_t *sistemaThreads;

    sistemaThreads = malloc (sizeof(pthread_t *) * numeroThreads);
    if (sistemaThreads == NULL) {
        printf("ERRO: malloc(sistemaThreads)\n");
        exit(-1);
    }

    // Semeia random e inicializa matrizes de entrada
    srandom(time(NULL));

    criaMatriz(aMatriz);
    // DEBUG
    //printf("MATRIZ A:\n");
    //imprimeMatriz(aMatriz);

    criaMatriz(bMatriz);
    // DEBUG
    //printf("\nMATRIZ B:\n");
    //imprimeMatriz(bMatriz);

    // Declaracao das variaveis de tomada de tempo
    double inicio, fim, sequencialDuracao, concorrenteDuracao;

    // Calculo da multiplicacao pelo algoritmo sequencial e tomada de tempo
    GET_TIME(inicio);
    multSequencial();
    GET_TIME(fim);

    sequencialDuracao = fim - inicio;

    // DEBUG
    //printf("\nMATRIZ SEQUENCIAL:\n");
    //imprimeMatriz(sequencialResultadoMatriz);

    // Declaracao de ponteiro para passagem de argumentos
    int *threadID;

    // Criacao das threads e tomada de tempo
    GET_TIME(inicio);
    for (int contador = 0; contador < numeroThreads; contador++) {
        // Aloca memoria para argumento
        threadID = malloc (sizeof(int));

        if (threadID == NULL) {
            printf("ERRO: malloc(threadID)\n");
            exit(-1);
        }

        // Conteudo do ponteiro recebe argumento
        *threadID = contador;

        // Cria threads
        if ( pthread_create(&sistemaThreads[ contador ], NULL, multConcorrente, (void *) threadID) ) {
            printf("ERRO: pthread_create()\n");
            exit(-1);
        }
    }

    // Main aguarda as threads finalizarem a execucao
    for ( int contador = 0; contador < numeroThreads; contador++) {
        if ( pthread_join(sistemaThreads[ contador ], NULL) ) {
            printf("ERRO: pthread_join()\n");
            exit(-1);
        }
    }
    GET_TIME(fim);

    concorrenteDuracao = fim - inicio;

    // DEBUG
    //printf("\nMATRIZ CONCORRENTE:\n");
    //imprimeMatriz(concorrenteResultadoMatriz);

    confereResultado();

    printf( "\n" );
    printf( "Numero de threads = %d\tTamanho das matrizes = %d x %d\n", numeroThreads, dimensao, dimensao );
    printf( "Tempo sequencial = %fs\n", sequencialDuracao );
    printf( "Tempo concorrente = %fs\n", concorrenteDuracao );
    printf( "Ganho de desempenho = %f\n\n", sequencialDuracao / concorrenteDuracao );

    free(aMatriz);
    free(bMatriz);
    free(sequencialResultadoMatriz);
    free(concorrenteResultadoMatriz);
    free(sistemaThreads);

    return 0;
}
