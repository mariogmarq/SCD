// Mario Garcia Marquez
// 77147974

// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.


#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

constexpr int
   num_productores = 10,
   num_consumidores = 10,
   num_items  = 40 ;     // número de items a producir/consumir

mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items], // contadores de verificación: producidos
   cont_cons[num_items], // contadores de verificación: consumidos
   cont_hebras[num_productores] = {0}; // contadores de verificación: hebras 

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(int i)
{
   int v = i *(num_items/num_productores) + cont_hebras[i] ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "producido " << i <<": " << v << endl << flush ;
   mtx.unlock();
   cont_prod[v] ++ ;
   cont_hebras[i] ++ ;
   return v;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   if ( num_items <= dato )
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   mtx.lock();
   cout << "                  consumido: " << dato << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void ini_contadores()
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  cont_prod[i] = 0 ;
      cont_cons[i] = 0 ;
   }
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << flush ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SC, un prod. y un cons.

class ProdCons2SU: public HoareMonitor
{
 private:
 static const int           // constantes:
   num_celdas_total = 10;   //  núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//  buffer de tamaño fijo, con los datos
   primera_ocupada,
   celdas_ocupadas,
   multiplos_de_5,
   multiplos_de_5_en_impresora,
   primera_libre ;          //  indice de celda de la próxima inserción
 CondVar         // colas condicion:
   impresora,
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdCons2SU(  ) ;           // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
   bool metodo_impresora();
} ;
// -----------------------------------------------------------------------------

ProdCons2SU::ProdCons2SU(  )
{
   ocupadas = newCondVar();
   libres   = newCondVar();
   impresora = newCondVar();
   primera_libre = 0 ;
   celdas_ocupadas = 0;
   primera_ocupada = 0;
   multiplos_de_5 = 0;
   multiplos_de_5_en_impresora = 0;
}
// -----------------------------------------------------------------------------

bool ProdCons2SU::metodo_impresora() {
   if(multiplos_de_5 == num_items / 5)
      return false;

   if(multiplos_de_5 - multiplos_de_5_en_impresora == 0)
      impresora.wait();
   
   assert(multiplos_de_5 - multiplos_de_5_en_impresora > 0);
   cout << "Impresora: " << multiplos_de_5 - multiplos_de_5_en_impresora 
      << " nuevos multiplos de 5" << endl;

   multiplos_de_5_en_impresora = multiplos_de_5;

   return true;
}

// función llamada por el consumidor para extraer un dato

int ProdCons2SU::leer(  )
{
   // esperar bloqueado hasta que 0 < num_celdas_ocupadas
   if ( celdas_ocupadas == 0 )
      ocupadas.wait();

   // hacer la operación de lectura, actualizando estado del monitor
   assert( 0 < celdas_ocupadas  );
   const int valor = buffer[primera_ocupada] ;
   celdas_ocupadas -- ;
   primera_ocupada ++;
   primera_ocupada %= num_celdas_total ;


   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdCons2SU::escribir( int valor )
{
   // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total
   if ( celdas_ocupadas == num_celdas_total )
      libres.wait();

   //cout << "escribir: ocup == " << num_celdas_ocupadas << ", total == " << num_celdas_total << endl ;
   assert( celdas_ocupadas < num_celdas_total );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ;
   primera_libre++ ;
   primera_libre %= num_celdas_total ;
   celdas_ocupadas ++ ;

   if(valor%5 == 0) {
      multiplos_de_5++;
      impresora.signal();
   }

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<ProdCons2SU> monitor, int id)
{
   int p = num_items/num_productores ;
   for( unsigned i =  0; i < p ; i++ )
   {
      int valor = producir_dato(id) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<ProdCons2SU> monitor )
{
   for( unsigned i = 0 ; i < num_items/num_consumidores ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------
void funcion_hebra_impresora(MRef<ProdCons2SU> monitor) {
   do{

   }while(monitor->metodo_impresora());
}


int main()
{
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (impresora, Monitor SU, buffer FIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush ;

   MRef<ProdCons2SU> monitor = Create<ProdCons2SU>();

   thread hebras_productoras[num_productores], hebras_consumidoras[num_consumidores] ;
   thread hebra_impresora(funcion_hebra_impresora, monitor);

   for(int i = 0; i < num_productores; i++)
      hebras_productoras[i] = thread( funcion_hebra_productora, monitor, i ) ;
   
   for(int i = 0; i < num_consumidores; i++)
      hebras_consumidoras[i] = thread( funcion_hebra_consumidora, monitor ) ;

   for(int i = 0; i < num_productores; i++)
      hebras_productoras[i].join() ;
   
   for(int i = 0; i < num_consumidores; i++)
      hebras_consumidoras[i].join() ;
   
   hebra_impresora.join();
   // comprobar que cada item se ha producido y consumido exactamente una vez
   test_contadores() ;
}
