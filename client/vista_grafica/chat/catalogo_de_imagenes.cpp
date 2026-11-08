#include "catalogo_de_imagenes.h"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

CatalogoDeImagenes::CatalogoDeImagenes(SDL_Renderer* renderer, const std::string& carpeta) {
    std::vector<std::filesystem::path> archivos;
    for (const auto& entrada : std::filesystem::directory_iterator(carpeta)) {
        const std::string extension = entrada.path().extension().string();
        if (extension == ".png" || extension == ".gif") {
            archivos.push_back(entrada.path());
        }
    }
    std::sort(archivos.begin(), archivos.end());

    for (const std::filesystem::path& archivo : archivos) {
        const std::string nombre = archivo.stem().string();
        if (nombre.find_first_of(":] \t") != std::string::npos) {
            throw std::runtime_error(
                "el nombre de la imagen no puede tener ':', ']' ni espacios: " + archivo.string());
        }

        SDL_Texture* textura = IMG_LoadTexture(renderer, archivo.string().c_str());
        if (textura == nullptr) {
            throw std::runtime_error(
                "no se pudo cargar " + archivo.string() + ": " + IMG_GetError());
        }
        SDL_SetTextureScaleMode(textura, SDL_ScaleModeLinear);
        imagenes.push_back(ImagenChat{nombre, textura});
    }
}

const std::vector<ImagenChat>& CatalogoDeImagenes::todas() const {
    return imagenes;
}

const ImagenChat* CatalogoDeImagenes::buscar(const std::string& nombre) const {
    const auto encontrada =
        std::find_if(imagenes.begin(), imagenes.end(), [&nombre](const ImagenChat& imagen) {
            return imagen.nombre == nombre;
        });
    return encontrada == imagenes.end() ? nullptr : &*encontrada;
}

CatalogoDeImagenes::~CatalogoDeImagenes() {
    for (const ImagenChat& imagen : imagenes) {
        SDL_DestroyTexture(imagen.textura);
    }
}
