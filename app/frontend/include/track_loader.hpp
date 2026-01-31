#ifndef TRACK_LOADER_HPP
#define TRACK_LOADER_HPP

#include <string>
#include "ui_types.hpp"

// Load track from JSON file
bool loadTrack(const std::string& filename, Track& track);

#endif // TRACK_LOADER_HPP
