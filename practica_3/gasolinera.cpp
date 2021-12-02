// Mario Garcia Marquez
// 77147974V

#include <mpi.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

const int ID_GASOLINERA = 0;
const int
    num_coches_gasoil = 6,
    num_surtidores_gasoil = 3,
    num_coches_gasolina = 5,
    num_surtidores_gasolina = 2;

const int
    etiq_empezar_a_repostar_gasoil = 2,
    etiq_terminar_gasoil = 3,
    etiq_empezar_a_repostar_gasolina = 0,
    etiq_terminar_gasolina = 1;

template <int min, int max>
int aleatorio()
{
    static default_random_engine generador((random_device())());
    static uniform_int_distribution<int> distribucion_uniforme(min, max);
    return distribucion_uniforme(generador);
}

void funcion_coche_gasolina(int id)
{
    const int dato = 0;
    while (true)
    {
        cout << "Coche gasolina " << id << ": Quiere entrar a repostar" << endl;
        MPI_Ssend(&dato, 1, MPI_INT, ID_GASOLINERA, etiq_empezar_a_repostar_gasolina, MPI_COMM_WORLD);
        cout << "Coche gasolina " << id << ": reposta" << endl;
        this_thread::sleep_for(milliseconds(aleatorio<50, 100>()));
        cout << "Coche gasolina " << id << ": Sale de repostar" << endl;
        MPI_Ssend(&dato, 1, MPI_INT, ID_GASOLINERA, etiq_terminar_gasolina, MPI_COMM_WORLD);
    }
}

void funcion_coche_gasoil(int id)
{
    const int dato = 0;
    while (true)
    {
        cout << "Coche gasoil " << id << ": Quiere entrar a repostar" << endl;
        MPI_Ssend(&dato, 1, MPI_INT, ID_GASOLINERA, etiq_empezar_a_repostar_gasoil, MPI_COMM_WORLD);
        cout << "Coche gasoil " << id << ": reposta" << endl;
        this_thread::sleep_for(milliseconds(aleatorio<50, 100>()));
        cout << "Coche gasoil " << id << ": Sale de repostar" << endl;
        MPI_Ssend(&dato, 1, MPI_INT, ID_GASOLINERA, etiq_terminar_gasoil, MPI_COMM_WORLD);
    }
}

void funcion_gasolinera()
{
    int dato = 0;
    int etiq = 0;
    MPI_Status estado;
    int esperando = 0;
    int surtidores_gasolina_libres = num_surtidores_gasolina;
    int surtidores_gasoil_libres = num_surtidores_gasoil;
    while (true)
    {
        // Hacemos una iteracion para cada tipo
        //Coches de gasolina
        // Primero vemos si algun coche quiere salir
        MPI_Iprobe(MPI_ANY_SOURCE, etiq_terminar_gasolina, MPI_COMM_WORLD, &esperando, &estado);
        if (esperando)
        {
            // Hay al menos uno, lo liberamos
            MPI_Recv(&dato, 1, MPI_INT, MPI_ANY_SOURCE, etiq_terminar_gasolina, MPI_COMM_WORLD, &estado);
            surtidores_gasolina_libres++;
        }
        if (surtidores_gasolina_libres)
        {
            // Vemos si alguien quiere entrar
            MPI_Iprobe(MPI_ANY_SOURCE, etiq_empezar_a_repostar_gasolina, MPI_COMM_WORLD, &esperando, &estado);
            if (esperando)
            {
                // Hay al menos uno, lo liberamos
                MPI_Recv(&dato, 1, MPI_INT, MPI_ANY_SOURCE, etiq_empezar_a_repostar_gasolina, MPI_COMM_WORLD, &estado);
                surtidores_gasolina_libres--;
            }
        }

        // Coches gasoil
        MPI_Iprobe(MPI_ANY_SOURCE, etiq_terminar_gasoil, MPI_COMM_WORLD, &esperando, &estado);
        if (esperando)
        {
            // Hay al menos uno, lo liberamos
            MPI_Recv(&dato, 1, MPI_INT, MPI_ANY_SOURCE, etiq_terminar_gasoil, MPI_COMM_WORLD, &estado);
            surtidores_gasoil_libres++;
        }

        if (surtidores_gasoil_libres)
        {
            // Vemos si alguien quiere entrar
            MPI_Iprobe(MPI_ANY_SOURCE, etiq_empezar_a_repostar_gasoil, MPI_COMM_WORLD, &esperando, &estado);
            if (esperando)
            {
                // Hay al menos uno, lo liberamos
                MPI_Recv(&dato, 1, MPI_INT, MPI_ANY_SOURCE, etiq_empezar_a_repostar_gasoil, MPI_COMM_WORLD, &estado);
                surtidores_gasoil_libres--;
            }
        }
        this_thread::sleep_for(milliseconds(20));
    }
}

int main(int argc, char** argv) {

   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( 1 + num_coches_gasoil + num_coches_gasolina == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if(id_propio == ID_GASOLINERA)
        funcion_gasolinera();
      else if ( id_propio <= num_coches_gasolina )          // si es par
         funcion_coche_gasolina( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_coche_gasoil( id_propio - num_coches_gasolina); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << 1 + num_coches_gasoil + num_coches_gasolina << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}