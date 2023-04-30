#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>


using namespace std;

const int CARS = 10;

enum states {
    CROSSING,
    WAITING,
    FINISHED,
    END
};

enum directions {
    N_TO_S,
    S_TO_N,
    NONE
};

typedef struct {
    int id;
    states state;
    directions direction;
} Car;

int totalCars = 0;
directions bridgeDirection = NONE;

pthread_cond_t crossNtoS;
pthread_cond_t crossStoN;
pthread_mutex_t lockBridge;

// Revisa si el vehiculo está esperando
void holding(Car *car) {
    if (car->state != WAITING) return;

    if (car->direction == N_TO_S) {
        cout << "Vehículo " << car->id <<" está esperando a cruzar de norte a sur" << endl;
    } else if (car->direction == S_TO_N) {
        cout << "Vehículo "<< car->id << " está esperando a cruzar de sur a norte" << endl;
    }
    sleep(1);
}

// Revisa si el vehiculo está cruzando
void crossing(Car *car) {
    if (car->state != CROSSING) return;

    if (car->direction == N_TO_S) {
        cout << "Vehículo " << car->id << " está cruzando de norte a sur" << endl;
    } else if (car->direction == S_TO_N) {
        cout << "Vehículo " << car->id << " está cruzando de sur a norte" << endl;
    }
    sleep(1);
}

// Verifica si el estado del automóvil es FINISHED
void finishing(Car *car) {
    if (car->state != FINISHED) return;

    if (car->direction == N_TO_S) {
        cout << "Vehículo " << car->id << " terminó de cruzar de norte a sur" << endl;
    } else if (car->direction == S_TO_N) {
        cout << "Vehículo " << car->id << " terminó de cruzar de sur a norte" << endl;
    }
    sleep(1);
}

// Verifica dirección del automóvil y libera al siguiente
void release(Car *car) {
    if (car->direction == N_TO_S) {
        pthread_cond_signal(&crossNtoS);
    } else if (car->direction == S_TO_N){
        pthread_cond_signal(&crossStoN);
    };
};

// Gestiona el paso del automóvil
void wait_to_cross(Car *car) {
    if (car->direction == N_TO_S) {
        pthread_cond_wait(&crossNtoS, &lockBridge);
    } else if (car->direction == S_TO_N){
        pthread_cond_wait(&crossStoN, &lockBridge);
    };
};

// Gestiona la llegada del automóvil
void arriveBridge(Car *car) {
    holding(car); 

    pthread_mutex_lock(&lockBridge);

    if (bridgeDirection == NONE) {
        bridgeDirection = car->direction;
    };

    if (bridgeDirection != car->direction || totalCars == 3)  // Máximo número de vehículos
    {
        wait_to_cross(car);
    };

    totalCars += 1;
    car->state = CROSSING;
    pthread_mutex_unlock(&lockBridge);
};

// Gestiona el cruce del automóvil
void crossBridge(Car *car) {
    crossing(car);
    car->state = FINISHED;
};

// Gestiona la salida del automóvil
void exitBridge(Car *car) {
    finishing(car);

    pthread_mutex_lock(&lockBridge);

    car->state = END;
    totalCars -= 1;

    if (totalCars == 0) {
        bridgeDirection = NONE;
        pthread_cond_signal(&crossNtoS);
        pthread_cond_signal(&crossStoN);
    } else {
        release(car);
    };

    pthread_mutex_unlock(&lockBridge);
};

// Función que ejecuta el hilo
void* drive(void* param) {
    Car* car;
    car = (Car*) param;

    while (true) {
        if(car->state == WAITING) {
            arriveBridge(car);
        };

        if(car->state == CROSSING) {
            crossBridge(car);
        };

        if(car->state == FINISHED) {
            exitBridge(car);
        };
    };

    pthread_exit(NULL);
};

int main(int argc, char* argv[]) {
    // Inicialización de variables
    pthread_t cars_threads[CARS];
    Car cars[CARS];

    // Inicialización de variables de condición y mutex
    pthread_mutex_init(&lockBridge, NULL);
    pthread_cond_init(&crossNtoS, NULL);
    pthread_cond_init(&crossStoN, NULL);

    for(int i = 0; i < CARS; i++){ 
        cars[i].id = i + 1;
        cars[i].state = WAITING;

        if (i % 2 == 0) {
            cars[i].direction = N_TO_S;
        } else {
            cars[i].direction = S_TO_N;
        };

        pthread_create(&cars_threads[i], NULL, drive, &cars[i]);
    }

    for(int i = 0; i < CARS; i++){
        pthread_join(cars_threads[i], NULL);
    }

    return 0;
}