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
consulta = "SELECT Rent,BHK FROM Houses\0"

consulta_bytes = consulta.encode()

consulta_len = len(consulta_bytes);

consulta_len_by = struct.pack("<Q",consulta_len)
consulta_by = struct.pack(f"<{consulta_len}s",consulta_bytes)


#numero de columnas
columnas_len = 2
columnas_len_by = struct.pack("<Q",columnas_len)

#columnas
columnas_array = [0,1]
columnas_array_by = struct.pack(f"<{columnas_len}i",*columnas_array)

#numero identificador
identificador = 100
identificador_by = struct.pack("<I",identificador);


#esperando la respuesta

payload = db_len_by + db_ruta_by + consulta_len_by + consulta_by + columnas_len_by + columnas_array_by + identificador_by

cliente.sendall(payload)

data_long = cliente.recv(8)
longitud = struct.unpack("<Q",data_long)[0]
print(f"Longitud = {longitud}")

data_array = cliente.recv(longitud*8)
array = struct.unpack(f"<{longitud}Q",data_array)
print(array)

cliente.close()

