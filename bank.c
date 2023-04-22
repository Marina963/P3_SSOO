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

int client_numop = 0; //Contador operciones cajero (productor)
int bank_numop = 0; //Contador operaciones trabajador (consumidor)
int global_balence = 0; //Dinero del banco
//int saldo_cuenta[];
struct element ** list_client_ops;

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */

int main (int argc, const char * argv[] ) {
    int num_cajeros, num_empleados, max_cuentas, tam_buff, max_operaciones;
    int lectura, i = 0;
    FILE * f;
    char str1[20]; 
    int param1;


    if (argc != 6) {
        perror("Numero de argumentos incorrecto\n");
    }

    f = fopen(argv[1], "r");
    if (f == NULL){
        perror("Error al abrir el fichero\n");
    }

    num_cajeros = atoi(argv[2]);
    num_empleados = atoi(argv[3]);
    max_cuentas = atoi(argv[4]);
    tam_buff = atoi(argv[5]);

    //Lectura de max_operaciones
    fscanf(f, "%d", &max_operaciones);
    if (ferror(f)) {
        perror("Error al leer del fichero\n");
    }
    //max_operaciones = atoi(buffer);
    if (max_operaciones > 200 || max_operaciones < 0){
        perror("Error numero de operaciones incorrecto");
    }

    list_client_ops = (struct element **) malloc(max_operaciones * sizeof(struct element **));

    while((lectura = fscanf(f, "%s", str1)) > 0 ){
        struct element elemento;
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
        }
        //Control de error en la lectura
        if (lectura == 0){         
            perror("Error en la lectura");
        }

        //Paso de valores a los atributos del elemento
        elemento.operacion = str1;
        elemento.num_cuenta = param1;
        if (param2 != 0){
            elemento.elem1 = param2;
        }
        else elemento.elem1 = 0;
        if (param3 != 0){
            elemento.elem2 = param3;
        }
        else elemento.elem2 = 0;
        list_client_ops[i] = &elemento;
        //printf("%s %d %d %d\n", elemento.operacion, elemento.num_cuenta, elemento.elem1, elemento.elem2);
        i ++;
        }
    for (int i = 0; i< max_operaciones; i++){
        printf("%s\n",list_client_ops[i]->operacion);
    }
    //Control de error en la lectura
    if (lectura == 0){
        perror("Error en la lectura");
    }

    
    

    return 0;
}
