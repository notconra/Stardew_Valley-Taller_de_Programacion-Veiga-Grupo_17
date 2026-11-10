#include "personaje.h"

#include <map>

namespace {

constexpr int FILA_ABAJO = 0;
constexpr int FILA_LATERAL = 1;
constexpr int FILA_ARRIBA = 2;

const std::map<Direccion, int> FILA_DE_DIRECCION{
    {Direccion::ABAJO, FILA_ABAJO},
    {Direccion::DERECHA, FILA_LATERAL},
    {Direccion::IZQUIERDA, FILA_LATERAL},
    {Direccion::ARRIBA, FILA_ARRIBA},
};

constexpr int FILA_ACCION_POR_DIRECCION[3] = {11, 8, 6};
constexpr int UMBRALES_DE_CUADRO_ACCION[4] = {150, 190, 230, 400};
constexpr Uint32 DURACION_ACCION_MS = 475;

constexpr int FILA_PELO_POR_DIRECCION[3] = {0, 32, 64};
constexpr int FILA_REMERA_POR_DIRECCION[3] = {0, 8, 24};
constexpr int FILA_SOMBRERO_POR_DIRECCION[3] = {0, 40, 60};

constexpr int ANCHO_CELDA_PELO = 16;
constexpr int ANCHO_CELDA_SOMBRERO = 20;
constexpr int ANCHO_CELDA_REMERA = 8;
constexpr int ALTO_BLOQUE_PELO = 32 * 3;
constexpr int ALTO_BLOQUE_REMERA = 8 * 3;
constexpr int ALTO_BLOQUE_SOMBRERO = 80;
constexpr int DESVIO_ACCION_ABAJO[4] = {1, 0, -1, -2};
constexpr int DESVIO_ACCION_LATERAL[4] = {0, -1, -2, -3};
constexpr int DESVIO_ACCION_ARRIBA[4] = {1, 1, 0, 0};
constexpr int FILA_BRAZOS_EN_ALTO = 9;
constexpr int COLUMNA_BRAZOS_EN_ALTO_FRENTE = 8;
constexpr int COLUMNA_BRAZOS_EN_ALTO_LATERAL = 9;
constexpr int TAMANO_ITEM_EN_ALTO = 16;
constexpr int Y_ITEM_EN_ALTO = -6;
constexpr int CORRIMIENTO_ITEM_LATERAL = 1;
constexpr int TAMANO_HERRAMIENTA = 16;

struct PoseDeMano {
    int x;
    int y;
    double angulo;
};
constexpr PoseDeMano MANO_ACCION[3][4] = {
    {{2, 13, -45}, {5, 16, -15}, {6, 20, 25}, {6, 22, 50}},
    {{5, 15, -60}, {9, 14, -20}, {15, 16, 40}, {14, 19, 70}},
    {{9, 16, -45}, {9, 18, -15}, {9, 20, 25}, {9, 22, 50}},
};

bool es_herramienta_de_mano(TipoItem tipo) {
    return tipo == TipoItem::PICO || tipo == TipoItem::PALA || tipo == TipoItem::ESPADA;
}

int cuadro_de_accion(Uint32 ms_de_accion) {
    int cuadro = 0;
    while (cuadro < 4 && ms_de_accion >= static_cast<Uint32>(UMBRALES_DE_CUADRO_ACCION[cuadro])) {
        ++cuadro;
    }
    return cuadro;
}

int desvio_de_accion(int fila_direccion, int cuadro) {
    const int indice = cuadro % 4;
    switch (fila_direccion) {
        case FILA_ABAJO:
            return DESVIO_ACCION_ABAJO[indice];
        case FILA_LATERAL:
            return DESVIO_ACCION_LATERAL[indice];
        default:
            return DESVIO_ACCION_ARRIBA[indice];
    }
}

constexpr const char* RUTA_CUERPO_MASCULINO = "Assets/Characters/Farmer/farmer_base.png";
constexpr const char* RUTA_CUERPO_FEMENINO = "Assets/Characters/Farmer/farmer_girl_base.png";

}  // namespace

Personaje::Personaje(
    const ConfiguracionJugador& configuracion_jugador,
    const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer)
    : cuerpo_masculino(RUTA_CUERPO_MASCULINO, renderer),
      cuerpo_femenino(RUTA_CUERPO_FEMENINO, renderer),
      pantalon(configuracion_jugador.ruta_pantalon, renderer),
      pelo(configuracion_jugador.ruta_pelo, renderer),
      sombrero(configuracion_jugador.ruta_sombrero, renderer),
      remera(configuracion_jugador.ruta_remera, renderer),
      ancho(16),
      alto(32),
      escala_de_tiles(configuracion_graficos.escala_de_tiles),
      ms_por_cuadro_caminando(configuracion_jugador.ms_por_cuadro_caminando) {
    ancho_hoja_cuerpo = 0;
    SDL_QueryTexture(cuerpo_masculino.ptr(), nullptr, nullptr, &ancho_hoja_cuerpo, nullptr);
    if (ancho_hoja_cuerpo <= 0) {
        SDL_QueryTexture(cuerpo_femenino.ptr(), nullptr, nullptr, &ancho_hoja_cuerpo, nullptr);
    }

    int ancho_hoja_pantalon = 0;
    SDL_QueryTexture(pantalon.ptr(), nullptr, nullptr, &ancho_hoja_pantalon, nullptr);
    columnas_pantalon = (ancho_hoja_cuerpo > 0) ? (ancho_hoja_pantalon / ancho_hoja_cuerpo) : 1;
    if (columnas_pantalon <= 0) {
        columnas_pantalon = 1;
    }

    int ancho_hoja_pelo = 0;
    SDL_QueryTexture(pelo.ptr(), nullptr, nullptr, &ancho_hoja_pelo, nullptr);
    columnas_pelo = ancho_hoja_pelo / ANCHO_CELDA_PELO;
    if (columnas_pelo <= 0) {
        columnas_pelo = 1;
    }

    int ancho_hoja_sombrero = 0;
    SDL_QueryTexture(sombrero.ptr(), nullptr, nullptr, &ancho_hoja_sombrero, nullptr);
    columnas_sombrero = ancho_hoja_sombrero / ANCHO_CELDA_SOMBRERO;
    if (columnas_sombrero <= 0) {
        columnas_sombrero = 1;
    }

    int ancho_hoja_remera = 0;
    SDL_QueryTexture(remera.ptr(), nullptr, nullptr, &ancho_hoja_remera, nullptr);
    columnas_remera = ancho_hoja_remera / ANCHO_CELDA_REMERA;
    if (columnas_remera <= 0) {
        columnas_remera = 1;
    }
}

int Personaje::ancho_del_sprite() const {
    return ancho;
}

int Personaje::alto_del_sprite() const {
    return alto;
}

void Personaje::dibujar(
    SDL_Renderer* renderer, const EstadoJugador& jugador, bool haciendo_accion, Uint32 ms_de_accion,
    int x_en_mapa, int y_en_mapa, int cam_x, int cam_y, const RanuraInventario& en_mano,
    const IconosDeItems& iconos) const {
    const auto encontrada = FILA_DE_DIRECCION.find(jugador.mirando);
    const int fila_direccion =
        encontrada != FILA_DE_DIRECCION.end() ? encontrada->second : FILA_ABAJO;
    const SDL_RendererFlip flip =
        jugador.mirando == Direccion::IZQUIERDA ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    int columna_cuerpo = 0;
    int columna_brazos = 6;
    int fila_cuerpo = fila_direccion;
    int fila_brazos = fila_direccion;
    int desvio_y_accesorios = 0;
    const int cuadro_accion = cuadro_de_accion(ms_de_accion) % 4;

    const bool tiene_item = en_mano.tipo != TipoItem::NINGUNO;
    const bool item_en_la_mano = tiene_item && haciendo_accion;
    const bool item_en_alto =
        tiene_item && !haciendo_accion && !es_herramienta_de_mano(en_mano.tipo);

    if (haciendo_accion) {
        fila_cuerpo = FILA_ACCION_POR_DIRECCION[fila_direccion];
        fila_brazos = fila_cuerpo;
        columna_cuerpo = cuadro_accion;
        columna_brazos = 6 + cuadro_accion;
        desvio_y_accesorios = desvio_de_accion(fila_direccion, cuadro_accion);
    } else if (jugador.camina) {
        const Uint32 ahora = SDL_GetTicks();
        const int cuadro = static_cast<int>((ahora / ms_por_cuadro_caminando) % 3);
        columna_cuerpo = cuadro;
        columna_brazos = 6 + cuadro;
        if (cuadro != 0) {
            desvio_y_accesorios = 1;  // +4 píxeles de pantalla a escala x4 original = 1 de mundo
        }
    }

    const int x_pantalla = x_en_mapa * escala_de_tiles - cam_x;
    const int y_pantalla = y_en_mapa * escala_de_tiles - cam_y;
    const SDL_Rect destino_cuerpo{
        x_pantalla,
        y_pantalla,
        ancho * escala_de_tiles,
        alto * escala_de_tiles};
    if (item_en_alto) {
        fila_brazos = FILA_BRAZOS_EN_ALTO;
        columna_brazos = fila_direccion == FILA_LATERAL ? COLUMNA_BRAZOS_EN_ALTO_LATERAL
                                                        : COLUMNA_BRAZOS_EN_ALTO_FRENTE;
    }
    const auto dibujar_item_en_la_mano = [&]() {
        const PoseDeMano& pose = MANO_ACCION[fila_direccion][cuadro_accion];
        const bool espejado = flip == SDL_FLIP_HORIZONTAL;
        const int x_mano = espejado ? ancho - pose.x : pose.x;
        const SDL_Rect destino{
            x_pantalla + (x_mano - TAMANO_HERRAMIENTA / 2) * escala_de_tiles,
            y_pantalla + (pose.y - TAMANO_HERRAMIENTA / 2) * escala_de_tiles,
            TAMANO_HERRAMIENTA * escala_de_tiles,
            TAMANO_HERRAMIENTA * escala_de_tiles};
        iconos.dibujar(
            renderer,
            en_mano.tipo,
            en_mano.calidad,
            destino,
            flip,
            espejado ? -pose.angulo : pose.angulo);
    };
    if (item_en_la_mano && fila_direccion == FILA_ARRIBA) {
        dibujar_item_en_la_mano();
    }

    const bool es_femenino = jugador.apariencia.es_femenino;
    const Textura& cuerpo = es_femenino ? cuerpo_femenino : cuerpo_masculino;
    const int offset_y_remera = es_femenino ? OFFSET_Y_REMERA_FEMENINO : OFFSET_Y_REMERA_MASCULINO;
    const int offset_y_pelo = es_femenino ? OFFSET_Y_PELO_FEMENINO : OFFSET_Y_PELO_MASCULINO;

    const SDL_Rect origen_cuerpo{columna_cuerpo * ancho, fila_cuerpo * alto, ancho, alto};
    SDL_RenderCopyEx(renderer, cuerpo.ptr(), &origen_cuerpo, &destino_cuerpo, 0.0, nullptr, flip);

    SDL_SetTextureColorMod(
        pantalon.ptr(),
        jugador.apariencia.color_pantalon.rojo,
        jugador.apariencia.color_pantalon.verde,
        jugador.apariencia.color_pantalon.azul);
    const int columna_pantalon =
        columnas_pantalon > 0 ? (jugador.apariencia.indice_pantalon % columnas_pantalon) : 0;
    SDL_Rect origen_pantalon = origen_cuerpo;
    origen_pantalon.x += columna_pantalon * ancho_hoja_cuerpo;
    SDL_RenderCopyEx(
        renderer,
        pantalon.ptr(),
        &origen_pantalon,
        &destino_cuerpo,
        0.0,
        nullptr,
        flip);

    const SDL_Rect origen_brazos{columna_brazos * ancho, fila_brazos * alto, ancho, alto};
    SDL_RenderCopyEx(renderer, cuerpo.ptr(), &origen_brazos, &destino_cuerpo, 0.0, nullptr, flip);

    SDL_SetTextureColorMod(
        pelo.ptr(),
        jugador.apariencia.color_pelo.rojo,
        jugador.apariencia.color_pelo.verde,
        jugador.apariencia.color_pelo.azul);
    const int columna_pelo =
        columnas_pelo > 0 ? (jugador.apariencia.indice_pelo % columnas_pelo) : 0;
    const int pagina_pelo =
        columnas_pelo > 0 ? (jugador.apariencia.indice_pelo / columnas_pelo) : 0;
    const SDL_Rect origen_pelo{
        columna_pelo * ANCHO_CELDA_PELO,
        FILA_PELO_POR_DIRECCION[fila_direccion] + pagina_pelo * ALTO_BLOQUE_PELO,
        ANCHO_CELDA_PELO,
        alto};
    const SDL_Rect destino_pelo{
        x_pantalla,
        y_pantalla + (offset_y_pelo + desvio_y_accesorios) * escala_de_tiles,
        ancho * escala_de_tiles,
        alto * escala_de_tiles};
    SDL_RenderCopyEx(renderer, pelo.ptr(), &origen_pelo, &destino_pelo, 0.0, nullptr, flip);

    if (jugador.apariencia.indice_sombrero >= 0 && columnas_sombrero > 0) {
        const int columna_sombrero = jugador.apariencia.indice_sombrero % columnas_sombrero;
        const int pagina_sombrero = jugador.apariencia.indice_sombrero / columnas_sombrero;
        const SDL_Rect origen_sombrero{
            columna_sombrero * ANCHO_CELDA_SOMBRERO,
            FILA_SOMBRERO_POR_DIRECCION[fila_direccion] + pagina_sombrero * ALTO_BLOQUE_SOMBRERO,
            ANCHO_CELDA_SOMBRERO,
            ANCHO_CELDA_SOMBRERO};
        const int margen_sombrero = (ANCHO_CELDA_SOMBRERO - ancho) / 2;
        const SDL_Rect destino_sombrero{
            x_pantalla - margen_sombrero * escala_de_tiles,
            y_pantalla + (desvio_y_accesorios - margen_sombrero) * escala_de_tiles,
            ANCHO_CELDA_SOMBRERO * escala_de_tiles,
            ANCHO_CELDA_SOMBRERO * escala_de_tiles};
        const SDL_RendererFlip flip_sombrero =
            flip == SDL_FLIP_NONE ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(
            renderer,
            sombrero.ptr(),
            &origen_sombrero,
            &destino_sombrero,
            0.0,
            nullptr,
            flip_sombrero);
    }

    const int columna_remera =
        columnas_remera > 0 ? (jugador.apariencia.indice_remera % columnas_remera) : 0;
    const int pagina_remera =
        columnas_remera > 0 ? (jugador.apariencia.indice_remera / columnas_remera) : 0;
    const SDL_Rect origen_remera{
        columna_remera * ANCHO_CELDA_REMERA,
        FILA_REMERA_POR_DIRECCION[fila_direccion] + pagina_remera * ALTO_BLOQUE_REMERA,
        ANCHO_CELDA_REMERA,
        ANCHO_CELDA_REMERA};
    const int margen_remera = (ancho - ANCHO_CELDA_REMERA) / 2;
    const SDL_Rect destino_remera{
        x_pantalla + margen_remera * escala_de_tiles,
        y_pantalla + (offset_y_remera + desvio_y_accesorios) * escala_de_tiles,
        ANCHO_CELDA_REMERA * escala_de_tiles,
        ANCHO_CELDA_REMERA * escala_de_tiles};
    SDL_RenderCopyEx(renderer, remera.ptr(), &origen_remera, &destino_remera, 0.0, nullptr, flip);

    if (haciendo_accion || item_en_alto) {
        SDL_RenderCopyEx(
            renderer,
            cuerpo.ptr(),
            &origen_brazos,
            &destino_cuerpo,
            0.0,
            nullptr,
            flip);
    }

    if (item_en_alto) {
        int corrimiento = 0;
        if (fila_direccion == FILA_LATERAL) {
            corrimiento = jugador.mirando == Direccion::IZQUIERDA ? -CORRIMIENTO_ITEM_LATERAL
                                                                  : CORRIMIENTO_ITEM_LATERAL;
        }
        const SDL_Rect destino_item{
            x_pantalla + ((ancho - TAMANO_ITEM_EN_ALTO) / 2 + corrimiento) * escala_de_tiles,
            y_pantalla + (Y_ITEM_EN_ALTO + desvio_y_accesorios) * escala_de_tiles,
            TAMANO_ITEM_EN_ALTO * escala_de_tiles,
            TAMANO_ITEM_EN_ALTO * escala_de_tiles};
        iconos.dibujar(renderer, en_mano.tipo, en_mano.calidad, destino_item, flip);
    } else if (item_en_la_mano && fila_direccion != FILA_ARRIBA) {
        dibujar_item_en_la_mano();
    }
}