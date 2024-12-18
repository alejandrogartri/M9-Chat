#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <thread>
#include <iostream>
#include <windows.h>
#include <vector>
#include <algorithm>

#define MAX_USERS 5
#define BUFFER_LENGTH 1024
#define PORT 12345

using namespace std;

void handleClient();

WSAData wsaData;
SOCKET serverSocket;
vector<SOCKET> clientSockets;

int main(int argc, char** argv)
{
    // Inicializamos WinSock 2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "Error al inicializar WinSock: " << WSAGetLastError() << endl;
        return WSAGetLastError();
    }

    int result;

    // Creamos el socket y la dirección IP

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(PORT);

    // Asignamos la dirección al socket
    if (result = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        cerr << "Error al asignar la dirección IP al socket: " << WSAGetLastError() << endl;
        return result;
    }

    // Iniciamos la escucha de clientes
    if (result = listen(serverSocket, MAX_USERS) != 0)
    {
        cerr << "Error al iniciar la escucha en el servidor (Código de error: " << result << ")" << endl;
        return result;
    }

    cout << "Esperando conexiones..." << endl;
    vector<thread> clientThreads;
    
    // Creamos tantos hilos como usuarios posibles
    for (int i = 0; i < MAX_USERS; i++)
    {
        clientThreads.emplace_back(handleClient);
    }

    // Esperamos a que todos la ejecución de los hilos haya terminado
    for (thread& t : clientThreads)
    {
        t.join();
    }

    WSACleanup();
    return 0;
}

void handleClient() 
{
    sockaddr_in clientAddress;
    int addressLength = sizeof(clientAddress);
    SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addressLength);
    
    cout << "Conexión aceptada desde " << inet_ntoa(clientAddress.sin_addr) << endl;

    char welcomeBuffer[BUFFER_LENGTH];
    if (recv(clientSocket, welcomeBuffer, BUFFER_LENGTH, 0))
    {
        for (SOCKET client : clientSockets)
        {
            send(client, welcomeBuffer, BUFFER_LENGTH, 0);
        }
    }

    clientSockets.push_back(clientSocket);

    while (bool connected = true)
    {
        char buffer[BUFFER_LENGTH];
        int bytes = recv(clientSocket, buffer, BUFFER_LENGTH, 0);
        if (bytes > 0)
        {

            if (strcmp(buffer, "/quit") == 0)
            {
                cout << inet_ntoa(clientAddress.sin_addr) << " ha abandonado la sala.";
                string message = "quit";
                send(clientSocket, message.c_str(), BUFFER_LENGTH, 0);
                connected = false;
            }
            else
            {
                for (SOCKET client : clientSockets)
                {
                    send(client, buffer, BUFFER_LENGTH, 0);
                }
            }
        }
        else if (bytes == 0)
        {
            cout << "Desconectado del servidor.";
            connected = false;
        }
        else
        {
            connected = false;
        }
    }

    clientSockets.erase(std::remove(clientSockets.begin(), clientSockets.end(), clientSocket), clientSockets.end());
    closesocket(clientSocket);
}