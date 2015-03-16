#ifndef _IVI_DUMMYCALL_H
#define _IVI_DUMMYCALL_H

#ifdef _WIN32
	#include <windows.h>
    #include <stdio.h>
	#include <math.h>
#if (_MSC_VER >= 1300)
	#include <tchar.h>
	#include <strsafe.h>
#endif

	///*!
	// Insert garbage code.
	//
	// Return value - None
	//
	// Supported platform:		All Windows
	// Performance overhead:	Low
	// Security level:			Low
	// Usage scope:				Macro scope
	// 
	// Note:\n
	// Insure every macro with a chosen Num value is used only once
	// within a block.  Please use different Num values if the macro
	// is to be called multiple times in block scope.
	//*/
	#define iviTR_DUMMY_CALL_1(Num)					\
		volatile int nVal1##Num = ##Num;			\
		if (nVal1##Num != ##Num)					\
		{											\
			HWND hwnd##Num = NULL;					\
			hwnd##Num = ::CreateWindow(				\
				NULL,								\
				NULL,								\
				WS_OVERLAPPEDWINDOW,				\
				CW_USEDEFAULT,						\
				CW_USEDEFAULT,						\
				CW_USEDEFAULT,						\
				CW_USEDEFAULT,						\
				(HWND) NULL,						\
				(HMENU) NULL,						\
				(HINSTANCE) NULL,					\
				(LPVOID) NULL);						\
			if (!hwnd##Num)							\
			{										\
				__asm ret							\
			}										\
			::ShowWindow(hwnd##Num, SW_SHOWDEFAULT);\
			::UpdateWindow(hwnd##Num);				\
		}

	#define iviTR_DUMMY_CALL_2(Num)							\
		volatile int nVal2##Num = ##Num;					\
		if ((nVal2##Num - 9) == nVal2##Num)					\
		{													\
			STARTUPINFO si##Num;							\
			PROCESS_INFORMATION pi##Num;					\
			::ZeroMemory(&si##Num, sizeof(si##Num));		\
			si##Num.cb = sizeof(si##Num);					\
			::ZeroMemory(&pi##Num, sizeof(pi##Num));		\
			if(!::CreateProcess(NULL,						\
				NULL,										\
				NULL,										\
				NULL,										\
				FALSE,										\
				0,											\
				NULL,										\
				NULL,										\
				&si##Num,									\
				&pi##Num)									\
			)												\
			{												\
				__asm ret									\
			}												\
			::WaitForSingleObject(pi##Num.hProcess, INFINITE);\
			::CloseHandle(pi##Num.hProcess);				\
			::CloseHandle(pi##Num.hThread);					\
		}

	#define iviTR_DUMMY_CALL_3(Num)														\
		volatile int nVal3##Num = ##Num;												\
		if (nVal3##Num != ##Num)														\
		{																				\
			char szKey##Num[9];														\
			char szSubKey##Num[14];													\
			BOOL bError##Num = TRUE;													\
			HKEY hKey##Num = 0;															\
			HKEY hSubKey##Num = 0;														\
			szKey##Num[4] = 'w'; szKey##Num[7] = 'e'; szKey##Num[1] = 'o';				\
			szKey##Num[5] = 'a'; szKey##Num[2] = 'f'; szKey##Num[3] = 't';				\
			szKey##Num[6] = 'r'; szKey##Num[8] = '\0'; szKey##Num[0] = 'S';				\
			szSubKey##Num[2] = 'n'; szSubKey##Num[0] = 'W'; szSubKey##Num[11] = 'i';	\
			szSubKey##Num[13] = '\0'; szSubKey##Num[1] = 'i'; szSubKey##Num[6] = 's';	\
			szSubKey##Num[9] = 'e'; szSubKey##Num[4] = 'o'; szSubKey##Num[12] = 'a';	\
			szSubKey##Num[3] = 'd'; szSubKey##Num[7] = ' '; szSubKey##Num[5] = 'w';		\
			szSubKey##Num[10] = 'd';	szSubKey##Num[8] = 'M';							\
			if (::RegOpenKeyExA(HKEY_LOCAL_MACHINE, szKey##Num, 0, KEY_QUERY_VALUE, &hKey##Num) == ERROR_SUCCESS) \
			{																			\
				if (::RegOpenKeyExA(hKey##Num, szSubKey##Num, 0, KEY_QUERY_VALUE, &hSubKey##Num) == ERROR_SUCCESS) \
				{																		\
					bError##Num = FALSE;												\
				}																		\
			}																			\
			if (hKey##Num)																\
				::RegCloseKey(hKey##Num);												\
			if (hSubKey##Num)															\
				::RegCloseKey(hSubKey##Num);											\
		}

	#define iviTR_DUMMY_CALL_4(Num)													\
		volatile int nVal4##Num = ##Num;											\
		if ((nVal4##Num * 3) == ##Num)												\
		{																			\
			int nSwap##Num;															\
			int nValue##Num[10];													\
			int iIndex##Num;														\
			for (iIndex##Num = 0; iIndex##Num < 10; iIndex##Num++)					\
			{																		\
				nValue##Num[iIndex##Num] = rand();									\
			}																		\
			for (iIndex##Num = 0; iIndex##Num < 10; iIndex##Num++)					\
			{																		\
				for (int j = 0; j < 10; j++)										\
				{																	\
					if (nValue##Num[j] > nValue##Num[j + 1])						\
					{																\
						nSwap##Num = nValue##Num[j];								\
						nValue##Num[j] = nValue##Num[j + 1];						\
						nValue##Num[j + 1] = nSwap##Num;							\
					}																\
				}																	\
			}																		\
		}
	
	#define iviTR_DUMMY_CALL_5(Num)													\
		volatile int nVal5##Num = ##Num;											\
		if (nVal5##Num != ##Num)													\
		{																			\
			int nValue##Num[16] = {81, 11, 4, 7, 39, 9, 42, 89, 45, 3, 6, 97, 62, 5, 31, 75}; \
			int iMid##Num = (16 + 0) / 2;											\
			if (nValue##Num[0] > nValue##Num[iMid##Num])							\
			{																		\
				int nTemp = nValue##Num[0];											\
				nValue##Num[0] = nValue##Num[iMid##Num];							\
				nValue##Num[iMid##Num] = nTemp;										\
			}																		\
			if (nValue##Num[0] > nValue##Num[15])									\
			{																		\
				int nTemp = nValue##Num[0];											\
				nValue##Num[0] = nValue##Num[15];									\
				nValue##Num[15] = nTemp;											\
			}																		\
			if (nValue##Num[iMid##Num] > nValue##Num[15])							\
			{																		\
				int nTemp = nValue##Num[iMid##Num];									\
				nValue##Num[iMid##Num] = nValue##Num[15];							\
				nValue##Num[15] = nTemp;											\
			}																		\
			int nMid##Num = nValue##Num[iMid##Num];									\
			int i##Num = 0;															\
			int j##Num = 15;														\
			do																		\
			{																		\
				while (nValue##Num[i##Num] < nMid##Num)								\
					i##Num++;														\
				while (nMid##Num < nValue##Num[j##Num])								\
					j##Num--;														\
				if (i##Num <= j##Num)												\
				{																	\
					int nTemp = nValue##Num[i##Num];								\
					nValue##Num[i##Num] = nValue##Num[j##Num];						\
					nValue##Num[j##Num] = nTemp;									\
					i##Num++;														\
					j##Num++;														\
				}																	\
			} while (i##Num <= j##Num);												\
		}
	#define iviTR_DUMMY_CALL_6(Num)													\
		volatile int nVal6##Num = ##Num;											\
		if ((nVal6##Num + 5) == ##Num)												\
		{																			\
			int nResult##Num;														\
			int nNum1 = nVal6##Num + (rand() % rand());								\
			int nNum2 = nVal6##Num + (rand() * rand() % rand());					\
			int nMod;																\
			if (nNum2 == 0)															\
			{																		\
				nResult##Num = nNum1;												\
			}																		\
			else																	\
			{																		\
				while (1)															\
				{																	\
					nMod = nNum1 % nNum2;											\
					if (nMod == 0)													\
					{																\
						nResult##Num = nNum2;										\
						break;														\
					}																\
					else															\
						nNum2 = nMod;												\
				}																	\
			}																		\
		}
#if (_MSC_VER >= 1300) && defined(_WIN32)
	#define iviTR_DUMMY_CALL_7(Num)													\
		volatile int nVal7##Num = ##Num;											\
		DWORD dwResult7##Num;														\
		if (##Num == (nVal7##Num - 41))												\
		{																			\
			LPVOID lpMsgBuf;														\
			LPVOID lpDisplayBuf;													\
			LPTSTR lpszFunction = TEXT("Error");									\
			DWORD dw = ::GetLastError();											\
			::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL ); \
			lpDisplayBuf = (LPVOID)::LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); \
			StringCbPrintf((LPTSTR)lpDisplayBuf, MAX_PATH, TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf); \
			::MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);		\
			::LocalFree(lpMsgBuf);													\
			::LocalFree(lpDisplayBuf);												\
			dwResult7##Num = dw;													\
		}
#else
	#define iviTR_DUMMY_CALL_7(Num)													\
		volatile int nVal7##Num = ##Num;											\
		DWORD dwResult7##Num;														\
		if (##Num == (nVal7##Num - 41))												\
		{																			\
			LPVOID lpMsgBuf;														\
			LPVOID lpDisplayBuf;													\
			LPTSTR lpszFunction = TEXT("Error");									\
			DWORD dw = ::GetLastError();											\
			::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL ); \
			lpDisplayBuf = (LPVOID)::LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); \
			wsprintf((LPTSTR)lpDisplayBuf, TEXT("%s failed with error %d: %s"), lpszFunction, dw, lpMsgBuf); \
			::MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);		\
			::LocalFree(lpMsgBuf);													\
			::LocalFree(lpDisplayBuf);												\
			dwResult7##Num = dw;													\
		}
#endif
	#define iviTR_DUMMY_CALL_8(Num)													\
		volatile int nVal8##Num = ##Num;											\
		if ((nVal8##Num * 3) == nVal8##Num)											\
		{																			\
			HCURSOR hCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));			\
			::ShowCursor(TRUE);														\
			POINT apt[2];															\
			RECT rect = {50, 48, 52, 54};											\
			apt[0].x = 10;															\
			apt[0].y = 10;															\
			apt[1].x = rect.right - 10;												\
			apt[1].y = 10;															\
			HDC hdc = ::GetDC(NULL);												\
			::SelectObject(hdc, ::GetStockObject(WHITE_PEN));						\
			::MoveToEx(hdc, apt[0].x, apt[0].y, NULL);								\
			::LineTo(hdc, apt[1].x, apt[1].y);										\
			::SetRect(&rect, rand() % apt[1].x, rand() % apt[0].x, rand() % apt[0].y, rand() % apt[1].y); \
			HBRUSH hBrush = ::CreateSolidBrush(RGB(rand() %256, rand() %256, 131));	\
			::FillRect(hdc, &rect, hBrush);											\
			double fRadius, fAngle;													\
			HRGN hRgnClip = ::CreateRectRgn(0, 0, 1, 1);							\
			::SetViewportOrgEx(hdc, nVal8##Num * 3 / 2, nVal8##Num * 5 /2, NULL);	\
			::SelectClipRgn(hdc, hRgnClip);											\
			double cxClient = nVal8##Num * 3 / 2;									\
			double cyClient = nVal8##Num * 5 /2;									\
			fRadius = _hypot(cxClient / 2.0, cyClient / 2.0);						\
			double fTwoPi = 2.0 * 3.14159;											\
			for (fAngle = 0.0; fAngle < fTwoPi; fAngle += fTwoPi / 360)				\
			{																		\
				MoveToEx(hdc, 0, 0, NULL);											\
				LineTo(hdc, (int)(fRadius * cos(fAngle) + 0.5), (int)(-fRadius * sin(fAngle) + 0.5)); \
			}																		\
			::SetCursor(hCursor);													\
			::ShowCursor(FALSE);													\
			::ReleaseDC(NULL, hdc);													\
		}

	#define iviTR_DUMMY_CALL_9(Num)													\
		volatile int nVal9##Num = ##Num;											\
		if (##Num != nVal9##Num)													\
		{																			\
			int cap = rand();														\
			int i, j, composite, t = 0;												\
			while(t < cap * cap)													\
			{																		\
				i = t / cap;														\
				j = t++ % cap;														\
				if(i <= 1);															\
				else if(j == 0)														\
					composite = 0;													\
				else if(j == i && !composite)										\
					printf("%d\t",i);												\
				else if(j > 1 && j < i)												\
					composite += !(i % j);											\
			}																		\
		}
	#define iviTR_DUMMY_CALL_10(Num)												\
		volatile int nVal10##Num = ##Num;											\
		double fResult##Num = NULL;													\
		if ((nVal10##Num - 81) == ##Num)											\
		{																			\
			bool bReturned = false;													\
			double arg = (double)nVal10##Num;										\
			int	nErrno;																\
			static double invpi	  = 1.27323954473516268;							\
			static double p0	 = -0.1306820264754825668269611177e+5;				\
			static double p1	  = 0.1055970901714953193602353981e+4;				\
			static double p2	 = -0.1550685653483266376941705728e+2;				\
			static double p3	  = 0.3422554387241003435328470489e-1;				\
			static double p4	  = 0.3386638642677172096076369e-4;					\
			static double q0	 = -0.1663895238947119001851464661e+5;				\
			static double q1	  = 0.4765751362916483698926655581e+4;				\
			static double q2	 = -0.1555033164031709966900124574e+3;				\
			double sign, temp, e, x, xsq;											\
			int flag, i;															\
			flag = 0;																\
			sign = 1.;																\
			if (arg < 0.)															\
			{																		\
				arg = -arg;															\
				sign = -1.;															\
			}																		\
			arg = arg * invpi;														\
			x = modf(arg, &e);														\
			i = (int)e;																\
			switch(i % 4)															\
			{																		\
			case 1:																	\
				x = 1. - x;															\
				flag = 1;															\
				break;																\
			case 2:																	\
				sign = - sign;														\
				flag = 1;															\
				break;																\
			case 3:																	\
				x = 1. - x;															\
				sign = - sign;														\
				break;																\
			case 0:																	\
				break;																\
			}																		\
			xsq = x * x;															\
			temp = ((((p4 * xsq + p3) * xsq + p2) * xsq + p1) * xsq + p0) * x;		\
			temp = temp / (((1.0 * xsq + q2) * xsq + q1) * xsq + q0);				\
			if(flag == 1) {															\
				if(temp == 0.) {													\
					nErrno = ERANGE;													\
					if (sign > 0)													\
						fResult##Num  = (HUGE);										\
					fResult##Num = (-HUGE);											\
					bReturned = true;												\
				}																	\
				temp = 1./temp;														\
			}																		\
			if (!bReturned)															\
				fResult##Num = sign * temp;											\
		}
#elif defined(__linux__)
	#define iviTR_DUMMY_CALL_1(Num)
	#define iviTR_DUMMY_CALL_2(Num)
	#define iviTR_DUMMY_CALL_3(Num)
	#define iviTR_DUMMY_CALL_4(Num)
	#define iviTR_DUMMY_CALL_5(Num)
	#define iviTR_DUMMY_CALL_6(Num)
	#define iviTR_DUNNY_CALL_7(Num)
	#define iviTR_DUMMY_CALL_8(Num)
	#define iviTR_DUMMY_CALL_9(Num)
	#define iviTR_DUMMY_CALL_10(Num)
#endif // _WIN32
#endif // _IVI_DUMMYCALL_H
