#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
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

//procedimiento para que correra cada hilo
void* sumarFilas(void*);

int main(int argc, char ** argv){
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
        // solicita el numero de elementos que contendra el arreglo
        std::cout << "Inserte el numero de columnas: ";
        std::cin >> columnas;
        std::cout << "Inserte el numero de filas: ";
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
            // es valido
        }
        else {
            std::cout << "ejecucion terminada con error, el numero de filas debe ser par" << std::endl;
            parametrosCorrectos = false;
        }
        
        
    }
    //Espera todos los procesos y dice si la ejecuicion continua
    MPI_Bcast(&parametrosCorrectos, 1, MPIR_CXX_BOOL, 0, MPI_COMM_WORLD);
    
    if( parametrosCorrectos ){
        // pasa a todos los procesos la cantidad de filas
        MPI_Bcast(&filas, 1, MPI_INT, 0, MPI_COMM_WORLD);
        /// pasa a todos los procesos la cantidad de columnas
        MPI_Bcast(&columnas, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        //iniciliza la semilla del random utilizando otros parametros para que varie de proceso a proceso
        srand(time(0)+idProceso*filas+idProceso/columnas);
        srand(time(0)+rand()*numProcesos);
        
        // inicializa el arreglo de resultados
        arregloTotal = new int[filas];
        for(int i = 0; i < filas; ++i){
            arregloTotal[i] = 0;
        }
        
        //crea la matriz local de cada proceso 
        matrizLocal = new int*[filas];
        for(int i = 0; i < filas; ++i){
            matrizLocal[i] = new int[columnas];
            for(int j = 0; j < columnas; ++j){
                matrizLocal[i][j] = ( rand()%201 )-100;
            }
        }
        
        //  los hilos que haran la suma
        pthread_t hiloA;
        pthread_t hiloB;
        
        // pone a correr los hilos que haran la suma
        pthread_create(&hiloA, NULL, sumarFilas, (void*)1);
        pthread_create(&hiloB, NULL, sumarFilas, (void*)0);
        
        // espera a que los hilos terminen
        pthread_join(hiloA, NULL);
        pthread_join(hiloB, NULL);
        
        //elimina la matriz local
        for(int i = 0; i < filas; ++i){
            delete[] matrizLocal[i];
        }
        delete[] matrizLocal;
           
        //reduce todas las sumas por filas a un solo arreglo
        MPI_Reduce(arregloTotal, arregloTotal, filas, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        
        if(idProceso==0){
            // abre el archivo V
            std::ofstream archivoListaFinal("V.txt");
            // verifica si puedo abrise
            if( archivoListaFinal.is_open() )
            {
                // copia los elementos del arreglo en el archivo
                for(int i = 0; i < filas;++i){
                    archivoListaFinal << arregloTotal[i] << std::endl;
                }
            }
            else{
                std::cerr << "No se pudo crear el archivo V.txt" << std::endl;
                //Se advierte pero se continua con la ejecución normal
            }
            
            // variable para la respuesta del usuario
            char respuesta = 0;
            std::cout << "Desea que se muestre en pantalla el vector de sumas? ( S | [N] )";
            std::cin >> respuesta;
            // si el usuario selecciono ver el resultado en pantalla lo imprime
            if(respuesta == 'S' | respuesta == 's')
            {
                for(int i=0; i<filas;++i){
                    std::cout << arregloTotal[i] << std::endl;
                }
            }
        }
        delete[] arregloTotal;  
    }
    // no se lleva a cabo la ejecucion
    else{
        // unicamente el proceso cero, indica que hubo un error en los parametros 
        if(idProceso == 0){
            std::cout << "parametros incorrectos" << std::endl;
       }
    }
    //Fin MPI
    MPI_Finalize();
    return 0;
}

void* sumarFilas(void* indicadorP){
    // convierte el valor enviado por parametro
    int indicador = (long)indicadorP;
    int i,j;
    //verifica si le toca sumar la primera o la segunda mitad
    if(indicador){
        i = 0;
        j = filas/2;
    }
    else{
        i = filas/2;
        j = filas;
    }
    //realiza la suma
    for(;i<j;++i){
        for(int c = 0; c < columnas;++c){
            arregloTotal[i] += matrizLocal[i][c];
        }
    }
}
