#ifndef DATOS_H
#define DATOS_H

#include <stddef.h>

struct Datos{
    int tipo;
    void **Array;
    size_t longitud;
};

void *BASE_DATOS(const char*Ruta_DB,const char*Consulta,int num_columnas);

#endif
