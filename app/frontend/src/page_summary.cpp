#include <ncurses.h>
#include <string>
#include "ui_pages.hpp"

PageResult runSummaryPage(WINDOW* win, const UIContext& ctx) {
    int input = 0;

    int yWin;
    int xWin;
    getmaxyx(win, yWin, xWin);

    while (true) {
        werase(win);
        box(win, '|', '-');

        mvwprintw(win, yWin / 28, 2, "Session Summary");
        mvwprintw(win, 9 * yWin / 28, 2, "Mode: %s", ctx.playAlong ? "Play Along" : "Solo");
        mvwprintw(win, 11 * yWin / 28, 2, "Track: %s", ctx.selectedTrack.c_str());
        mvwprintw(win, 13 * yWin / 28, 2, "Device: %s", ctx.selectedDevice.c_str());
        mvwprintw(win, 21 * yWin / 28, 2, "** Press ENTER to return **");

        wrefresh(win);
        input = wgetch(win);
        if (input == 10 || input == KEY_ENTER) {
            return {PageId::MainMenu, ctx};
        }
    }
}
