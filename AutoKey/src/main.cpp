#include "win_common.h"

#include "main_window.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCmd) {
    return autokey::MainWindow::Run(instance, showCmd);
}
