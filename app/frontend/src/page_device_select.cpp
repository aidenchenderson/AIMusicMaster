#include <ncurses.h>
#include <string>
#include <vector>
#include "ui_pages.hpp"

PageResult runDeviceSelectPage(WINDOW* win, const UIContext& ctx) {
    std::vector<std::string> devices = {
        "Device 1",
        "Device 2",
        "Device 3"
    };

    int highlighted = 0;
    int input = 0;

    int yWin;
    int xWin;
    getmaxyx(win, yWin, xWin);

    while (true) {
        werase(win);
        box(win, '|', '-');

        int buttonY = 9 * yWin / 28;
        int buttonX = 4;

        for (size_t i = 0; i < devices.size(); i++) {
            if ((int)i == highlighted) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, buttonY + i * yWin / 7, buttonX, "[%s]", devices[i].c_str());
            if ((int)i == highlighted) {
                wattroff(win, A_REVERSE);
            }
        }

        mvwprintw(win, yWin / 28, 2, "Select Device");
        mvwprintw(win, 21 * yWin / 28, 2, "** ENTER to select, LEFT to go back **");

        wrefresh(win);
        input = wgetch(win);

        switch (input) {
            case KEY_UP:
                highlighted--;
                if (highlighted < 0) highlighted = (int)devices.size() - 1;
                break;
            case KEY_DOWN:
                highlighted++;
                if (highlighted >= (int)devices.size()) highlighted = 0;
                break;
            case 10:
            case KEY_ENTER: {
                UIContext nextCtx = ctx;
                nextCtx.selectedDevice = devices[highlighted];
                return {PageId::MainMenu, nextCtx};
            }
            case KEY_LEFT:
                return {PageId::MainMenu, ctx};
        }
    }
}
