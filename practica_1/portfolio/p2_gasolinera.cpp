// Mario Garcia Marquez
// 77147974

#include <iostream>
#include <thread>
#include "scd.h"
#include <chrono>


using namespace std;
using namespace scd;

constexpr int numero_coches_gasoil = 4, numero_coches_gasolina = 6;

// Gasolinera
class Gasolinera: public HoareMonitor {
    private:
        const int
            surtidores_gasolina = 3,
            surtidores_gasoil = 2;
        int
            surtidor_gasolina_libre = surtidores_gasolina,
            surtidor_gasoil_libre = surtidores_gasoil;
        
        CondVar
            espera_gasoil,
            espera_gasolina;
    public:
        Gasolinera();
        void entra_coche_gasoil(int i);
        void entra_coche_gasolina(int i);
        void sale_coche_gasoil(int i);
        void sale_coche_gasolina(int i);
};

Gasolinera::Gasolinera() {
    espera_gasoil = newCondVar();
    espera_gasolina = newCondVar();
}

void Gasolinera::entra_coche_gasoil(int i) {
    if(surtidor_gasoil_libre == 0)
        espera_gasoil.wait();
    
    surtidor_gasoil_libre--;
    assert(surtidor_gasoil_libre>=0);

    cout << "Coche gasoil " << i << ": Entra gasolinera, quedan " << surtidor_gasoil_libre
        << " surtidores libres" << endl;
}

void Gasolinera::entra_coche_gasolina(int i) {
    if(surtidor_gasolina_libre == 0)
        espera_gasolina.wait();
    
    surtidor_gasolina_libre--;
    assert(surtidor_gasolina_libre>=0);

    cout << "Coche gasolina " << i << ": Entra gasolinera, quedan " << surtidor_gasolina_libre
        << " surtidores libres" << endl;
}

void Gasolinera::sale_coche_gasoil(int i) {
    surtidor_gasoil_libre++;
    assert(surtidor_gasoil_libre <= surtidores_gasoil);
    cout << "Coche gasoil " << i << ": Sale gasolinera, quedan " << surtidor_gasoil_libre
        << " surtidores libres" << endl;
    espera_gasoil.signal();
}

void Gasolinera::sale_coche_gasolina(int i) {
    surtidor_gasolina_libre++;
    assert(surtidor_gasolina_libre <= surtidores_gasolina);
    cout << "Coche gasolina " << i << ": Sale gasolinera, quedan " << surtidor_gasoil_libre
        << " surtidores libres" << endl;
    espera_gasolina.signal();
}

void repostar(int numero_coche) {
    chrono::milliseconds duracion(scd::aleatorio<10,300>());
    cout << "Coche " << numero_coche << " empieza a repostar " << duracion.count() << " ms" << endl;

    this_thread::sleep_for(duracion);

    cout << "Coche " << numero_coche <<  " termina" << endl;
}

void esperar_fuera(int numero_coche) {
    chrono::milliseconds duracion(scd::aleatorio<10,300>());
    cout << "Coche " << numero_coche <<  " espera fuera " << duracion.count() << " ms" << endl;

    this_thread::sleep_for(duracion);

    cout << "Coche " << numero_coche <<  " termina de esperar" << endl;
}

void funcion_hebra_gasoil(int i, MRef<Gasolinera> monitor) {
    while(true){
        monitor->entra_coche_gasoil(i);
        repostar(i);
        monitor->sale_coche_gasoil(i);
        esperar_fuera(i);
    }
}

void funcion_hebra_gasolina(int i, MRef<Gasolinera> monitor) {
    while(true){
        monitor->entra_coche_gasolina(i);
        repostar(i);
        monitor->sale_coche_gasolina(i);
        esperar_fuera(i);
    }
}

int main() {
    MRef<Gasolinera> monitor = Create<Gasolinera>();
    thread gasolina[numero_coches_gasolina], gasoil[numero_coches_gasoil];
    for(int i = 0; i < numero_coches_gasolina; i++) 
        gasolina[i] = thread(funcion_hebra_gasolina, i, monitor);
    for(int i = 0; i < numero_coches_gasoil; i++) 
        gasoil[i] = thread(funcion_hebra_gasoil, i, monitor);
    for(int i = 0; i < numero_coches_gasolina; i++) 
        gasolina[i].join();
    for(int i = 0; i < numero_coches_gasoil; i++) 
        gasoil[i].join();

}
