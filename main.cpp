#include <cstdlib>
#include <iostream>
#include <fstream>

#include "mpi.h"
#include <pthread.h>

/**
 * variables globales, para los hilos
**/

// puntero a los arreglos
int * arregloTotal;
int **matrizLocal;

// cantidad de numeros de la matriz
int filas = 0;
int columnas = 0;

//procedimiento para el hilo
void* sumarFilas(void*);

nt main(int argc, char ** argv){
    // identificador de cada proceso;
    int idProceso;
    // numero total de procesos;
    int numProcesos;
    // Iniciliza MPI
    MPI_Init(&argc,&argv);
    MPI_Status status;
    
    // bandera de comprobacion
    bool parametrosCorrectos = true;
    
    //Obtiene el numero total de procesos
    MPI_Comm_size(MPI_COMM_WORLD,&numProcesos); 
   
    //Obtiene el identificador del proceso
    MPI_Comm_rank(MPI_COMM_WORLD,&idProceso);
    
    if( idProceso == 0 ){
        srand(time(0));
        // solicita el numero de elementos que contendra el arreglo
        std::cout << "Inserte el numero de columnas";
        std::cin >> columnas;
        std::cout << "Inserte el numero de filas";
        std::cin >> filas;
        if(! filas || ! columnas){ // se ingreso un numero invalido
            std::cout << "Error en el numero ingresado, o ingresado un 0" << std::endl;
        }
        
            // verifica que las matriz sea bidimensional
        if( filas && columnas &&
            // verifica que el numero de filas sea par
            !(filas%2)
         )
        {
            // inicializa el arreglo de resultados
            arregloTotal = new int[filas];
        }
        else {
            std::cout << "ejecucion terminada con error, el numero de filas debe ser par" << std::endl;
            parametrosCorrectos = false;
        }
        
        
    }
    //Espera todos los procesos
    MPI_Bcast(&parametrosCorrectos, 1, MPIR_CXX_BOOL, 0, MPI_COMM_WORLD);
    
    if( parametrosCorrectos ){
        // pasa a todos los procesos la cantidad de filas
        MPI_Bcast(&filas, 1, MPI_INT, 0, MPI_COMM_WORLD);
        /// pasa a todos los procesos la cantidad de columnas
        MPI_Bcast(&columnas, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        //crea la matriz local de cada proceso 
        arregloLocal = new int*[filas];
        for(int i = 0; i < filas; ++i){
            arregloLocal[i] = new int[columnas];
            for(int j = 0; j < columnas; ++j){
                arregloLocal[i] = ( rand()%201 )-100;
            }
        }
        std::ofstream archivoListaP("Lista"+idProceso+".txt");
        // verifica si puedo abrise
        if( archivoListaP.is_open() )
        {
            for(int i = 0; i < filas; ++i){
                for(int j = 0; j < columnas; ++j){
                    archivoListaP << arregloLocal[i] << " ";
                }
                archivoListaP << std::endl;
            }
            
        }
            
        // los dos hilos que se van a usar para realizar la suma
        pthread_t hiloA;
        pthread_t hiloB;
        
        pthread_create(&hilo1, NULL, hiloA, (void*)1);
        pthread_create(&hilo1, NULL, hiloA, (void*)1);
        
        pthread_join(hilo1, NULL);
        pthread_join(hilo2, NULL);
           
        MPI_Reduce(arregloTotal, arregloTotal, filas, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
        
        if(idProceso==0){
            delete[] arregloTotal;
            // abre el archivo ListaF
            std::ofstream archivoListaFinal("ListaF.txt");
            // verifica si puedo abrise
            if( archivoListaFinal.is_open() )
            {
                // copia los elementos del arreglo en el archivo
                for(int i = 0; i < tamArreglo;++i){
                    archivoListaFinal << arregloLocal[i] << std::endl;
                }
            }
            else{
                std::cerr << "No se pudo crear el archivo ListaI.txt" << std::endl;
                //Se advierte pero se continua con la ejecución normal
            }
            
            // variable para la respuesta del usuario
            char respuesta = 0;
            std::cout << "Desea que se muestre en pantalla el vector ordenado? ( S | [N] )";
            std::cin >> respuesta;
            // si el usuario selecciono ver el resultado en pantalla lo imprime
            if(respuesta == 'S' | respuesta == 's')
            {
                for(int i=0; i<tamArreglo;++i){
                    std::cout << arregloLocal[i] << std::endl;
                }
            }
            
            // elimina su arreglo local antes de finalizar
            delete[] arregloLocal;
        }
        
    }
    // no se lleva a cabo la ejecucion
    else{
        // unicamente el proceso cero, indica que hubo un error en los parametros 
        if(idProceso == 0){
            std::cout << "parametros incorrectos, el numero de procesos debe ser potencia de dos y la cantidad de elementos a ordenar debe ser multiplo de la cantidad de procesos" << std::endl;
        }
    }
    //Fin MPI
    MPI_Finalize();
    return 0;
}

void* sumarFilas(void* indicador){
    int indicador = (int)indicador;
    int i,f;
    if(indicador%2){
        i = 0;
        j = filas/2;
    }
    else{
        i = filas/2;
        j = filas;
    }
    for(;i<j;++i){
        for(int c = 0; c < columnas;++c){
            arregloTotal[i] += matrizLocal[i][c];
        }
    }
}
