// Mario Garcia Marquez 77147974V
// Grupo practicas jueves

/*
 * Respuesta a las preguntas del ejercicio
 * 1 - Cual es el minimo tiempo de espera que queda al final de las iteraciones
 *     del ciclo secundario con tu solucion?
 * R - El minimo es el restante en el primer ciclo secundario que es de 10ms, pues
 *     cada ciclo secundario dura 500ms y en este tienen lugar la tarea A, B y C
 *     de 100ms, 150ms y 240ms durando entre las 3 490ms.
 *
 * 2 - Seria planificable si la tarea D tuviese un tiempo de computo de 250ms?
 * R - Si la tarea D durase 250ms entonces seguiria siendo planificable con el mismo
 *     esquema propuesto en este ejercicio. Sin embargo llenaria completamente el ciclo
 *     secundario, cosa que no debe de suponer ningun problema en un escenario controlado
 *     como este, sin embargo en la realidad hay que tener esto en cuanto de cara a que
 *     la duracion real de las tareas pueden no ser exactamente estas y en caso de que
 *     alguna tarea durase mas tiempo que el planteado el sistema sufriria retrasos.
 */

// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo2.cpp
//
//   Datos de las tareas:
//   ------------
//   Ta.  T    C
//   ------------
//   A  500   100
//   B  500   150
//   C  1000  200
//   D  2000  240
//  -------------
//
//  Planificación (con Ts == 500 ms)
//  *---------*----------*---------*--------*
//  | A B D   | A B C    | A B     | A B C  |
//  *---------*----------*---------*--------*
//
//
// -----------------------------------------------------------------------------

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
//typedef duration<float,ratio<1,1>>    seconds_f ;
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:

void TareaA() { Tarea( "A", milliseconds(100) );  }
void TareaB() { Tarea( "B", milliseconds( 150) );  }
void TareaC() { Tarea( "C", milliseconds( 200 ));  }
void TareaD() { Tarea( "D", milliseconds( 240) );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario (en unidades de milisegundos, enteros)
   const milliseconds Ts_ms( 500 );

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now();

   while( true ) // ciclo principal
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;

      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;

         switch( i )
         {
            case 1 : TareaA(); TareaB(); TareaD();           break ;
            case 2 : TareaA(); TareaB(); TareaC();           break ;
            case 3 : TareaA(); TareaB();                     break ;
            case 4 : TareaA(); TareaB(); TareaC();           break ;
         }

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts_ms ;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );
        auto diff = steady_clock::now() - ini_sec;
         cout << endl << "Retraso de " << milliseconds_f(diff).count() << "ms" << endl;
      }
   }
}
