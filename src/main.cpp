#include <iostream>
#include "MainWindow.h"

using namespace std;
using namespace kc1fsz;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int nCmdShow) {

    cout << "hello izzy" << endl;

    MainWindow::reg(hInstance);

    MainWindow w(hInstance);
    w.show(nCmdShow);

    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
