#include <windowsx.h>
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "Shlwapi.h"
#include <stdio.h>
#include <stdlib.h>
using namespace std;

const TCHAR* clsName = _T("tic-tac-toe");
const TCHAR* rpName = _T("rp"); //указатель на имя отображения
int N = 4; //размерность сетки (N-1)*(N-1) ]
char map[3][3];
int height;//высота окна в пикселях
int weight;//ширина 
int Red, Gr, Bl; //цвета фона
int R, G, B;    //цвета сетки
double rast = 0.1;
int** mas;
int all_yach = 0, stop = 0;
char* MasMap;
int* kresnol;
PCWSTR PATH = L"C:\\Users\\user\\Desktop\\test.txt"; //64-bit code segment
HANDLE Doc, MapDoc, semaphore1, semaphore_finaly;
LPSTR buffer = (CHAR*)calloc(360, sizeof(CHAR)); //содержимое файла
DWORD iNumRead = 0, iNumWrite = 0; //перемещаемся по файлу
OVERLAPPED olf = { 0 }; //структура, в которой задана позиция в файле
LARGE_INTEGER li = { 0 };
HWND hwnd;
HBRUSH hBrush;    // текущий цвет фона  
HPEN Pen; // цвет для сетки
HPEN Pen_krest = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));  //кисть крестика крестика
HPEN Pen_okr = CreatePen(PS_SOLID, 3, RGB(10, 10, 10));//кисть окружности окружности
int height1, weight1;//высота и ширина рабочего окна
int height_yach, weight_yach;
RECT rect; //в данной структуре есть координаты правой, левой, верхней и нижней границ
HDC hdc; //текущая позиции
int x, y;
int number_player;
string fale_data;
UINT mes, mes1, gameover;
HANDLE hThread_color;


//запускает блокнот
void RunNotepad(void)
{
	STARTUPINFO sInfo;
	PROCESS_INFORMATION pInfo;

	ZeroMemory(&sInfo, sizeof(STARTUPINFO));

	puts("Starting Notepad...");
	CreateProcess(_T("C:\\Windows\\Notepad.exe"),
		NULL, NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo);
}

BOOL checkLanes(char symb) 
{
	bool cols, rows;
	for (int col = 0; col < (N - 1); col++) {
		cols = true;
		rows = true;
		for (int row = 0; row < (N - 1); row++) {
			cols &= (map[col][row] == symb);
			rows &= (map[row][col] == symb);
		}
		if (cols || rows) return true;
	}

	return false;
}

BOOL checkDiagonal(char symb) 
{
	bool toright, toleft;
	toright = true;
	toleft = true;
	for (int i = 0; i < (N-1); i++) {
		toright &= (map[i][i] == symb);
		toleft &= (map[(N - 1) - i - 1][i] == symb);
	}

	if (toright || toleft) return true;

	return false;
}

BOOL Draw_Line(int x1, int y1, int x2, int y2)
{
	MoveToEx(hdc, x1, y1, NULL); //обновляаем позиция до новой текущей точки
	return LineTo(hdc, x2, y2); //рисуем прямую линию до текущей точки
}

void Grid() {
	SelectObject(hdc, Pen); //используемый объект
	for (int i = 0; i <= weight1; i = i + weight_yach)
		Draw_Line(i, 0, i, height1);

	for (int i = 0; i <= height1; i = i + height_yach)
		Draw_Line(0, i, weight1, i);
	DeleteObject(Pen);
}

void Circle()
{

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{

			if (MasMap[N * i + j] == 1)
			{
				SelectObject(hdc, hBrush); //внутри окружности должен быть такой же цвет
				SelectObject(hdc, Pen_okr);
				Ellipse(hdc, i * weight_yach + weight_yach * rast, j * height_yach + height_yach * rast, (i + 1) * weight_yach - weight_yach * rast, (j + 1) * height_yach - height_yach * rast);
				//i-показывает на какое количество ячеек мы должны передвинуться по ширине по линиям сетки
				//j-на какое количесво ячеек должны подняться вверх по линиям сетки
				//нужна координата 

			}
		}
}

void X()
{

	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			if (MasMap[N * i + j] == 2)
			{
				SelectObject(hdc, Pen_krest);
				Draw_Line(i * weight_yach + weight_yach * rast, j * height_yach + height_yach * rast, (i + 1) * weight_yach - weight_yach * rast, (j + 1) * height_yach - height_yach * rast);
				Draw_Line((i + 1) * weight_yach - weight_yach * rast, j * height_yach + height_yach * rast, i * weight_yach + weight_yach * rast, (j + 1) * height_yach - height_yach * rast);
			}
}

DWORD WINAPI Thread_color(LPVOID)
{
	while (true) //пока поток не приостановили
	{
		HBRUSH hBrush1 = hBrush;
		Bl = 255;
		if (Gr < 254) Gr += 1;
		else Gr = 0;
		if (Red < 254) Red += 1;
		else Red = 0;
		hBrush = CreateSolidBrush(RGB(Red, Gr, Bl));
		hBrush1 = (HBRUSH)SetClassLong(hwnd, GCL_HBRBACKGROUND, (LONG)hBrush);
		RedrawWindow(hwnd, 0, NULL, RDW_ERASE | RDW_INVALIDATE);
		DeleteObject(hBrush1);
		Sleep(80);//чтобы не схватить эпилепсию
	}
	return 0;
}


//функция, которая обрабатывает сообщения от пользователя
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == gameover) //если поступило сообщение о завершении игры, то зыкрываем окна
		PostMessage(hwnd, WM_DESTROY, 0, 0);
	if (message == mes)
	{
		all_yach = lParam;
		InvalidateRect(hwnd, NULL, TRUE);
		//RedrawWindow(hwnd, 0, NULL, RDW_ERASE | RDW_INVALIDATE);//обновляем окно для поступившего сообщения об изменениях
	}
	switch (message)                 //обработка сообщений
	{
	case WM_DESTROY:
		TerminateThread(hThread_color, 0); //если закрываем окна, то заканчиваем работу с потоком
		Doc = CreateFile(PATH, GENERIC_WRITE, FILE_SHARE_READ, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		GetWindowRect(hwnd, &rect);
		fale_data = to_string(N) + " " + to_string(rect.bottom - rect.top) + " " + to_string(rect.right - rect.left) + " " + to_string(Red) + " " + to_string(Gr) + " " + to_string(Bl) + " " + to_string(R) + " " + to_string(G) + " " + to_string(B);
		WriteFile(Doc, fale_data.c_str(), fale_data.size(), &iNumWrite, &olf);
		CloseHandle(Doc);
		PostQuitMessage(0);       //если получили сообщение о закрытии окна, добавляем в очередь это сообщение
		return 0;
	case WM_KEYDOWN:    //обрабатываем сообщение нажатия клавиш
		switch (wParam) //wParam- содержит код клавиши, которая была нажата
		{
		case 0x43:
			if (GetAsyncKeyState(VK_SHIFT)) //если зажаты клавиши с+shift открываем блокнот
				RunNotepad();
			break;
		case 0x51:
			if (GetAsyncKeyState(VK_CONTROL)) // если зажаты клавиши Q+ctrl
				PostQuitMessage(0);
			return 0;
		case VK_ESCAPE: //если нажали клавшу esc
			PostQuitMessage(0);//закрытие окна
			return 0;
			  case VK_RETURN: //нажали клавишу enter- сменили цвет на рандомный
				  Red = rand() % 256;
				  Gr = rand() % 256;
				  Bl = rand() % 256;
				  hBrush = CreateSolidBrush(RGB(Red, Gr, Bl));//меняем цвет на рандомный
				  HANDLE d;
				  d = (HANDLE)SetClassLong(hwnd, GCL_HBRBACKGROUND, (LONG)hBrush); // смена цвета фона
				  DeleteObject(d);
				  RedrawWindow(hwnd, 0, NULL, RDW_ERASE | RDW_INVALIDATE); //обновление окна
				  break;
		//case 0x31: //нажимаем клавишу 1 Работает, но надо гонять через аиду
		//{
		//	cout << "1" << endl;
		//	SetThreadPriority(hThread_color, THREAD_PRIORITY_IDLE);
		//	// "Фоновый приоритет!"
		//	return 0;
		//}
		//case 0x32: //нажимаем клавишу 2
		//{
		//	cout << "2" << endl;
		//	SetThreadPriority(hThread_color, THREAD_PRIORITY_LOWEST);
		//	//"Наинизший приоритет!" 
		//	return 0;
		//}
		//case 0x33: //нажимаем клавишу 3
		//{
		//	cout << "3" << endl;
		//	SetThreadPriority(hThread_color, THREAD_PRIORITY_BELOW_NORMAL);
		//	// "Приоритет ниже среднего!" 
		//	return 0;
		//}
		//case 0x34: //нажимаем клавишу 4
		//{
		//	cout << "4" << endl;
		//	SetThreadPriority(hThread_color, THREAD_PRIORITY_NORMAL);
		//	// "Нормальный приоритет!" 
		//	return 0;
		//}
		//case 0x35://нажимаем клавишу 5
		//{
		//	cout << "5" << endl;
		//	SetThreadPriority(hThread_color, THREAD_PRIORITY_ABOVE_NORMAL);
		//	// "Приоритет выше среднего!"
		//	return 0;
		//}
		//case 0x36://нажимаем клавишу 6
		//{
		//	cout << "6" << endl;
		//	SetThreadPriority(hThread_color, THREAD_PRIORITY_HIGHEST);
		//	//"Наивысший приоритет!" 
		//	return 0;
		//}
		//case 0x37: //нажимаем клавишу 7
		//{
		//	cout << "7" << endl;
		//	SetThreadPriority(hThread_color, THREAD_PRIORITY_TIME_CRITICAL);
		//	// "Приоритет реального времени!" 
		//	return 0;
		//}
		case VK_SPACE:
			if (stop == 0) //изначально stop=0
			{
				SuspendThread(hThread_color); //приостановить работу потока
				stop++;
			}
			else
			{
				ResumeThread(hThread_color); //возобновить работу потока
				stop--;
			}
		}
		return 0;
	case WM_LBUTTONDOWN: //получили сообщение о нажатии левой кнопки мыши - рисуем круг
		if (all_yach % 2 != (number_player % 2 && kresnol[number_player - 1]) == 0) 
		{
			x = GET_X_LPARAM(lParam); //макрос, позволяющий получить координаты точки
			y = GET_Y_LPARAM(lParam);

			if (MasMap[N * (x / weight_yach) + y / height_yach] == 0)
			{
				MasMap[N * (x / weight_yach) + y / height_yach] = 1;
				all_yach++;
				map[(x / weight_yach)][(y / height_yach)] =  'O';
			}
			PostMessage(HWND_BROADCAST, mes, NULL, all_yach);//отправляет указанное сообщение в окно или окна
			InvalidateRect(hwnd, NULL, TRUE);
			if (checkDiagonal('O') || checkLanes('O'))
			{
				MessageBox(hwnd, _T("Circles won"), _T("GAMEOVER"), MB_OK);
				return 0;
			}
			else if (all_yach == (N - 1) * (N - 1)) MessageBox(hwnd, _T("DRAW"), _T("GAMEOVER"), MB_OK);
		}
		else MessageBox(hwnd, _T("It's not your turn!"), _T("Error"), MB_OK);
		return 0;
	case WM_RBUTTONDOWN://получили сообщение о нажатии правой кнопки мыши - рисуем крестик
		if (all_yach % 2 != number_player % 2 && kresnol[number_player - 1] == 1)
		{
			x = GET_X_LPARAM(lParam); //макрос, позволяющий получить координаты точки
			y = GET_Y_LPARAM(lParam);
			if (MasMap[N * (x / weight_yach) + y / height_yach] == 0)
			{
				MasMap[N * (x / weight_yach) + y / height_yach] = 2;
				all_yach++;
				map[(x / weight_yach)][(y / height_yach)] = 'X';
			}
			PostMessage(HWND_BROADCAST, mes, NULL, all_yach);//отправляет указанное сообщение в окно или окна
			InvalidateRect(hwnd, NULL, TRUE);
			if (checkDiagonal('X') || checkLanes('X'))
			{
				MessageBox(hwnd, _T("X won"), _T("GAMEOVER"), MB_OK);
				return 0;
			}
			else if (all_yach == (N - 1) * (N - 1)) MessageBox(hwnd, _T("DRAW"), _T("GAMEOVER"), MB_OK);
		}
		else MessageBox(hwnd, _T("It's not your turn!"), _T("Error"), MB_OK);
		return 0;

	case WM_MOUSEWHEEL:
		R = 255;
		if (G < 254) G += 2;
		else G = 30;
		if (B < 254) B += 2;
		else B = 30;
		//RedrawWindow(hwnd, 0, NULL, RDW_ERASE | RDW_INVALIDATE);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	case WM_SIZE:
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
		//RedrawWindow(hwnd, 0, NULL, RDW_ERASE | RDW_INVALIDATE);
	case WM_PAINT: //рисуем в рабочей области
	{
		PAINTSTRUCT paint;
		Pen = CreatePen(PS_SOLID, 3, RGB(R, G, B)); //создаем кисть для сетки с указанным стилем, шириной и цветом
		hdc = BeginPaint(hwnd, &paint); //Подготавливает окно к pаскpаске в ответ на сообщение WM_PAINT
		GetClientRect(hwnd, &rect);//получаем размер рaбочего окна
		weight1 = rect.right - rect.left;
		height1 = rect.bottom - rect.top;
		weight_yach = weight1 / (N - 1);// ширина ячейки с учетом высоты и длины рабочего окна
		height_yach = height1 / (N - 1); //высота ячейки
		Grid();
		Circle();
		X();
		EndPaint(hwnd, &paint);
		return 0;
	}
	}
	// for messages that we don't deal with 
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int main(int argc, char** argv)
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	//Семафор для сигнализации освобождения или захвата ресурса между окнами, в нашем случае очередность хода
	semaphore1 = CreateSemaphore(NULL, 2, 2, _T("Sem"));//для работы с 2 окнамиС
	if (WaitForSingleObject(semaphore1, 0) == WAIT_TIMEOUT)
	{
		MessageBox(hwnd, L"Only 2 players", L"Warning", MB_OK);
		return 0;
	}
	WIN32_FIND_DATA wfd;
	Doc = FindFirstFile(PATH, &wfd); //первое открытие файла, в случае неудачи вернет INVALID...
	li.QuadPart = 0;
	olf.Offset = li.LowPart;//младший разряд того, куда нужно поставить курсор
	olf.OffsetHigh = li.HighPart;//старший разряд


	if (Doc == INVALID_HANDLE_VALUE)//если файл не открылся
	{
		Doc = CreateFile(PATH, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		LPCSTR c_str = "4 320 240 0 0 255 255 0 0";
		WriteFile(Doc, c_str, strlen(c_str), &iNumWrite, &olf);
		weight = 320; height = 240; Red = 0; Gr = 0; Bl = 255; R = 255; G = 0; B = 0;
	}
	else {
		Doc = CreateFile(PATH,
			GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		ReadFile(Doc, buffer, 360, &iNumRead, &olf);
		string s = buffer;
		sscanf_s(buffer, "%d %d %d %d %d %d %d %d %d", &N, &weight, &height, &Red, &Gr, &Bl, &R, &G, &B);//преобразуем в числa;
	}
	CloseHandle(Doc);
	if (argc > 1)  //если передаем информацию через командую строку, то количество передаваемых параметров будет больше 1
		N = atoi(argv[1]); //преобразуем в число

	//разделенный участок памяти
	MapDoc = NULL;
	MapDoc = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, rpName);
	HANDLE players = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 2 * sizeof(int), _T("pl"));
	kresnol = (int*)MapViewOfFile(players, FILE_MAP_ALL_ACCESS, 0, 0, 2 * sizeof(int));
	if (MapDoc == NULL)
	{
		MapDoc = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, N * N, rpName); //создаёт или открывает отображение файла на память
		MasMap = (char*)MapViewOfFile(MapDoc, FILE_MAP_ALL_ACCESS, NULL, NULL, N * N); //создается окно просмотра, с которым можно обращаться, как с линейным массивом байтов
		for (int i = 0; i < N * N; i++) MasMap[i] = 0;
		number_player = 1;
		kresnol[0] = 1;
	}
	else
	{
		MasMap = (char*)MapViewOfFile(MapDoc, FILE_MAP_ALL_ACCESS, NULL, NULL, N * N);
		if (kresnol[0] == 0) number_player = 1;
		else number_player = 2;
		kresnol[number_player - 1] = 3 - kresnol[2 - number_player];
	}

	mes = RegisterWindowMessage(L"WM_USER11");//регистрируем новый тип сообщения
	gameover = RegisterWindowMessage(L"WM_USER12");
	//WM_USER1-для коммуникации окон в одном приложении

	BOOL bMessageOk;
	MSG message;            //Здесь сохраняется сообщение для приложения 
	WNDCLASS wincl = { 0 };         //Структура данных для оконного класса


	int nCmdShow = SW_SHOW;
	//для получения ручки
	HINSTANCE hThisInstance = GetModuleHandle(NULL);

	//характеристики окна
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = clsName;
	wincl.lpfnWndProc = WindowProcedure;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	hBrush = CreateSolidBrush(RGB(Red, Gr, Bl));
	wincl.hbrBackground = hBrush;//закрашиваем фон

	//зарегистрируйте класс окна, и если он не сможет выйти из программы
	if (!RegisterClass(&wincl))
		return 0;

	//создаем окно
	hwnd = CreateWindow(
		clsName,          //имя класса
		clsName,           //текст заголовка
		WS_OVERLAPPEDWINDOW, //окно по умолчанию 
		CW_USEDEFAULT,       // Windows решает позицию 
		CW_USEDEFAULT,       //где окно заканчивается на экране
		height,                 //высота в пикселях
		weight,                 //ширина
		HWND_DESKTOP,        //окно является дочерним окном рабочего стола
		NULL,               //нет меню 
		hThisInstance,
		NULL                //нет данных о создании окна
	);

	//сделать окно видимым на экране
	ShowWindow(hwnd, nCmdShow);
	hThread_color = CreateThread(NULL, 0, Thread_color, NULL, 0, NULL);//создаем поток отрисовки поля
	//Thread_color-адрес функции, которое будет выполняться потоком

	BOOL bOk;
	MSG msg;
	//запустите цикл сообщений. Он будет выполняться до тех пор, пока GetMessage() не вернет 0
	while ((bOk = GetMessage(&message, NULL, 0, 0)) != 0)
	{
		if (bOk < 0)
			cout << GetLastError() << endl;

		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	//kresnol[number_player - 1] = 0;
	ReleaseSemaphore(semaphore1, 1, NULL);//когда поток завершает использование ресурса, вызываем функция,для увеличения счетчика
	DestroyWindow(hwnd);
	CloseHandle(hThread_color);
	CloseHandle(semaphore1);
	UnregisterClass(clsName, hThisInstance);
	DeleteObject(Pen_okr);
	DeleteObject(hBrush);
	return 0;
}

