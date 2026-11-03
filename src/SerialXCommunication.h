#pragma once
// =============================================================================
//  SerialXCommunication.h  -  header-only, stessa interfaccia su due piattaforme
//
//   * Arduino  : wrapper inline su Serial (costo ~zero, un solo membro da 4 byte)
//   * Desktop  : server TCP (Windows / Linux / macOS), una connessione client
//
//  Interfaccia comune:
//      SerialXCommunication comm(porta_o_baud);
//      comm.begin();            // Serial.begin  | socket+bind+listen
//      comm.open();             // (Serial: pronta?) | accept
//      comm.readLine(...);      // legge una riga terminata da '\n'
//      comm.sendLine(...);      // invia una riga (aggiunge '\n')
//      comm.close();            // chiude il client
//      comm.destroy();          // chiude tutto
//
//  Windows: linka ws2_32 (MSVC: gia' incluso dal pragma, MinGW: -lws2_32)
// =============================================================================

// -----------------------------------------------------------------------------
//  ARDUINO
// -----------------------------------------------------------------------------
#if defined(ARDUINO)

#include <Arduino.h>

using StringType = String;

class SerialXCommunication {
public:
    explicit SerialXCommunication(unsigned long baud) : baud_(baud) {}

    bool begin()   { Serial.begin(baud_); return true; }
    bool open()    { return static_cast<bool>(Serial); }
    void close()   {}
    void destroy() {}

    // --- Lettura -------------------------------------------------------------
    // Versione leggera: buffer fisso, nessun uso dello heap (consigliata su AVR).
    // Attenzione: se non arriva '\n' attende fino al timeout (Serial.setTimeout).
    bool readLine(char* buf, size_t len) {
        if (len == 0 || !Serial.available()) return false;
        size_t n = Serial.readBytesUntil('\n', buf, len - 1);
        if (n > 0 && buf[n - 1] == '\r') n--;      // gestisce "\r\n"
        buf[n] = '\0';
        return true;
    }

    // Versione comoda con String (usa lo heap: evitala se la RAM e' poca).
    bool readLine(String& out) {
        if (!Serial.available()) return false;
        out = Serial.readStringUntil('\n');
        out.trim();
        return true;
    }

    // --- Scrittura -----------------------------------------------------------
    // Un solo template inoltra a Serial.println: int, long, char, String, F("...")...
    template <typename T>
    bool sendLine(const T& v) { Serial.println(v); return true; }

    bool sendLine() { Serial.println(); return true; }

    // Decimali (il linker li scarta se non li chiami)
    bool sendLine(float v,  int decimals = 2) { Serial.println(v, decimals); return true; }
    bool sendLine(double v, int decimals = 2) { Serial.println(v, decimals); return true; }

    bool isClientConnected() const { return static_cast<bool>(Serial); }

private:
    unsigned long baud_;
};

// -----------------------------------------------------------------------------
//  DESKTOP (Windows / Linux / macOS)
// -----------------------------------------------------------------------------
#else

#include <string>
#include <cstdio>
#include <type_traits>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>          // PRIMA di windows.h
    #include <ws2tcpip.h>
    #include <windows.h>
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
    #endif
    using SocketHandle = SOCKET;
    namespace sx_detail {
        constexpr SocketHandle kInvalid = INVALID_SOCKET;
        constexpr int          kSendFlags = 0;
        inline void closeSocket(SocketHandle s) { ::closesocket(s); }
    }
    using SocketLen = int;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using SocketHandle = int;
    namespace sx_detail {
        constexpr SocketHandle kInvalid = -1;
        #ifdef MSG_NOSIGNAL
            constexpr int kSendFlags = MSG_NOSIGNAL;   // niente SIGPIPE se il client chiude
        #else
            constexpr int kSendFlags = 0;              // macOS: vedi SO_NOSIGPIPE in open()
        #endif
        inline void closeSocket(SocketHandle s) { ::close(s); }
    }
    using SocketLen = socklen_t;
#endif

using StringType = std::string;

class SerialXCommunication {
public:
    explicit SerialXCommunication(int port) : port_(port) {}
    ~SerialXCommunication() { destroy(); }

    // Non copiabile (possiede socket)
    SerialXCommunication(const SerialXCommunication&) = delete;
    SerialXCommunication& operator=(const SerialXCommunication&) = delete;

    // Crea il socket, fa bind e listen. false in caso di errore.
    bool begin() {
        if (server_ != sx_detail::kInvalid) return true;   // gia' avviato

    #ifdef _WIN32
        if (!wsaStarted_) {
            WSADATA wsa;
            if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;
            wsaStarted_ = true;
        }
    #endif

        server_ = ::socket(AF_INET, SOCK_STREAM, 0);
        if (server_ == sx_detail::kInvalid) return false;

        int yes = 1;
        ::setsockopt(server_, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&yes), sizeof(yes));

        address_ = sockaddr_in{};
        address_.sin_family      = AF_INET;
        address_.sin_addr.s_addr = htonl(INADDR_ANY);
        address_.sin_port        = htons(static_cast<unsigned short>(port_));

        if (::bind(server_, reinterpret_cast<sockaddr*>(&address_), sizeof(address_)) != 0 ||
            ::listen(server_, backlog_) != 0) {
            sx_detail::closeSocket(server_);
            server_ = sx_detail::kInvalid;
            return false;
        }
        return true;
    }

    // Accetta una nuova connessione client (bloccante).
    bool open() {
        if (server_ == sx_detail::kInvalid) return false;
        close();                                            // chiude il client precedente

        sockaddr_in peer{};
        SocketLen   len = sizeof(peer);
        SocketHandle c = ::accept(server_, reinterpret_cast<sockaddr*>(&peer), &len);
        if (c == sx_detail::kInvalid) return false;

    #if defined(__APPLE__)
        int one = 1;
        ::setsockopt(c, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
    #endif

        client_ = c;
        rxBuffer_.clear();

        char ip[INET_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET, &peer.sin_addr, ip, sizeof(ip));
        clientIp_ = ip;
        return true;
    }

    // Legge una riga terminata da '\n' (bloccante). Rimuove '\n' e '\r'.
    // Ritorna false se il client si disconnette o c'e' un errore.
    bool readLine(StringType& outLine) {
        if (client_ == sx_detail::kInvalid) return false;

        for (;;) {
            std::string::size_type pos = rxBuffer_.find('\n');
            if (pos != std::string::npos) {
                outLine = rxBuffer_.substr(0, pos);
                rxBuffer_.erase(0, pos + 1);
                if (!outLine.empty() && outLine.back() == '\r') outLine.pop_back();
                return true;
            }

            char tmp[512];
            int n = static_cast<int>(::recv(client_, tmp, sizeof(tmp), 0));
            if (n <= 0) {                                   // 0 = chiuso, <0 = errore
                close();
                return false;
            }
            rxBuffer_.append(tmp, static_cast<std::size_t>(n));
        }
    }

    // Invia una riga (aggiunge '\n').
    bool sendLine(const StringType& line) {
        if (client_ == sx_detail::kInvalid) return false;
        std::string data = line;
        data += '\n';

        std::size_t sent = 0;
        while (sent < data.size()) {
            int n = static_cast<int>(::send(client_, data.data() + sent,
                                            static_cast<int>(data.size() - sent),
                                            sx_detail::kSendFlags));
            if (n <= 0) { close(); return false; }
            sent += static_cast<std::size_t>(n);
        }
        return true;
    }

    bool sendLine()                  { return sendLine(StringType()); }          // solo '\n'
    bool sendLine(const char* text)  { return sendLine(StringType(text ? text : "")); }
    bool sendLine(char value)        { return sendLine(StringType(1, value)); }

    // Tutti i tipi interi (int, long, unsigned, size_t, long long, ...)
    template <typename T,
              typename = typename std::enable_if<std::is_integral<T>::value>::type>
    bool sendLine(T value) { return sendLine(std::to_string(value)); }

    // Decimali con numero di cifre controllato
    bool sendLine(float value,  int decimals = 2) { return sendLine(static_cast<double>(value), decimals); }
    bool sendLine(double value, int decimals = 2) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.*f", decimals, value);
        return sendLine(StringType(buf));
    }

    // Chiude il client corrente (non il server).
    void close() {
        if (client_ != sx_detail::kInvalid) {
            sx_detail::closeSocket(client_);
            client_ = sx_detail::kInvalid;
        }
        rxBuffer_.clear();
        clientIp_.clear();
    }

    // Chiude tutto: client + server. Libera le risorse.
    void destroy() {
        close();
        if (server_ != sx_detail::kInvalid) {
            sx_detail::closeSocket(server_);
            server_ = sx_detail::kInvalid;
        }
    #ifdef _WIN32
        if (wsaStarted_) { WSACleanup(); wsaStarted_ = false; }
    #endif
    }

    bool       isClientConnected() const { return client_ != sx_detail::kInvalid; }
    StringType clientIp()          const { return clientIp_; }

private:
    int          port_;
    int          backlog_ = 5;
    SocketHandle server_  = sx_detail::kInvalid;
    SocketHandle client_  = sx_detail::kInvalid;
    sockaddr_in  address_{};
    StringType   clientIp_;
    StringType   rxBuffer_;                 // accumula i dati fino a '\n'
#ifdef _WIN32
    bool         wsaStarted_ = false;
#endif
};

#endif // ARDUINO