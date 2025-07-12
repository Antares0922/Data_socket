#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include "modules/sqlite3.h"

typedef struct{
    long long int *Array;
    size_t longitud;
} Datos;

//conexion con cada cliente
void*Conexion_Cliente(void*arg);

//Conexion con la base de datos
void* Base_Datos(const char*Ruta_DB,const char*Consulta,unsigned int columnas);



int main(){
    //Socket Server
    int server_fd = socket(AF_INET,SOCK_STREAM,0);
    if (server_fd < 0){
        std::cerr << "ERROR AL CREAR EL FD SERVER\n";
        exit(1);
    }
    //Socket Server Conf
    struct sockaddr_in server_conf;
    server_conf.sin_addr.s_addr = inet_addr("192.168.0.156");
    server_conf.sin_port = htons(8080);
    server_conf.sin_family = AF_INET;
    for (int i = 0; i<8; i++){server_conf.sin_zero[i] = 0;}

    //Asociacion de ip y puerto
    if (bind(server_fd,(struct sockaddr*)&server_conf,sizeof(server_conf)) < 0){
        std::cerr << "ERROR AL BINDEAR EL SERVER\n";
        close(server_fd);
        exit(1);
    }

    std::cout << "Escuchando en el puerto 8080" << std::endl;
    listen(server_fd,2);

    //Atendiendo cada cliente
    while(true){
        //Cliente FD
        struct sockaddr_in cliente_conf;
        int cliente_fd = connect(cliente_fd,(struct sockaddr*)&cliente_conf,sizeof(cliente_conf));  

        //HILO PARA CADA CLIENTE
        int *cp_cliente_fd = (int*)malloc(sizeof(int));
        cp_cliente_fd = &cliente_fd;

        pthread_t thread;
        pthread_create(&thread,NULL,&Conexion_Cliente,(void*)cp_cliente_fd);
        pthread_detach(thread);
    }
    return 0;
}

void*Conexion_Cliente(void*arg){
    int cliente_fd = *(int*)arg;
    free(arg);
    //verificar el FD
    if(cliente_fd < 0){
        std::cerr << "ERROR CON EL CLIENTE Hilo:" << pthread_self() << std::endl;
        pthread_exit(nullptr);
    }
    //opciones de data
    bool estado = true;
    while(estado){
        void*Resultados = (Datos*)malloc(sizeof(Datos));
        //num de solicitud de dato
        unsigned int solicitud;
        recv(cliente_fd,&solicitud,sizeof(unsigned int),0);
        //parametros
        char* Ruta_DB;
        size_t ruta_long;

        char *Consulta;
        size_t consulta_long;

        unsigned int columnas;

        switch(solicitud){
            case 100:   //Da el array
                //recibiendo la ruta
                recv(cliente_fd,&ruta_long,sizeof(size_t),0);
                recv(cliente_fd,Ruta_DB,sizeof(char)*ruta_long,0);

                //recibiendo la consulta
                recv(cliente_fd,&consulta_long,sizeof(size_t),0);
                recv(cliente_fd,Consulta,sizeof(char)*consulta_long,0);

                //numero de columnas de la consulta
                recv(cliente_fd,&columnas,sizeof(unsigned int),0);

                Resultados = (Datos*)Base_Datos(Ruta_DB,Consulta,columnas);

                //enviando la longitud del array
                send(cliente_fd,&(Datos*)Resultados->longitud,sizeof(size_t),0);
                send(cliente_fd,Resultados->Array,sizeof(long long int)*Resultados->longitud,0);
                break;
            case 1:
                estado = false;
                break;
        }
    }
    return nullptr;
}

void* Base_Datos(const char*Ruta_DB,const char*Consulta,unsigned int columnas){
    //conexion a la base de datos
    sqlite3 *DB;
    if(sqlite3_open(Ruta_DB,&DB) != SQLITE_OK){
        std::cerr << "Error al abrir la base de datos " << sqlite3_errmsg(DB) << std::endl;
        return nullptr;
    }
    //preparando la consulta
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(DB,Consulta,-1,&stmt,nullptr) != SQLITE_OK){
        std::cerr << "Error al preparar la consulta\n";
        return nullptr;
    }
    //Obteniendo los datos
    Datos *Data = (Datos*)malloc(sizeof(Datos));
    Data->longitud = 0;

    while(sqlite3_step(stmt) == SQLITE_ROW){
        //saca la informacion de toda la fila
        for(int i = 0;i<columnas;i++){
            Data->longitud++;
            long long int numero = sqlite3_column_int64(stmt,i);
            Data->Array = (long long int*)realloc(Data->Array,Data->longitud * sizeof(long long int));
            Data->Array[Data->longitud-1] = numero;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    //retorna el puntero de la lista
    return Data;
}
