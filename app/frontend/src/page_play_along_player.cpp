#include <ncurses.h>
#include <string>
#include <chrono>
#include <thread>
#include "ui_pages.hpp"

void renderTabBar(WINDOW* win, const Bar& bar, int startX, int startY, bool isLastBar = false) {
    const char* strings[] = {"e", "B", "G", "D", "A", "E"};
    
    // Draw each string line
    for (int i = 0; i < 6; i++) {
        mvwprintw(win, startY + i, startX, "%s----|----|----|----", strings[i]);
    }
    
    // Place notes on the tab
    for (const Note& note : bar.beats) {
        int stringIdx = 6 - note.string; // Convert string number to index (1-6 -> 5-0)
        int xPos = startX + 2 + (note.beat - 1) * 5; // Position based on beat (1-4)
        
        // Draw the fret number
        if (note.fret < 10) {
            mvwprintw(win, startY + stringIdx, xPos, "%d", note.fret);
        } else {
            mvwprintw(win, startY + stringIdx, xPos, "%d", note.fret);
        }
    }
}

void renderEndBar(WINDOW* win, int startX, int startY) {
    const char* strings[] = {"e", "B", "G", "D", "A", "E"};
    
    // Draw each string line
    for (int i = 0; i < 6; i++) {
        mvwprintw(win, startY + i, startX, "%s----|----|----|----", strings[i]);
    }
    
    // Draw "END" in ASCII art style in the middle of the bar
    // E
    mvwprintw(win, startY + 1, startX + 7, "|--");
    mvwprintw(win, startY + 2, startX + 7, "|_ ");
    mvwprintw(win, startY + 3, startX + 7, "|__");
    
    // N
    mvwprintw(win, startY + 1, startX + 11, "|\\ ");
    mvwprintw(win, startY + 2, startX + 11, "| \\");
    mvwprintw(win, startY + 3, startX + 11, "|  ");
    
    // D
    mvwprintw(win, startY + 1, startX + 15, "|\\");
    mvwprintw(win, startY + 2, startX + 15, "| ");
    mvwprintw(win, startY + 3, startX + 15, "|/");
}

PageResult runPlayAlongPlayerPage(WINDOW* win, const UIContext& ctx) {
    int yWin, xWin;
    getmaxyx(win, yWin, xWin);
    (void)yWin; // Suppress unused variable warning
    (void)xWin;
    
    int currentBar = -2; // Start at -2 to allow 8 beat count-in (2 bars of 4 beats)
    int currentBeat = 1;
    bool playing = true;
    
    // Calculate beat duration in milliseconds
    int beatDurationMs = 60000 / ctx.trackData.bpm;
    
    auto lastBeatTime = std::chrono::steady_clock::now();
    
    while (playing) {
        // Check for input (non-blocking)
        nodelay(win, TRUE);
        int input = wgetch(win);
        nodelay(win, FALSE);
        
        if (input == 'q' || input == 27) { // q or ESC
            return {PageId::Summary, ctx};
        }
        
        werase(win);
        box(win, '|', '-');
        
        // Title and instructions with count-in/bar info inline
        if (currentBar < 0) {
            // Count-in only for bars -2 and -1 (8 beats total)
            int totalCountIn = (currentBar + 2) * 4 + currentBeat;
            mvwprintw(win, 1, 2, "%s - %d BPM | Count-in: %d/8 | Press 'q' or ESC to finish", 
                      ctx.trackData.title.c_str(), ctx.trackData.bpm, totalCountIn);
        } else {
            // Show bar number for actual track
            mvwprintw(win, 1, 2, "%s - %d BPM | Bar %d/%d | Press 'q' or ESC to finish", 
                      ctx.trackData.title.c_str(), ctx.trackData.bpm, 
                      currentBar + 1, (int)ctx.trackData.bars.size());
        }
        
        int tabStartY = 4;
        int tabStartX = 4;
        int barSpacing = 22;
        
        // Calculate which bars to display (always show 3 bars)
        // Display: prev bar (left), current bar with indicator (middle), next bar (right)
        int displayBar1 = currentBar - 1;
        int displayBar2 = currentBar;
        int displayBar3 = currentBar + 1;
        
        // Draw beat indicator over the current bar (middle position)
        int indicatorX = tabStartX + barSpacing + 2 + (currentBeat - 1) * 5;
        mvwprintw(win, tabStartY - 1, indicatorX, "^");
        
        // Draw first bar (left)
        if (displayBar1 >= 0 && displayBar1 < (int)ctx.trackData.bars.size()) {
            renderTabBar(win, ctx.trackData.bars[displayBar1], tabStartX, tabStartY);
        } else if (displayBar1 == (int)ctx.trackData.bars.size()) {
            // Show END bar after last track bar
            renderEndBar(win, tabStartX, tabStartY);
        } else {
            // Draw empty tab for count-in
            const char* strings[] = {"e", "B", "G", "D", "A", "E"};
            for (int i = 0; i < 6; i++) {
                mvwprintw(win, tabStartY + i, tabStartX, "%s----|----|----|----", strings[i]);
            }
        }
        
        // Draw second bar (middle - current)
        if (displayBar2 >= 0 && displayBar2 < (int)ctx.trackData.bars.size()) {
            renderTabBar(win, ctx.trackData.bars[displayBar2], tabStartX + barSpacing, tabStartY);
        } else if (displayBar2 == (int)ctx.trackData.bars.size()) {
            // Show END bar after last track bar
            renderEndBar(win, tabStartX + barSpacing, tabStartY);
        } else {
            // Draw empty tab for count-in
            const char* strings[] = {"e", "B", "G", "D", "A", "E"};
            for (int i = 0; i < 6; i++) {
                mvwprintw(win, tabStartY + i, tabStartX + barSpacing, "%s----|----|----|----", strings[i]);
            }
        }
        
        // Draw third bar (right)
        if (displayBar3 >= 0 && displayBar3 < (int)ctx.trackData.bars.size()) {
            renderTabBar(win, ctx.trackData.bars[displayBar3], tabStartX + barSpacing * 2, tabStartY);
        } else if (displayBar3 == (int)ctx.trackData.bars.size()) {
            // Show END bar after last track bar
            renderEndBar(win, tabStartX + barSpacing * 2, tabStartY);
        } else {
            // Draw empty tab
            const char* strings[] = {"e", "B", "G", "D", "A", "E"};
            for (int i = 0; i < 6; i++) {
                mvwprintw(win, tabStartY + i, tabStartX + barSpacing * 2, "%s----|----|----|----", strings[i]);
            }
        }
        
        wrefresh(win);
        
        // Check if we need to advance the beat
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBeatTime).count();
        
        if (elapsed >= beatDurationMs) {
            currentBeat++;
            if (currentBeat > 4) {
                currentBeat = 1;
                currentBar++;
                
                // Check if song is finished
                if (currentBar >= (int)ctx.trackData.bars.size()) {
                    playing = false;
                }
            }
            lastBeatTime = now;
        }
        
        // Small delay to not hog CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return {PageId::Summary, ctx};
}
