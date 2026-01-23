#include <string>
#include <fstream>
#include <vector>
#include <stdio.h> 

#include "feature_writer.hpp"

FeatureWriter::FeatureWriter(const std::string& file_path) {
    // Initialize file path
    this->file_path = file_path;
}

FeatureWriter::~FeatureWriter() {
    // Close file if open
    if(file.is_open()){
        file.close();
    }
}

void FeatureWriter::open_file() {
    // Open file, throw error if fails
    file.open(this->file_path);
    if(!file.is_open()){
        throw std::runtime_error("Failed to open CSV file");
    }
    is_open = true;
}

void FeatureWriter::write_row(std::vector<float> &features){
    if(!is_open){
        throw std::runtime_error("File not open");
    }
    // Write features in separate collumns of a new row
    for(int i = 0; i<features.size(); i++){
        file << features[i];
        if(i<features.size()-1){
            file << ",";
        }
    }
    file << "\n";
}

void FeatureWriter::close_file(){
    // Flush file and close
    if(is_open){
        file.flush();
        file.close();
        is_open = false;
    }
}



