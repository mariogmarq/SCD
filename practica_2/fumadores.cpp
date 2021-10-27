#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "scd.h"

using namespace std;
using namespace scd;

const u_int numero_fumadores = 3;

// Monitor custom
class Estanco: public HoareMonitor {
    private:
        const int sin_ingrediente = -1;
        CondVar cola_fumar;
        CondVar cola_estanquero;
        int ingrediente;

    public:
        Estanco();
        void obtenerIngrediente(int i);
        void ponerIngrediente(int i);
        void esperarRecogidaIngrediente();
        void avisarEstanquero();
};

Estanco::Estanco() {
    cola_fumar = newCondVar();
    cola_estanquero = newCondVar();
    ingrediente = sin_ingrediente;
}

void Estanco::ponerIngrediente(int i) {
    ingrediente = i;
    cout << "Estanquero: pone ingrediente " << i << "\n";
}

void Estanco::esperarRecogidaIngrediente() {
    cout << "Estanquero: espera recogida ingrediente\n";
    // Despierta a todos los fumadores para que comprueben que ingrediente es
    for(int i = 0; i < numero_fumadores; i++)
        cola_fumar.signal();
    // En cuanto haga signal al ser SU el estanquero se bloquea automaticamente en el monitor
    cola_estanquero.wait();
}

void Estanco::obtenerIngrediente(int i) {
    while(ingrediente != i) {
        cola_fumar.wait();
    }

    cout << "Fumador " << i << ": Coge ingrediente\n";
    // Lo han llamado y ha encontrado su ingrediente
    // Cambiamos el ingrediente por uno vacio para que no haya condicion de carrera
    ingrediente = sin_ingrediente;
}

void Estanco::avisarEstanquero() {
    cola_estanquero.signal();
}

int ProducirIngrediente() {
    int sleep = aleatorio<50, 100>();
    cout << "Estanquero: Produciendo ingrediente con sleep de " << sleep << "ms\n";
    this_thread::sleep_for(chrono::milliseconds(sleep));
    int ingrediente = aleatorio<0, numero_fumadores-1>();
    return ingrediente;
}

void fumar(int i) {
    int sleep = aleatorio<200, 300>();
    cout << "Fumador " << i << ": Empieza a fumar con sleep de " << sleep << "ms\n";
    this_thread::sleep_for(chrono::milliseconds(sleep));
    cout << "Fumador " << i << ": Termina de fumar\n";
}

void funcion_hebra_estanquero(MRef<Estanco> monitor) {
    while(true) {
        int ingrediente = ProducirIngrediente();
        monitor->ponerIngrediente(ingrediente);
        monitor->esperarRecogidaIngrediente();
    }
}

void funcion_hebra_fumador(MRef<Estanco> monitor, int i) {
    while(true) {
        monitor->obtenerIngrediente(i);
        monitor->avisarEstanquero();
        fumar(i);
    }
}

int main() {
    thread fumadores[numero_fumadores];
    auto m = Create<Estanco>();
    auto t1 = thread(funcion_hebra_estanquero, m);
    for(int i = 0; i < numero_fumadores; i++)
        fumadores[i] = thread(funcion_hebra_fumador, m, i);
    t1.join();
}