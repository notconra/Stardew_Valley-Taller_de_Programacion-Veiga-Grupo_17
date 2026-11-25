#ifndef CATALOGO_DE_IMAGENES_H
#define CATALOGO_DE_IMAGENES_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

struct ImagenChat {
    std::string nombre;
    SDL_Texture* textura = nullptr;
};

// Imágenes del chat cargadas desde una carpeta: cada .png o .gif es una imagen y su nombre es el
// del archivo sin extensión. Sirve para emojis (":nombre:") y para emotes ("[emote:nombre]"), por
// eso la vista tiene dos catálogos, uno por carpeta. Los .gif se muestran con su primer cuadro.
//
// Lanza una excepción si la carpeta no existe, si una imagen no se puede cargar o si su nombre
// tiene ':', ']' o espacios (rompería el formato con el que viaja en el mensaje).
class CatalogoDeImagenes {
public:
    CatalogoDeImagenes(SDL_Renderer* renderer, const std::string& carpeta);

    CatalogoDeImagenes(const CatalogoDeImagenes&) = delete;
    CatalogoDeImagenes& operator=(const CatalogoDeImagenes&) = delete;

    // En orden alfabético por nombre de archivo.
    const std::vector<ImagenChat>& todas() const;

    // nullptr si no hay ninguna imagen con ese nombre.
    const ImagenChat* buscar(const std::string& nombre) const;

    ~CatalogoDeImagenes();

private:
    std::vector<ImagenChat> imagenes;

    void liberar();
};

#endif
