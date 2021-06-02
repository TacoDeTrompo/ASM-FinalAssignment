#define WIN32_LEAN_AND_MEAN //No agrega librerías que no se vayan a utilizar

#include <Windows.h>
#include <stdio.h>
#include <dinput.h>
#include <gdiplus.h>
#include <gl/gl.h>
#include <gl/glu.h>

using namespace Gdiplus;

//Variables constantes
const int ANCHO_VENTANA = 800;
const int ALTO_VENTANA = 600;
const int BPP = 4;
const int TICK = 15;
const unsigned int BLUE = 0xFF0000FF;
const unsigned int GREEN = 0xFF00FF00;
const unsigned int RED = 0xFFFF0000;
unsigned char * ptrImagen;
unsigned char * ptrFigura;
unsigned char * ptrTiles;
unsigned int ancho, alto;
bool movin = false;
int movi = 3;
int koli = 0;
bool ladin = true;
bool sen = false;
bool jump = false;
int jumanim = 105;
int korv = 0;
unsigned char redton = 0, blueton = 0, greenton = 0;

struct POSITION {
	int X;
	int Y;
};

struct DIMENSION {
	int ANCHO;
	int ALTO;
};

struct Input
{
	enum Keys
	{
		Backspace = 0x08, Tab,
		Clear = 0x0C, Enter,
		Shift = 0x10, Control, Alt,
		Escape = 0x1B,
		Space = 0x20, PageUp, PageDown, End, Home, Left, Up, Right, Down,
		Zero = 0x30, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
		A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		NumPad0 = 0x60, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7, NumPad8, NumPad9, 
		F1 = 0x70, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
	};
}input;

//Variables Globales
int *ptrBuffer;
unsigned int anchoBmp;
unsigned int altoBmp;
DIMENSION dmnFigura;
POSITION posFigura;
bool KEYS[256];

//Definicion de funciones
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void LimpiarFondo(int *ptrBuffer, unsigned int color, int x, int y);
void DibujaTiles(int *ptrBuffer, unsigned int color, int x, int y);
void DibujaFigura(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, DIMENSION dimension, POSITION position, int move);
void DibujaFiguramirror(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, DIMENSION dimension, POSITION position, int move);
void DibujaFigurajump(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, DIMENSION dimension, POSITION position, int jumpl);
void DibujaFigurajumpmirror(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, DIMENSION dimension, POSITION position, int jumpl);
void DibujaBloque(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, int ancho, int alto, int x, int y, unsigned char red, unsigned char green, unsigned char blue);
int jumpy(int, int, bool);
void MainRender(HWND hWnd);
void Init();
void KeysEvents();
void CargaImagen(WCHAR rutaImagen[]);

int WINAPI wWinMain(HINSTANCE hInstance, 
					 HINSTANCE hPrevInstance, 
					 PWSTR pCmdLine, 
					 int nCmdShow)
{
	WNDCLASSEX wc;									// Windows Class Structure
	HWND hWnd;
	MSG msg;

	TCHAR szAppName[] = TEXT("MyWinAPIApp");		
	TCHAR szAppTitle[] = TEXT("LENS App");

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance	
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= (HBRUSH) (COLOR_WINDOW + 1);			// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu	
	wc.lpszClassName	= szAppName;							// Set The Class Name
	wc.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);
	
	if (!RegisterClassEx(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,
			L"Fallo al registrar clase (Failed To Register The Window Class).",
			L"ERROR",
			MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	hWnd = CreateWindowEx(	
		WS_EX_CLIENTEDGE | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,	// Extended Style For The Window
		szAppName,							// Class Name
		szAppTitle,							// Window Title
		WS_OVERLAPPEDWINDOW |				// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		ANCHO_VENTANA,						// Calculate Window Width
		ALTO_VENTANA,						// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL);								// Pass this class To WM_CREATE								

	if(hWnd == NULL) {
		MessageBox(NULL, 
			L"Error al crear ventana (Window Creation Error).", 
			L"ERROR", 
			MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}
		
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	Init();
	ShowWindow(hWnd, nCmdShow);
	SetFocus(hWnd);

	SetTimer(hWnd, TICK, TICK, NULL);
	ZeroMemory(&msg, sizeof(MSG));

	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return(int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)									// Check For Windows Messages
	{
		case WM_TIMER:
			if(wParam == TICK)
			{
				MainRender(hWnd);
			}
			break;
		case WM_PAINT:
			{
				HDC hdc; 
				PAINTSTRUCT ps;
				hdc = BeginPaint(hWnd, &ps);
				// TODO: agregar código de dibujo aquí...

				BITMAP bm;
				HBITMAP h_CMC = CreateBitmap(ANCHO_VENTANA, ALTO_VENTANA, 1, 32, ptrBuffer);
				HDC hdcMem = CreateCompatibleDC(hdc);
				HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, h_CMC);
				GetObject(h_CMC, sizeof(bm), &bm);

				BitBlt(hdc, 0, 0, ANCHO_VENTANA, ALTO_VENTANA, hdcMem, 0, 0, SRCCOPY);

				DeleteObject(h_CMC);
				SelectObject(hdcMem, hbmOld);
				DeleteDC(hdcMem);
				DeleteObject(hbmOld);
			}
			break;		
		case WM_KEYDOWN:							
			{
				KEYS[ wParam ] = true;
			}
			break;
		case WM_KEYUP:
			{
				KEYS[ wParam ] = false;
				movin = false;
				if (movi == 139) movi = 3;
			}
			break;
		case WM_CLOSE: 
			{
				DestroyWindow(hWnd);
			}
			break;
		case WM_DESTROY: //Send A Quit Message
			{
				KillTimer(hWnd, TICK);
				PostQuitMessage(0);
			}
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

/* Inicializacion de variables y reserva de memoria.
	*/
void Init() 
{
	posFigura.X = 925;
	posFigura.Y = 455;
	dmnFigura.ANCHO = 17;
	dmnFigura.ALTO = 24;
	for(int i = 0; i < 256; i++)
	{
		KEYS[i] = false;
	}
	ptrBuffer = new int[ANCHO_VENTANA * ALTO_VENTANA];
	try
	{
		ptrFigura = new unsigned char[(475*255) * 4];
		ptrTiles = new unsigned char[(545*589) * 4];
	}
	catch (...)
	{
		return;
	}
	CargaImagen(L"mario sprite.png");
	for (int i = 0; i < ((475 * 255) * 4); i++)
	{
		ptrFigura[i] = ptrImagen[i];
	}
	CargaImagen(L"tiles.png");
	for (int h = 0; h < ((545 * 589) * 4); h++)
	{
		ptrTiles[h] = ptrImagen[h];
	}
	CargaImagen(L"backgrounds.png");
}

/* Funcion principal. Encargada de hacer el redibujado en pantalla cada intervalo (o "tick") del timer que se haya creado.
	@param hWnd. Manejador de la ventana.
	*/
void MainRender(HWND hWnd) 
{
	KeysEvents();

	LimpiarFondo(ptrBuffer, (unsigned int)ptrImagen, 400,300);
	DibujaTiles(ptrBuffer, (unsigned int)ptrTiles, 16, 16);
	//CambiaAzul((int*)ptrImagen, 1032, 2174, blueton);
	//CambiaRojo((int*)ptrImagen, 1032, 2174, redton);
	//CambiaVerde((int*)ptrImagen, 1032, 2174, greenton);
	DibujaBloque(ptrBuffer, (unsigned int)ptrTiles, ANCHO_VENTANA, ALTO_VENTANA, 16, 16, 32, 32, 0, 0, 0);
	DibujaBloque(ptrBuffer, (unsigned int)ptrTiles, ANCHO_VENTANA, ALTO_VENTANA, 16, 16, 96, 32, 0, 0, 200);
	DibujaBloque(ptrBuffer, (unsigned int)ptrTiles, ANCHO_VENTANA, ALTO_VENTANA, 16, 16, 160, 32, 0, 200, 0);
	DibujaBloque(ptrBuffer, (unsigned int)ptrTiles, ANCHO_VENTANA, ALTO_VENTANA, 16, 16, 224, 32, 200, 0, 0);
	DibujaBloque(ptrBuffer, (unsigned int)ptrTiles, ANCHO_VENTANA, ALTO_VENTANA, 16, 16, 96, 96, 0, 0, blueton);
	DibujaBloque(ptrBuffer, (unsigned int)ptrTiles, ANCHO_VENTANA, ALTO_VENTANA, 16, 16, 160, 96, 0, greenton, 0);
	DibujaBloque(ptrBuffer, (unsigned int)ptrTiles, ANCHO_VENTANA, ALTO_VENTANA, 16, 16, 224, 96, redton, 0, 0);
	if (ladin == true)
	{
		if (jump == true)
		{
			DibujaFigurajump(ptrBuffer, (unsigned int)ptrFigura, ANCHO_VENTANA,ALTO_VENTANA,dmnFigura, posFigura, jumanim);
		}
		else
		{
			DibujaFigura(ptrBuffer, (unsigned int)ptrFigura, ANCHO_VENTANA, ALTO_VENTANA, dmnFigura, posFigura, movi);
		}
	}
	else
	{
		if (jump == true)
		{
			DibujaFigurajumpmirror(ptrBuffer, (unsigned int)ptrFigura, ANCHO_VENTANA, ALTO_VENTANA, dmnFigura, posFigura, jumanim);
		}
		else
		{
			DibujaFiguramirror(ptrBuffer, (unsigned int)ptrFigura, ANCHO_VENTANA, ALTO_VENTANA, dmnFigura, posFigura, movi);
		}
	}

	if (jump == true)
	{
		if (korv == 20)
		{
			if (posFigura.Y < 455)
			{
				if (jumanim == 105) jumanim = 122;
				posFigura.Y += 5;
			}
			else
			{
				korv = 0;
				jump = false;
				jumanim = 105;
			}
		}
		else
		{
			posFigura.Y -= 5;
			korv++;
		}
	}

	if (movin == true)
	{
		if (koli == 5)
		{
			if (movi == 3)
			{
				movi += 17;
				koli = 0;
			}
			else
			{
				movi -= 17;
				koli = 0;
			}
		}
		else
		{
			koli++;
		}
	}
	else
	{
		movi = 3;
	}

	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);
}

void KeysEvents() 
{
	if(KEYS[input.W] || KEYS[input.Up])
	{ 
		jump = true;
		//posFigura.Y-=5;
	}
	if((KEYS[input.D] || KEYS[input.Right]))
	{
		if (posFigura.X < 1550)
		{
			posFigura.X += 5;
		}
		movin = true;
		ladin = true;
	}
	if(KEYS[input.S] || KEYS[input.Down])
	{
		if(posFigura.Y < 455) posFigura.Y+=5;
		if ((movi == 3 || movi == 20) && posFigura.Y == 455) movi = 156;
	}
	if(KEYS[input.A] || KEYS[input.Left])
	{
		if (posFigura.X > 795)
		{
			posFigura.X -= 5;
		}
		movin = true;
		ladin = false;
	}	
	if (KEYS[input.R])
	{
		redton++;
	}
	if (KEYS[input.F])
	{
		redton--;
	}
	if (KEYS[input.T])
	{
		greenton++;
	}
	if (KEYS[input.G])
	{
		greenton--;
	}
	if (KEYS[input.Y])
	{
		blueton++;
	}
	if (KEYS[input.H])
	{
		blueton--;
	}
}

/* Funcion para cargar imagenes y obtener un puntero al area de memoria reservada para la misma.
	@param rutaImagen.	Nombre o direccion del archivo.
	@return VOID.		No devuelve nada la funcion pero podrian poner que devuelva el puntero a la imagen.
	*/
void CargaImagen(WCHAR rutaImagen[])
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR  gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap *bitmap=new Bitmap(rutaImagen);
	BitmapData *bitmapData=new BitmapData;

	ancho = bitmap->GetWidth();
	alto = bitmap->GetHeight();

	Rect rect(0, 0, ancho, alto);

	//Reservamos espacio en memoria para la imagen
	bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, bitmapData);

	//"pixels" es el puntero al area de memoria que ocupa la imagen
	unsigned char* pixels = (unsigned char*)bitmapData->Scan0;

	//"tamaño" lo usaremos para reservar los bytes que necesita la imagen. 
	//Para calcular la cantidad de bytes total necesitamos multiplicamos el area de la imagen * 4. 
	//Se multiplica por 4 debido a que cada pixel ocupa 4 bytes de memoria. Noten el 3er parametro de la funcion LockBits, dos lineas de codigo arriba.
	//PixelFormat32bppARGB -> Specifies that the format is 32 bits per pixel; 8 bits each are used for the alpha, red, green, and blue components.
	//Mas info: https://msdn.microsoft.com/en-us/library/system.drawing.imaging.pixelformat(v=vs.110).aspx
	int tamaño;
	tamaño = ancho*alto*4;
	//hagamos un try de la reserva de memoria
	try
	{
		ptrImagen = new unsigned char [tamaño]; 
	}
	catch(...)
	{
		return;
	}

	//Después de este for, ptrImagen contiene la direccion en memoria de la imagen.
	for(int i=0, j=alto*ancho*4;i<j;i++)
	{
		ptrImagen[i]=pixels[i];
	}

	//Es necesario liberar el espacio en memoria, de lo contrario marcaria una excepcion de no hay espacio de memoria suficiente.
	bitmap->UnlockBits(bitmapData);
	delete bitmapData;
	delete bitmap;
	  
	GdiplusShutdown(gdiplusToken);
}

int jumpy(int vel, int acel, bool sent)
{
	int disty;
	


	return disty;
}

#pragma region LENS_CODE
/* Pinta el fondo de la ventana de acuerdo al color especificado.
	@param *ptrBuffer.	Puntero al area de memoria reservada para el proceso de dibujado.
	@param color.		Color expresado en formato hexadecimal.
	@param area.		Area de la ventana.
	*/
void LimpiarFondo(int *ptrBuffer, unsigned int color, int x, int y) 
{
	int jump = (1032 * 134) * 4;
	int bytesporlinea = x * 4;
	__asm {
			mov edi, ptrBuffer
			mov ecx, y
			mov esi, color
			add esi, jump
			add esi, 2*4

			draw: 
				push ecx
				mov ecx, 2
				scale2:
					push ecx
					mov ecx, x
					scale:
						push ecx
						mov ecx, 2
						draw2:
							mov eax, [esi]
							mov [edi], eax
							add edi, BPP
							//add esi, BPP
						loop draw2
						add esi, BPP
						pop ecx
					loop scale
					pop ecx
					sub esi, bytesporlinea
				loop scale2
				add esi, bytesporlinea
				pop ecx
				add esi, 632*4
				//add edi, 400*4
			loop draw
	}
}

void DibujaTiles(int *ptrBuffer, unsigned int color, int x, int y)
{
	int jump = ((545 * 202) + 444)*4;
	int jump2 = (504 * 800) * 4;
	int jump22 = ((545 * 219)+ 444) * 4;
	int jump222 = (536 * 800) * 4;
	__asm
	{
		mov edi, ptrBuffer
		mov esi, color
		add esi, jump
		add edi, jump2
		mov ecx, y

		draw:
			push ecx
			mov ecx, 2
			scale2:
				push ecx
				mov ecx, 25
				move:
					push ecx
					mov ecx, x
					scale:
						push ecx
						mov ecx, 2
						draw2:
							 mov eax, [esi]
							 mov [edi], eax
							 add edi, BPP
						loop draw2
						add esi, BPP
						pop ecx
					loop scale
					sub esi, 16*4
					pop ecx
				loop move
				pop ecx
			loop scale2
			add esi, 16*4
			add esi, 529*4
			pop ecx
		loop draw

		mov esi, color
		mov edi, ptrBuffer
		add esi, jump22
		add edi, jump222
		mov ecx, y

		draw22:
			push ecx
			mov ecx, 2
			scale22:
				push ecx
				mov ecx, 25
				move2:
					push ecx
					mov ecx, x
					scale3:
						push ecx
						mov ecx, 2
						draw3:
							mov eax, [esi]
							mov[edi], eax
							add edi, BPP
						loop draw3
						add esi, BPP
						pop ecx
					loop scale3
					sub esi, 16 * 4
					pop ecx
				loop move2
				pop ecx
			loop scale22
			add esi, 16 * 4
			add esi, 529 * 4
			pop ecx
		loop draw22
	}
}

/* Funcion que pinta una figura rectangular en pantalla.
	@param *ptrBuffer.	Puntero al area de memoria reservada para el dibujado.
	@param color.		Color de la figura expresado en formato hexadecimal.
	@param anchoWnd.	Ancho total de la ventana.
	@param altoWnd.		Alto total de la ventana.
	@param dmnFigura.	Especifica las dimensiones de la figura en relacion con la ventana.
	@param posFigura.	Posiciona la figura en la ventana.
	*/
void DibujaFiguramirror(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, DIMENSION dmnFigura, POSITION posFigura, int move)
{
	int x = posFigura.X;
	int y = posFigura.Y;
	int ancho = dmnFigura.ANCHO;
	int alto = dmnFigura.ALTO;
	int mover = (438 + 20) * 4;
	int anchoWndBPP = anchoWnd * 4;
	int anchodos = ancho * 2;
	int anchin = ancho * 4;
	int jump = ((475 * 5) + move) * 4;
	int holi = 0;
	__asm {
			mov edi, ptrBuffer
			mov esi, color
			add esi, jump

			mov eax, x
			mul BPP
			add edi, eax

			mov eax, y
			mul BPP
			mul anchoWnd
			add edi, eax

			mov eax, color

			mov ecx, alto
			altini:
				push ecx
				mov ecx, 2
				scale2:
					push ecx
					mov ecx, ancho
					add edi, 34 * 4
					anchini:
						push ecx
						mov ecx, 2
						scale:
							mov eax, [esi]
							cmp eax, 0xFFFF8040
							je oli
								mov [edi], eax
							oli:
							//add esi, BPP
							sub edi, BPP
						loop scale
						add esi, BPP
						pop ecx
					loop anchini
					add edi, 34 * 4
					pop ecx
					push eax
					mov eax, anchoWnd
					sub eax, anchodos
					mul BPP
					add edi, eax
					pop eax
					sub esi, anchin
				loop scale2
				add esi, anchin
				add esi, mover
				pop ecx
			loop altini

	}
}

void DibujaFigura(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, DIMENSION dimension, POSITION position, int move)
{
	int x = posFigura.X;
	int y = posFigura.Y;
	int ancho = dmnFigura.ANCHO;
	int alto = dmnFigura.ALTO;
	int mover = (438 + 20) * 4;
	int anchoWndBPP = anchoWnd * 4;
	int anchodos = ancho * 2;
	int anchin = ancho * 4;
	int jump = ((475 * 5) + move) * 4;
	int holi = 0;
	__asm {
		mov edi, ptrBuffer
			mov esi, color
			add esi, jump

			mov eax, x
			mul BPP
			add edi, eax

			mov eax, y
			mul BPP
			mul anchoWnd
			add edi, eax

			mov eax, color

			mov ecx, alto
			altini :
				push ecx
				mov ecx, 2
				scale2 :
					push ecx
					mov ecx, ancho
					anchini :
						push ecx
						mov ecx, 2
						scale :
							mov eax, [esi]
							cmp eax, 0xFFFF8040
							je oli
								mov[edi], eax
							oli :
							//add esi, BPP
							add edi, BPP
						loop scale
						add esi, BPP
						pop ecx
					loop anchini
					pop ecx
					push eax
					mov eax, anchoWnd
					sub eax, anchodos
					mul BPP
					add edi, eax
					pop eax
					sub esi, anchin
				loop scale2
				add esi, anchin
				add esi, mover
				pop ecx
			loop altini

	}
}

void DibujaFigurajump(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, DIMENSION dimension, POSITION position, int jumpl)
{
	int x = posFigura.X;
	int y = posFigura.Y;
	int ancho = dmnFigura.ANCHO;
	int alto = dmnFigura.ALTO;
	int mover = (438 + 20) * 4;
	int anchoWndBPP = anchoWnd * 4;
	int anchodos = ancho * 2;
	int anchin = ancho * 4;
	int jump = ((475 * 5) + jumpl) * 4;
	int holi = 0;
	__asm {
		mov edi, ptrBuffer
			mov esi, color
			add esi, jump

			mov eax, x
			mul BPP
			add edi, eax

			mov eax, y
			mul BPP
			mul anchoWnd
			add edi, eax

			mov eax, color

			mov ecx, alto
		altini :
		push ecx
			mov ecx, 2
		scale2 :
			   push ecx
			   mov ecx, ancho
		   anchini :
		push ecx
			mov ecx, 2
		scale :
			  mov eax, [esi]
			  cmp eax, 0xFFFF8040
			  je oli
			  mov[edi], eax
		  oli :
		//add esi, BPP
		add edi, BPP
			loop scale
			add esi, BPP
			pop ecx
			loop anchini
			pop ecx
			push eax
			mov eax, anchoWnd
			sub eax, anchodos
			mul BPP
			add edi, eax
			pop eax
			sub esi, anchin
			loop scale2
			add esi, anchin
			add esi, mover
			pop ecx
			loop altini

	}
}

void DibujaFigurajumpmirror(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, DIMENSION dmnFigura, POSITION posFigura, int jumpl)
{
	int x = posFigura.X;
	int y = posFigura.Y;
	int ancho = dmnFigura.ANCHO;
	int alto = dmnFigura.ALTO;
	int mover = (438 + 20) * 4;
	int anchoWndBPP = anchoWnd * 4;
	int anchodos = ancho * 2;
	int anchin = ancho * 4;
	int jump = ((475 * 5) + jumpl) * 4;
	int holi = 0;
	__asm {
		mov edi, ptrBuffer
			mov esi, color
			add esi, jump

			mov eax, x
			mul BPP
			add edi, eax

			mov eax, y
			mul BPP
			mul anchoWnd
			add edi, eax

			mov eax, color

			mov ecx, alto
		altini :
		push ecx
			mov ecx, 2
		scale2 :
			   push ecx
			   mov ecx, ancho
			   add edi, 34 * 4
		   anchini :
				   push ecx
				   mov ecx, 2
			   scale :
					 mov eax, [esi]
					 cmp eax, 0xFFFF8040
					 je oli
					 mov[edi], eax
				 oli :
		//add esi, BPP
		sub edi, BPP
			loop scale
			add esi, BPP
			pop ecx
			loop anchini
			add edi, 34 * 4
			pop ecx
			push eax
			mov eax, anchoWnd
			sub eax, anchodos
			mul BPP
			add edi, eax
			pop eax
			sub esi, anchin
			loop scale2
			add esi, anchin
			add esi, mover
			pop ecx
			loop altini

	}
}

void DibujaBloque(int *ptrBuffer, unsigned int color, int anchoWnd, int altoWnd, int ancho, int alto, int x, int y, unsigned char red, unsigned char green, unsigned char blue)
{
	int mover = (89553 + 173) * 4;
	int anchoWndBPP = anchoWnd * 4;
	int anchodos = ancho * 2;
	int anchin = ancho * 4;
	int holi = 0;
	__asm {
		mov edi, ptrBuffer
		mov esi, color
		add esi, 89553*4

		mov eax, x
		mul BPP
		add edi, eax

		mov eax, y
		mul BPP
		mul anchoWnd
		add edi, eax

		mov eax, color

		mov ecx, alto
		altini :
			push ecx
			mov ecx, 2
			scale2 :
				push ecx
				mov ecx, ancho
				anchini :
					push ecx
					mov ecx, 2
					scale :
						mov eax, [esi]
						cmp eax, 0xFF0E5EF1
						je oli
							mov[edi], eax
							push eax
							xor eax, eax
							mov al, blue
							add[edi], eax
							xor eax, eax
							mov al, red
							shl eax, 16
							add[edi], eax
							xor eax, eax
							mov al, green
							shl eax, 8
							add[edi], eax
							pop eax
						oli :
						add edi, BPP
					loop scale
					add esi, BPP
					pop ecx
				loop anchini
				pop ecx
				push eax
				mov eax, anchoWnd
				sub eax, anchodos
				mul BPP
				add edi, eax
				pop eax
				sub esi, anchin
			loop scale2
			add esi, anchin
			add esi, 529*4
			pop ecx
		loop altini

	}
}

//void CambiaAzul(int* punteroImagen, int ancho, int alto, unsigned char color){
//
//	int totalpixels = ancho * alto;
//
//	__asm{
//
//		mov edi, punteroImagen
//			mov ecx, totalpixels
//
//			; add edi, 89553
//
//		tonoloop :
//			mov eax, 0xFFFFFF00
//			; and[edi], eax; Limpia el componente azul de la imagen
//			; Mascaras para los otros colores :
//			; Rojo: 0xFF00FFFF
//			; Verde: 0xFFFF00FF
//			; Azul: 0xFFFFFF00
//			xor eax, eax
//			add al, color
//			; shl eax, 16; Ponemos el valor del color en eax, luego lo movemos a la izquierda para que quede en el lugar del componente rojo
//			; Cantidad para el rojo : 16
//			; Cantidad para el color verde : 8
//			; Cantidad para azul : 0
//			add[edi], eax
//
//			add edi, 4
//		loop tonoloop
//
//	}
//
//}
//

#pragma endregion