// Mario Garcia Marquez
// 77147974

#include <iostream>
#include <thread>
#include <mutex>
#include "scd.h"

using namespace std;
using namespace scd;

mutex salida;

class Lec_Esc: public HoareMonitor {
		private:
				// true si un escritor esta escribiendo
				bool escrib;
				// lectores escribiendo
				unsigned n_lec;
				CondVar lectura, escritura;
		public:
				Lec_Esc();
				void ini_lectura();
				void fin_lectura();
				void ini_escritura();
				void fin_escritura();
};

Lec_Esc::Lec_Esc() {
		escrib = false;
		n_lec = 0;
		lectura = newCondVar();
		escritura = newCondVar();
}

void Lec_Esc::ini_lectura() {
		if(escrib)
				lectura.wait();
		n_lec++;
		lectura.signal();
}

void Lec_Esc::fin_lectura() {
		n_lec--;
		if(n_lec==0)
				escritura.signal();
}

void Lec_Esc::ini_escritura() {
		if(n_lec>0 || escrib)
				escritura.wait();
		escrib = true;
}

void Lec_Esc::fin_escritura() {
		escrib = false;
		if(!lectura.empty())
				lectura.signal();
		else
				escritura.signal();
}

void leer(int i) {
    int sleep = aleatorio<50, 100>();
	salida.lock();
    std::cout << "Lector " << i << ": leyendo con sleep de " << sleep << "ms\n";
	salida.unlock();
    this_thread::sleep_for(chrono::milliseconds(sleep));
}

void esperar_lector(int i) {
    int sleep = aleatorio<200, 300>();
	salida.lock();
    std::cout << "Lector " << i << ": descanso con sleep de " << sleep << "ms\n";
	salida.unlock();
    this_thread::sleep_for(chrono::milliseconds(sleep));
}

void esperar_escritor(int i) {
    int sleep = aleatorio<50, 100>();
	salida.lock();
    std::cout << "Escritor " << i << ": descanso con sleep de " << sleep << "ms\n";
	salida.unlock();
    this_thread::sleep_for(chrono::milliseconds(sleep));
}

void escribir(int i){
    int sleep = aleatorio<50, 100>();
	salida.lock();
    std::cout << "Escritor " << i << ": leyendo con sleep de " << sleep << "ms\n";
	salida.unlock();
    this_thread::sleep_for(chrono::milliseconds(sleep));
}

void hebra_lector(int i, MRef<Lec_Esc> monitor) {
		while(true){
				monitor->ini_lectura();
				leer(i);
				monitor->fin_lectura();
				esperar_lector(i);
		}
}

void hebra_escritor(int i, MRef<Lec_Esc> monitor) {
		while(true){
				monitor->ini_escritura();
				escribir(i);
				monitor->fin_escritura();
				esperar_escritor(i);
		}
}

int main() {
		const int numero_lectores = 4, numero_escritores = 3;
		MRef<Lec_Esc> monitor = Create<Lec_Esc>();

		thread escritores[numero_escritores], lectores[numero_lectores];
		for(int i = 0; i < numero_lectores; i++) 
				lectores[i] = thread(hebra_lector, i, monitor);
		for(int i = 0; i < numero_escritores; i++) 
				escritores[i] = thread(hebra_escritor, i, monitor);
		for(int i = 0; i < numero_lectores; i++) 
				lectores[i].join();
		for(int i = 0; i < numero_escritores; i++) 
				escritores[i].join();
}




