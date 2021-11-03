// Mario Garcia Marquez
// 77147974

#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"
#include <vector>

using namespace std ;
using namespace scd ;

// numero de fumadores 

const int num_fumadores = 3 ;

// Semaforos
// Estos semaforos empezaran inicializados a 0 e indicaran en la posicion i la cantidad de ingredientes i disponibles,
// en este caso el maximo valor sera 1 ya que solo puede haber un elemento en el mostrador
vector<Semaphore> ingredientes; 
// Indica cuando hay espacio en el mostrador, este semaforo esta inicializado al valor maximo del mostrador que es 1 en nuestro caso
Semaphore mostrador_vacio(1);

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
  while(true) {
    auto producido = producir_ingrediente();
    sem_wait(mostrador_vacio);
    cout << "Estanquero coloca ingrediente: " << producido << endl;
    sem_signal(ingredientes[producido]);
  }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      sem_wait(ingredientes[num_fumador]);
      cout << "Fumador " << num_fumador << " retira ingrediente" << endl;
      sem_signal(mostrador_vacio);
      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
  thread fumadores[num_fumadores];
  thread estanquero;

  // inicializamos el vector de ingredientes
  for(int i = 0; i < num_fumadores; i++)
    ingredientes.push_back(Semaphore(0));

  // Lanzamos fumadores
  for(int i = 0; i < num_fumadores; i++)
    fumadores[i] = thread(funcion_hebra_fumador, i);

  // Lanzamos estanquero
  estanquero = thread(funcion_hebra_estanquero);

  // hacemos join
  estanquero.join();
  for(int i = 0; i < num_fumadores; i++)
    fumadores[i].join();

  return 0;
}
