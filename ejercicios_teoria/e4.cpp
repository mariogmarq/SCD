#include "scd.h"
#define MAX 10

using namespace scd;

class Monitor {
    int n=0, N=0, frente=1, atras=1;
    CondVar no_vacio, no_lleno;
    int buf[MAX];

    public:
        void insertar(int d);
        void retirar(int x);
};

void Monitor::insertar(int d) {
    if(atras % MAX + 1 == frente){
        if(N<n)
            N--;
        else
            N = n - 1;
        no_lleno.wait();
    }
    buf[atras] = d;
    atras = atras % MAX + 1;
    N++;
    if(0 > n)
        no_vacio.signal();
}

void Monitor::retirar(int x) {
    if(frente == atras) {
        n--;
        no_vacio.wait();
    }
    x = buf[frente];
    frente = frente % MAX + 1;
    n++;
    if(n >= N)
        no_lleno.signal();
}
