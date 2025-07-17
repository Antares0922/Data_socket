#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <cstdlib>
#include <cstring>
extern "C"{
#include "modules/datos.h"
}

#define PUERTO 8080
#define IP "192.168.0.222"
 

//conexion con cada cliente
void*Conexion_Cliente(void*arg);


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

    //obtencion de los numeros de columnas
    int num_columnas;
    recv(cliente_fd,&num_columnas,sizeof(num_columnas),0);

    //obtencion del codigo identificador
    unsigned int identificador;
    recv(cliente_fd,&identificador,sizeof(identificador),0);

    struct Datos *Data;

    switch(identificador){
        //da la struct sin modificaciones
        case 100:
            //rellenando la struct
            Data = (struct Datos *)BASE_DATOS(Ruta_db,Consulta,num_columnas);
            free(Ruta_db);
            free(Consulta);
            //mandando sus datos
            for(int i = 0; i<num_columnas;i++){
                switch(Data[i].tipo){
                    //int long long int
                    case 1:
                        //tipo
                        send(cliente_fd,&Data[i].tipo,sizeof(int),0);
                        //longitud array
                        send(cliente_fd,&Data[i].longitud,sizeof(size_t),0);
                        //array
                        send(cliente_fd,Data[i].Array[0],Data[i].longitud*sizeof(long long int),0);
                        free(Data[i].Array[0]);
                        free(Data[i].Array);
                        break;
                    case 2:
                        break;
                    case 3:
                        //tipo
                        send(cliente_fd,&Data[i].tipo,sizeof(int),0);
                        //longitud del array
                        send(cliente_fd,&Data[i].longitud,sizeof(size_t),0);
                        //array
                        for (size_t j = 0; j<Data[i].longitud; j++){
                            size_t len_string = strlen((char*)Data[i].Array[j]);
                            //len del strings
                            send(cliente_fd,&len_string,sizeof(size_t),0);
                            //se manda el string
                            send(cliente_fd,Data[i].Array[j],sizeof(char)*len_string,0);
                        }
                        break;
                }
            }
            break;
    }

    //terminacion del hilo
    free(Data);
    close(cliente_fd);
    return nullptr;
}


