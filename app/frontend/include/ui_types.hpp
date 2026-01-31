#ifndef UI_TYPES_HPP
#define UI_TYPES_HPP

#include <string>
#include <vector>

struct Note {
    int beat;
    int string;
    int fret;
};

struct Bar {
    int number;
    std::vector<Note> beats;
};

struct Track {
    std::string title;
    int bpm;
    std::string timeSignature;
    std::vector<Bar> bars;
};

enum class PageId {
    MainMenu,
    DeviceSelect,
    PlayAlongList,
    PlayAlongPlayer,
    SoloStart,
    Summary,
    Exit
};

struct UIContext {
    std::string selectedDevice;
    std::string selectedTrack;
    std::string trackFilename;
    Track trackData;
    bool playAlong = false;
};

struct PageResult {
    PageId nextPage;
    UIContext context;
};

#endif // UI_TYPES_HPP
