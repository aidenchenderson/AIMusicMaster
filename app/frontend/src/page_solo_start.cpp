#include <ncurses.h>
#include <string>
#include <vector>
#include "ui_pages.hpp"

PageResult runSoloStartPage(WINDOW* win, const UIContext& ctx) {
    std::vector<std::string> options = {
        "Start",
        "Back"
    };

    int highlighted = 0;
    int input = 0;

    int yWin;
    int xWin;
    getmaxyx(win, yWin, xWin);

    while (true) {
        werase(win);
        box(win, '|', '-');

        int listY = 9 * yWin / 28;
        int listX = 4;

        for (size_t i = 0; i < options.size(); i++) {
            if ((int)i == highlighted) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, listY + i * yWin / 7, listX, "[%s]", options[i].c_str());
            if ((int)i == highlighted) {
                wattroff(win, A_REVERSE);
            }
        }

        mvwprintw(win, yWin / 28, 2, "Solo Playing");
        mvwprintw(win, 21 * yWin / 28, 2, "** ENTER to continue, LEFT to go back **");

        wrefresh(win);
        input = wgetch(win);

        switch (input) {
            case KEY_UP:
                highlighted--;
                if (highlighted < 0) highlighted = (int)options.size() - 1;
                break;
            case KEY_DOWN:
                highlighted++;
                if (highlighted >= (int)options.size()) highlighted = 0;
                break;
            case 10:
            case KEY_ENTER:
                if (highlighted == 0) {
                    UIContext nextCtx = ctx;
                    nextCtx.playAlong = false;
                    nextCtx.selectedTrack = "(solo session)";
                    return {PageId::Summary, nextCtx};
                }
                return {PageId::MainMenu, ctx};
            case KEY_LEFT:
                return {PageId::MainMenu, ctx};
        }
    }
}
