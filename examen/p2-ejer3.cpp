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

using namespace std;
using namespace scd;

constexpr int
    num_productores = 12,
    num_consumidores = 10,
    num_items = num_consumidores * num_productores * 10; // número de items a producir/consumir

int numeros_producidos[num_productores] = {0}, num_consumidos[num_consumidores] = {0}; // Cuantos items han producido / consumido cada hebra
int minimo_consumidos = 0, minimo_producidos = 0;                                      // el minimo de consumidos y producidos, se actualizan en finProd y finCons

mutex
    mtx; // mutex de escritura en pantalla
unsigned
    cont_prod[num_items],               // contadores de verificación: producidos
    cont_cons[num_items],               // contadores de verificación: consumidos
    cont_hebras[num_productores] = {0}; // contadores de verificación: hebras

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

int producir_dato(int i)
{
   int v = i * (num_items / num_productores) + cont_hebras[i];
   this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));
   mtx.lock();
   cout << "producido " << i << ": " << v << endl
        << flush;
   mtx.unlock();
   cont_prod[v]++;
   cont_hebras[i]++;
   return v;
}
//----------------------------------------------------------------------

void consumir_dato(unsigned dato)
{
   if (num_items <= dato)
   {
      cout << " dato === " << dato << ", num_items == " << num_items << endl;
      assert(dato < num_items);
   }
   cont_cons[dato]++;
   this_thread::sleep_for(chrono::milliseconds(aleatorio<20, 100>()));
   mtx.lock();
   cout << "                  consumido: " << dato << endl;
   mtx.unlock();
}
//----------------------------------------------------------------------

void ini_contadores()
{
   for (unsigned i = 0; i < num_items; i++)
   {
      cont_prod[i] = 0;
      cont_cons[i] = 0;
   }
}

//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true;
   cout << "comprobando contadores ...." << flush;

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

// *****************************************************************************
// clase para monitor buffer, version LIFO, semántica SC, un prod. y un cons.

class ProdCons2SU : public HoareMonitor
{
private:
   static const int           // constantes:
       num_celdas_total = 10; //  núm. de entradas del buffer
   bool manteniendo;
   int                           // variables permanentes
       buffer[num_celdas_total], //  buffer de tamaño fijo, con los datos
       primera_ocupada,
       celdas_ocupadas,
       hebras_produciendo_pares,
       hebras_produciendo_impares,
       hebras_consumiendo,
       primera_libre; //  indice de celda de la próxima inserción
   CondVar            // colas condicion:
       cola_prod_par,
       cola_prod_impar,
       cola_mant,
       cola_cons,
       ocupadas, //  cola donde espera el consumidor (n>0)
       libres;   //  cola donde espera el productor  (n<num_celdas_total)

public:                             // constructor y métodos públicos
   ProdCons2SU();                   // constructor
   int leer(int i);                 // extraer un valor (sentencia L) (consumidor)
   void escribir(int valor, int i); // insertar un valor (sentencia E) (productor)
   void iniProd(int i);
   void finProd(int i);
   void iniCons(int i);
   void finCons(int i);
   void iniMant();
   void finMant();
};
// -----------------------------------------------------------------------------

ProdCons2SU::ProdCons2SU()
{
   ocupadas = newCondVar();
   libres = newCondVar();
   cola_cons = newCondVar();
   cola_prod_par = newCondVar();
   cola_prod_impar = newCondVar();
   cola_mant = newCondVar();
   primera_libre = 0;
   celdas_ocupadas = 0;
   primera_ocupada = 0;
   manteniendo = false;
   hebras_consumiendo = hebras_produciendo_pares = hebras_produciendo_impares = 0;
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdCons2SU::leer(int i)
{
   if (manteniendo)
      if (num_consumidos[i] != minimo_consumidos)
         cola_mant.wait();
   // esperar bloqueado hasta que 0 < num_celdas_ocupadas
   if (celdas_ocupadas == 0)
      ocupadas.wait();

   // hacer la operación de lectura, actualizando estado del monitor
   assert(0 < celdas_ocupadas);
   const int valor = buffer[primera_ocupada];
   celdas_ocupadas--;
   primera_ocupada++;
   primera_ocupada %= num_celdas_total;

   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor;
}
// -----------------------------------------------------------------------------

void ProdCons2SU::escribir(int valor, int i)
{
   if (manteniendo)
      if (numeros_producidos[i] != minimo_producidos)
         cola_mant.wait();
   // esperar bloqueado hasta que num_celdas_ocupadas < num_celdas_total
   if (celdas_ocupadas == num_celdas_total)
      libres.wait();

   //cout << "escribir: ocup == " << num_celdas_ocupadas << ", total == " << num_celdas_total << endl ;
   assert(celdas_ocupadas < num_celdas_total);

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor;
   primera_libre++;
   primera_libre %= num_celdas_total;
   celdas_ocupadas++;

   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}

void ProdCons2SU::iniProd(int i)
{
   if (manteniendo)
      if (numeros_producidos[i] != minimo_producidos)
         cola_mant.wait();

   if (i % 2 == 0)
   {
      if (hebras_produciendo_pares == 3)
         cola_prod_par.wait();
      hebras_produciendo_pares++;
   }
   else
   {
      if (hebras_produciendo_impares == 3)
         cola_prod_impar.wait();
      hebras_produciendo_impares++;
   }
}

void ProdCons2SU::finProd(int i)
{
   if (i % 2 == 0)
   {
      hebras_produciendo_pares--;
      if (!cola_prod_par.empty())
         cola_prod_par.signal();
   }
   else
   {
      hebras_produciendo_impares--;
      if (!cola_prod_impar.empty())
         cola_prod_impar.signal();
   }

   // actualizamos el minimo de producidos
   // int minimo = -1;
   // for (int j = 0; j < num_productores; i++)
   //    minimo = numeros_producidos[j] < minimo? numeros_producidos[j] : minimo;
   // minimo_producidos = minimo;

   // No funciona con el codigo de arriba descomentado
}

void ProdCons2SU::iniCons(int i)
{
   if (manteniendo)
      if (num_consumidos[i] != minimo_consumidos)
         cola_mant.wait();

   if (hebras_consumiendo == num_consumidores / 2)
      cola_cons.wait();
   hebras_consumiendo++;
}

void ProdCons2SU::finCons(int i)
{
   hebras_consumiendo--;
   if (!cola_cons.empty())
      cola_cons.signal();
   // actualizamos los consumidos
   // int minimo = -1;
   // for (int j = 0; j < num_consumidores; i++)
   //    minimo = num_consumidos[j] < minimo? num_consumidos[j] : minimo;
   // minimo_consumidos = minimo;

   // No funciona con el codigo de arriba descomentado
}

void ProdCons2SU::iniMant()
{
   manteniendo = true;
}

void ProdCons2SU::finMant()
{
   manteniendo = false;
   while (!cola_mant.empty()) // se desbloquean a todos que se han parado por el mantenimiento
      cola_mant.signal();
}

// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora(MRef<ProdCons2SU> monitor, int id)
{
   int p = num_items / num_productores;
   for (unsigned i = 0; i < p; i++)
   {
      monitor->iniProd(id);
      int valor = producir_dato(id);
      numeros_producidos[id]++;
      monitor->finProd(id);
      monitor->escribir(valor, id);
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora(MRef<ProdCons2SU> monitor, int id)
{
   for (unsigned i = 0; i < num_items / num_consumidores; i++)
   {
      int valor = monitor->leer(id);
      monitor->iniCons(id);
      consumir_dato(valor);
      num_consumidos[id]++;
      monitor->finCons(id);
   }
}

void funcion_hebra_mantenimiento(MRef<ProdCons2SU> monitor)
{
   while (true)
   {
      this_thread::sleep_for(chrono::milliseconds(1000));
      monitor->iniMant();
      this_thread::sleep_for(chrono::milliseconds(5000));
      monitor->finMant();
   }
}
// -----------------------------------------------------------------------------

int main()
{
   assert(num_productores % 2 == 0 && num_productores > 9 && num_consumidores % 2 == 0 && num_consumidores > 9 && num_consumidores != num_productores);
   cout << "-------------------------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (varios prod/cons, Monitor SU, buffer FIFO). " << endl
        << "-------------------------------------------------------------------------------" << endl
        << flush;

   MRef<ProdCons2SU> monitor = Create<ProdCons2SU>();

   thread hebras_productoras[num_productores], hebras_consumidoras[num_consumidores];
   thread mantenimiento = thread(funcion_hebra_mantenimiento, monitor);

   for (int i = 0; i < num_productores; i++)
      hebras_productoras[i] = thread(funcion_hebra_productora, monitor, i);

   for (int i = 0; i < num_consumidores; i++)
      hebras_consumidoras[i] = thread(funcion_hebra_consumidora, monitor, i);

   for (int i = 0; i < num_productores; i++)
      hebras_productoras[i].join();

   for (int i = 0; i < num_consumidores; i++)
      hebras_consumidoras[i].join();
   mantenimiento.join();
   // comprobar que cada item se ha producido y consumido exactamente una vez
   test_contadores();
}
