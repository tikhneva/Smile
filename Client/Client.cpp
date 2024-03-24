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

SOCKET ConnectSocket = INVALID_SOCKET;

const int windowWidth = 100;
const int windowHeight = 100;

int smileyX = 5;
int smileyY = 5;

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

DWORD WINAPI Sender(LPVOID param) {

	char message[2] = " ";

	DrawSmiley();

	while (true) {
		if (_kbhit()) {
			int key = _getch();
			if (key == 224 || key == 0) key = _getch();
			// cout << key << "\n";
			if (key == 72) { // Стрелка вверх
				message[0] = 'w';
				if (smileyY > 0) --smileyY;
			}
			else if (key == 80) { // Стрелка вниз
				message[0] = 's';
				if (smileyY < windowHeight - 1) ++smileyY;
			}
			else  if (key == 77) { // Стрелка вправо
				message[0] = 'd';
				if (smileyX < windowWidth - 3) ++smileyX;
			}
			else if (key == 75) { // Стрелка влево
				message[0] = 'a';
				if (smileyX > 0) --smileyX;
			}

			int iSendResult = send(ConnectSocket, message, 2, 0);

			if (iSendResult == SOCKET_ERROR) {
				cout << "send завершился с ошибкой: " << WSAGetLastError() << "\n";
				cout << "упс, отправка (send) ответного сообщения не состоялась ((\n";
				closesocket(ConnectSocket);
				WSACleanup();
				return 7;
			}

			DrawSmiley();
		}
	}

	return 0;
}

DWORD WINAPI Receiver(LPVOID param) {
	while (true) {
		char message[DEFAULT_BUFLEN];
		int iResult = recv(ConnectSocket, message, DEFAULT_BUFLEN, 0);
		message[iResult] = '\0';

		if (iResult > 0) {
			char direction = message[0];

			switch (direction) {
			case 'a': // Влево
				if (smileyX > 0)
					--smileyX;
				break;
			case 'd': // Вправо
				++smileyX;
				break;
			case 'w': // Вверх
				if (smileyY > 0)
					--smileyY;
				break;
			case 's': // Вниз
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
	system("title КЛИЕНТ");

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup завершился с ошибкой: " << iResult << "\n";
		return 11;
	}

	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	const char* ip = "localhost";
	addrinfo* result = NULL;
	iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

	if (iResult != 0) {
		cout << "getaddrinfo завершился с ошибкой: " << iResult << "\n";
		WSACleanup();
		return 12;
	}

	for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET) {
			cout << "socket завершился с ошибкой: " << WSAGetLastError() << "\n";
			WSACleanup();
			return 13;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		cout << "невозможно подключиться к серверу!\n";
		WSACleanup();
		return 14;
	}

	DrawSmiley();
	// Создаем потоки с использованием CreateThread
	HANDLE senderThreadHandle = CreateThread(0, 0, Sender, 0, 0, 0);
	HANDLE receiverThreadHandle = CreateThread(0, 0, Receiver, 0, 0, 0);

	if (senderThreadHandle == NULL || receiverThreadHandle == NULL) {
		cout << "Ошибка создания потоков.\n";
		closesocket(ConnectSocket);
		WSACleanup();
		return 16;
	}



	// Ожидаем завершения потоков
	WaitForSingleObject(senderThreadHandle, INFINITE);
	WaitForSingleObject(receiverThreadHandle, INFINITE);

	// Закрываем дескрипторы потоков
	CloseHandle(senderThreadHandle);
	CloseHandle(receiverThreadHandle);

	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}