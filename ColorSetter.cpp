#include <Windows.h>
#include <Stdio.h>
#include <Tchar.h>
#include <Wininet.h>
#include <Shlobj.h>
#include <Commctrl.h>
#pragma comment(lib,"Comctl32.lib")

#include "Resource.h"

HINSTANCE		g_hinst;
HWND			g_hMainDlg;

//获取注册表值
DWORD ReadRegValue(
	HKEY hkey, LPTSTR lpSubKey, 
	LPTSTR lpValueName,
	LPBYTE lpValue, 
	DWORD &cbValue
	)
{
	HKEY		hResult;
	DWORD		dwType;

	//打开键
	if(RegOpenKeyEx(hkey, lpSubKey, 0, KEY_READ, &hResult) != ERROR_SUCCESS)
		return -1;

	//读取值
	if(RegQueryValueEx(hResult, lpValueName, 0, &dwType, lpValue, &cbValue) != ERROR_SUCCESS)
	{
		RegCloseKey(hResult);
		return -1;
	}

	//关闭键
	RegCloseKey(hResult);
	
	return dwType;
}

//设置注册值
BOOL SetRegValue(
	HKEY hkey, 
	LPTSTR lpSubKey, 
	LPTSTR lpValueName, 
	LPBYTE lpValue, 
	DWORD cbValue, 
	DWORD dwType
	)
{
	HKEY		hResult;

	//打开键
	if( RegOpenKeyEx(hkey, lpSubKey, 0, KEY_WRITE, &hResult) != ERROR_SUCCESS )
		return FALSE;

	//设置注册表值
	if( RegSetValueEx(hResult, lpValueName, 0, dwType, lpValue, cbValue) != ERROR_SUCCESS )
	{
		RegCloseKey(hResult);
		return FALSE;
	}

	//关闭键
	RegCloseKey(hResult);

	return TRUE;
}

//选择颜色
BOOL ChooseColorDlg(
	HWND hWnd, 
	COLORREF &rgbResult, 
	COLORREF *prgbCustom
	)
{
	CHOOSECOLOR		clr = {0};

	clr.lStructSize = sizeof(CHOOSECOLOR);
	clr.hwndOwner = hWnd;
	clr.hInstance = 0;
	clr.rgbResult = rgbResult;
	clr.lpCustColors = prgbCustom;
	clr.Flags =  CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;
	if(!ChooseColor(&clr))
		return FALSE;
	
	rgbResult = clr.rgbResult;

	return TRUE;
}

//获取墙纸
BOOL GetWallPaper(
	LPWSTR lpszWallPaper, 
	LPDWORD lpdwStyle
	)
{
	HRESULT				hr;
	WALLPAPEROPT		wallPaperOpt;
	IActiveDesktop		*pActiveDesktop;
	WCHAR				szPathFile[MAX_PATH];

	hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
		IID_IActiveDesktop, (void**)&pActiveDesktop);
	if(hr != S_OK)
		return FALSE;

	if(lpdwStyle)
	{
		wallPaperOpt.dwSize = sizeof(WALLPAPEROPT);
		wallPaperOpt.dwStyle = 0;
		hr = pActiveDesktop->GetWallpaperOptions(&wallPaperOpt, 0);
		if(hr != S_OK)
		{
			pActiveDesktop->Release();
			return FALSE;
		}
		*lpdwStyle = wallPaperOpt.dwStyle;
	}
	if(lpszWallPaper)
	{
		hr = pActiveDesktop->GetWallpaper(szPathFile, MAX_PATH, AD_GETWP_LAST_APPLIED);
		if(hr != S_OK)
		{
			pActiveDesktop->Release();
			return FALSE;
		}
		lstrcpyW(lpszWallPaper, szPathFile);
	}

	pActiveDesktop->Release();
	return TRUE;
}

//设置墙纸
BOOL SetWallPaper(
	LPCWSTR lpszWallPaper, 
	LPDWORD lpdwStyle
	)
{
	HRESULT				hr;
	WALLPAPEROPT		wallPaperOpt;
	IActiveDesktop		*pActiveDesktop;

	hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
		IID_IActiveDesktop, (void**)&pActiveDesktop);
	if(hr != S_OK)
		return FALSE;

	if(lpdwStyle)
	{
		wallPaperOpt.dwSize = sizeof(WALLPAPEROPT);
		wallPaperOpt.dwStyle = *lpdwStyle;
		hr = pActiveDesktop->SetWallpaperOptions(&wallPaperOpt, 0);
		if(hr != S_OK)
		{
			pActiveDesktop->Release();
			return FALSE;
		}
	}
	if(lpszWallPaper)
	{
		pActiveDesktop->SetWallpaper(lpszWallPaper, 0);
		if(hr != S_OK)
		{
			pActiveDesktop->Release();
			return FALSE;
		}
	}
	pActiveDesktop->ApplyChanges(AD_APPLY_ALL);
		
	pActiveDesktop->Release();
	return TRUE;
}

INT_PTR CALLBACK MainDlgProc(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (message)
	{
	case WM_PAINT:
		{
			HDC				hDC;
			PAINTSTRUCT		ps;

			hDC = BeginPaint(hDlg, &ps);
			EndPaint(hDlg, &ps);
		}
		break;
	case WM_INITDIALOG:
		{
			HWND			hwndFocus = (HWND) wParam;
			LPARAM			lInitParam = lParam; 

			SetFocus(GetDlgItem(hDlg, IDC_BTN_QUIT));
			SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
		}
		break;
	case WM_COMMAND:
		{
			WORD			wNotifyCode = HIWORD(wParam);
			WORD			wID = LOWORD(wParam);
			HWND			hwndCtl = (HWND) lParam;

			switch(wID)
			{
			case IDC_BTN_REFRESH://刷新
				{
					TCHAR			szText[120];
					COLORREF		rgbColor;

					rgbColor = GetSysColor(COLOR_WINDOW);
					wsprintf(szText, _T("%d %d %d"), GetRValue(rgbColor), GetGValue(rgbColor), GetBValue(rgbColor));//0x%02X%02X%02X
					SetDlgItemText(hDlg, IDC_EDIT_COLOR, szText);

					rgbColor = GetSysColor(COLOR_BACKGROUND);
					wsprintf(szText, _T("%d %d %d"), GetRValue(rgbColor), GetGValue(rgbColor), GetBValue(rgbColor));
					SetDlgItemText(hDlg, IDC_EDIT_COLOR2, szText);
				}
				break;
			case IDC_BTN_PEAGREEN://窗口背景豆绿色
				{
					COLORREF		aRgbValues[] = {RGB(204, 232, 207)};
					INT				aElements[] = {COLOR_WINDOW};

					SetSysColors(1, aElements, aRgbValues);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
				}
				break;
			case IDC_BTN_DEFAULT://窗口背景默认色(白色)
				{
					COLORREF		aRgbValues[] = {RGB(255, 255, 255)};
					INT				aElements[] = {COLOR_WINDOW};

					SetSysColors(1, aElements, aRgbValues);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
				}
				break;
			case IDC_BTN_CUSTOM://自定义窗口背景色
				{
					COLORREF		aRgbCustom[16];
					COLORREF		aRgbValues[] = {RGB(255, 255, 255)};
					INT				aElements[] = {COLOR_WINDOW};

					if(ChooseColorDlg(hDlg, aRgbValues[0], aRgbCustom))
					{
						SetSysColors(1, aElements, aRgbValues);
						SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
					}
				}
				break;
			case IDC_BTN_PEAGREEN2://桌面背景豆绿色
				{
					COLORREF		aRgbValues[] = {RGB(204, 232, 207)};
					INT				aElements[] = {COLOR_BACKGROUND};
					WCHAR			szWallPaper[MAX_PATH];

					GetWallPaper(szWallPaper, NULL);//取得当前壁纸
					if(lstrcmpiW(szWallPaper, L""))
					{
						SetWallPaper(L"", NULL);//取消壁纸
					}

					SetSysColors(1, aElements, aRgbValues);
					SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
				}
				break;
			case IDC_BTN_CUSTOM2://桌面自定义背景颜色
				{
					COLORREF		aRgbCustom[16];
					COLORREF		aRgbValues[] = {RGB(153, 217, 234)};
					INT				aElements[] = {COLOR_BACKGROUND};
					WCHAR			szWallPaper[MAX_PATH];

					if(ChooseColorDlg(hDlg, aRgbValues[0], aRgbCustom))
					{
						GetWallPaper(szWallPaper, NULL);//取得当前壁纸
						if(lstrcmpiW(szWallPaper, L""))
						{
							SetWallPaper(L"", NULL);//取消壁纸
						}

						SetSysColors(1, aElements, aRgbValues);
						SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BTN_REFRESH, 0), 0);
					}
				}
				break;
			case IDC_BTN_SAVE://由于SetSysColors设置的颜色重启后就无效了，因此需要保存到注册表里面
				{
					TCHAR			szText[120];
					TCHAR			szColor[120];
					DWORD			cbValue;

					GetDlgItemText(hDlg, IDC_EDIT_COLOR, szText, sizeof(szText) / sizeof(TCHAR));
					cbValue = sizeof(szColor) / sizeof(TCHAR);
					ReadRegValue(HKEY_CURRENT_USER, _T("Control Panel\\Colors"), _T("Window"), (LPBYTE)szColor, cbValue);
					if(lstrcmpi(szText, szColor))
					{
						cbValue = sizeof(szColor) / sizeof(TCHAR);
						SetRegValue(HKEY_CURRENT_USER, _T("Control Panel\\Colors"), _T("Window"), (LPBYTE)szText, cbValue, REG_SZ);
					}
					
					GetDlgItemText(hDlg, IDC_EDIT_COLOR2, szText, sizeof(szText) / sizeof(TCHAR));
					cbValue = sizeof(szColor) / sizeof(TCHAR);
					ReadRegValue(HKEY_CURRENT_USER, _T("Control Panel\\Colors"), _T("Background"), (LPBYTE)szColor, cbValue);
					if(lstrcmpi(szText, szColor))
					{
						cbValue = sizeof(szColor) / sizeof(TCHAR);
						SetRegValue(HKEY_CURRENT_USER, _T("Control Panel\\Colors"), _T("Background"), (LPBYTE)szText, cbValue, REG_SZ);
					}
				}
				break;
			case IDC_BTN_QUIT://退出
				{
					SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
				break;
			}
		}
		break;
	case WM_CLOSE:
		{
			EndDialog(hDlg, (INT_PTR)lParam);
		}
		break;
	}

	return FALSE;
}

//主函数
int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow
	)
{
	g_hinst = hInstance;
			
	CoInitialize(0);

	InitCommonControls();
	DialogBox(hInstance, (LPCTSTR)IDD_MAIN, NULL, (DLGPROC)MainDlgProc); //创建控件

	CoUninitialize();

	return 1;
}