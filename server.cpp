#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <thread>

using namespace std;

const int FILAS = 6;
const int COLUMNAS = 7;
const char VACIO = ' ';
const char JUGADOR1 = 'S';
const char JUGADOR2 = 'C';

struct Juego {
    int socketJugador1;
    int socketJugador2;
    char tablero[FILAS][COLUMNAS];
    bool juegoEnCurso;
    char turno;
    string ipPuertoJugador1;
    string ipPuertoJugador2;
};

vector<Juego> juegos; // Almacenar juegos activos

void inicializarTablero(char tablero[FILAS][COLUMNAS]) {
    for (int i = 0; i < FILAS; ++i) {
        for (int j = 0; j < COLUMNAS; ++j) {
            tablero[i][j] = VACIO;
        }
    }
}

void imprimirTablero(const char tablero[FILAS][COLUMNAS]) {
    for (int i = FILAS - 1; i >= 0; --i) {
        for (int j = 0; j < COLUMNAS; ++j) {
            cout << tablero[i][j] << " ";
        }
        cout << endl;
    }
    cout << "-------------" << endl;
    for (int j = 1; j <= COLUMNAS; ++j) {
        cout << j << " ";
    }
    cout << endl;
}

bool verificarVictoria(const char tablero[FILAS][COLUMNAS], char jugador) {
    // Verificar horizontalmente
    for (int i = 0; i < FILAS; ++i) {
        for (int j = 0; j <= COLUMNAS - 4; ++j) {
            if (tablero[i][j] == jugador &&
                tablero[i][j+1] == jugador &&
                tablero[i][j+2] == jugador &&
                tablero[i][j+3] == jugador) {
                return true;
            }
        }
    }

    // Verificar verticalmente
    for (int j = 0; j < COLUMNAS; ++j) {
        for (int i = 0; i <= FILAS - 4; ++i) {
            if (tablero[i][j] == jugador &&
                tablero[i+1][j] == jugador &&
                tablero[i+2][j] == jugador &&
                tablero[i+3][j] == jugador) {
                return true;
            }
        }
    }

    // Verificar en diagonal (ascendente)
    for (int i = 3; i < FILAS; ++i) {
        for (int j = 0; j <= COLUMNAS - 4; ++j) {
            if (tablero[i][j] == jugador &&
                tablero[i-1][j+1] == jugador &&
                tablero[i-2][j+2] == jugador &&
                tablero[i-3][j+3] == jugador) {
                return true;
            }
        }
    }

    // Verificar en diagonal (descendente)
    for (int i = 3; i < FILAS; ++i) {
        for (int j = 3; j < COLUMNAS; ++j) {
            if (tablero[i][j] == jugador &&
                tablero[i-1][j-1] == jugador &&
                tablero[i-2][j-2] == jugador &&
                tablero[i-3][j-3] == jugador) {
                return true;
            }
        }
    }

    return false;
}

bool verificarColumnaLlena(const char tablero[FILAS][COLUMNAS], int columna) {
    return tablero[FILAS-1][columna] != VACIO; // La fila más baja de la columna no está vacía
}

bool colocarFicha(char tablero[FILAS][COLUMNAS], int columna, char jugador) {
    if (columna < 0 || columna >= COLUMNAS) {
        return false; // Columna fuera de rango
    }

    // Buscar la primera fila vacía (de arriba hacia abajo) en la columna seleccionada
    for (int fila = FILAS - 1; fila >= 0; --fila) {
        if (tablero[fila][columna] == VACIO) {
            tablero[fila][columna] = jugador;
            return true;
        }
    }

    return false; // Columna completa
}

void manejarJuego(Juego& juego) {
    inicializarTablero(juego.tablero);
    juego.juegoEnCurso = true;
    juego.turno = JUGADOR1; // Comienza el jugador 1

    cout << "Juego nuevo [" << juego.ipPuertoJugador1 << "]" << endl;
    if (juego.socketJugador2 != -1) {
        cout << "Juego nuevo [" << juego.ipPuertoJugador2 << "]" << endl;
    }

    while (juego.juegoEnCurso) {
        char estadoTablero[1024];
        sprintf(estadoTablero, "TABLERO\n");
        for (int i = 0; i < FILAS; ++i) {
            for (int j = 0; j < COLUMNAS; ++j) {
                sprintf(estadoTablero + strlen(estadoTablero), "%c ", juego.tablero[i][j]);
            }
            sprintf(estadoTablero + strlen(estadoTablero), "\n");
        }
        sprintf(estadoTablero + strlen(estadoTablero), "-------------\n");
        for (int j = 1; j <= COLUMNAS; ++j) {
            sprintf(estadoTablero + strlen(estadoTablero), "%d ", j);
        }
        sprintf(estadoTablero + strlen(estadoTablero), "\n");

        send(juego.socketJugador1, estadoTablero, strlen(estadoTablero), 0);
        if (juego.socketJugador2 != -1) {
            send(juego.socketJugador2, estadoTablero, strlen(estadoTablero), 0);
        }

        int jugadorSocket = (juego.turno == JUGADOR1) ? juego.socketJugador1 : juego.socketJugador2;
        send(jugadorSocket, "Tu turno. Ingrese columna (1-7): ", 36, 0);

        char buffer[1024];
        int bytesRecibidos = recv(jugadorSocket, buffer, sizeof(buffer), 0);

        if (bytesRecibidos > 0) {
            buffer[bytesRecibidos] = '\0'; // Añadir terminador nulo
            int columna = atoi(buffer) - 1; // Convertir entrada a número de columna y ajustar índice
            if (colocarFicha(juego.tablero, columna, juego.turno)) {
                // La ficha se ha colocado correctamente en el tablero
                // Verificar victoria después de colocar la ficha
                if (verificarVictoria(juego.tablero, juego.turno)) {
                    send(jugadorSocket, "¡Ganaste!\n", 10, 0);
                    int otroJugadorSocket = (juego.turno == JUGADOR1) ? juego.socketJugador2 : juego.socketJugador1;
                    if (otroJugadorSocket != -1) {
                        send(otroJugadorSocket, "¡Perdiste!\n", 10, 0);
                    }
                    juego.juegoEnCurso = false;
                } else {
                    juego.turno = (juego.turno == JUGADOR1) ? JUGADOR2 : JUGADOR1;
                }
            } else {
                send(jugadorSocket, "Jugada inválida. Columna llena.\n", 32, 0);
            }
        } else {
            if (bytesRecibidos == 0) {
                cout << "El jugador ha cerrado la conexión." << endl;
            } else {
                cerr << "Error al recibir datos del jugador: " << strerror(errno) << endl;
            }
            juego.juegoEnCurso = false;
        }
    }

    close(juego.socketJugador1);
    if (juego.socketJugador2 != -1) {
        close(juego.socketJugador2);
    }

    // Mostrar resultados del juego al finalizar
    if (!juego.juegoEnCurso) {
        if (juego.turno == JUGADOR1) {
            cout << "Juego [" << juego.ipPuertoJugador1 << "]: gana JUGADOR1." << endl;
            if (juego.socketJugador2 != -1) {
                cout << "Juego [" << juego.ipPuertoJugador2 << "]: fin del juego." << endl;
            }
        } else {
            cout << "Juego [" << juego.ipPuertoJugador2 << "]: gana JUGADOR2." << endl;
            cout << "Juego [" << juego.ipPuertoJugador1 << "]: fin del juego." << endl;
        }
    } else {
        cout << "Juego [" << juego.ipPuertoJugador1 << "]: empate." << endl;
        if (juego.socketJugador2 != -1) {
            cout << "Juego [" << juego.ipPuertoJugador2 << "]: fin del juego." << endl;
        }
    }
}

void handleClient(int socketCliente, sockaddr_in direccionCliente) {
    if (juegos.empty() || juegos.back().socketJugador2 != -1) {
        Juego nuevoJuego;
        nuevoJuego.socketJugador1 = socketCliente;
        nuevoJuego.socketJugador2 = -1;
        nuevoJuego.ipPuertoJugador1 = inet_ntoa(direccionCliente.sin_addr) + string(":") + to_string(ntohs(direccionCliente.sin_port));
        juegos.push_back(nuevoJuego);

        send(socketCliente, "Esperando a un segundo jugador...\n", 33, 0);
    } else {
        juegos.back().socketJugador2 = socketCliente;
        juegos.back().ipPuertoJugador2 = inet_ntoa(direccionCliente.sin_addr) + string(":") + to_string(ntohs(direccionCliente.sin_port));
        send(juegos.back().socketJugador1, "¡Juego listo! Comienza el JUGADOR1.\n", 36, 0);
        send(juegos.back().socketJugador2, "¡Juego listo! Comienza el JUGADOR1.\n", 36, 0);

        thread gameThread(manejarJuego, ref(juegos.back()));
        gameThread.detach();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <puerto>" << endl;
        return 1;
    }

    int puerto = atoi(argv[1]);
    int socketServidor = 0;
    sockaddr_in direccionServidor, direccionCliente;

    cout << "Creando socket de escucha ..." << endl;
    if ((socketServidor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cerr << "Error al crear el socket de escucha" << endl;
        return 1;
    }

    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
    direccionServidor.sin_port = htons(puerto);

    cout << "Configurando estructura de dirección del socket ..." << endl;
    if (bind(socketServidor, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0) {
        cerr << "Error al llamar a bind()" << endl;
        return 1;
    }

    cout << "Escuchando en el puerto " << puerto << " ..." << endl;
    if (listen(socketServidor, 10) < 0) {
        cerr << "Error al llamar a listen()" << endl;
        return 1;
    }

    while (true) {
        socklen_t addrSize = sizeof(struct sockaddr_in);
        int socketCliente;

        cout << "Esperando la solicitud del cliente ..." << endl;
        if ((socketCliente = accept(socketServidor, (struct sockaddr *)&direccionCliente, &addrSize)) < 0) {
            cerr << "Error al llamar a accept()" << endl;
            continue;
        }

        thread clientThread(handleClient, socketCliente, direccionCliente);
        clientThread.detach();
    }

    close(socketServidor);
    return 0;
}
