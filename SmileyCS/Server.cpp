#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
#include <cstdlib> 
#include <conio.h> 
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

SOCKET ClientSocket = INVALID_SOCKET;
SOCKET ListenSocket = INVALID_SOCKET;

int smileyX = 5;
int smileyY = 5;

const int windowWidth = 100;
const int windowHeight = 100;

bool isKeyPressed = false;
int keyDirection = 0;

void DrawSmiley() {
    system("cls");

    COORD position;
    position.X = smileyX;
    position.Y = smileyY;
    SetConsoleCursorPosition(GetStdHandle(-11), position);

    cout << ":-)";
}

DWORD WINAPI Sender(void* param) {
    char message[2] = " ";

    DrawSmiley();

    while (true) {
        if (_kbhit()) {
            int key = _getch();
            if (key == 224 || key == 0) key = _getch();
            // cout << key << "\n";
            if (key == 72) { // ������� �����
                message[0] = 'w';
                if (smileyY > 0) --smileyY;
            }
            else if (key == 80) { // ������� ����
                message[0] = 's';
                if (smileyY < windowHeight - 1) ++smileyY;
            }
            else  if (key == 77) { // ������� ������
                message[0] = 'd';
                if (smileyX < windowWidth - 3) ++smileyX;
            }
            else if (key == 75) { // ������� �����
                message[0] = 'a';
                if (smileyX > 0) --smileyX;
            }

            int iSendResult = send(ClientSocket, message, 2, 0);

            if (iSendResult == SOCKET_ERROR) {
                cout << "send ���������� � �������: " << WSAGetLastError() << "\n";
                cout << "���, �������� (send) ��������� ��������� �� ���������� ((\n";
                closesocket(ClientSocket);
                WSACleanup();
                return 7;
            }

            DrawSmiley();
        }
    }

    return 0;


    return 0;
}

DWORD WINAPI Receiver(void* param) {
    while (true) {
        char message[DEFAULT_BUFLEN];
        int iResult = recv(ClientSocket, message, DEFAULT_BUFLEN, 0);
        message[iResult] = '\0';

        if (iResult > 0) {
            char direction = message[0];

            switch (direction) {
            case 'a': // �����
                if (smileyX > 0)
                    --smileyX;
                break;
            case 'd': // ������
                ++smileyX;
                break;
            case 'w': // �����
                if (smileyY > 0)
                    --smileyY;
                break;
            case 's': // ����
                ++smileyY;
                break;
            }

            system("cls");
            DrawSmiley();
            cout.flush();
            Sleep(10);
        }
    }
    return 0;
}

int main() {
    setlocale(0, "");
    system("title ������");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup ���������� � �������: " << iResult << "\n";
        cout << "����������� Winsock.dll ������ � �������!\n";
        return 1;
    }

    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* result = NULL;
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo ���������� � �������: " << iResult << "\n";
        cout << "��������� ������ � ����� ������� ������ c �������!\n";
        WSACleanup();
        return 2;
    }

    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "socket ���������� � �������: " << WSAGetLastError() << "\n";
        cout << "�������� ������ ������ c �������!\n";
        freeaddrinfo(result);
        WSACleanup();
        return 3;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << "bind ���������� � �������: " << WSAGetLastError() << "\n";
        cout << "��������� ������ �� IP-������ ������ � �������!\n";
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 4;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        cout << "listen ���������� � �������: " << WSAGetLastError() << "\n";
        cout << "������������� ���������� �� ������� �� ��������. ���-�� ����� �� ���!\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 5;
    }

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        cout << "accept ���������� � �������: " << WSAGetLastError() << "\n";
        cout << "���������� � ���������� ����������� �� �����������!\n";
        closesocket(ListenSocket);
        WSACleanup();
        return 6;
    }

    DrawSmiley();

    CreateThread(0, 0, Receiver, 0, 0, 0);
    CreateThread(0, 0, Sender, 0, 0, 0);

    Sleep(INFINITE);

    return 0;
}