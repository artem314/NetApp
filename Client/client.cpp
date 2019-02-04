#include "stdafx.h"
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
#pragma comment(lib, "Ws2_32.lib")
#define PORT 2500
#define SERVER "127.0.0.1"
//#define SERVER "172.20.10.5"
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
int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	LARGE_INTEGER filesize;
	LONGLONG size;
	FileHeader FH;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in remote_addr;
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(PORT);
	remote_addr.sin_addr.s_addr = inet_addr(SERVER);
	cout << "Сколько файлов вы хотите скачать?" << endl;
	cin >> FH.CountOfFiles;
	FH.Version = 1;
	connect(sock, (sockaddr *)&remote_addr, sizeof(remote_addr));
	sendn(sock, (char *)&FH, sizeof(FH));
	for (int i = 0; i < FH.CountOfFiles; i++)
	{
		GetFileRequest GFRequest;
		string name;
		cout << "Введите имя " << i + 1 << "-го файла:" << endl;
		cin >> name;
		StringCchCopyA(GFRequest.NameFile, _countof(GFRequest.NameFile), name.c_str());
		sendn(sock, (char *)&GFRequest, sizeof(GFRequest));
	}
	for (int i = 0; i < FH.CountOfFiles; i++)
	{
		GetFileResponse GFResponse;
		recvn(sock, (char *)&GFResponse, sizeof(GFResponse));
		filesize.LowPart = GFResponse.fd.nFileSizeLow;
		filesize.HighPart = GFResponse.fd.nFileSizeHigh;
		size = 0;
		char buffer[4096];
		if (GFResponse.fd.nFileSizeLow != 0)
		{
			cout << GFResponse.fd.nFileSizeLow << endl;
			HANDLE hFile = CreateFile(GFResponse.FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile != FALSE)
			{
				int bytes_recv;
				DWORD dwBytesWritten;
				do
				{
					bytes_recv = recv(sock, buffer, filesize.QuadPart, 0);
					WriteFile(hFile, buffer, bytes_recv, &dwBytesWritten, NULL);
					size += bytes_recv;
				} while ((size != filesize.QuadPart) && (bytes_recv != SOCKET_ERROR));
				SetFileTime(hFile, &GFResponse.fd.ftCreationTime, &GFResponse.fd.ftLastAccessTime, &GFResponse.fd.ftLastWriteTime);
				CloseHandle(hFile);
				SetFileAttributes(GFResponse.fd.cFileName, GFResponse.fd.dwFileAttributes);
				cout << "Файл " << GFResponse.FileName << " - скачан!" << endl;
			}
			else cout << "Файл не получен!" << endl;
		}
		else cout << "Файл " << GFResponse.FileName << " не скачан!" << endl;
	}
	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}
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