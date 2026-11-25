#ifndef IDENTIDAD_CLIENTE_H
#define IDENTIDAD_CLIENTE_H

#include <cstdint>

// Administra la identidad local del cliente mediante un ID persistido en disco.
// Si el archivo no existe, no es válido o contiene un ID nulo (0), se genera
// un nuevo ID aleatorio mayor a 0 y se almacena en el archivo.
class IdentidadCliente {
public:
    // Obtiene el ID persistido en disco o genera y guarda uno nuevo si no existe.
    // Garantiza que el ID devuelto sea estrictamente mayor a 0.
    static uint32_t obtener_o_crear();

private:
    static uint32_t leer_id();
    static void guardar_id(uint32_t id);
    static uint32_t generar_id_aleatorio();
};

#endif  // IDENTIDAD_CLIENTE_H
