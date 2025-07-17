import socket
import struct

ip = "192.168.0.222"
puerto = 8080

cliente = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
cliente.connect((ip, puerto))


#se manda la ruta de la database
db_ruta = "../../data/DB/DB_Casas.db\0"  
db_ruta_bytes = db_ruta.encode()

db_len = len(db_ruta_bytes)
db_len_by = struct.pack("<Q", db_len)

db_ruta_by = struct.pack(f"<{db_len}s", db_ruta_bytes)


#se manda la consulta
consulta = "SELECT Floor FROM Houses\0"

consulta_bytes = consulta.encode()

consulta_len = len(consulta_bytes);

consulta_len_by = struct.pack("<Q",consulta_len)
consulta_by = struct.pack(f"<{consulta_len}s",consulta_bytes)

#numero de columnas
columnas_len = 1
columnas_len_by = struct.pack("<i",columnas_len)

#numero identificador
identificador = 100
identificador_by = struct.pack("<I",identificador);


#esperando la respuesta

payload = db_len_by + db_ruta_by + consulta_len_by + consulta_by + columnas_len_by + identificador_by

cliente.sendall(payload)

for _ in range(0,columnas_len):
    tipo_dato_by = cliente.recv(4)
    tipo_dato = struct.unpack("<i",tipo_dato_by)[0]
    print(f"tipo de dato es {tipo_dato}")

    longitud_by = cliente.recv(8)
    longitud = struct.unpack("<Q",longitud_by)[0]
    print(f"longitud del array {longitud}")

    match tipo_dato:
        case 1:
            data_array = cliente.recv(longitud*8)
            array = struct.unpack(f"<{longitud}Q",data_array)
            print(array)
        case 3:
            array = []

            for i in range(0,longitud):
                len_string_by = cliente.recv(8)
                len_string = struct.unpack("<Q",len_string_by)[0]

                palabra_by = cliente.recv(len_string)
                palabra = struct.unpack(f"<{len_string}s",palabra_by)
                palabra = palabra_by.decode('utf-8')
                array.append(palabra)
                
            print(array)



cliente.close()

