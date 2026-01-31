#ifndef UI_TYPES_HPP
#define UI_TYPES_HPP

#include <string>

enum class PageId {
    MainMenu,
    DeviceSelect,
    PlayAlongList,
    SoloStart,
    Summary,
    Exit
};

struct UIContext {
    std::string selectedDevice;
    std::string selectedTrack;
    bool playAlong = false;
};

struct PageResult {
    PageId nextPage;
    UIContext context;
};

#endif // UI_TYPES_HPP
