/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: mglevil
 *
 * Created on 7 de diciembre de 2018, 17:52
 */

#include <cstdlib>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <semaphore.h>
#include <string>

#define GREEN "\033[1;32m"
#define BLUE  "\x1B[34m"
#define RED "\033[1;31m"
#define RESET_COLOR "\033[0m"

using namespace std;

//variables globales
int tamanio_buffer;//tamaño maximo del vector
int num_hebras_prod;//numero hebras productoras
bool terminan_prod;//true para que productoras paren al completarse. False para que no terminen
int num_hebras_cons;//numero hebras consumidoras
bool terminan_cons;//true para que consumidoras paren al completarse. False para que no terminen

int contador, entrada, salida;
vector<int> buffer;    

//sem_t mutex, lleno, vacio;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//cerrojo de exclusion mutua con atributos por defecto
//Variables de ocndicion
pthread_cond_t lleno = PTHREAD_COND_INITIALIZER;
pthread_cond_t vacio = PTHREAD_COND_INITIALIZER;



//Funciones
void *funConsumir(void* arg){
    while(true){
        //sem_wait(&mutex);//espero?. restar y comprobar si el numero es negativo o 0, Si lo es espera aque otra hebra haga un signal para desbloquearla.

        pthread_mutex_lock(&mutex);
        //Mientras el buffer este vacio mantener en cola
        while (contador == 0) {
            pthread_cond_wait(&vacio, &mutex);
         //sem_wait(&lleno);
         //sem_wait(&mutex);
        }
        
        /*seccion critica*/
        buffer[salida] = 0;
        cout<< "HebraConsumidora: " << pthread_self() << " consume en posicion: " << salida <<"\n"<<endl;
        cout<<"Buffer consumir: ";
    
        for(int i = 0; i < tamanio_buffer; i++){
            if(i == salida){
                cout<< " " <<BLUE<< buffer[i] <<RESET_COLOR<< " |";
            }else{
                cout<< " " << buffer[i] << " |";
            }
        }
        
        salida = (salida + 1) % tamanio_buffer; //buffer circular
        cout<<"\n"<<endl;
        
        contador -= 1;
        /*fin de seccion critica*/
        
        //liberar hebras en espera(envio de señal +1)
        //sem_post(&mutex);
        //sem_post(&vacio);
        
        pthread_cond_broadcast(&lleno);//despierta a todas los hebras que estén suspendidas
        pthread_mutex_unlock(&mutex);//Abre el cerrojo
        
        //Si el valor es true voy eliminando hebras
        if(terminan_cons == true){
            //num_hebras_cons = (num_hebras_cons - 1);
            pthread_exit(0);
        }
    }
    /*            
    cout<<"Se acabaron las hebras consumidoras... :/"<<endl;
    exit(0);*/      
}


void *funProducir(void* arg){

    while(true){
     	//sem_wait(&mutex);

        //Mientras el buffer este lleno esperar
        pthread_mutex_lock(&mutex);
        while (contador == tamanio_buffer){
            pthread_cond_wait(&lleno, &mutex);
           // sem_wait(&vacio);
            //sem_wait(&mutex);
        }

        /*Seccion critica*/
        buffer[entrada] = 1;

        cout << "HebraProductora: " << pthread_self() << " produce en posicion: " << entrada <<"\n"<<endl;        
        cout<<"Buffer producir: " <<endl;
        
        for(int i = 0; i < tamanio_buffer; i++){
            if(i == entrada){
                cout<< " " <<BLUE<< buffer[i] <<RESET_COLOR<< " |";
            }else{
                cout<< " " << buffer[i] << " |";
            }
        }
        cout<<"\n"<<endl;
        //cout<<"*****************";
        entrada = (entrada + 1) % tamanio_buffer;//buffer circular
        
        contador += 1;
        /*Fin seccion critica*/
        
        //enviar señal
        //sem_post(&mutex);
        //sem_post(&lleno);
        pthread_cond_broadcast(&vacio);//despierta a todas los hebras que estén suspendidas
        pthread_mutex_unlock(&mutex);//Abre el cerrojo
        
        //Si el valor es true voy eliminando hebras
        if(terminan_prod == true){
            //num_hebras_prod = (num_hebras_prod - 1);
            pthread_exit(0);  
        }                
    }
    /*            
    cout<<"Se acabaron las hebras productoras... :/"<<endl;
    exit(0);*/       
}

/*
 * 
 */
int main(int argc, char** argv) {
    
    if(argc != 6){
        cout<< "Introduzca 5 parametros" << endl;
        exit(0);
    }else{
        //establecer valores introducidos
        tamanio_buffer = atoi(argv[1]);
        num_hebras_prod = atoi(argv[2]);
        terminan_prod = atoi(argv[3]);
        num_hebras_cons = atoi(argv[4]);
        terminan_cons = atoi(argv[5]);

        //asignar tamaño buffer
        buffer.resize(tamanio_buffer);
        
        /*
        for (int i: buffer){//mostar buffer
            cout<< " " << i << " |";
        }*/
    }

    
    cout<< "Tamaño buffer: "<< tamanio_buffer<< "\t";
    cout<< "Hebras productoras: " << num_hebras_prod << "\t";
    cout<< "Hebras consumidoras: " << num_hebras_cons << "\t";
    cout<< "Consumir: " << terminan_cons << "\t";
    cout<< "Producir: " << terminan_prod <<"\n"<<endl;
    
    //declarar hebras
    pthread_t consumidora[num_hebras_cons], productora[num_hebras_prod];
    
    //iniciar semaforos
    /*sem_init(&mutex, 0, 1);
    sem_init(&vacio, 0, tamanio_buffer);
    sem_init(&lleno, 0, 0);*/
    
    //crear hebras
    for(int i = 0; i < num_hebras_prod; i++){
        pthread_create(&productora[i], NULL, funProducir, NULL);
    }
    for(int i = 0; i < num_hebras_cons; i++){
        pthread_create(&consumidora[i], NULL, funConsumir,NULL);
    }
 
    //join, esperar que acaben las hebras
    for(int i = 0; i < num_hebras_prod; i++){
        pthread_join(productora[i], NULL);
    }
    for(int i = 0; i < num_hebras_cons; i++){
        pthread_join(consumidora[i], NULL);
    }
    
    //liberar memoria ocupada
    /*sem_destroy(&mutex);
    sem_destroy(&vacio);
    sem_destroy(&lleno);*/
    int pthread_cond_destroy(pthread_cond_t *lleno);
    int pthread_cond_destroy(pthread_cond_t *vacio);
    int pthread_mutex_destroy(pthread_mutex_t *mutex);
    
    cout<<"Acabaron todas las hebras productoras y consumidoras"<<endl;
    exit(0);
}

