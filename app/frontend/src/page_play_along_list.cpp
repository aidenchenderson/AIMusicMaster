#include <ncurses.h>
#include <string>
#include <vector>
#include "ui_pages.hpp"

PageResult runPlayAlongListPage(WINDOW* win, const UIContext& ctx) {
    std::vector<std::string> tracks = {
        "Track 01 - Blues Jam",
        "Track 02 - Rock Groove",
        "Track 03 - Jazz Swing"
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

        for (size_t i = 0; i < tracks.size(); i++) {
            if ((int)i == highlighted) {
                wattron(win, A_REVERSE);
            }
            mvwprintw(win, listY + i * yWin / 7, listX, "[%s]", tracks[i].c_str());
            if ((int)i == highlighted) {
                wattroff(win, A_REVERSE);
            }
        }

        mvwprintw(win, yWin / 28, 2, "Play Along - Select a Track");
        mvwprintw(win, 21 * yWin / 28, 2, "** ENTER to choose, LEFT to go back **");

        wrefresh(win);
        input = wgetch(win);

        switch (input) {
            case KEY_UP:
                highlighted--;
                if (highlighted < 0) highlighted = (int)tracks.size() - 1;
                break;
            case KEY_DOWN:
                highlighted++;
                if (highlighted >= (int)tracks.size()) highlighted = 0;
                break;
            case 10:
            case KEY_ENTER: {
                UIContext nextCtx = ctx;
                nextCtx.selectedTrack = tracks[highlighted];
                nextCtx.playAlong = true;
                return {PageId::Summary, nextCtx};
            }
            case KEY_LEFT:
                return {PageId::MainMenu, ctx};
        }
    }
}
