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

const int ID_GASOLINERA = 0,
    ID_IMPRESORA = 1;
const int
    num_coches = 15,
    num_combustibles = 3,
    num_surtidores[num_combustibles] = {3, 2, 5};

const int
    etiq_acabar = 0,
    etiq_entrar[num_combustibles] = {1, 2, 3};

template <int min, int max>
int aleatorio()
{
    static default_random_engine generador((random_device())());
    static uniform_int_distribution<int> distribucion_uniforme(min, max);
    return distribucion_uniforme(generador);
}

void imprimir(const string& s) {
    MPI_Ssend(s.c_str(), s.length(), MPI_CHAR, ID_IMPRESORA, 0, MPI_COMM_WORLD);
}

void funcion_coche(int id)
{
    const int tipo = aleatorio<0, num_combustibles - 1>();
    string s;
    while (true)
    {
        s = string("Coche " + to_string(id) + " tipo " + to_string(tipo) + ": quiere repostar");
        imprimir(s);
        MPI_Ssend(&tipo, 1, MPI_INT, ID_GASOLINERA, etiq_entrar[tipo], MPI_COMM_WORLD);
        s = string("Coche " + to_string(id) + " tipo " + to_string(tipo) + ": reposta");
        imprimir(s);
        this_thread::sleep_for(milliseconds(aleatorio<50, 100>()));
        MPI_Ssend(&tipo, 1, MPI_INT, ID_GASOLINERA, etiq_acabar, MPI_COMM_WORLD);
        s = string("Coche " + to_string(id) + " tipo " + to_string(tipo) + ": sale de repostar");
        imprimir(s);
    }
}

void funcion_gasolinera()
{
    int dato = 0;
    int etiq = 0;
    MPI_Status estado;
    int esperando = 0;
    int surtidores[num_combustibles];
    for (int i = 0; i < num_combustibles; i++)
        surtidores[i] = num_surtidores[i];

    while (true)
    {
        // Vemos si alguien quiere salir
        MPI_Iprobe(MPI_ANY_SOURCE, etiq_acabar, MPI_COMM_WORLD, &esperando, &estado);
        if (esperando)
        {
            // Hay al menos uno, lo liberamos
            MPI_Recv(&dato, 1, MPI_INT, MPI_ANY_SOURCE, etiq_acabar, MPI_COMM_WORLD, &estado);
            surtidores[dato]++;
        }

        // Vemos si alguien quiere entrar para cada tipo
        for (int i = 0; i < num_combustibles; i++)
        {
            if (surtidores[i])
            {
                // Vemos si alguien quiere entrar
                MPI_Iprobe(MPI_ANY_SOURCE, etiq_entrar[i], MPI_COMM_WORLD, &esperando, &estado);
                if (esperando)
                {
                    // Hay al menos uno, lo liberamos
                    MPI_Recv(&dato, 1, MPI_INT, MPI_ANY_SOURCE, etiq_entrar[i], MPI_COMM_WORLD, &estado);
                    surtidores[i]--;
                }
            }
        }
        this_thread::sleep_for(milliseconds(20));
    }
}

void funcion_impresora() {
    MPI_Status estado;
    int size;
    while(true) {
        MPI_Probe(MPI_ANY_TAG, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
        MPI_Get_count(&estado, MPI_CHAR, &size);
        char*buffer = new char[size+1];
        MPI_Recv(buffer, size, MPI_CHAR,estado.MPI_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
        buffer[size] = '\0';
        cout << buffer << endl;
        delete[] buffer;
    }
}

int main(int argc, char **argv)
{

    int id_propio, num_procesos_actual;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id_propio);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procesos_actual);

    if (2 + num_coches == num_procesos_actual)
    {
        // ejecutar la función correspondiente a 'id_propio'
        if (id_propio == ID_GASOLINERA)
            funcion_gasolinera();
        else if (id_propio == ID_IMPRESORA)
            funcion_impresora();
        else                                                      // si es impar
            funcion_coche(id_propio - 1); 
    }
    else
    {
        if (id_propio == 0) // solo el primero escribe error, indep. del rol
        {
            cout << "el número de procesos esperados es:    " << 2 + num_coches << endl
                 << "el número de procesos en ejecución es: " << num_procesos_actual << endl
                 << "(programa abortado)" << endl;
        }
    }

    MPI_Finalize();
    return 0;
}