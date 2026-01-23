#ifndef H_FEATURE_WRITER
#define H_FEATURE_WRITER

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>

class FeatureWriter {
    private:
        std::string file_path;
        std::ofstream file;
        bool is_open = false;

    public:
        FeatureWriter(const std::string& file_path);
        ~FeatureWriter();

        void open_file();
        void write_row(std::vector<float> &features);
        void close_file();
};

#endif // H_FEATURE_WRITER