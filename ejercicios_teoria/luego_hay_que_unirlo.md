# Ejercicio 65

### a)

Si ocurriese que P1 llegase a q.signal() antes de que P2 llegue q.wait() el programa entraria en deadlock pues P2 se quedaria esperado una señal que nunca llegaria y P1 volveria a ejecutar p.wait() por lo que ambos procesos se quedarian bloqueados.

### b)

La mejor manera de resolver este problema seria modificando los metodos de la siguiente manera

```cpp
void stop {
    p.wait();
    ...;
    while(q.empty()){}
    q.signal();
}

void sigue() {
    while(p.empty()){}
    p.signal();
    q.wait();
}
```

# Ejercicio 66

El primer monitor es valido tanto para SU como para SW, sin embargo en SC habria condicion de carrera y habria que poner s como variable atomica o rodear su actualizacion con un mutex.

El segundo monitor no garantiza la exclusion mutua en caso de SC, habria que asegurarse de que las modificaciones a s se hagan en exclusion mutua en ese caso.

El tercer monitor  no es valido para ninguna semantica pues no garantiza la exclusion mutua al dato s para ninguna semántica, se propone cambiar su implementacion por la del segundo semaforo o el primero con la correspondiente modificacion ya propuesta.

# Ejercicio 67

```cpp
class Monitor {
    CondVar c,d;
    int s = 0, p = 0, v = 0;
    void P();
    void V();
};


void Monitor::P() {
    while(s == 0)
        c.wait();
    p++;
    s--;
    p--;
    if(v && !d.empty())
        d.signal();
}

void Monitor::V() {
    v++;
    c.signal_all();
    if(p!= 0)
        d.wait();
    s++;
    v--;
}
```

# Ejercicio 68

```cpp
class Monitor {
    int dato, esperando = 0;
    bool transmitiendo = false;
    CondVar c, d;
    void broadcast(int m);
    int fetch();
};


void Monitor::broadcast(int m) {
    if(transmitiendo)
        d.wait();
    transmitiendo = true;
    dato = m;
    while(esperando)
        c.signal();
    transmitiendo = false;
}


int Monitor::fetch() {
    esperando++;
    if(transmitiendo){
        esperando--;
        return dato;
    }
    
    c.wait();
    esperando--;
    return dato;
}
```

# Ejercicio 69

### a)

```cpp
class Monitor {
    int buf[MAX]; // cada pos almacena las paginas necesarias
    int paginas_libres;
    int primero_libre, primero_ocupada;
    CondVar espera[MAX];
    
    void adquirir(n);
    void liberar(n);
};

void Monitor::adquirir(n) {
    if(paginas_libres >= n)
        paginas_libres -= n;
    else {
        buf[primero_libre] = n;
        espera[primero_libre].wait();
        primero_libre = primero_libre + 1 % MAX;
        paginas_libres -= n;
    }
}

void Monitor::liberar(n) {
    paginas_libres += n;
    while(buf[primero_ocupado] <= paginas_libres) {
       espera[primero_ocupado].signal();
        primero_ocupado = primero_ocupado + 1 % MAX;
    }
}
```

### b)

Se podria resolver cambiando buf y espera por colas con prioridad basado en el valor de las paginas en lugar de un buffer fifo. Como es un cambio que radica en la estructura de datos interna no se va a programar.



# Ejercicio 70

### a)

```cpp
class Monitor() {
    int C, esperando;
    CondVar esperar_a_lleno, controlador;
    
    void abrir_cerrar_valvula(int cantidad) {
        if(C >= cantidad) {
            C -= cantidad;
            return;
        }

        if(C == 0) {
            while(C == 0) {
                controlador.signal();
                esperando++;
                esperar_a_lleno.wait();
                esperando--;
            }
             C = C - cantidad < 0 ? 0 : C - cantidad;
        } else {
            C = 0;
        }
    }

    void control_en_espera() {
        controlador.wait();
    }

    void control_rellenando() {
        C = max;
        for(esperando)
            esperar_a_lleno.signal(),
    }
}
```
