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
//int list_client_ops;






/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */

int main (int argc, const char * argv[] ) {
    int num_cajeros, num_empleados, max_cuentas, tam_buff, max_operaciones;
    int fichero, i = 0, l = 0;
    char buffer[4];

    if (argc != 6) {
        perror("Numero de argumentos incorrecto");
    }

    if ((fichero = open(argv[1], O_RDONLY)) == -1 ){
        perror("Error al abrir el fichro");
    }

    num_cajeros = atoi(argv[2]);
    num_empleados = atoi(argv[3]);
    max_cuentas = atoi(argv[4]);
    tam_buff = atoi(argv[5]);

    while(l == 0){
        if ((read(fichero, &buffer[i],1))== -1){
            perror("Error al leer el fichero");
        }
        if (buffer[i] == '\n'){
            
            buffer[i] = '\0';
            l = 1;
        }
        i++;
    }
    while(read(fichero, &buffer[i],1)> 0){
        
    }
        

    max_operaciones = atoi(buffer);
    if (max_operaciones > 200 || max_operaciones < 0){
        perror("Error numero de operaciones incorrecto");
    }

    return 0;
}
