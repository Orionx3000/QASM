#include <windows.h>
#include <iostream>

WNDPROC OriginalWndProc = nullptr;

LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CLOSE || (uMsg == WM_SYSCOMMAND && (wParam & 0xfff0) == SC_CLOSE)) {
        ShowWindow(hwnd, SW_HIDE);
        return 0; // Prevent window from closing
    }
    
    if (uMsg == WM_APP + 1) {
        if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
        }
        return 0;
    }
    
    return CallWindowProc(OriginalWndProc, hwnd, uMsg, wParam, lParam);
}

int main() {
    std::cout << "Test compile OK!" << std::endl;
    return 0;
}
