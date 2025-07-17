#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include <string.h>
#include "datos.h"

void *BASE_DATOS(const char*Ruta_DB,const char*Consulta,int num_columnas){
    //conexion a la base de datos
    sqlite3 *DB;
    if(sqlite3_open(Ruta_DB,&DB) != SQLITE_OK){
        fprintf(stderr,"FALLO LA CONEXION A LA BASE DE DATOS\n");
        return NULL;
    }
    //preparando la consulta
    sqlite3_stmt *stmt;
    if(sqlite3_prepare_v2(DB,Consulta,-1,&stmt,NULL) != SQLITE_OK){
        fprintf(stderr,"FALLO LA PREPARACION DEL STMT\n");
        sqlite3_close(DB);
        return NULL;
    }

    //sacando los datos
    struct Datos *Data = (struct Datos*)malloc(num_columnas * sizeof(struct Datos));

    for(int i = 0; i<num_columnas;i++){
        //sacando el tipo de la columna
        if(sqlite3_step(stmt) != SQLITE_ROW){
            fprintf(stderr,"ERROR CON LAS COLUMNAS\n");
            sqlite3_finalize(stmt);
            sqlite3_close(DB);
            return NULL;
        }

        int tipo = sqlite3_column_type(stmt,i);
        Data[i].tipo = tipo;
        sqlite3_reset(stmt);

        //preparando el Array
        Data[i].Array = NULL;
        if(Data[i].tipo != 3){
            switch(Data[i].tipo){
                //int32/int64
                case 1:
                    Data[i].Array = (void**)malloc(sizeof(long long int*));
                    Data[i].Array[0] = NULL;
                    break;
                //float/double
                case 2:
                    Data[i].Array = (void**)malloc(sizeof(double*));
                    Data[i].Array[0] = NULL;
                    break;
            }
        }

        Data[i].longitud = 0;
        while(sqlite3_step(stmt) == SQLITE_ROW){
            void* temp = NULL;
            int bytes = 0;
            switch(Data[i].tipo){
                //numero int32/int64
                case 1:
                    Data[i].longitud++;
                    temp = (long long int*)realloc(temp,sizeof(long long int));
                    *(long long int*)temp = sqlite3_column_int64(stmt,i);

                    Data[i].Array[0] = (long long int*)realloc(Data[i].Array[0],Data[i].longitud*sizeof(long long int));
                    ((long long int*)Data[i].Array[0])[Data[i].longitud-1] = *(long long int*)temp;

                    free(temp);
                    temp = NULL;
                    break;
                //float/double
                case 2:
                    Data[i].longitud++;
                    temp = (double*)realloc(temp,sizeof(double));
                    *(double*)temp = sqlite3_column_double(stmt,i);

                    Data[i].Array[0] = (double*)realloc(Data[i].Array[0],Data[i].longitud*sizeof(double));
                    ((double*)Data[i].Array[0])[Data[i].longitud-1] = *(double*)temp;

                    free(temp);
                    temp = NULL;
                    break;
                //strings 
                case 3:
                    Data[i].longitud++;
                    bytes = sqlite3_column_bytes(stmt,i);

                    Data[i].Array = (void**)realloc(Data[i].Array,Data[i].longitud*sizeof(char*));
                    Data[i].Array[Data[i].longitud-1] = (char*)calloc(bytes+1,sizeof(char));

                    memcpy(Data[i].Array[Data[i].longitud-1],sqlite3_column_text(stmt,i),bytes);
                    ((char*)Data[i].Array[Data[i].longitud-1])[bytes] = '\0';
                    break;
            }
        }

    }

    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    //retorna el un puntero de la struct
    return Data;
}

