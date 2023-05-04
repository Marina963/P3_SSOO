//SSOO-P3 2022-2023

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

//Estructura para crear las distintas cuentas bancarias donde se almacena su numero y el dinero que hay en ella
typedef struct cuentas{
    int id;
    int saldo;
}cuentas;

//Variables globales
int max_operaciones = 0;
int client_numop = 0; //Contador operciones cajero (productor)
int bank_numop = 0; //Contador operaciones trabajador (consumidor)
int global_balence = 0; //Dinero del banco
int max_cuentas = 0; //Numero máximo de cuentas
int fin = 0;
int num_cuentas = 0; //Numero de cuentas actuales en el banco

//Declaración de objetos
struct element * list_client_ops;
cuentas *list_cuentas;
queue *cola;

//Declaración de mutex
pthread_mutex_t mutex;
pthread_cond_t no_lleno, no_vacio;


/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */

//Funcion que se encarga de crear las cuentas
void crear(int id){
    cuentas res;
    int fin = 0, i = 0; 

    //Si ha llegado a la máximo número de cuentas  
    if (num_cuentas == max_cuentas){
        perror("No se pueden crear mas cuentas\n");
        exit(-1);
    }

    //Bucle para buscar la posición donde crear la cuenta 
    while(fin == 0){  
        //Si encuenta la posición
        if (num_cuentas == i){
            fin = 1;
            res.id = id;
            res.saldo = 0;
            list_cuentas[i] = res;
            num_cuentas ++;
            printf("%d CREAR %d SALDO=%d TOTAL=%d\n", bank_numop + 1, list_cuentas[i].id, list_cuentas[i].saldo, global_balence);
        }

        // Si la cuenta ya está creada
        else if (list_cuentas[i].id == id){
            perror("Esta cuenta ya esta creada\n");
            exit(-1);
        }
        i ++;
    }
    
}

//Función que ingresa el dinero en la cuenta
void ingresar(int id, int dinero){
    int fin = 0, i = 0; 

    //Bucle que busca la cuenta
    while(fin == 0){
        //Si la cuenta no existe
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            return;
        }
        //Si la encuentra inserta el dinero
        else if (list_cuentas[i].id == id){
            fin = 1;
            list_cuentas[i].saldo = list_cuentas[i].saldo + dinero;
            global_balence = global_balence + dinero;
            printf("%d INGRESAR %d %d SALDO=%d TOTAL=%d\n", bank_numop + 1,  list_cuentas[i].id, dinero, list_cuentas[i].saldo, global_balence);
        }
        i ++;
    }
}

//Función para retirar el dinero 
void retirar(int id, int dinero){
    int fin = 0, i = 0; 

    //Bucle para buscar la cuenta
    while(fin == 0){
        //Si la cuenta no existe
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            exit(-1);
        }
        //Si encuentra la cuenta retira el dinero
        else if (list_cuentas[i].id == id){
            fin = 1;
            list_cuentas[i].saldo = list_cuentas[i].saldo - dinero;
            global_balence = global_balence - dinero;
            printf("%d RETIRAR %d %d SALDO=%d TOTAL=%d\n", bank_numop + 1, list_cuentas[i].id, dinero, list_cuentas[i].saldo, global_balence);
        }
        i ++;
    }
}

//Función para ver el dinero que hay en la cuenta
void saldo(int id){
    int fin = 0, i = 0;
    //Bucle que busca la cuenta 
    while(fin == 0){
        //Si la cuenta no existe
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            exit(-1);
        }
        //Si encuentra la cuenta
        else if (list_cuentas[i].id == id){
            fin = 1;
            printf("%i SALDO %d SALDO=%d TOTAL=%d\n", bank_numop + 1, list_cuentas[i].id, list_cuentas[i].saldo, global_balence);
        }
        i ++;
    }
}

//Función para pasar dinero de una cuenta a otra
void traspasar(int id1, int id2, int dinero){
    int fin = 0, i = 0, indice = 0, indice2 = 0; 
    //Bucle que busca las dos cuentas
    while(fin <2 ){
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            exit(-1);
        }
        else if (list_cuentas[i].id == id1){
            fin ++;
            indice = i;
        }
        else if (list_cuentas[i].id == id2){
            fin ++;
            indice2 = i;
        }
        i ++;
    }
    
    //Resta el dinero de la primera cuenta
    list_cuentas[indice].saldo = list_cuentas[indice].saldo - dinero;
    //Suma el dinero en la segunda cuenta
    list_cuentas[indice2].saldo = list_cuentas[indice2].saldo + dinero;

    printf("%d TRASPASAR %d %d %d SALDO=%d TOTAL=%d\n", bank_numop + 1, list_cuentas[indice].id, list_cuentas[indice2].id, dinero, list_cuentas[indice2].saldo, global_balence);
}


//Funcion productor
void cajero(){
    struct element dato;
    struct element* ptr;

    //Bucle que se encarga de producir las distintas ooperaciones
    while(client_numop < max_operaciones){
        //Inicio de la sección critica
        pthread_mutex_lock(&mutex);
        //Si la cola esta llena no produce
        while(queue_full(cola) == 0){
            pthread_cond_wait(&no_lleno, &mutex);
        }
        //Inserta el dato en el buffer
        dato = list_client_ops[client_numop];
        ptr = (struct element *) malloc(sizeof(struct element));
        memcpy(ptr, &dato, sizeof(struct element));
        queue_put(cola, ptr);
        //Aumenta la variable global y sale del mutex
        client_numop ++;
        pthread_cond_signal(&no_vacio);
        pthread_mutex_unlock(&mutex);
        //Fin de la sección critica

    }
    //Cuando se terminas las operaciones se manda la señal de despertar a los consumidores que se han quedado bloqueados
    pthread_mutex_lock(&mutex);
    fin=1;
    pthread_cond_broadcast(&no_vacio);
    pthread_mutex_unlock(&mutex);
    pthread_exit(0);
}

void empleado(){
    struct element *dato;
    //Bucle que consume las operaciones
    while(bank_numop < max_operaciones){
        //Inicio sección crítica
        pthread_mutex_lock(&mutex);
        //Si la cola está vacía no se consume
        while(queue_empty(cola) == 0){
            //Si ha acabdo se desbloquea el mutex y se matan a los hilos
            if (fin==1) {                
                pthread_mutex_unlock(&mutex);
                pthread_exit(0);
            }
            pthread_cond_wait(&no_vacio, &mutex);
        }
        //Se obtiene el dato de la cola
        dato = queue_get(cola);
        
        //Se llama a la operación correspondiente
        if(strcmp(dato->operacion, "CREAR") == 0){
            crear(dato->num_cuenta);
        }
        else if (strcmp(dato->operacion, "INGRESAR") == 0){
            ingresar(dato->num_cuenta, dato->elem1);
        }
        else if (strcmp(dato->operacion, "RETIRAR") == 0){
            retirar(dato->num_cuenta, dato->elem1);
        }
        else if (strcmp(dato->operacion, "SALDO") == 0){
            saldo(dato->num_cuenta);
        }
        else if (strcmp(dato->operacion, "TRASPASAR") == 0){
            traspasar(dato->num_cuenta, dato->elem1, dato->elem2);
        }
        else{
            perror("Operacion no admitida\n");
        }
        

		free(dato);
        bank_numop ++;
        pthread_cond_signal(&no_lleno);
        pthread_mutex_unlock(&mutex);
        //Fin sección crítica
    }
    pthread_exit(0);
}

int main (int argc, const char * argv[] ) {
    int num_cajeros, num_empleados, tam_buff;
    int lectura, i = 0;
    int param1;
    char str1[20]; 
    FILE * f;
    
    //Comprobación de que la función tiene 6 argumentos
    if (argc != 6) {
        perror("Numero de argumentos incorrecto\n");
        exit(-1);
    }

    //Se abre el fichero con las operaciones y se comprueba si se ha abierto correctamente 
    f = fopen(argv[1], "r");

    if (f == NULL){
        perror("Error al abrir el fichero\n");
        exit(-1);
    }

    //Se guardan en variables los argumentos
    num_cajeros = atoi(argv[2]);
    num_empleados = atoi(argv[3]);
    max_cuentas = atoi(argv[4]);
    tam_buff = atoi(argv[5]); 

    //Comprobaciones de los argumentos

    if (num_cajeros < 1 || num_cajeros > 200 ){
        perror("Numero de cajeros erroneo");
        exit(-1);
    }

    if (num_empleados < 1 || num_empleados > 200 ){
        perror("Numero de cajeros erroneo");
        exit(-1);
    }
    if (tam_buff < 1 ){
        perror("Numero de cajeros erroneo");
        exit(-1);
    }
    if (max_cuentas < 1 ){
        perror("Numero de cajeros erroneo");
        exit(-1);
    }

    //Lectura de max_operaciones
    fscanf(f, "%d", &max_operaciones);
    if (ferror(f)) {
        perror("Error al leer del fichero\n");
    }
    //max_operaciones = atoi(buffer);
    if (max_operaciones > 200 || max_operaciones < 0){
        perror("Error numero de operaciones incorrecto");
    }

    //Reserva de memoria dinamica
    pthread_t *list_cajeros = (pthread_t *) malloc(num_cajeros * sizeof(pthread_t));
    pthread_t *list_empleados = (pthread_t *) malloc(num_empleados * sizeof(pthread_t));
    
    list_client_ops = (struct element *) malloc(max_operaciones * sizeof(struct element ));
    list_cuentas = (cuentas *)malloc(max_cuentas * sizeof(cuentas));

    int contador = 0;
    // Bucle que lee el fichero
    while((lectura = fscanf(f, "%s", str1)) > 0 ){
        int param2 = 0, param3 = 0;
    
        //Check de la operación a realizar
        if (strcmp(str1, "INGRESAR") == 0){
            lectura = fscanf(f, "%d %d", &param1, &param2);
        }
        else if (strcmp(str1, "CREAR") == 0){
            lectura = fscanf(f, "%d", &param1);
        }
        else if (strcmp(str1,"RETIRAR") == 0){
            lectura = fscanf(f, "%d %d", &param1, &param2);
        }
        else if (strcmp(str1, "TRASPASAR") == 0){
            lectura = fscanf(f, "%d %d %d", &param1, &param2, &param3);
        }
        else if (strcmp(str1, "SALDO") == 0){
            lectura = fscanf(f,"%d", &param1);
        }
        //El fichero no tiene una operación válida
        else{
            perror("Esa operación no existe");
            exit(-1);
        }
        contador ++;
        //Control de error en la lectura
        if (lectura == 0){         
            perror("Error en la lectura");
            exit(-1);
        }

        //Paso de valores a los atributos del elemento
        strcpy(list_client_ops[i].operacion, str1);
        list_client_ops[i].num_cuenta = param1;
        if (param2 != 0){
            list_client_ops[i].elem1 = param2;
        }
        else list_client_ops[i].elem1 = 0;
        if (param3 != 0){
            list_client_ops[i].elem2 = param3;
        }
        else list_client_ops[i].elem2 = 0;
        i ++;
        }
    
    //Comprobación del número max de operaciones
    if (contador != max_operaciones){
        perror("Numero de operaciones mal indicado");
        exit(-1);
    }
    
    //Control de error en la lectura
    if (lectura == 0){
        perror("Error en la lectura");
    }

    //Inicialización de los mutex
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&no_lleno,NULL);
    pthread_cond_init(&no_vacio,NULL);

    //Inicialización de la cola
    cola = queue_init(tam_buff);
    
    //Creacion de los pthread
    for (int i = 0; i< num_cajeros; i++){
        pthread_create(&list_cajeros[i],NULL, (void*)cajero, NULL);
    }

    for (int j = 0; j< num_empleados; j++){
        pthread_create(&list_empleados[j],NULL, (void*)empleado, NULL);
    }

    //Espera de los pthread
    for (int i = 0; i< num_cajeros; i++){
        pthread_join(list_cajeros[i],NULL);
    }
    
    for (int j = 0; j< num_empleados; j++){
        pthread_join(list_empleados[j],NULL);
    }

    //Destrucción del mutex
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_lleno);
    pthread_cond_destroy(&no_vacio);

    //Liberar memoria dinámica
    free(list_cajeros);
    free(list_empleados);
    free(list_client_ops);
    free(list_cuentas);

    queue_destroy(cola);
    
    return 0;
}
