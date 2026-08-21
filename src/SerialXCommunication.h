#pragma once

#if defined(ARDUINO)
    #include <Arduino.h>
    using StringType = String;
#else
    #include <string>

    using namespace std;

    using StringType = string;
    
    #ifdef _WIN32
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif
        #include <windows.h>
        #include <winsock2.h>
    #else
        #include <netinet/in.h>
    #endif
#endif


class SerialXCommunication {
public:
    // Il costruttore deve avere il nome della classe!
    explicit SerialXCommunication(int port); // in seriale è baud_rate
    ~SerialXCommunication();

    // Non copiabile (possiede file descriptor)
    SerialXCommunication(const SerialXCommunication&) = delete;
    SerialXCommunication& operator=(const SerialXCommunication&) = delete;

    // Crea il socket, fa bind e listen. Ritorna false in caso di errore.
    bool begin();

    // Accetta una nuova connessione client (bloccante o gestita).
    bool open();

    // Legge una riga terminata da '\n' dal client corrente.
    bool readLine(StringType& outLine);

    // Invia una riga (aggiunge '\n') al client corrente.
    bool sendLine(const StringType& line);
    // Sovraccarichi per inviare righe vuote o altri tipi senza ambiguità
    bool sendLine(); // Per inviare solo un a capo '\n'
    bool sendLine(int value);
    bool sendLine(long value);
    bool sendLine(unsigned int value);
    bool sendLine(unsigned long value);
    bool sendLine(float value, int decimals = 2);
    bool sendLine(double value, int decimals = 2);
    bool sendLine(char value);

    // Chiude la connessione col client corrente (non il server).
    void close();

    // Chiude tutto: client corrente + socket server. Libera le risorse.
    void destroy();

    // Info utili
    bool isClientConnected() const { return client_fd_ >= 0; }
    #ifndef ARDUINO // Only NET mode
    StringType clientIp() const { return clientIp_; }
    #endif

private:
    int port_;
    int backlog_;
    int server_fd_ = -1;
    int client_fd_ = -1;
    
    // Gestione cross-platform del tipo di indirizzo socket
    #ifdef _WIN32
      SOCKET server_socket_ = INVALID_SOCKET;
      SOCKET client_socket_ = INVALID_SOCKET;
    #endif
    #ifndef ARDUINO
    sockaddr_in address_{};
    string clientIp_;
    #endif
};