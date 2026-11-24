#ifndef LLEGADAS_H
#define LLEGADAS_H

// Nombres de los lugares con nombre de los mapas (objetos con tipo = "llegada" en la capa de
// transiciones). Una transición que nombra una llegada lleva a ese lugar del mapa destino, así los
// interiores (la casa, el mercado, el centro comunitario) y la mazmorra sirven para cualquier
// granja o pueblo, sea el nuestro o uno armado con el editor.

// En la granja: delante de la puerta de la casa y delante de la cueva.
#define LLEGADA_PUERTA_DE_LA_CASA "puerta_casa"
#define LLEGADA_CUEVA "cueva"

// En el pueblo: delante de la puerta del mercado y de la del centro comunitario.
#define LLEGADA_PUERTA_DEL_MERCADO "puerta_mercado"
#define LLEGADA_PUERTA_DEL_CENTRO "puerta_centro"

// En un interior: donde aparece el que entra.
#define LLEGADA_ENTRADA "entrada"

#endif
