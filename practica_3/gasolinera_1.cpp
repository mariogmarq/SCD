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
    num_coches = 11,
    num_surtidores = 5;
const int
    etiq_salir = 0,
    etiq_entrar = 1;

template <int min, int max>
int aleatorio()
{
    static default_random_engine generador((random_device())());
    static uniform_int_distribution<int> distribucion_uniforme(min, max);
    return distribucion_uniforme(generador);
}

void funcion_coche(int id)
{
    const int dato = 0;
    while (true)
    {
        cout << "Coche " << id << ": Quiere entrar a repostar" << endl;
        MPI_Ssend(&dato, 1, MPI_INT, ID_GASOLINERA, etiq_entrar, MPI_COMM_WORLD);
        cout << "Coche " << id << ": reposta" << endl;
        this_thread::sleep_for(milliseconds(aleatorio<50, 100>()));
        cout << "Coche " << id << ": Sale de repostar" << endl;
        MPI_Ssend(&dato, 1, MPI_INT, ID_GASOLINERA, etiq_salir, MPI_COMM_WORLD);
    }
}

void funcion_gasolinera()
{
    int dato = 0;
    int etiq_valida = 0;
    int libres = num_surtidores;
    MPI_Status estado;
    while (true)
    {
        etiq_valida = libres > 0 ? MPI_ANY_TAG : etiq_salir;
        MPI_Recv(&dato, 1, MPI_INT, MPI_ANY_SOURCE, etiq_valida, MPI_COMM_WORLD, &estado);
        libres += estado.MPI_TAG == etiq_entrar ? -1 : 1;
    }
}

int main(int argc, char **argv)
{

    int id_propio, num_procesos_actual;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);

    if (1 + num_coches == num_procesos_actual)
    {
        // ejecutar la función correspondiente a 'id_propio'
        if (id_propio == ID_GASOLINERA)
            funcion_gasolinera();
        else // si es par
            funcion_coche(id_propio);
    }
    else
    {
        if (id_propio == 0) // solo el primero escribe error, indep. del rol
        {
            cout << "el número de procesos esperados es:    " << 1 + num_coches << endl
                 << "el número de procesos en ejecución es: " << num_procesos_actual << endl
                 << "(programa abortado)" << endl;
        }
    }

    MPI_Finalize();
    return 0;
}