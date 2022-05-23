#include "utils.hpp"


namespace utils {

    Json::Value ParseFile(const char* inputFile) {
        Json::Value val;
        std::ifstream ifs(inputFile);
        Json::Reader reader;
        reader.parse(ifs, val);
        return val;
    }

}