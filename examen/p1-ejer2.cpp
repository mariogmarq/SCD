// Mario Garcia Marquez
// 77147974
#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std;
using namespace scd;

//**********************************************************************
// Variables globales
const int num_cons = 14, num_prods = 16;
const unsigned
    num_items = num_cons * num_prods, // número de items
    tam_vec = 10;                     // tamaño del buffer
unsigned
    cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
    cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
    siguiente_dato = 0;         // siguiente dato a producir en 'producir_dato' (solo se usa ahí)

int vec[tam_vec];
Semaphore leerDeVec(0), escribirEnVec(tam_vec), producir(1), consumir(1);

int siguiente_a_leer = 0, siguiente_a_escribir = 0;

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));

   producir.sem_wait();

   const unsigned dato_producido = siguiente_dato;
   siguiente_dato++;
   cont_prod[dato_producido]++;

   producir.sem_signal();

   cout << "producido: " << dato_producido << endl
        << flush;
   return dato_producido;
}
//----------------------------------------------------------------------

void consumir_dato(unsigned dato)
{
   assert(dato < num_items);
   cont_cons[dato]++;
   this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));

   cout << "                  consumido: " << dato << endl;
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true;
   cout << "comprobando contadores ....";
   for (unsigned i = 0; i < num_items; i++)
   {
      if (cont_prod[i] != 1)
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl;
         ok = false;
      }
      if (cont_cons[i] != 1)
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl;
         ok = false;
      }
   }
   if (ok)
      cout << endl
           << flush << "solución (aparentemente) correcta." << endl
           << flush;
}

//----------------------------------------------------------------------

void funcion_hebra_productora()
{
   // N / prods = cons
   for (unsigned i = 0; i < num_cons; i++)
   {
      int dato = producir_dato();
      escribirEnVec.sem_wait();
      vec[siguiente_a_escribir] = dato;
      siguiente_a_escribir = ++siguiente_a_escribir % tam_vec;
      leerDeVec.sem_signal();
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora()
{
   // N / Cons = prods
   for (unsigned i = 0; i < num_prods; i++)
   {
      int dato;
      // completar ......
      leerDeVec.sem_wait();
      consumir.sem_wait();
      dato = vec[siguiente_a_leer];
      siguiente_a_leer = ++siguiente_a_leer % tam_vec;
      consumir.sem_signal();
      consumir_dato(dato);
      escribirEnVec.sem_signal();
   }
}
//----------------------------------------------------------------------

int main()
{
   assert(num_cons % 2 == 0 && num_cons > 12 && num_prods % 2 == 0 && num_prods > 12 && num_prods != num_cons);
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO)." << endl
        << "------------------------------------------------------------------" << endl
        << flush;

   thread hebras_consumidoras[num_cons], hebras_productoras[num_prods];
   for (int i = 0; i < num_cons; i++)
      hebras_consumidoras[i] = thread(funcion_hebra_consumidora);
   for (int i = 0; i < num_prods; i++)
      hebras_productoras[i] = thread(funcion_hebra_productora);
   for (int i = 0; i < num_cons; i++)
      hebras_consumidoras[i].join();
   for (int i = 0; i < num_prods; i++)
      hebras_productoras[i].join();
   test_contadores();
}
