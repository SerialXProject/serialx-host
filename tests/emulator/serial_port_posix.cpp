// Implementazione POSIX (termios) dell'accesso alla porta seriale per
// PC_HardwareSerial. Compilato SOLO su Linux/macOS (vedi build.ps1).
// Su Windows viene compilato serial_port_win.cpp al suo posto: l'interfaccia
// pubblica in Arduino.h resta identica su entrambe le piattaforme.
//
// Pensato per essere usato con una coppia di pseudo-terminali creata con:
//   socat -d -d pty,raw,echo=0 pty,raw,echo=0
// socat stampa due percorsi (es. /dev/pts/3 e /dev/pts/4): uno va passato
// via SERIALX_TEST_PORT a questo programma, l'altro si apre con un
// terminale seriale (es. minicom, screen, PuTTY su WSL).
#include "Arduino.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sys/ioctl.h>

using namespace std;

namespace {

// -1 = non connesso. portHandle in PC_HardwareSerial punta a questo intero
// allocato sull'heap, per mantenere l'handle opaco nell'header comune.
struct PosixPortState {
    int fd = -1;
};

// Nome/percorso della porta configurabile via variabile d'ambiente
// SERIALX_TEST_PORT, ad es. SERIALX_TEST_PORT=/dev/pts/4. Se non impostata,
// usa /dev/pts/4 come default: adatta al percorso stampato da socat sul
// tuo sistema, che cambia ad ogni esecuzione.
std::string resolvePortName()
{
    const char* envPort = std::getenv("SERIALX_TEST_PORT");
    if (envPort && envPort[0] != '\0') {
        return std::string(envPort);
    }
    return "/dev/pts/4";
}

speed_t baudToSpeed(int baud)
{
    switch (baud) {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:     return B9600;
    }
}

} // namespace

PC_HardwareSerial::PC_HardwareSerial() : portHandle(nullptr), connected(false) {}
PC_HardwareSerial::~PC_HardwareSerial() { end(); }

void PC_HardwareSerial::end()
{
    if (portHandle) {
        auto* state = static_cast<PosixPortState*>(portHandle);
        if (state->fd >= 0) {
            close(state->fd);
        }
        delete state;
        portHandle = nullptr;
    }
    connected = false;
}

void PC_HardwareSerial::begin(int baud)
{
    std::string portName = resolvePortName();

    int fd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        cout << "[SERIAL ERROR] Impossibile aprire " << portName
             << " (" << strerror(errno) << "). "
             << "Imposta SERIALX_TEST_PORT per usarne un'altra." << endl;
        return;
    }

    termios tty{};
    if (tcgetattr(fd, &tty) != 0) {
        cout << "[SERIAL ERROR] tcgetattr fallita su " << portName
             << " (" << strerror(errno) << ")" << endl;
        close(fd);
        return;
    }

    cfmakeraw(&tty); // 8N1, no echo, no elaborazione: comportamento "grezzo" come una seriale reale
    speed_t speed = baudToSpeed(baud);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cc[VMIN]  = 0; // read non bloccante: available()/read() gestiscono il polling
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        cout << "[SERIAL ERROR] tcsetattr fallita su " << portName
             << " (" << strerror(errno) << ")" << endl;
        close(fd);
        return;
    }

    auto* state = new PosixPortState();
    state->fd = fd;
    portHandle = state;
    connected = true;
    cout << "[SERIAL] Connesso a " << portName << " (" << baud << " baud)" << endl;
}

int PC_HardwareSerial::available()
{
    if (!connected || !portHandle)
        return 0;
    auto* state = static_cast<PosixPortState*>(portHandle);
    int bytesAvailable = 0;
    if (ioctl(state->fd, FIONREAD, &bytesAvailable) < 0) {
        return 0;
    }
    return bytesAvailable;
}

char PC_HardwareSerial::read()
{
    char c = 0;
    if (connected && portHandle) {
        auto* state = static_cast<PosixPortState*>(portHandle);
        ssize_t n = ::read(state->fd, &c, 1);
        if (n == 1) {
            cout << "[SERIAL] Read: " << c << endl;
        }
    }
    return c;
}

// Usata da PC_HardwareSerial::print() in Arduino.cpp per inviare byte
// sulla porta reale, indipendentemente dalla piattaforma.
void writeToPort(void* portHandle, bool connected, const char* s)
{
    if (!connected || !portHandle)
        return;
    auto* state = static_cast<PosixPortState*>(portHandle);
    size_t len = strlen(s);
    ssize_t written = ::write(state->fd, s, len);
    (void)written; // best-effort: coerente con il comportamento originale su Windows
    cout << "[SERIAL] Write: " << s << endl;
}
