#include <iostream>
#include <ncurses/ncurses.h>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

int main(int argc, char** argv) {
    //device names
    vector<string> devices={
        "Device 1",
        "Device 2",
        "Device 3"
    };
    //buttons
    vector<string> buttons={
        "Start Playing",
        "Devices",
        "Exit"
    };
    int highlighted=0;
    int clicked=0;
    //ncurses window
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    //window coord-->window position
    int yMax;
    int xMax;
    getmaxyx(stdscr,yMax,xMax);
    WINDOW *win = newwin(9*yMax/14,xMax/2,yMax/4,xMax/4);
    keypad(win,TRUE);
    //window dim-->window elements position
    int input=0;
    int yWin;
    int xWin;
    getmaxyx(win,yWin,xWin);

    //selected device 
    string device="No device selected";

    while(input!='q'){
        werase(win);
        box(win,'|','-');
        //button coords
        int buttonY=9*yWin/28;
        int buttonX=4;
        //draw buttons
        for(int i=0;i<buttons.size();i++){
            if(i==highlighted){
                wattron(win,A_REVERSE);
            }
            //print button labels
            mvwprintw(win,buttonY+i*yWin/7,buttonX,"[%s]",buttons[i].c_str());

            if(i==highlighted){
                wattroff(win,A_REVERSE);
            }
        }
        //title
        mvwprintw(win,yWin/28,2,"AI-Music-Master");
        
        //instruction text
        mvwprintw(win,21*yWin/28,2,"** Use UP/DOWN to move ** click on ENTER to select**");

        //device information

        mvwprintw(win,24*yWin/28,2," Current device: %s",device.c_str());

        wrefresh(win);

        input=wgetch(win);

         //control keys
        switch(input){
        case KEY_UP:
            highlighted--;
            if(highlighted<0) highlighted=2;
            break;
            
        case KEY_DOWN:
            highlighted++;
            if(highlighted>=3) highlighted=0;
            break;
        case 10:
        case KEY_ENTER:
            clicked=highlighted;
            if(clicked==2) 
            input='q';

            //second window
            if(clicked==1){
                //second window 
                WINDOW *devwin=newwin(9*yMax/14,xMax/2,yMax/4,xMax/4);
                keypad(devwin,TRUE);
                int secInput=0;
                int secHighLighted=0;
                while(secInput!=KEY_ENTER && secInput!=KEY_LEFT && secInput!=KEY_RIGHT && secInput !=10){
                    werase(devwin);
                    box(devwin,'|','-');
                    for(int j=0;j<devices.size();j++){
                        if(j==secHighLighted){
                            wattron(devwin,A_REVERSE);
                        }
                        //print device labels on second window
                        mvwprintw(devwin,buttonY+j*yWin/7,buttonX,"[%s]",devices[j].c_str());
                        if(j==secHighLighted){
                            wattroff(devwin,A_REVERSE);
                        }
                    }

                    //second window instruction
                    mvwprintw(devwin,21*yWin/28,2,"**Pick a device or use LEFT/RIGHT to go back**");
                    //second window input 
                    wrefresh(devwin);
                    secInput=wgetch(devwin);

                    //second window controls
                    switch(secInput){
                    case KEY_UP:
                        secHighLighted--;
                        if(secHighLighted<0) secHighLighted=devices.size()-1;
                        break;
                    case KEY_DOWN:
                        secHighLighted++;
                        if(secHighLighted>=devices.size()) secHighLighted=0;
                        break;
                    case 10:
                    case KEY_ENTER:
                        device=devices[secHighLighted];
                        break;
                    case KEY_LEFT:
                        break;
                    case KEY_RIGHT:
                        break;
                }
                wrefresh(devwin);
            }
            delwin(devwin);
            
            }     
    }
        wrefresh(win);
    }
   
   
    //cleanup
    delwin(win);
    endwin();

    

    return 0;
}