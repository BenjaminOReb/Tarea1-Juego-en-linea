#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

const int MAX_BUFFER = 1024;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <dirección IP> <puerto>" << endl;
        return 1;
    }

    const char *ipServidor = argv[1];
    int puerto = atoi(argv[2]);

    int socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socketCliente == -1) {
        cerr << "Error al crear el socket." << endl;
        return 1;
    }

    sockaddr_in direccionServidor;
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_port = htons(puerto);
    inet_pton(AF_INET, ipServidor, &direccionServidor.sin_addr);

    if (connect(socketCliente, (sockaddr *)&direccionServidor, sizeof(direccionServidor)) == -1) {
        cerr << "Error al intentar conectarse al servidor." << endl;
        close(socketCliente);
        return 1;
    }

    cout << "Conectado al servidor." << endl;

    char buffer[MAX_BUFFER];
    int bytesRecibidos;
    int columna; // Variable para almacenar la columna seleccionada

    while (true) {
        // Recibir mensajes del servidor
        bytesRecibidos = recv(socketCliente, buffer, MAX_BUFFER - 1, 0);
        if (bytesRecibidos == -1) {
            cerr << "Error al recibir datos del servidor." << endl;
            break;
        } else if (bytesRecibidos == 0) {
            cerr << "Conexión cerrada por el servidor." << endl;
            break;
        }

        buffer[bytesRecibidos] = '\0';
        cout << buffer;

        if (strstr(buffer, "¡Ganaste!") != nullptr) {
            break; // Terminar el juego si el cliente ganó
        }

        // Pedir entrada al usuario (columna)
        cout << "Ingrese la columna (1-7): ";
        if (!(cin >> columna) || columna < 1 || columna > 7) {
            cerr << "Entrada inválida. Debe ser un número entre 1 y 7." << endl;
            break;
        }

        // Enviar la jugada al servidor
        snprintf(buffer, MAX_BUFFER, "%d", columna);
        if (send(socketCliente, buffer, strlen(buffer), 0) == -1) {
            cerr << "Error al enviar datos al servidor." << endl;
            break;
        }
    }

    close(socketCliente);
    return 0;
}

