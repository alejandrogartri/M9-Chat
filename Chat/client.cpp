#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <thread>
#include <iostream>
#include <windows.h>
#include <string>
#include <time.h>

#define BUFFER_LENGTH 1024
#define PORT 12345

using namespace std;

void receiveMessages(SOCKET clientSocket);

WSAData wsaData;
bool connected;

string username;

int main(int argc, char** argv)
{
    if (WSAStartup(WORD((2, 2)), &wsaData) != 0)
    {
        cerr << "Error al inicializar WinSock: " << WSAGetLastError() << endl;
        return WSAGetLastError();
    }

    cout << "Introduce tu nombre: ";
    while (username == "")
    {
        cin >> username;
    }

    int result;
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress = {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(PORT);

    if (result = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0)
    {
        cerr << "Error al conectarse al servidor (Código de error: " << result << ")" << endl;
        return result;
    }

    cout << "Conectado al servidor con éxito.\n====================================" << endl;
    connected = true;

    // Envía un mensaje de bienvenida a los demás usuarios
    string welcomeMessage = username + " se ha unido a la sala.";
    send(clientSocket, welcomeMessage.c_str(), BUFFER_LENGTH, 0);

    // Comienza el hilo de recepción de mensajes
    thread recvThread(receiveMessages, clientSocket);

    while (connected)
    {
        cout << "> ";
        string message;
        getline(cin, message);
        
        if (message != "")
        {
            if (message[0] != '/')
            {
                message = username + " : " + message;
            }
            send(clientSocket, message.c_str(), BUFFER_LENGTH, 0);
        }
    }

    recvThread.join();
    
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}

void receiveMessages(SOCKET clientSocket)
{
    while (connected)
    {
        char buffer[BUFFER_LENGTH];
        int bytes = recv(clientSocket, buffer, BUFFER_LENGTH, 0);
        if (bytes > 0)
        {
            if (strcmp(buffer, "quit") == 0)
            {
                cout << "Desconectado (comando \"/quit\")";
                connected = false;
            }
            else
            {
                cout << "\r" << buffer << endl << "> ";
            }
        }
        else if (bytes == 0)
        {
            cout << "Desconectado (no hay datos)";
            connected = false;
        }
        else
        {
            connected = false;
        }
    }
}