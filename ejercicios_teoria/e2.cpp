#include "scd.h"

using namespace std;
using namespace scd;

class Monitor: public HoareMonitor {
    private:
        int espacios_libres = 100;
        int siguiente_libre = 0;
        int coches[100] = {-1};
        CondVar cola_espera,
                cola_cobrar[100],
                cola_lavado[100];
    public:
        Monitor();
        void lavado_de_coche(int i);
        unsigned siguiente_coche();
        void terminar_y_cobrar(int i);
};

Monitor::Monitor() {
    cola_espera = newCondVar();
    for(int i = 0; i < 100; i++)
        cola_lavado[i] = newCondVar();
    for(int i = 0; i < 100; i++)
        cola_cobrar[i] = newCondVar();
}

void Monitor::lavado_de_coche(int i) {
    if(espacios_libres == 0) {
        cola_espera.wait();
    }

    espacios_libres--;
    auto hueco = siguiente_libre;
    siguiente_libre += (siguiente_libre + 1) % 100;
    coches[hueco] = i;
    cola_lavado[hueco].wait();
    cola_cobrar[hueco].wait();
    coches[hueco] = -1;
    espacios_libres++;
    cola_lavado[hueco].signal();
}

unsigned Monitor::siguiente_coche() {
    for(int i = 0; i < 100; i++)
        if(coches[i] != -1)
            return static_cast<unsigned>(i);
    return 0;
}

void Monitor::terminar_y_cobrar(int i) {
    for(int j = 0; j < 100; j++)
        if(coches[j] == i){
            cola_cobrar[j].signal();
        }
}
