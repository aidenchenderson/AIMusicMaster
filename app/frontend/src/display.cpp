#include <ncurses.h>
#include <string>
#include "ui_controller.hpp"
#include "ui_pages.hpp"
#include "ui_types.hpp"

std::string runMainUI() {
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    int yMax;
    int xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW *win = newwin(9 * yMax / 14, xMax / 2, yMax / 4, xMax / 4);
    keypad(win, TRUE);

    UIContext ctx;
    ctx.selectedDevice = "No device selected";

    PageId current = PageId::MainMenu;
    while (current != PageId::Exit) {
        PageResult result;
        switch (current) {
            case PageId::MainMenu:
                result = runMainMenuPage(win, ctx);
                break;
            case PageId::DeviceSelect:
                result = runDeviceSelectPage(win, ctx);
                break;
            case PageId::PlayAlongList:
                result = runPlayAlongListPage(win, ctx);
                break;
            case PageId::PlayAlongPlayer:
                result = runPlayAlongPlayerPage(win, ctx);
                break;
            case PageId::SoloStart:
                result = runSoloStartPage(win, ctx);
                break;
            case PageId::Summary:
                result = runSummaryPage(win, ctx);
                break;
            case PageId::Exit:
                result = {PageId::Exit, ctx};
                break;
        }
        ctx = result.context;
        current = result.nextPage;
    }

    delwin(win);
    endwin();

    return ctx.selectedDevice;
}
