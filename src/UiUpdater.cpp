#include <string>
#include "UiUpdater.h"
#include "PdhQuerier.h"

#define WIDGET_HEIGHT 30
#define TEXT_SIZE 20
#define ICON_SIZE 24
#define TEXT_PADDING_TOP_BTM ((WIDGET_HEIGHT - TEXT_SIZE)/2)
#define ICON_PADDING_TOP_BTM ((WIDGET_HEIGHT - ICON_SIZE)/2)
#define TEXT_LEFT (ICON_PADDING_TOP_BTM + ICON_SIZE + 6)

void UiUpdater::UpdateUI(){
	char textBuff[128] = {0};
	
	PAINTSTRUCT paintInfo = {0};
	HDC drawDc = BeginPaint(hwnd, &paintInfo);
	
	SelectObject(drawDc, font);
	
	RECT textRect = {TEXT_LEFT, TEXT_PADDING_TOP_BTM, WINDOW_SIZE_WIDTH, TEXT_PADDING_TOP_BTM + TEXT_SIZE};
	RECT iconRect = {ICON_PADDING_TOP_BTM, ICON_PADDING_TOP_BTM, 0, 0};

	DrawIconEx(drawDc, iconRect.left, iconRect.top, icons[0], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
    if(cachedData.status==0){
    	snprintf(textBuff, 128, "%.0f%%", cachedData.cpuUsage);
    	DrawText(drawDc, (LPCTSTR) textBuff, -1, &textRect, DT_LEFT);
	}else{
		DrawText(drawDc, (LPCTSTR) "ERROR", -1, &textRect, DT_LEFT);
	}
	
	iconRect.top+=WIDGET_HEIGHT;
	textRect.top+=WIDGET_HEIGHT;
	textRect.bottom+=WIDGET_HEIGHT;
	DrawIconEx(drawDc, iconRect.left, iconRect.top, icons[1], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
    snprintf(textBuff, 128, "%d%%", cachedData.memUsage);
	DrawText(drawDc, (LPCTSTR) textBuff, -1, &textRect, DT_LEFT);
	
	iconRect.top+=WIDGET_HEIGHT;
	textRect.top+=WIDGET_HEIGHT;
	textRect.bottom+=WIDGET_HEIGHT;
	if(cachedData.powerBatteryLv==-1){
		DrawIconEx(drawDc, iconRect.left, iconRect.top, icons[4], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
		DrawText(drawDc, (LPCTSTR) "AC Power", -1, &textRect, DT_LEFT);			
	}else{
		DrawIconEx(drawDc, iconRect.left, iconRect.top, icons[cachedData.powerAcStatus?2:3], ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL);
		snprintf(textBuff, 128, "%d%%", cachedData.powerBatteryLv);
		DrawText(drawDc, (LPCTSTR) textBuff, -1, &textRect, DT_LEFT);			
	}
	
	RECT rect = {0, WIDGET_HEIGHT - 3, 0, WIDGET_HEIGHT - 1};
	
	rect.right=WINDOW_SIZE_WIDTH*((float)cachedData.cpuUsage/100);
	FillRect(drawDc,&rect,brush);
	
	rect.top+=WIDGET_HEIGHT;
	rect.bottom+=WIDGET_HEIGHT;
	rect.right=WINDOW_SIZE_WIDTH*((float)cachedData.memUsage/100);
	FillRect(drawDc,&rect,brush);	
	
	if(!cachedData.powerAcStatus){
		rect.top+=WIDGET_HEIGHT;
		rect.bottom+=WIDGET_HEIGHT;
		rect.right=WINDOW_SIZE_WIDTH*((float)cachedData.powerBatteryLv/100);
		FillRect(drawDc,&rect,brush);
	}

	EndPaint(hwnd, &paintInfo);
}

LRESULT UiUpdater::WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
	switch(Message) {
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			KillTimer(hwnd, COUNTER_UPDATE_TIMER);
			KillTimer(hwnd, BATTETY_LEVEL_UPDATE_TIMER);
			DeleteObject(brush);
			DeleteObject(font);
			for(int i=0;i<5;i++){
				DestroyIcon(icons[i]);
			}
			delete[] icons;
			PostQuitMessage(0);
			break;
		}
		case WM_PAINT:
			UpdateUI();
			break;
		case WM_TIMER:
			if(wParam == BATTETY_LEVEL_UPDATE_TIMER){
				querier.UpdateBatteryNextTick();
			}else if(wParam==COUNTER_UPDATE_TIMER){
				cachedData = {0};
				querier.GetCounters(&cachedData);
				InvalidateRect(hwnd, NULL, true);
			}
			break;
        case WM_DISPLAYCHANGE:
        	SetWindowPos(hwnd,HWND_TOPMOST,(lParam & 0xFFFF)-180,(lParam >> 16)-80,windowSize.right - windowSize.left,windowSize.bottom - windowSize.top,SWP_SHOWWINDOW);
        	break;
        case WM_POWERBROADCAST:
        	querier.UpdateBatteryNextTick();
        	return true;
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

int UiUpdater::InitialWindow(HINSTANCE hInstance){
	windowSize = {0, 0, WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT};
	AdjustWindowRectEx(&windowSize, WS_VISIBLE|WS_CAPTION, FALSE, WS_EX_TOPMOST|WS_EX_TOOLWINDOW);
	cachedData = {0};
	
	MSG msg; /* A temporary location for all messages */
	
	hwnd = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,"WindowClass","Performance Meter",WS_VISIBLE|WS_CAPTION,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		windowSize.right - windowSize.left, /* width */
		windowSize.bottom - windowSize.top, /* height */
		NULL,NULL,hInstance,NULL);
		
	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	
	brush=CreateSolidBrush(RGB(0,128,128));

	querier.SetupCounters();
	SetTimer(hwnd, COUNTER_UPDATE_TIMER, COUNTER_UPDATE_FREQ, NULL);
	SetTimer(hwnd, BATTETY_LEVEL_UPDATE_TIMER, BATTERY_LEVEL_UPDATE_FREQ, NULL);
	
	unsigned int piconid;
	icons = new HICON[5];
	PrivateExtractIcons("SHELL32.dll",15 ,32, 32, &icons[0], &piconid, 1, 0); //CPU
	PrivateExtractIcons("SHELL32.dll",12 ,32, 32, &icons[1], &piconid, 1, 0); //Memory
	PrivateExtractIcons("powercpl.dll",0 ,32, 32, &icons[2], &piconid, 1, 0); //Battery - Plugged In
	PrivateExtractIcons("powercpl.dll",1 ,32, 32, &icons[3], &piconid, 1, 0); //On Battery
	PrivateExtractIcons("powercpl.dll",2 ,32, 32, &icons[4], &piconid, 1, 0); //On AC
	
	font=CreateFont(
		TEXT_SIZE,0,0,0,700,
		0,0,0,ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY,FF_DONTCARE,
		"Consolas"
	);
	
	querier.UpdateBatteryNextTick();
	
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}
