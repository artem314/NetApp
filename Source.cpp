
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <strsafe.h>
#include <io.h>
#include <iostream>
#include <vector>
#include <string>
#pragma comment(lib, "ws2_32.lib")
//#define PORT 2500
using namespace std;
#pragma pack(1)
struct FileHeader
{
	byte Version;
	int CountOfFiles;
};
struct GetFileRequest {
	char NameFile[255];
};
struct GetFileResponse
{
	char FileName[255];
	WIN32_FIND_DATA fd;
};
#pragma pack()
int sendn(SOCKET s, const char *buf, int len);
int recvn(SOCKET s, char *buf, int len);
int sendn(SOCKET s, const char *buf, int len)
{
	int size = len;
	while (size > 0)
	{
		int n = send(s, buf, size, 0);
		if (n <= 0)
		{
			if (WSAGetLastError() == WSAEINTR)
			{
				continue;
			}
			return SOCKET_ERROR;
		}
		buf += n;
		size -= n;
	}
	return len;
}
int recvn(SOCKET s, char *buf, int len)
{
	int size = len;
	while (size > 0)
	{
		int n = recv(s, buf, size, 0);
		if (n <= 0)
		{
			if (WSAGetLastError() == WSAEINTR)
			{
				continue;
			}
			return SOCKET_ERROR;
		}
		buf += n;
		size -= n;
	}
	return len;
}

int main(int argc, char* argv[])
{
	string PORT;
	setlocale(LC_ALL, "");
	SYSTEMTIME st;
	FileHeader FH;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in local_addr;

	printf("Укажите Порт сервера\n");
	cin >> PORT;

	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(stoi(PORT));
	local_addr.sin_addr.s_addr = INADDR_ANY;

	bind(sock, (sockaddr *)&local_addr, sizeof(local_addr));
	listen(sock, 1);
	SOCKET client_sock;
	sockaddr_in client_addr;
	int client_addr_size = sizeof(client_addr);

	printf("\t\t\t Лог сервера для отправки данных\n\n");

	while (client_sock = accept(sock, (sockaddr *)&client_addr, &client_addr_size))
	{
		int count = 0;

		recvn(client_sock, (char *)&FH, sizeof(FH));

		string *FilesList = new string[FH.CountOfFiles];
	
		if (FH.Version == 1)
		{
			HOSTENT *remotehost;
			remotehost = gethostbyaddr((char *)&client_addr.sin_addr.s_addr, 4, AF_INET);

			GetSystemTime(&st);
			printf(">[%d.%d %d:%d] %s [%s] новое соединение\n", st.wDay, st.wMonth, st.wHour, st.wMinute,
				(remotehost) ? remotehost->h_name : "", inet_ntoa(client_addr.sin_addr));

			for (int i = 0; i < FH.CountOfFiles; i++)
			{
				GetFileRequest GFRequest;
				GetFileResponse GFResponse;

				recvn(client_sock, (char *)&GFRequest, sizeof(GFRequest));

				HANDLE hFind = FindFirstFile(GFRequest.NameFile, &GFResponse.fd);
				if (hFind != INVALID_HANDLE_VALUE)
				{
					FilesList[count] = GFRequest.NameFile;
					count++;
				}
				else
				{
					printf(">Файл - %s запрошен,но не найден\n", GFRequest.NameFile);
				}
			}

			FH.Version = 2;
			FH.CountOfFiles = count;
			sendn(client_sock, (char *)&FH, sizeof(FH));

			for (int i = 0; i < FH.CountOfFiles; i++)
			{
				GetFileResponse GFResponse;
				HANDLE hFind = FindFirstFile(FilesList[i].c_str(), &GFResponse.fd);
				StringCchCopyA(GFResponse.FileName, _countof(GFResponse.FileName), FilesList[i].c_str());

				if (hFind != INVALID_HANDLE_VALUE)
				{
					sendn(client_sock, (char *)&GFResponse, sizeof(GFResponse));
					HANDLE hFile = CreateFile(GFResponse.fd.cFileName, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

					if (GFResponse.fd.nFileSizeLow == 0 && hFile != INVALID_HANDLE_VALUE)
					{
						sendn(client_sock, 0, 0);
						printf(">Файл - %s отправлен\n", GFResponse.FileName);
						continue;
					}
						
					if (hFile != INVALID_HANDLE_VALUE && GFResponse.fd.nFileSizeLow != 0)
					{
						DWORD dwBytesRead;
						ULARGE_INTEGER size = { GFResponse.fd.nFileSizeLow + GFResponse.fd.nFileSizeHigh };
						char *buffer = new char[size.QuadPart];

						if (ReadFile(hFile, buffer, size.QuadPart, &dwBytesRead, NULL))
						{
							printf(">Файл - %s отправлен\n", GFResponse.FileName);
							sendn(client_sock, buffer, dwBytesRead);
						}
						
					}
					CloseHandle(hFile);
				}
				FindClose(hFind);
			}

		}
		else
		{
			printf(">Не подходящая версия.\n");
		}
		
	}
	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}
