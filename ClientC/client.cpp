#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <Process.h>
#include <string>
#include <WS2tcpip.h>
#include <winsock.h>
#include <stdio.h>
#include <conio.h>
#include <strsafe.h>
#include <io.h>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")
//#define PORT 2500
//#define SERVER "127.0.0.1"
//#define SERVER "172.20.10.2"
//#define SERVER "192.168.0.36"
//#define SERVER "192.168.43.94"
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
	setlocale(LC_ALL, "");
	string SERVER, PORT;
	LARGE_INTEGER filesize;
	FileHeader FH;
	FileHeader FHAnswer;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in remote_addr;

	printf("Укажите IP-адрес сервера\n");
	cin >> SERVER;
	printf("Укажите Порт сервера\n");
	cin >> PORT;
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(stoi(PORT));
	remote_addr.sin_addr.s_addr = inet_addr(SERVER.c_str()); 
	/*remote_addr.sin_port = htons(PORT);
	remote_addr.sin_addr.s_addr = inet_addr(SERVER);*/

	printf(">Укажите количество файлов\n");

	scanf("%d", &FH.CountOfFiles);

	FH.Version = 1;

	vector<string> FilesList(FH.CountOfFiles);
	connect(sock, (sockaddr *)&remote_addr, sizeof(remote_addr));
	sendn(sock, (char *)&FH, sizeof(FH));

	for (int i = 0; i < FH.CountOfFiles; i++)
	{

		string name;
		printf(">Введите название %d -го файла\n", i + 1);
		cin >> name;
		FilesList[i] = name;
	}
	for (int i = 0; i < FH.CountOfFiles; i++)
	{
		GetFileRequest GFRequest;
		StringCchCopyA(GFRequest.NameFile, _countof(GFRequest.NameFile), FilesList[i].c_str());
		sendn(sock, (char *)&GFRequest, sizeof(GFRequest));
	}

	recvn(sock, (char *)&FHAnswer, sizeof(FHAnswer));
	
	if (FHAnswer.Version == 2)
	{
		printf(">Будет получено файлов: %d\n", FHAnswer.CountOfFiles);

		for (int i = 0; i < FHAnswer.CountOfFiles; i++)
		{
			GetFileResponse GFResponse;
			recvn(sock, (char *)&GFResponse, sizeof(GFResponse));
			filesize.LowPart = GFResponse.fd.nFileSizeLow;
			filesize.HighPart = GFResponse.fd.nFileSizeHigh;

			ULARGE_INTEGER size = { GFResponse.fd.nFileSizeLow + GFResponse.fd.nFileSizeHigh };
			char *buffer = new char[size.QuadPart];

			if (GFResponse.fd.nFileSizeLow != 0)
			{
				HANDLE hFile = CreateFile(GFResponse.FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (hFile != FALSE)
				{
					int bytes_recv;
					DWORD dwBytesWritten;
					bytes_recv = recvn(sock, buffer, size.QuadPart);
					WriteFile(hFile, buffer, bytes_recv, &dwBytesWritten, NULL);
					SetFileTime(hFile, &GFResponse.fd.ftCreationTime, &GFResponse.fd.ftLastAccessTime, &GFResponse.fd.ftLastWriteTime);
					CloseHandle(hFile);
					SetFileAttributes(GFResponse.fd.cFileName, GFResponse.fd.dwFileAttributes);
					printf(">Файл %s скачан\n", GFResponse.FileName);
					printf(">Размер файла: %d Байт\n", size.QuadPart);
			
				}
				else 
					printf(">Файл %s не получен\n", GFResponse.FileName);
			}
			else
			{
				HANDLE hFile = CreateFile(GFResponse.FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (hFile != FALSE)
				{
					SetFileTime(hFile, &GFResponse.fd.ftCreationTime, &GFResponse.fd.ftLastAccessTime, &GFResponse.fd.ftLastWriteTime);
					CloseHandle(hFile);
					SetFileAttributes(GFResponse.fd.cFileName, GFResponse.fd.dwFileAttributes);
					printf(">Файл %s скачан\n", GFResponse.FileName);
				}
				else
				{
					printf(">Файл %s не получен\n", GFResponse.FileName);
				}
			} 

			delete[] buffer;
		}
	}
	else
	{
		printf(">Ошбика на стороне сервера\n");
	}
		
	closesocket(sock);
	
	WSACleanup();
	system("pause");
	return 0;
}
