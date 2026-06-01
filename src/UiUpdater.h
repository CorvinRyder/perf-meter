#include <windows.h>
#include "PdhQuerier.h"

#define WINDOW_SIZE_WIDTH 120
#define WINDOW_SIZE_HEIGHT 90
#define COUNTER_UPDATE_TIMER 706
#define COUNTER_UPDATE_FREQ 1000
#define BATTETY_LEVEL_UPDATE_TIMER 707
#define BATTERY_LEVEL_UPDATE_FREQ 30000

class UiUpdater {
	public:
		int InitialWindow(HINSTANCE hInstance);
		LRESULT WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	
	private:
		RECT windowSize;
		HWND hwnd;
		HBRUSH brush;
		HICON* icons;
		HFONT font;
		PdhQuerier querier;
		PerfCounter cachedData;
		void UpdateUI();
};
