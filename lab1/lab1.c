/*
 *********************************************************************
 *																																	 *
 *	O codigo implementado apresenta duas estrategias para a divisao  *
 *	do vetor: divisao em blocos e passo alternado.									 *
 *	A estrategia utilizada varia a cada execucao, pois eh escolhida  *
 *	pelo valor de uma variavel randomica. 													 *
 *                                                                   *
 *********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NTHREADS 2
#define TAM 10000

int vetor[ TAM ];

// Funcao da thread - divide vetor em blocos
void * bloco( void * tid ) {
  int id = * ( int * ) tid;
  int ini, fim; 
  
  // Determina o bloco do vetor que sera iterado pela thread
  ini = id * TAM / 2;
  
  if ( id == 0 )
  	fim = 4999;
  else
  	fim = 9999;

  // Incrementa elementos do vetor
  for ( int cont = ini; cont <= fim; cont++ )
    vetor[ cont ] *= vetor[ cont ];

  pthread_exit( NULL );
}

// Funcao da thread - itera posicoes alternadas do vetor
void * alterna( void * tid ) {
  int id = * ( int * ) tid;

  // Itera pelo vetor com passo de tamanho 2
  for ( ;id < TAM; id += 2 )
    vetor[ id ] *= vetor[ id ];

  pthread_exit( NULL );
}

int main( void ) {

  // Inicializacao do vetor. Todas as posicoes do vetor recebem o mesmo valor
  for ( int cont = 0; cont < TAM; cont++ )
    vetor[ cont ] = 2;

  // Exibe vetor inicial
  printf( "\nVetor inicial:\n");
  for ( int cont = 0; cont < TAM; cont++ )
    printf( "%d ", vetor[ cont ] );
  printf( "\n\n" );

  // Variaveis identificadoras das threads no sistena e localmente
  pthread_t threads[ NTHREADS ];
  int id[ NTHREADS ];

  // Variavel para escolher qual estrategia de divisao da tareda sera usada
  srand(time(NULL));
  int escolha = rand() % 2;

  if ( escolha == 0 ) {  // Escolhe estrategia de divisao em blocos
    printf( "Estrategia escolhida: divisao em blocos\n\n" );
    // Cria threads
    for ( int cont = 0; cont < NTHREADS; cont++ ) {
      id[ cont ] = cont;
      if ( pthread_create( &threads[ cont ], NULL, bloco, ( void * ) &id[ cont ] ) ) {
        printf( "[!] ERROR: pthread_create()\n" );
        exit( -1 );
      }
    }
  } else { // Escolhe estrategia de passo alternado
    printf( "Estrategia escolhida: passo alternado\n\n" );
    // Cria threads
    for ( int cont = 0; cont < NTHREADS; cont++ ) {
      id[ cont ] = cont;
      if ( pthread_create( &threads[ cont ], NULL, alterna, ( void * ) &id[ cont ] ) ) {
        printf( "[!] ERROR: pthread_create()\n" );
        exit( -1 );
      }
    }
  }

  // Espera threads executarem
  for ( int cont = 0; cont < NTHREADS; cont++ ) {
    if ( pthread_join( threads[ cont ], NULL ) ) {
      printf( "[!] ERROR: pthread_join()\n" );
      exit( -1 );
    }
  }
	
  // Exibe vetor final
  printf( "Vetor final:\n" );
  for ( int cont = 0; cont < TAM; cont++ )
    printf( "%d ", vetor[ cont ] );
  printf( "\n\n" );

  pthread_exit( NULL );
  return 0;
}
