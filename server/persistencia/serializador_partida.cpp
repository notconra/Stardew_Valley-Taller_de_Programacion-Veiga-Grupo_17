#include "serializador_partida.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "escritor_binario.h"
#include "lector_binario.h"

constexpr std::array<uint8_t, 4> FIRMA_ARCHIVO = {'S', 'V', 'P', 'T'};
constexpr uint16_t VERSION_ACTUAL = 6;

void SerializadorPartida::serializar(std::ostream& stream, const DatosPartidaGuardada& datos) {
    EscritorBinario escritor(stream);

    escritor.escribir_bytes(FIRMA_ARCHIVO.data(), FIRMA_ARCHIVO.size());
    escritor.escribir_u16(VERSION_ACTUAL);

    escritor.escribir_texto(datos.codigo);
    escritor.escribir_texto(datos.nombre);
    escritor.escribir_u8(datos.jugadores_maximos);
    escritor.escribir_bool(datos.multijugador);
    escritor.escribir_u32(datos.mundo.dia);

    escritor.escribir_u8(static_cast<uint8_t>(datos.mundo.jugadores.size()));
    for (const auto& jugador : datos.mundo.jugadores) {
        escritor.escribir_u32(jugador.id_cliente);
        escritor.escribir_u32(jugador.id_jugador);
        escritor.escribir_texto(jugador.nombre_jugador);
        escritor.escribir_texto(jugador.mapa_actual);
        escritor.escribir_double(jugador.x);
        escritor.escribir_double(jugador.y);
        escritor.escribir_u8(static_cast<uint8_t>(jugador.mirando));
        escritor.escribir_u8(jugador.ranura_activa);

        escritor.escribir_u8(static_cast<uint8_t>(jugador.inventario.size()));
        for (const auto& ranura : jugador.inventario) {
            escritor.escribir_u8(static_cast<uint8_t>(ranura.tipo));
            escritor.escribir_u16(ranura.cantidad);
            escritor.escribir_u8(static_cast<uint8_t>(ranura.calidad));
        }

        escritor.escribir_bool(jugador.apariencia.es_femenino);
        escritor.escribir_u8(jugador.apariencia.indice_pelo);
        escritor.escribir_u8(jugador.apariencia.color_pelo.rojo);
        escritor.escribir_u8(jugador.apariencia.color_pelo.verde);
        escritor.escribir_u8(jugador.apariencia.color_pelo.azul);
        escritor.escribir_u8(jugador.apariencia.indice_remera);
        escritor.escribir_u8(jugador.apariencia.indice_pantalon);
        escritor.escribir_u8(jugador.apariencia.color_pantalon.rojo);
        escritor.escribir_u8(jugador.apariencia.color_pantalon.verde);
        escritor.escribir_u8(jugador.apariencia.color_pantalon.azul);
        escritor.escribir_u16(static_cast<uint16_t>(jugador.apariencia.indice_sombrero));

        escritor.escribir_u16(jugador.vida);
        escritor.escribir_u16(jugador.energia);
        escritor.escribir_bool(jugador.tiene_cama);
        escritor.escribir_texto(jugador.mapa_cama);
        escritor.escribir_u32(jugador.columna_cama);
        escritor.escribir_u32(jugador.fila_cama);
    }

    escritor.escribir_u16(static_cast<uint16_t>(datos.mundo.mapas.size()));
    for (const auto& mapa : datos.mundo.mapas) {
        escritor.escribir_texto(mapa.id_mapa);
        escritor.escribir_u32(static_cast<uint32_t>(mapa.casillas_modificadas.size()));
        for (const auto& casilla : mapa.casillas_modificadas) {
            escritor.escribir_u32(casilla.columna);
            escritor.escribir_u32(casilla.fila);
            escritor.escribir_u8(casilla.propiedad);
        }
    }

    escritor.escribir_u8(static_cast<uint8_t>(datos.mundo.niveles_mazmorra.size()));
    for (const auto& nivel : datos.mundo.niveles_mazmorra) {
        escritor.escribir_texto(nivel);
    }

    escritor.escribir_u16(static_cast<uint16_t>(datos.mundo.cofres.size()));
    for (const auto& cofre : datos.mundo.cofres) {
        escritor.escribir_texto(cofre.id_mapa);
        escritor.escribir_u32(cofre.columna);
        escritor.escribir_u32(cofre.fila);
        escritor.escribir_u8(static_cast<uint8_t>(cofre.inventario.size()));
        for (const auto& ranura : cofre.inventario) {
            escritor.escribir_u8(static_cast<uint8_t>(ranura.tipo));
            escritor.escribir_u16(ranura.cantidad);
            escritor.escribir_u8(static_cast<uint8_t>(ranura.calidad));
        }
    }

    escritor.escribir_u16(static_cast<uint16_t>(datos.mundo.muebles.size()));
    for (const auto& mueble : datos.mundo.muebles) {
        escritor.escribir_texto(mueble.id_mapa);
        escritor.escribir_u8(static_cast<uint8_t>(mueble.tipo));
        escritor.escribir_u32(mueble.columna);
        escritor.escribir_u32(mueble.fila);
    }

    escritor.escribir_u16(static_cast<uint16_t>(datos.mundo.cultivos.size()));
    for (const auto& cultivo : datos.mundo.cultivos) {
        escritor.escribir_texto(cultivo.id_mapa);
        escritor.escribir_u32(cultivo.columna);
        escritor.escribir_u32(cultivo.fila);
        escritor.escribir_u8(static_cast<uint8_t>(cultivo.tipo));
        escritor.escribir_u16(cultivo.dias_de_crecimiento);
        escritor.escribir_bool(cultivo.madura);
        escritor.escribir_u16(cultivo.faltas);
    }
}

DatosPartidaGuardada SerializadorPartida::deserializar(std::istream& stream) {
    LectorBinario lector(stream);

    std::array<uint8_t, 4> firma{};
    lector.leer_bytes(firma.data(), firma.size());
    if (firma != FIRMA_ARCHIVO) {
        throw std::runtime_error("Firma de archivo inválida: no es un archivo de partida válido");
    }

    uint16_t version = lector.leer_u16();
    if (version != VERSION_ACTUAL) {
        throw std::runtime_error("Versión de partida no soportada: " + std::to_string(version));
    }

    DatosPartidaGuardada datos;
    datos.codigo = lector.leer_texto();
    datos.nombre = lector.leer_texto();
    datos.jugadores_maximos = lector.leer_u8();
    datos.multijugador = lector.leer_bool();
    datos.mundo.dia = lector.leer_u32();

    uint8_t cant_jugadores = lector.leer_u8();
    for (uint8_t indice_jugador = 0; indice_jugador < cant_jugadores; ++indice_jugador) {
        DatosJugadorGuardado jugador;
        jugador.id_cliente = lector.leer_u32();
        jugador.id_jugador = lector.leer_u32();
        jugador.nombre_jugador = lector.leer_texto();
        jugador.mapa_actual = lector.leer_texto();
        jugador.x = lector.leer_double();
        jugador.y = lector.leer_double();
        jugador.mirando = static_cast<Direccion>(lector.leer_u8());
        jugador.ranura_activa = lector.leer_u8();

        uint8_t cant_ranuras = lector.leer_u8();
        for (uint8_t indice_ranura = 0; indice_ranura < cant_ranuras; ++indice_ranura) {
            RanuraInventario ranura;
            ranura.tipo = static_cast<TipoItem>(lector.leer_u8());
            ranura.cantidad = lector.leer_u16();
            ranura.calidad = static_cast<Calidad>(lector.leer_u8());
            jugador.inventario.push_back(ranura);
        }

        jugador.apariencia.es_femenino = lector.leer_bool();
        jugador.apariencia.indice_pelo = lector.leer_u8();
        jugador.apariencia.color_pelo.rojo = lector.leer_u8();
        jugador.apariencia.color_pelo.verde = lector.leer_u8();
        jugador.apariencia.color_pelo.azul = lector.leer_u8();
        jugador.apariencia.indice_remera = lector.leer_u8();
        jugador.apariencia.indice_pantalon = lector.leer_u8();
        jugador.apariencia.color_pantalon.rojo = lector.leer_u8();
        jugador.apariencia.color_pantalon.verde = lector.leer_u8();
        jugador.apariencia.color_pantalon.azul = lector.leer_u8();
        jugador.apariencia.indice_sombrero = static_cast<int16_t>(lector.leer_u16());

        jugador.vida = lector.leer_u16();
        jugador.energia = lector.leer_u16();
        jugador.tiene_cama = lector.leer_bool();
        jugador.mapa_cama = lector.leer_texto();
        jugador.columna_cama = lector.leer_u32();
        jugador.fila_cama = lector.leer_u32();

        datos.mundo.jugadores.push_back(jugador);
    }

    uint16_t cant_mapas = lector.leer_u16();
    for (uint16_t indice_mapa = 0; indice_mapa < cant_mapas; ++indice_mapa) {
        DatosMapaGuardado mapa;
        mapa.id_mapa = lector.leer_texto();

        uint32_t cant_casillas = lector.leer_u32();
        for (uint32_t indice_casilla = 0; indice_casilla < cant_casillas; ++indice_casilla) {
            ModificacionCasilla casilla;
            casilla.columna = lector.leer_u32();
            casilla.fila = lector.leer_u32();
            casilla.propiedad = lector.leer_u8();
            mapa.casillas_modificadas.push_back(casilla);
        }

        datos.mundo.mapas.push_back(mapa);
    }

    uint8_t cant_niveles_mazmorra = lector.leer_u8();
    for (uint8_t i = 0; i < cant_niveles_mazmorra; ++i) {
        datos.mundo.niveles_mazmorra.push_back(lector.leer_texto());
    }

    uint16_t cant_cofres = lector.leer_u16();
    for (uint16_t i = 0; i < cant_cofres; ++i) {
        DatosCofreGuardado cofre;
        cofre.id_mapa = lector.leer_texto();
        cofre.columna = lector.leer_u32();
        cofre.fila = lector.leer_u32();
        uint8_t cant_ranuras = lector.leer_u8();
        for (uint8_t j = 0; j < cant_ranuras; ++j) {
            RanuraInventario ranura;
            ranura.tipo = static_cast<TipoItem>(lector.leer_u8());
            ranura.cantidad = lector.leer_u16();
            ranura.calidad = static_cast<Calidad>(lector.leer_u8());
            cofre.inventario.push_back(ranura);
        }
        datos.mundo.cofres.push_back(cofre);
    }

    uint16_t cant_muebles = lector.leer_u16();
    for (uint16_t i = 0; i < cant_muebles; ++i) {
        DatosMuebleGuardado mueble;
        mueble.id_mapa = lector.leer_texto();
        mueble.tipo = static_cast<TipoObjeto>(lector.leer_u8());
        mueble.columna = lector.leer_u32();
        mueble.fila = lector.leer_u32();
        datos.mundo.muebles.push_back(mueble);
    }

    uint16_t cant_cultivos = lector.leer_u16();
    for (uint16_t i = 0; i < cant_cultivos; ++i) {
        DatosCultivoGuardado cultivo;
        cultivo.id_mapa = lector.leer_texto();
        cultivo.columna = lector.leer_u32();
        cultivo.fila = lector.leer_u32();
        cultivo.tipo = static_cast<TipoCultivo>(lector.leer_u8());
        cultivo.dias_de_crecimiento = lector.leer_u16();
        cultivo.madura = lector.leer_bool();
        cultivo.faltas = lector.leer_u16();
        datos.mundo.cultivos.push_back(cultivo);
    }

    return datos;
}

void SerializadorPartida::guardar_en_archivo(
    const std::string& ruta_archivo, const DatosPartidaGuardada& datos) {
    std::filesystem::path ruta(ruta_archivo);
    if (ruta.has_parent_path()) {
        std::filesystem::create_directories(ruta.parent_path());
    }

    std::ofstream archivo(ruta_archivo, std::ios::binary | std::ios::trunc);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo para escritura: " + ruta_archivo);
    }

    serializar(archivo, datos);
}

DatosPartidaGuardada SerializadorPartida::cargar_desde_archivo(const std::string& ruta_archivo) {
    std::ifstream archivo(ruta_archivo, std::ios::binary);
    if (!archivo.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo para lectura: " + ruta_archivo);
    }

    return deserializar(archivo);
}
