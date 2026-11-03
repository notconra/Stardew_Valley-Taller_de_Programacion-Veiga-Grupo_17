#ifndef ISOCKET_H_
#define ISOCKET_H_

// Interfaz de un socket ya conectado.
// Existe para poder reemplazar el Socket real por uno falso en los tests.
// Las firmas copian las del Socket de la cátedra.
// No incluye accept() porque lo vamos a usar para mockear sockets ya conectados.
class ISocket {
protected:
    ISocket() = default;
    ISocket(ISocket&&) = default;
    ISocket& operator=(ISocket&&) = default;

public:
    virtual ~ISocket() = default;

    virtual int sendsome(const void* data, unsigned int size) = 0;
    virtual int recvsome(void* data, unsigned int size) = 0;

    virtual int sendall(const void* data, unsigned int size) = 0;
    virtual int recvall(void* data, unsigned int size) = 0;

    virtual void shutdown(int how) = 0;
    virtual int close() = 0;

    ISocket(const ISocket&) = delete;
    ISocket& operator=(const ISocket&) = delete;
};

#endif
