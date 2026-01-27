#include <string>
#include <fstream>
#include <vector>
#include <stdio.h> 

#include "feature_writer.hpp"

FeatureWriter::FeatureWriter(const std::string& file_path) : file_path(file_path) {}

FeatureWriter::~FeatureWriter() {
    if (file.is_open()) {
        file.close();
    }
}

void FeatureWriter::open_file() {
    file.open(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open CSV file");
    }
    is_open = true;
}

void FeatureWriter::write_row(std::vector<float> &features){
    if (!is_open) {
        throw std::runtime_error("File not open");
    }
    for (int i = 0; i < static_cast<int>(features.size()); i++) {
        file << features[i];
        if (i < static_cast<int>(features.size()-1)) {
            file << ",";
        }
    }
    file << "\n";
}

void FeatureWriter::close_file(){
    if (is_open) {
        file.flush();
        file.close();
        is_open = false;
    }
}



