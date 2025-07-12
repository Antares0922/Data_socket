#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
extern "C"{
#include "modules/sqlite3.h"
}

#define PUERTO 8080
#define IP "192.168.0.222"

typedef struct{
    //1 int,long long int
    //2 float, double
    //3 char
    //4 bytes
    int tipo_dato;
    void *Array;
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
    server_conf.sin_addr.s_addr = inet_addr(IP);
    server_conf.sin_port = htons(PUERTO);
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
        struct sockaddr_in cliente_dir;
        socklen_t cliente_len = sizeof(cliente_dir);
        int cliente_fd = accept(server_fd,(struct sockaddr*)&cliente_dir,&cliente_len);  

        //HILO PARA CADA CLIENTE
        int *cp_cliente_fd = (int*)malloc(sizeof(int));
        *cp_cliente_fd = cliente_fd;

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
        close(cliente_fd);
        pthread_exit(nullptr);
    }
    //Obtencion de la ruta a la db
    size_t ruta_len;
    recv(cliente_fd,&ruta_len,sizeof(ruta_len),0);

    char *Ruta_db = (char*)malloc(ruta_len * sizeof(char));
    recv(cliente_fd,Ruta_db,sizeof(char) * ruta_len,0);

    //obtencion de la consulta
    size_t consulta_len;
    recv(cliente_fd,&consulta_len,sizeof(consulta_len),0);

    char *Consulta = (char*)malloc(consulta_len * sizeof(char));
    recv(cliente_fd,Consulta,sizeof(char)*consulta_len,0);

    //obtencion de las columnas
    unsigned int columnas;
    recv(cliente_fd,&columnas,sizeof(columnas),0);

    //obtencion del codigo identificador
    unsigned int identificador;
    recv(cliente_fd,&identificador,sizeof(identificador),0);

    switch(identificador){
        //da la struct sin modificaciones
        case 100: {
                      //se rellena la struct
                      Datos *datos_array = (Datos*)Base_Datos(Ruta_db,Consulta,columnas);
                      free(Ruta_db);
                      free(Consulta);

                      if(datos_array == nullptr){
                          std::cerr << "ERROR CON FUNCION BASE_DATOS\n";
                          free(datos_array);
                          pthread_exit(nullptr);
                      }

                      //se escriben los datos
                      send(cliente_fd,&datos_array->longitud,sizeof(datos_array->longitud),0);  //longitud del array
                      switch(datos_array->tipo_dato){
                          case 1:
                              send(cliente_fd,datos_array->Array,sizeof(long long int) * datos_array->longitud,0);
                              free(datos_array->Array);
                              free(datos_array);
                              break;
                          case 2:
                              send(cliente_fd,datos_array->Array,sizeof(double) * datos_array->longitud,0);
                              free(datos_array->Array);
                              free(datos_array);
                              break;
                      }
                  }
        default:
            break;
    }

    //terminacion del hilo
    close(cliente_fd);
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
        //tipo de dato a extraer
        int tipo = sqlite3_column_type(stmt,0);
        Data->tipo_dato = tipo;
        //saca la informacion de toda la columna
        for(int i = 0;i<columnas;i++){
            Data->longitud++;
            void *dato;
            switch(tipo){
                //int/long long int
                case 1:
                    //rellena el dato
                    dato = (long long int*)malloc(sizeof(long long int));
                    *(long long int*)dato = sqlite3_column_int64(stmt,i);
                    //se coloca el contenido en el array
                    Data->Array = (long long int*)realloc(Data->Array,Data->longitud * sizeof(long long int));
                    ((long long int*)Data->Array)[Data->longitud-1] = *(long long int*)dato;
                    free(dato);
                    break;

                default:
                    break;
            }
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    //retorna el un puntero de la struct
    return Data;
}
