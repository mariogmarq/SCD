#include <iostream>
#include "scd.h"

using namespace std;
using namespace scd;

constexpr int num_sillas_espera = 3, num_sillas_barbero = 4;

class MonitorBarberia: HoareMonitor {
    private:
        int sillas_espera[num_sillas_espera],
            sillas_espera_libres,
            sillas_barbero[num_sillas_barbero],
            sillas_barbero_libres,
            barberos_libres,
            siguiente_cliente_espera,
            siguiente_cliente_pelarse,
            siguiente_libre_espera,
            siguiente_libre_pelarse;
        CondVar cola_espera[num_sillas_espera],
                cola_pelandose[num_sillas_barbero],
                cola_cobrar,
                cola_entrar;
	public:
        MonitorBarberia();
		void corte_de_pelo(int i);
		int siguiente_cliente();
		void termina_corte_de_pelo(int i);
}; 

MonitorBarberia::MonitorBarberia() {
    barberos_libres = 0;
    sillas_espera_libres = 0;
    sillas_barbero_libres = 0;
    siguiente_cliente_espera = 0;
    siguiente_cliente_pelarse = 0;
    siguiente_libre_pelarse = 0;
    siguiente_libre_espera = 0;
    for(int i = 0; i < num_sillas_espera; i++)
        sillas_espera[i] = -1;
    for(int i = 0; i < num_sillas_barbero; i++)
        sillas_barbero[i] = -1;
    for(int i = 0; i < num_sillas_espera; i++)
        cola_espera[i] = newCondVar();
    for(int i = 0; i < num_sillas_barbero; i++)
        cola_pelandose[i] = newCondVar();
    cola_entrar = newCondVar();
}

void MonitorBarberia::corte_de_pelo(int i) {
    auto silla_ocupada = 0;
    if(barberos_libres > 0 && sillas_barbero_libres) {
            sillas_barbero[siguiente_libre_pelarse] = i;
            silla_ocupada = siguiente_libre_pelarse;
            siguiente_libre_pelarse += (siguiente_libre_pelarse + 1) % num_sillas_barbero;
            sillas_barbero_libres--;

    } else {
        if(sillas_espera_libres == 0)
            cola_entrar.wait();
        sillas_espera[siguiente_libre_espera] = i;
        silla_ocupada = siguiente_libre_espera;
        siguiente_libre_espera += (siguiente_libre_espera + 1) % num_sillas_espera;
        sillas_espera_libres--;
        cola_espera[silla_ocupada].wait();
        sillas_espera_libres++;
        cola_espera[silla_ocupada].signal();
        sillas_barbero[siguiente_libre_pelarse] = i;
        silla_ocupada = siguiente_libre_pelarse;
        siguiente_libre_pelarse += (siguiente_libre_pelarse + 1) % num_sillas_barbero;
        sillas_barbero_libres--;
    }

    cola_pelandose[silla_ocupada].wait();
    sillas_barbero_libres++;
    cola_pelandose[silla_ocupada].signal();
    cola_cobrar.wait();
    cola_entrar.signal();
}


int MonitorBarberia::siguiente_cliente() {
    if(siguiente_libre_pelarse != 0)
        return sillas_barbero[siguiente_libre_pelarse -1];
    return sillas_barbero[num_sillas_barbero -1];
}

void MonitorBarberia::termina_corte_de_pelo(int i) {
    cola_cobrar.signal();
}

