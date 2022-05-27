#pragma once

#include <jsoncpp/json/json.h>
#include <fstream>

namespace utils {

    Json::Value ParseFile(const char* inputFile);

    void WriteFile(Json::Value value, const char* outputFile);

    void FillData();

}