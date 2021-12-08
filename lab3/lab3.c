#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

long long int dimensao;                         // Dimensao do vetor
float *vetor, limiteInferior, limiteSuperior;   // Vetor e variaveis de limites
int numeroThreads;                              // Numero de threads

// Gera vetor aleatorio. Cada elemento do vetor esta entre 0 e 1000
void inicializaoVetor ( ) {
    srand( time(NULL) );    // Semente do pseudorandom

    // Loop que gera cada elemento do vetor
    for ( long long int contador = 0; contador < dimensao; contador++ )
        vetor[ contador ] = (float) rand( ) / (float) ( RAND_MAX / 1000. );

    // DEBUG
    // printf( "\n ________ VETOR ________\n" );
    // printf( "|\t\t\t|\n");
    // for ( long long int contador = 0; contador < dimensao; contador++ )
    //     printf( "|\t%f\t|\n", vetor[ contador ]);
    // printf( "|_______________________|\n" );
}

// Algoritmo sequencial para encontrar os valores dentro da faixa informada
int verificaSequencial ( ) {
    long long int quantidade = 0;   // Variavel que armazena resultado

    // Loop que percorre o vetor
    for ( long long int contador = 0; contador < dimensao; contador++ )

        // Se elemento vetor[ contador ] estiver na faixa informada...
        if ( limiteInferior < vetor[ contador ] && vetor[ contador ] < limiteSuperior )
            quantidade++;   // ... incrementa quantidade encontrada

    return quantidade;  // Retorna resultado
}

// Algoritmo concorrente para encontrar valores na faixa informada
void * verificaConcorrente ( void * tID ) {
    long int threadID = ( long int ) tID;                   // ID da thread
    long long int somaLocal = 0;                            // Variavel que armazena quantidade encontrada pela thread
    long long int tamanhoBloco = dimensao / numeroThreads;  // Tamanho do bloco do vetor percorrido pela thread
    long long int inicio = threadID * tamanhoBloco;         // Posicao que a thread começa a percorrer vetor
    long long int fim;                                      // Thread percorre vetor ate a posicao fim - 1

    if ( threadID < numeroThreads - 1 ) // Se a thread nao for a ultima...
        fim = inicio + tamanhoBloco;    // ... vai ate posicao inicio + tamanhoBloco - 1, ...
    else                                // ... senao ...
        fim = dimensao;                 // ... vai ate final do vetor

    // Percorre bloco do vetor
    for ( ; inicio < fim; inicio++ )
        // Se vetor[ inicio ] estiver na faixa informada...
        if ( limiteInferior < vetor[ inicio ] && vetor[ inicio ] < limiteSuperior )
            somaLocal++;    // ... incrementa variavel que armazena quantidade encontrada pela thread

    pthread_exit( (void *) somaLocal ); // Retorna quantidade encontrada pela thread
}

// MAIN
int main( int argc, char const *argv[] ) {

    // Verifica se a chamada do programa esta correta
    if ( argc < 3 ) {
        fprintf( stderr, "Utilizacao do programa: %s <tamanho do vetor> <numero de threads>\n\n", argv[0] );
        exit( -1 );
    }

    dimensao = atoll( argv[1] );        // Dimensao do vetor
    numeroThreads = atoi( argv[2] );    // Numero de threads

    // printf( "Insira o limite inferior: " ); // Pede input do limite inferior
    // scanf( "%f", &limiteInferior );
    // printf( "Insira o limite superior: " ); // Pede input do limite superior
    // scanf( "%f", &limiteSuperior );

    limiteInferior = 0.;
    limiteSuperior = 1000.;

    // Aloca memoria para vetor e testa se nao houve erros
    vetor = (float *) malloc( sizeof( float ) * dimensao );
    if ( vetor == NULL ) {
        fprintf( stderr, "ERRO: malloc(vetor)\n" );
        exit( -1 );
    }

    // Gera vetor aleatorio
    inicializaoVetor( );

    // Variaveis de tomada de tempo
    double inicio, fim, tempoSequencial, tempoConcorrente;
    // Variaveis que armazenam resultados
    long long int resultadoSequencial, resultadoThread, resultadoTotalConcorrente = 0;

    GET_TIME( inicio );                             // Tempo de inicio do algoritmo sequencial
    resultadoSequencial = verificaSequencial( );    // Executa algoritmo sequencial
    GET_TIME( fim );                                // Tempo final do algoritmo sequencial

    tempoSequencial = fim - inicio;                 // Duracao do algoritmo sequencial

    GET_TIME( inicio );                             // Tempo de inicio do algoritmo concorrente

    // Aloca memoria para identificadores de threads e verifica se houve erro
    pthread_t * sistemaThreads = ( pthread_t * ) malloc( sizeof( pthread_t ) * numeroThreads );
    if ( sistemaThreads == NULL ) {
        fprintf( stderr, "ERRO: malloc(sistemaThreads);\n" );
        exit( -1 );
    }

    // Cria threads
    for ( long int contador = 0; contador < numeroThreads; contador++ ) {
        if ( pthread_create( &sistemaThreads[ contador ], NULL, verificaConcorrente, (void *) contador ) ) {
            fprintf( stderr, "ERRO: pthread_create(%ld)\n", contador );
            exit( -1 );
        }
    }

    // Aguarda finalizacao das threads
    for ( int contador = 0; contador < numeroThreads; contador++) {
        if ( pthread_join( sistemaThreads[ contador ], (void **) &resultadoThread  ) ) {
            printf( "ERRO: pthread_join()\n" );
            exit( -1 );
        }

        // Calcula total encontrado pelo algoritmo concorrente
        resultadoTotalConcorrente += resultadoThread;
    }

    // Tempo final de execucao do algoritmo concorrente
    GET_TIME( fim );

    // Duracao da execucao concorrente
    tempoConcorrente = fim - inicio;

    // Apresentação de resultados
    printf( "\n\n\t\t\tALGORITMO SEQUENCIAL\n\n");
    printf( "Algoritmo sequencial encontrou %lld valores na faixa informada\n", resultadoSequencial );
    printf( "\t\t\t TEMPO = %fs\n\n\n\n", tempoSequencial );
    printf( "\t\t\tALGORITMO CONCORRENTE\n\n");
    printf( "Algoritmo concorrente encontrou %lld valores na faixa informada\n", resultadoTotalConcorrente );
    printf( "\t\t\t TEMPO = %fs\n", tempoConcorrente );
    printf( "\n\n\t\t\tGANHO DE DESEMPENHO = %f\n\n", tempoSequencial / tempoConcorrente );

    // Libera memoria alocada
    free( vetor );
    free( sistemaThreads );

    return 0;
}
