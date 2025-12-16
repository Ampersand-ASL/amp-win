/**
 * Copyright (C) 2025, Bruce MacKinnon KC1FSZ
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#ifndef UNICODE
#define UNICODE
#endif 

#include <string>
#include <windows.h>

#include "kc1fsz-tools/threadsafequeue.h"

#include "amp-thread.h"

namespace kc1fsz {

class Log;

class MainWindowEvents {
public:
    virtual void connect(const std::string& nodeNumber) = 0;
    virtual void disconnectAll() = 0;
};

class MainWindow {
public:

    static void reg(HINSTANCE hInstance);

    MainWindow(HINSTANCE hInstance, Log& log, const char* localNodeNumber,
        threadsafequeue<Request>& msgQueue);
    ~MainWindow();
    void show(int nCmdShow);

private:

    static LRESULT CALLBACK _windProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _msg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


    Log& _log;
    HWND _hwnd;
    HWND _hEditNode;
    HBRUSH _whiteBrush;
    std::string _localNodeNumber;
    threadsafequeue<Request>& _msgQueue;
    bool _pttToggle = false;

    HWND _hPttButton;
};

}