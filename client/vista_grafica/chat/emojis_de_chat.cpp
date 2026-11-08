#include "emojis_de_chat.h"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

bool es_png(const fs::path& ruta) {
    std::string extension = ruta.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return extension == ".png";
}

// El nombre se escribe entre ":" y se separa de las palabras con espacios, así que no puede
// contener ninguno de los dos.
bool es_nombre_valido(const std::string& nombre) {
    return !nombre.empty() && nombre.find_first_of(": \t") == std::string::npos;
}

}  // namespace

EmojisDeChat::EmojisDeChat(SDL_Renderer* renderer, const std::string& carpeta) {
    std::error_code error;
    if (!fs::is_directory(carpeta, error)) {
        std::cerr << "[chat] no existe la carpeta de emojis '" << carpeta
                  << "': el chat no tendrá emojis\n";
        return;
    }

    std::vector<fs::path> archivos;
    for (const fs::directory_entry& entrada : fs::directory_iterator(carpeta, error)) {
        if (entrada.is_regular_file(error) && es_png(entrada.path())) {
            archivos.push_back(entrada.path());
        }
    }
    std::sort(archivos.begin(), archivos.end());

    for (const fs::path& archivo : archivos) {
        const std::string nombre = archivo.stem().string();
        if (!es_nombre_valido(nombre)) {
            std::cerr << "[chat] emoji ignorado, el nombre no puede tener ':' ni espacios: "
                      << archivo.string() << '\n';
            continue;
        }

        SDL_Texture* textura = IMG_LoadTexture(renderer, archivo.string().c_str());
        if (textura == nullptr) {
            std::cerr << "[chat] no se pudo cargar el emoji " << archivo.string() << ": "
                      << IMG_GetError() << '\n';
            continue;
        }
        SDL_SetTextureScaleMode(textura, SDL_ScaleModeLinear);

        posicion_por_nombre.emplace(nombre, emojis.size());
        emojis.push_back(Emoji{nombre, textura});
    }
}

EmojisDeChat::~EmojisDeChat() {
    for (const Emoji& emoji : emojis) {
        SDL_DestroyTexture(emoji.textura);
    }
}

const std::vector<EmojisDeChat::Emoji>& EmojisDeChat::todos() const {
    return emojis;
}

const EmojisDeChat::Emoji* EmojisDeChat::buscar(const std::string& nombre) const {
    const auto encontrado = posicion_por_nombre.find(nombre);
    return encontrado == posicion_por_nombre.end() ? nullptr : &emojis[encontrado->second];
}
