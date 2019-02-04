#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h> 
#include <process.h>
#include "resource.h"
#include "psapi.h"
#include "strsafe.h"

#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <list>
using namespace std;

#define MSGLNGTH 5
DWORD IP = 0;
u_short Port/* = 2050*/;
HANDLE htread = INVALID_HANDLE_VALUE;

#pragma pack(1)
struct message_Enter {
	int NumberofMesseges;
	int Index;
	char Name[50];
	char message[MSGLNGTH];
	int LngFragment;
};
#pragma pack()


struct Coords
{
	int X;
	int Y;
};

int X = 20;
int Y = 20;

LPSTR BufferText = NULL;
char UserName[255] = "";
HDC hDC;
MSG msg;
HWND hwnd = NULL;
BOOL bRet;
HWND hDlg = NULL;
HWND hFindDlg = NULL;
HACCEL hAccel = NULL;

WSADATA wsa_data;
SOCKET s;

#define ID_TEXT 2003
#define ID_CHAT 2004
#define ID_SEND 2005
#define ID_PORT 2006
#define ID_CONNECT 2007
#define ID_BROADCAST 2008
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void OnIdle(HWND hwnd);

BOOL PreTranslateMessage(LPMSG lpMsg);
unsigned _stdcall Receive(void * message);
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL Dialog_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

void Dialog_OnClose(HWND hwnd);

void Dialog_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpszCmdLine, int nCmdShow)
{
	if (WSAStartup(0x101, &wsa_data) || wsa_data.wVersion != 0x101)
	{
		MessageBox(NULL, TEXT("Ошибка инициализации"), NULL, MB_OK | MB_ICONERROR);
		//system("pause");
		return -1;
	}

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
	{
		MessageBox(NULL, TEXT("Не удалось создать сокет!"), TEXT("Ошибка"), MB_ICONEXCLAMATION | MB_OK);
	}



	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = TEXT("Windowslass");;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (0 == RegisterClassEx(&wcex)) // регистрирует класс
	{
		MessageBox(NULL, TEXT("Не удалось зарегестрировать класс!"), TEXT("Ошибка"), MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	LoadLibrary(TEXT("ComCtl32.dll"));

	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("Windowslass"), TEXT("Чат"), WS_OVERLAPPEDWINDOW,
		400, 100, 600, 550, NULL, NULL, hInstance, NULL);

	if (NULL == hwnd)
	{
		MessageBox(NULL, TEXT("Не удалось создать окно!"), TEXT("Ошибка"), MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}
	ShowWindow(hwnd, nCmdShow);

	for (;;)
	{
		while (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			//OnIdle(hwnd);
		} // while
		bRet = GetMessage(&msg, NULL, 0, 0);

		if (bRet == -1)
		{
		}
		else if (FALSE == bRet)
		{
			break;
		}
		else if (!PreTranslateMessage(&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CloseHandle(htread);

	return (int)msg.wParam;
}
void OnIdle(HWND hwnd)
{}


BOOL PreTranslateMessage(LPMSG lpMsg)
{
	BOOL bRet = TRUE;

	if (!TranslateAccelerator(hwnd, hAccel, lpMsg))
	{
		bRet = IsDialogMessage(hDlg, lpMsg);

		if (FALSE == bRet)
			bRet = IsDialogMessage(hFindDlg, lpMsg);
	}

	return bRet;
}

unsigned _stdcall Receive(void *message)
{

	SYSTEMTIME st;
	//message_Enter msg;
	Coords XY;
	SOCKADDR_IN recv;
	recv = { 0 };
	recv.sin_family = AF_INET;
	recv.sin_port = htons(Port);
	recv.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(s, (SOCKADDR *)&recv, sizeof(recv));

	list<message_Enter> msgs;

	for (;;)
	{
		int sizeof_recv = sizeof(recv);
		int len = sizeof(recv);

		int n = recvfrom(s, (char *)&XY, sizeof(XY), 0, (SOCKADDR *)&recv, &sizeof_recv);

		if (n != SOCKET_ERROR)
		{
			hDC = GetDC(hwnd);
			HPEN hpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			HGDIOBJ hold = SelectObject(hDC, hpen);

			SetPixel(hDC, XY.X, XY.Y, RGB(0, 0, 0));

			SelectObject(hDC, hold);

			DeleteObject(hpen);

			//if (msg.NumberofMesseges > 1)
			//{

			//	msgs.push_back(msg);

			//	if (msgs.size() == msg.NumberofMesseges)
			//	{
			//		string FullMessage;
			//		FullMessage.resize((msg.NumberofMesseges - 1) * MSGLNGTH + msg.LngFragment);

			//		for (const auto &_msg : msgs)
			//		{
			//			memcpy(&FullMessage[_msg.Index * MSGLNGTH], _msg.message, _msg.LngFragment);
			//		} // for


			//		GetLocalTime(&st);
			//		char TimeUserStamp[150] = "";
			//		HWND hwndCtl = GetDlgItem(hwnd, ID_TEXT);

			//		char UserName[50];
			//		StringCchPrintfA(UserName, _countof(UserName), "%s", msg.Name);

			//		/*StringCchPrintfA(TimeUserStamp, _countof(TimeUserStamp), "%d:%d:%d [%s] > \r", st.wHour, st.wMinute, st.wSecond, UserName);
			//		SendMessageA(hwndCtl, EM_REPLACESEL, TRUE, (LPARAM)(PWSTR)TimeUserStamp);*/

			//		SendMessageA(hwndCtl, EM_REPLACESEL, TRUE, (LPARAM)(PWSTR)FullMessage.c_str());
			//		SendMessageA(hwndCtl, EM_REPLACESEL, TRUE, (LPARAM)(PWSTR)"\r\n");

			//		msgs.clear();
			//	} // if
			//}
			//else
			//{
			//	GetLocalTime(&st);
			//	char TimeUserStamp[150] = "";
			//	HWND hwndCtl = GetDlgItem(hwnd, ID_TEXT);

			//	/*	char UserName[50];
			//		StringCchPrintfA(UserName, _countof(UserName), "%s", msg.Name);

			//		StringCchPrintfA(TimeUserStamp, _countof(TimeUserStamp), "%d:%d:%d [%s] > \r", st.wHour, st.wMinute, st.wSecond, UserName);
			//		SendMessageA(hwndCtl, EM_REPLACESEL, TRUE, (LPARAM)(PWSTR)TimeUserStamp);*/

			//	char buf[MSGLNGTH + 3];

			//	StringCchCopyNA(buf, _countof(buf), msg.message, msg.LngFragment);
			//	StringCchCatA(buf, _countof(buf), "\r\n");

			//	SendMessageA(hwndCtl, EM_REPLACESEL, TRUE, (LPARAM)buf);
			//}
		}
		else
			break;
	}

	return 0;
}

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	RECT rect = { 0 };
	GetWindowRect(hwnd, &rect);
	LONG width = rect.right - rect.left;
	LONG height = rect.bottom - rect.top;

	//HWND hwndCtl = CreateWindowEx(0, TEXT("Edit"), NULL,
	//	WS_CHILD | WS_VISIBLE | WS_VSCROLL |
	//	ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER | ES_READONLY, 10, 10, width - 40, 200, hwnd, (HMENU)ID_TEXT, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Edit"), NULL,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, 10, 220, width - 40, 200, hwnd, (HMENU)ID_CHAT, lpCreateStruct->hInstance, NULL);

	CreateWindowEx(0, TEXT("Button"), TEXT("Отправить"),
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 430, width - 60, 20,
		hwnd, (HMENU)ID_SEND, lpCreateStruct->hInstance, NULL);


	CreateWindowEx(0, TEXT("Button"), TEXT("Широковещательный"),
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		10, 460, 250, 30, hwnd, (HMENU)ID_BROADCAST,
		lpCreateStruct->hInstance, NULL);

	return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int x, y;
	static BOOL mTracking = FALSE;
	switch (uMsg)
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
	case WM_CLOSE:
	{
		closesocket(s);
		WSACleanup();
		DestroyWindow(hwnd);
	}
	break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;

	case WM_MBUTTONUP:
		mTracking = FALSE;
		break;

	case WM_MBUTTONDOWN:
	{
		hDC = GetDC(hwnd);
		mTracking = TRUE;
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		MoveToEx(hDC, x, y, NULL);
	}
	break;

	case WM_MOUSEMOVE:
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		if (mTracking)
		{
			HPEN hpen = CreatePen(PS_SOLID, 1, RGB(0,0,0));
			HGDIOBJ hold = SelectObject(hDC, hpen);

			SetPixel(hDC, x, y, RGB(0, 0, 0));
			SelectObject(hDC, hold);
			DeleteObject(hpen);
		}
	}
	return 0;

	case WM_KEYDOWN:
	{


		SOCKADDR_IN sa;
		sa = { 0 };
		string IPBROADCAST = "255.255.255.255";
		if (IsDlgButtonChecked(hwnd, ID_BROADCAST) == BST_CHECKED) // флажок установлен
		{
			BOOL optval;			int optlen = sizeof(optval);

			int err = getsockopt(s, SOL_SOCKET, SO_BROADCAST,
				(char *)&optval, &optlen);

			if ((SOCKET_ERROR != err) && (TRUE != optval))
			{
				optval = TRUE;
				optlen = sizeof(optval);
				err = setsockopt(s, SOL_SOCKET, SO_BROADCAST,
					(char *)&optval, optlen);

				if (SOCKET_ERROR == err)
				{
					MessageBox(NULL, TEXT("Ошибка включения широковещательной передачи"), NULL, MB_OK | MB_ICONERROR);
				}
			} // if
		} // if

		sa.sin_family = AF_INET;
		sa.sin_port = htons(Port);

		if (IsDlgButtonChecked(hwnd, ID_BROADCAST) == BST_CHECKED)
		{
			sa.sin_addr.S_un.S_addr = INADDR_BROADCAST;
		}
		else
		{
			if (IP != 0)
			{
				sa.sin_addr.S_un.S_addr = htonl(IP);
			}
			else
			{
				sa.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
			}
		}

		switch (wParam)
		{
		case VK_LEFT:
		{
			Coords XY = { 0 };
			X--;
			XY.Y = Y;
			XY.X = X;
			sendto(s, (const char*)&XY, sizeof(XY), 0, (struct sockaddr *)&sa, sizeof(sa));

		}
		break;
		case VK_RIGHT:
		{
			Coords XY = { 0 };
			X++;
			XY.Y = Y;
			XY.X = X;
			sendto(s, (const char*)&XY, sizeof(XY), 0, (struct sockaddr *)&sa, sizeof(sa));
		}
		break;
		case VK_DOWN:
		{
			Coords XY = { 0 };
			Y++;
			XY.Y = Y;
			XY.X = X;
			sendto(s, (const char*)&XY, sizeof(XY), 0, (struct sockaddr *)&sa, sizeof(sa));
		}
		break;

		case VK_UP:
		{
			Coords XY = { 0 };
			Y--;
			XY.Y = Y;
			XY.X = X;
			sendto(s, (const char*)&XY, sizeof(XY), 0, (struct sockaddr *)&sa, sizeof(sa));
		}
		break;

		}//switch (wParam)
	}
	default:
		break;

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	HINSTANCE hInstance = GetWindowInstance(hwnd);
	switch (id)
	{
	case ID_SEND:
	{
		message_Enter msg;
		SOCKADDR_IN sa;
		sa = { 0 };
		string IPBROADCAST = "255.255.255.255";
		if (IsDlgButtonChecked(hwnd, ID_BROADCAST) == BST_CHECKED) // флажок установлен
		{
			BOOL optval;			int optlen = sizeof(optval);

			int err = getsockopt(s, SOL_SOCKET, SO_BROADCAST,
				(char *)&optval, &optlen);

			if ((SOCKET_ERROR != err) && (TRUE != optval))
			{
				optval = TRUE;
				optlen = sizeof(optval);
				err = setsockopt(s, SOL_SOCKET, SO_BROADCAST,
					(char *)&optval, optlen);

				if (SOCKET_ERROR == err)
				{
					MessageBox(NULL, TEXT("Ошибка включения широковещательной передачи"), NULL, MB_OK | MB_ICONERROR);
				}
			} // if
		} // if

		sa.sin_family = AF_INET;
		sa.sin_port = htons(Port);

		if (IsDlgButtonChecked(hwnd, ID_BROADCAST) == BST_CHECKED)
		{
			sa.sin_addr.S_un.S_addr = INADDR_BROADCAST;
		}
		else
		{
			if (IP != 0)
			{
				sa.sin_addr.S_un.S_addr = htonl(IP);
			}
			else
			{
				sa.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
			}
		}

		HWND hwndctl = GetDlgItem(hwnd, ID_CHAT);
		int length = SendMessageA(hwndctl, WM_GETTEXTLENGTH, 0, 0);
		LPSTR LpMessage = (LPSTR)malloc(length + 1);
		SendMessageA(hwndctl, WM_GETTEXT, length + 1, (LPARAM)(LpMessage));

		int NumberOfMessages = (int)ceilf((float)length / (float)MSGLNGTH); //делить на длину сообщения
		//в комментариях пояснение предыдущего
		int LastLength = abs(length - (MSGLNGTH * NumberOfMessages - 1)); //было без -1

		vector<int> slist(NumberOfMessages);
		iota(slist.begin(), slist.end(), 0);

		std::random_device rd;
		std::mt19937 g(rd());

		std::shuffle(slist.begin(), slist.end(), g);

		msg.NumberofMesseges = NumberOfMessages;

		if (UserName[0] == '\0')
			StringCchPrintfA(UserName, _countof(UserName), "Default Name");

		StringCchPrintfA(msg.Name, _countof(msg.Name), UserName);

		for (size_t i = 0; i < NumberOfMessages; i++)
		{

			msg.Index = slist[i];
			msg.LngFragment = ((msg.Index <= (NumberOfMessages - 1)) || (0 == LastLength)) ? MSGLNGTH : LastLength; //было <=

			memcpy(msg.message, &LpMessage[msg.Index * MSGLNGTH], msg.LngFragment);

			sendto(s, (const char*)&msg, sizeof(msg), 0, (struct sockaddr *)&sa, sizeof(sa));
		}
		//}

		SetDlgItemText(hwnd, ID_CHAT, NULL);

	}
	break;
	case ID_40003:
	{
		if (IsWindow(hDlg) == FALSE)
		{
			hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, DialogProc);
			ShowWindow(hDlg, SW_SHOW);

			hDlg = FALSE;
		}
	}
	break;

	
	}//switch
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		BOOL bRet = HANDLE_WM_INITDIALOG(hwndDlg, wParam, lParam, Dialog_OnInitDialog);
		return SetDlgMsgResult(hwndDlg, uMsg, bRet);
	}
	case WM_CLOSE:
		HANDLE_WM_CLOSE(hwndDlg, wParam, lParam, Dialog_OnClose);
		return TRUE;
	case WM_COMMAND:
		HANDLE_WM_COMMAND(hwndDlg, wParam, lParam, Dialog_OnCommand);
		return TRUE;
	} // switch

	return FALSE;
}

BOOL Dialog_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	if (htread != INVALID_HANDLE_VALUE)
	{
		CloseHandle(htread);
		htread = INVALID_HANDLE_VALUE;
	}
	return TRUE;
}

void Dialog_OnClose(HWND hwnd)
{
	if (hwnd == hDlg)
	{
		// уничтожаем немодальное диалоговое окно
		DestroyWindow(hwnd);
	}
	else
	{
		// завершаем работу модального диалогового окна
		EndDialog(hwnd, IDCLOSE);
	}
}

void Dialog_OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDOK:
	{
		HWND hwndctl = GetDlgItem(hWnd, IDC_IPADDRESS1);
		int iCount = (int)SendMessageA(hwndctl, IPM_GETADDRESS, 0, (LPARAM)&IP);
		GetDlgItemTextA(hWnd, IDC_EDIT1, UserName, _countof(UserName));
		EndDialog(hWnd, IDOK);

		Port = GetDlgItemInt(hWnd, IDC_EDIT2, FALSE, FALSE);
		Port = Port;

		if (htread == INVALID_HANDLE_VALUE)
		{
			htread = (HANDLE)_beginthreadex(NULL, 0, Receive, NULL, 0, NULL);
		}

	} // if 
	break;

	case IDCANCEL: // нажата кнопка "Отмена"
		if (hWnd == hDlg)
		{
			// уничтожает немодальное диалоговое окно
			DestroyWindow(hWnd);
		} // if
		else
		{
			// завершает работу модального диалогового окна
			EndDialog(hWnd, IDCANCEL);
		} // else
		break;
	} // switch
} // Dialog_OnCommand