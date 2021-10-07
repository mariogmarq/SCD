#include <iostream>
#include "scd.h"
#include <chrono>
#include <mutex>
#include <thread>

using namespace std;

// Surtidores de la gasolinera(originalmente todos libres)
scd::Semaphore surtidores_gasolina(3);
scd::Semaphore surtidores_gasoil(2);

// cout bonito
mutex salida;

// tipos de coche
enum coche {
    GASOLINA,
    GASOIL,
};

void repostar(int numero_coche, coche tipo) {
    chrono::milliseconds duracion(scd::aleatorio<10,300>());
    salida.lock();
    cout << "Coche " << numero_coche << (tipo == coche::GASOLINA ? " gasolina:" : " gasoil:") << " empieza a repostar " << duracion.count() << " ms" << endl;
    salida.unlock();

    this_thread::sleep_for(duracion);

    salida.lock();
    cout << "Coche " << numero_coche << (tipo == coche::GASOLINA ? " gasolina:" : " gasoil:") << " termina" << endl;
    salida.unlock();
}

void esperar_fuera(int numero_coche, coche tipo) {
    chrono::milliseconds duracion(scd::aleatorio<10,300>());
    salida.lock();
    cout << "Coche " << numero_coche << (tipo == coche::GASOLINA ? " gasolina:" : " gasoil:") << " espera fuera " << duracion.count() << " ms" << endl;
    salida.unlock();

    this_thread::sleep_for(duracion);

    salida.lock();
    cout << "Coche " << numero_coche << (tipo == coche::GASOLINA ? " gasolina:" : " gasoil:") << " termina de esperar" << endl;
    salida.unlock();
}

void funcion_hebra_gasolina(int numero) {
    while(true) {
        sem_wait(surtidores_gasolina);
        repostar(numero, coche::GASOLINA);
        sem_signal(surtidores_gasolina);
        esperar_fuera(numero, coche::GASOLINA);
    }
}


void funcion_hebra_gasoil(int numero) {
    while(true) {
        sem_wait(surtidores_gasoil);
        repostar(numero, coche::GASOIL);
        sem_signal(surtidores_gasoil);
        esperar_fuera(numero, coche::GASOIL);
    }
}

int main() {
    thread coches[10];
    // coches gasolina
    for(int i = 0; i < 10; i++)
        coches[i] = i < 6 ? thread(funcion_hebra_gasolina, i) : thread(funcion_hebra_gasoil, i);

    for(int i = 0; i < 10; i++)
        coches[i].join();    
    
}