#include "personaje.h"

#include <SDL2/SDL_image.h>

#include <map>
#include <stdexcept>
#include <string>

namespace {

constexpr const char* RUTA_HOJA_PIELES = "Assets/Characters/Farmer/skinColors.png";

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
constexpr int COLUMNAS_REMERA = 16;
constexpr int ALTO_BLOQUE_PELO = 32 * 3;
constexpr int ALTO_BLOQUE_REMERA = 32;
constexpr int ALTO_BLOQUE_SOMBRERO = 80;

constexpr int COLUMNAS_HOJA_PANTALON = 10;
constexpr int ANCHO_ESTILO_PANTALON = 192;
constexpr int ALTO_ESTILO_PANTALON = 688;

constexpr int DESVIO_ACCION_ABAJO[4] = {1, 0, -1, -2};
constexpr int DESVIO_ACCION_LATERAL[4] = {0, -1, -2, -3};
constexpr int DESVIO_ACCION_ARRIBA[4] = {1, 1, 0, 0};

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

SDL_Texture* cargar_textura_cuerpo(
    const std::string& ruta, int indice_piel, const ColorRGB& color_ojos, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(ruta.c_str());
    if (!surface) {
        throw std::runtime_error("no se pudo cargar la imagen " + ruta + ": " + IMG_GetError());
    }
    SDL_Surface* surface_rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surface);
    surface = surface_rgba;

    if (indice_piel > 0) {
        SDL_Surface* pieles = IMG_Load(RUTA_HOJA_PIELES);
        if (pieles) {
            SDL_Surface* pieles_rgba = SDL_ConvertSurfaceFormat(pieles, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(pieles);

            if (indice_piel < pieles_rgba->h) {
                Uint32* pixeles_piel = static_cast<Uint32*>(pieles_rgba->pixels);
                int pitch_piel = pieles_rgba->pitch / sizeof(Uint32);
                Uint32 color_base_sombra = pixeles_piel[0];
                Uint32 color_base_medio = pixeles_piel[1];
                Uint32 color_base_brillo = pixeles_piel[2];

                Uint32 color_destino_sombra = pixeles_piel[indice_piel * pitch_piel + 0];
                Uint32 color_destino_medio = pixeles_piel[indice_piel * pitch_piel + 1];
                Uint32 color_destino_brillo = pixeles_piel[indice_piel * pitch_piel + 2];

                Uint32* pixeles = static_cast<Uint32*>(surface->pixels);
                int total_pixeles = (surface->pitch / sizeof(Uint32)) * surface->h;
                for (int i = 0; i < total_pixeles; ++i) {
                    Uint32 pixel_actual = pixeles[i];
                    Uint8 canal_alfa = (pixel_actual >> 24) & 0xFF;
                    if (canal_alfa == 0) {
                        continue;
                    }
                    Uint32 color_rgb = pixel_actual & 0x00FFFFFF;
                    if (color_rgb == (color_base_sombra & 0x00FFFFFF)) {
                        pixeles[i] =
                            (color_destino_sombra & 0x00FFFFFF) | (pixel_actual & 0xFF000000);
                    } else if (color_rgb == (color_base_medio & 0x00FFFFFF)) {
                        pixeles[i] =
                            (color_destino_medio & 0x00FFFFFF) | (pixel_actual & 0xFF000000);
                    } else if (color_rgb == (color_base_brillo & 0x00FFFFFF)) {
                        pixeles[i] =
                            (color_destino_brillo & 0x00FFFFFF) | (pixel_actual & 0xFF000000);
                    }
                }
            }
            SDL_FreeSurface(pieles_rgba);
        }
    }

    // Tintado de ojos
    Uint32 color_base_ojos = SDL_MapRGBA(surface->format, 104, 43, 15, 255) & 0x00FFFFFF;
    Uint32 color_dest_ojos =
        SDL_MapRGBA(surface->format, color_ojos.rojo, color_ojos.verde, color_ojos.azul, 255) &
        0x00FFFFFF;

    Uint32* pixeles = static_cast<Uint32*>(surface->pixels);
    int total_pixeles = (surface->pitch / sizeof(Uint32)) * surface->h;
    for (int i = 0; i < total_pixeles; ++i) {
        Uint32 pixel_actual = pixeles[i];
        if (((pixel_actual >> 24) & 0xFF) == 0) {
            continue;
        }
        if ((pixel_actual & 0x00FFFFFF) == color_base_ojos) {
            pixeles[i] = color_dest_ojos | (pixel_actual & 0xFF000000);
        }
    }

    SDL_Texture* textura_cuerpo = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!textura_cuerpo) {
        throw std::runtime_error("no se pudo crear textura cuerpo: " + std::string(SDL_GetError()));
    }
    return textura_cuerpo;
}

}  // namespace

Personaje::Personaje(
    const ConfiguracionJugador& configuracion_jugador,
    const ConfiguracionGraficos& configuracion_graficos, SDL_Renderer* renderer)
    : cuerpo(cargar_textura_cuerpo(
          configuracion_jugador.ruta_cuerpo, configuracion_jugador.indice_piel,
          configuracion_jugador.color_ojos, renderer)),
      pantalon(configuracion_jugador.ruta_pantalon, renderer),
      pelo(configuracion_jugador.ruta_pelo, renderer),
      sombrero(configuracion_jugador.ruta_sombrero, renderer),
      remera(configuracion_jugador.ruta_remera, renderer),
      ancho(16),
      alto(32),
      escala_de_tiles(configuracion_graficos.escala_de_tiles),
      ms_por_cuadro_caminando(configuracion_jugador.ms_por_cuadro_caminando),
      indice_pelo(configuracion_jugador.indice_pelo),
      indice_sombrero(configuracion_jugador.indice_sombrero),
      indice_remera(configuracion_jugador.indice_remera),
      indice_pantalon(configuracion_jugador.indice_pantalon),
      offset_y_remera(configuracion_jugador.offset_y_remera),
      offset_y_pelo(configuracion_jugador.offset_y_pelo),
      offset_y_sombrero(configuracion_jugador.offset_y_sombrero) {
    SDL_SetTextureColorMod(
        pelo.ptr(),
        configuracion_jugador.color_pelo.rojo,
        configuracion_jugador.color_pelo.verde,
        configuracion_jugador.color_pelo.azul);
    SDL_SetTextureColorMod(
        pantalon.ptr(),
        configuracion_jugador.color_pantalon.rojo,
        configuracion_jugador.color_pantalon.verde,
        configuracion_jugador.color_pantalon.azul);

    int ancho_hoja_pelo = 0;
    SDL_QueryTexture(pelo.ptr(), nullptr, nullptr, &ancho_hoja_pelo, nullptr);
    columnas_pelo = ancho_hoja_pelo / ANCHO_CELDA_PELO;

    int ancho_hoja_sombrero = 0;
    SDL_QueryTexture(sombrero.ptr(), nullptr, nullptr, &ancho_hoja_sombrero, nullptr);
    columnas_sombrero = ancho_hoja_sombrero / ANCHO_CELDA_SOMBRERO;

    int ancho_hoja_remera = 0;
    SDL_QueryTexture(remera.ptr(), nullptr, nullptr, &ancho_hoja_remera, nullptr);
    columnas_remera = COLUMNAS_REMERA;
}

int Personaje::ancho_del_sprite() const {
    return ancho;
}

int Personaje::alto_del_sprite() const {
    return alto;
}

void Personaje::dibujar(
    SDL_Renderer* renderer, const EstadoJugador& jugador, bool haciendo_accion, Uint32 ms_de_accion,
    int cam_x, int cam_y) const {
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

    if (haciendo_accion) {
        const int cuadro = cuadro_de_accion(ms_de_accion);
        fila_cuerpo = FILA_ACCION_POR_DIRECCION[fila_direccion];
        fila_brazos = fila_cuerpo;
        columna_cuerpo = cuadro % 4;
        columna_brazos = 6 + (cuadro % 4);
        desvio_y_accesorios = desvio_de_accion(fila_direccion, cuadro);
    } else if (jugador.camina) {
        const Uint32 ahora = SDL_GetTicks();
        const int cuadro = static_cast<int>((ahora / ms_por_cuadro_caminando) % 3);
        columna_cuerpo = cuadro;
        columna_brazos = 6 + cuadro;
        if (cuadro != 0) {
            desvio_y_accesorios = 1;  // +4 píxeles de pantalla a escala x4 original = 1 de mundo
        }
    }

    const int x_pantalla = static_cast<int>(jugador.x) * escala_de_tiles - cam_x;
    const int y_pantalla = static_cast<int>(jugador.y) * escala_de_tiles - cam_y;
    const SDL_Rect destino_cuerpo{
        x_pantalla,
        y_pantalla,
        ancho * escala_de_tiles,
        alto * escala_de_tiles};

    const SDL_Rect origen_cuerpo{columna_cuerpo * ancho, fila_cuerpo * alto, ancho, alto};
    SDL_RenderCopyEx(renderer, cuerpo.ptr(), &origen_cuerpo, &destino_cuerpo, 0.0, nullptr, flip);

    const int columna_estilo_pantalon = indice_pantalon % COLUMNAS_HOJA_PANTALON;
    const int fila_estilo_pantalon = indice_pantalon / COLUMNAS_HOJA_PANTALON;
    SDL_Rect origen_pantalon = origen_cuerpo;
    origen_pantalon.x += columna_estilo_pantalon * ANCHO_ESTILO_PANTALON;
    origen_pantalon.y += fila_estilo_pantalon * ALTO_ESTILO_PANTALON;
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

    const int columna_pelo = indice_pelo % columnas_pelo;
    const int pagina_pelo = indice_pelo / columnas_pelo;
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

    if (indice_sombrero >= 0 && columnas_sombrero > 0) {
        const int columna_sombrero = indice_sombrero % columnas_sombrero;
        const int pagina_sombrero = indice_sombrero / columnas_sombrero;
        const SDL_Rect origen_sombrero{
            columna_sombrero * ANCHO_CELDA_SOMBRERO,
            FILA_SOMBRERO_POR_DIRECCION[fila_direccion] + pagina_sombrero * ALTO_BLOQUE_SOMBRERO,
            ANCHO_CELDA_SOMBRERO,
            ANCHO_CELDA_SOMBRERO};
        const int margen_sombrero = (ANCHO_CELDA_SOMBRERO - ancho) / 2;
        const SDL_Rect destino_sombrero{
            x_pantalla - margen_sombrero * escala_de_tiles,
            y_pantalla +
                (offset_y_sombrero + desvio_y_accesorios - margen_sombrero) * escala_de_tiles,
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

    const int columna_remera = indice_remera % columnas_remera;
    const int pagina_remera = indice_remera / columnas_remera;
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

    if (haciendo_accion) {
        SDL_RenderCopyEx(
            renderer,
            cuerpo.ptr(),
            &origen_brazos,
            &destino_cuerpo,
            0.0,
            nullptr,
            flip);
    }
}
