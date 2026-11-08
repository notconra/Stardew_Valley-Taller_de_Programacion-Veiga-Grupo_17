#ifndef EMOJIS_DE_CHAT_H
#define EMOJIS_DE_CHAT_H

#include <SDL2/SDL.h>

#include <string>
#include <unordered_map>
#include <vector>

// Emojis del chat, cargados solos desde una carpeta. Cada .png es un emoji y su nombre es el del
// archivo sin extensión. En un mensaje se escriben como :nombre: así viajan por el servidor como
// texto común y cada cliente los dibuja con su propia imagen.
class EmojisDeChat {
public:
    struct Emoji {
        std::string nombre;
        SDL_Texture* textura = nullptr;
    };

    // Si la carpeta no existe, el chat funciona igual, solo que sin emojis.
    EmojisDeChat(SDL_Renderer* renderer, const std::string& carpeta);
    ~EmojisDeChat();

    EmojisDeChat(const EmojisDeChat&) = delete;
    EmojisDeChat& operator=(const EmojisDeChat&) = delete;

    // En orden alfabético por nombre.
    const std::vector<Emoji>& todos() const;

    // nullptr si no hay ningún emoji con ese nombre.
    const Emoji* buscar(const std::string& nombre) const;

private:
    std::vector<Emoji> emojis;
    std::unordered_map<std::string, size_t> posicion_por_nombre;
};

#endif
