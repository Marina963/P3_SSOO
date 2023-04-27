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

typedef struct cuentas{
    int id;
    int saldo;
}cuentas;

int max_operaciones = 0;
int client_numop = 0; //Contador operciones cajero (productor)
int bank_numop = 0; //Contador operaciones trabajador (consumidor)
int global_balence = 0; //Dinero del banco
int max_cuentas = 0;
int fin = 0;
int num_cuentas = 0;

//int saldo_cuenta[];
struct element * list_client_ops;
cuentas *list_cuentas;
queue *cola;

pthread_mutex_t mutex;
pthread_cond_t no_lleno, no_vacio;
pthread_attr_t lista;

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */

void crear(int id){
    cuentas res;
    int fin = 0, i = 0; 
    if (num_cuentas == max_cuentas){
        perror("No se pueden crear mas cuentas\n");
        exit(-1);
    }
    while(fin == 0){
        
            
        if (num_cuentas == i){
            fin = 1;
            res.id = id;
            res.saldo = 0;
            list_cuentas[i] = res;
            num_cuentas ++;
            printf("%d CREAR %d SALDO=%d TOTAL=%d\n", bank_numop + 1, list_cuentas[i].id, list_cuentas[i].saldo, global_balence);
        }
        else if (list_cuentas[i].id == id){
            perror("Esta cuenta ya esta creada\n");
            exit(-1);
        }
        i ++;
    }
    
}

void ingresar(int id, int dinero){
    int fin = 0, i = 0; 
    while(fin == 0){
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            return;
        }
        else if (list_cuentas[i].id == id){
            fin = 1;
            list_cuentas[i].saldo = list_cuentas[i].saldo + dinero;
            global_balence = global_balence + dinero;
            printf("%d INGRESAR %d %d SALDO=%d TOTAL=%d\n", bank_numop + 1,  list_cuentas[i].id, dinero, list_cuentas[i].saldo, global_balence);
        }
        i ++;
    }
}

void retirar(int id, int dinero){
    int fin = 0, i = 0; 
    while(fin == 0){
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            exit(-1);
        }
        else if (list_cuentas[i].id == id){
            fin = 1;
            list_cuentas[i].saldo = list_cuentas[i].saldo - dinero;
            global_balence = global_balence - dinero;
            printf("%d RETIRAR %d %d SALDO=%d TOTAL=%d\n", bank_numop + 1, list_cuentas[i].id, dinero, list_cuentas[i].saldo, global_balence);
        }
        i ++;
    }
}

void saldo(int id){
    int fin = 0, i = 0; 
    while(fin == 0){
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            exit(-1);
        }
        else if (list_cuentas[i].id == id){
            fin = 1;
            printf("%i SALDO %d SALDO=%d TOTAL=%d\n", bank_numop + 1, list_cuentas[i].id, list_cuentas[i].saldo, global_balence);
        }
        i ++;
    }
}

void traspasar(int id1, int id2, int dinero){
    int fin = 0, i = 0, indice = 0, indice2 = 0; 
    while(fin == 0){
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            exit(-1);
        }
        else if (list_cuentas[i].id == id1){
            fin = 1;
            indice = i;
        }
        i ++;
    }
    
    i = 0;
    fin = 0;

    while(fin == 0){
        if (num_cuentas == i){
            perror("No existe la cuenta\n");
            exit(-1);
        }
        else if (list_cuentas[i].id == id2){
            fin = 1;
            indice2 = i;
        }
        i ++;
    }

    list_cuentas[indice].saldo = list_cuentas[indice].saldo - dinero;
    list_cuentas[indice2].saldo = list_cuentas[indice2].saldo + dinero;

    printf("%d TRASPASAR %d %d %d SALDO=%d TOTAL=%d\n", bank_numop + 1, list_cuentas[indice].id, list_cuentas[indice2].id, dinero, list_cuentas[indice2].saldo, global_balence);
}


void cajero(){
    
    struct element dato;
    struct element* ptr;
    while(client_numop < max_operaciones){
        pthread_mutex_lock(&mutex);
        while(queue_full(cola) == 0){
            pthread_cond_wait(&no_lleno, &mutex);
        }
        dato = list_client_ops[client_numop];
        //printf("Numero de elementos en la cola productor: %d\n", cola->n_elem);
        ptr = (struct element *) malloc(sizeof(struct element));
        //printf("cajero %p\n", ptr);
        //ptr = &dato;
        memcpy(ptr, &dato, sizeof(struct element));
        //printf("cajero 2 %p\n", ptr);
        queue_put(cola, ptr);
        client_numop ++;
        pthread_cond_signal(&no_vacio);
        pthread_mutex_unlock(&mutex);

    }
    pthread_mutex_lock(&mutex);
    fin=1;
    pthread_cond_broadcast(&no_vacio);
    pthread_mutex_unlock(&mutex);
    //printf("Cajero\n");
    pthread_exit(0);
}

void empleado(){
    struct element *dato;
    while(bank_numop < max_operaciones){
        pthread_mutex_lock(&mutex);
        while(queue_empty(cola) == 0){
            if (fin==1) {
                //fprintf(stderr, "Finalizando servicio\n");                    
                pthread_mutex_unlock(&mutex);
                pthread_exit(0);
            }
            pthread_cond_wait(&no_vacio, &mutex);
        }
        //printf("Numero de elementos en la cola consumidor: %d\n", cola->n_elem);
        dato = queue_get(cola);
        //printf("empleado %p\n", cola);
        
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
    }
    //printf("Empleado\n");
    pthread_exit(0);
}

int main (int argc, const char * argv[] ) {
    int num_cajeros, num_empleados, tam_buff;
    int lectura, i = 0;
    FILE * f;
    char str1[20]; 
    int param1;


    if (argc != 6) {
        perror("Numero de argumentos incorrecto\n");
        exit(-1);
    }

    f = fopen(argv[1], "r");
    if (f == NULL){
        perror("Error al abrir el fichero\n");
        exit(-1);
    }

    num_cajeros = atoi(argv[2]);
    num_empleados = atoi(argv[3]);
    max_cuentas = atoi(argv[4]);
    tam_buff = atoi(argv[5]); //tam_buff

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

    pthread_t *list_cajeros = (pthread_t *) malloc(num_cajeros * sizeof(pthread_t));
    pthread_t *list_empleados = (pthread_t *) malloc(num_empleados * sizeof(pthread_t));
    
    list_client_ops = (struct element *) malloc(max_operaciones * sizeof(struct element ));
    list_cuentas = (cuentas *)malloc(max_cuentas * sizeof(cuentas));

    int contador = 0;
    while((lectura = fscanf(f, "%s", str1)) > 0 ){
        //printf("%i\n",i);
        //struct element elemento;
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
        //list_client_ops[i] = elemento;
        //printf("%s %d %d %d\n", list_client_ops[i].operacion, list_client_ops[i].num_cuenta, list_client_ops[i].elem1, list_client_ops[i].elem2);
        i ++;
        }
    if (contador != max_operaciones){
        perror("Numero de operaciones mal indicado");
        exit(-1);
    }

    for (int j = 0; j< max_operaciones; j++){
        //printf("%s\n",list_client_ops[j].operacion);   
    }
    //Control de error en la lectura
    if (lectura == 0){
        perror("Error en la lectura");
    }

    //declaracion semaforos

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&no_lleno,NULL);
    pthread_cond_init(&no_vacio,NULL);

    //pthread_attr_init(&lista);

    cola = queue_init(tam_buff);
    

    for (int i = 0; i< num_cajeros; i++){
        pthread_create(&list_cajeros[i],NULL, (void*)cajero, NULL);
    }

    for (int j = 0; j< num_empleados; j++){
        pthread_create(&list_empleados[j],NULL, (void*)empleado, NULL);
    }

    for (int i = 0; i< num_cajeros; i++){
        pthread_join(list_cajeros[i],NULL);
    }
    
    for (int j = 0; j< num_empleados; j++){
        pthread_join(list_empleados[j],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_lleno);
    pthread_cond_destroy(&no_vacio);

    //pthread_attr_destroy();

    //Borrar semafor

    free(list_cajeros);
    free(list_empleados);
    free(list_client_ops);
    queue_destroy(cola);
    
    

    return 0;
}
