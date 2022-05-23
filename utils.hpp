#pragma once

#include <jsoncpp/json/json.h>
#include <fstream>

namespace utils {

    Json::Value ParseFile(const char* inputFile);

}