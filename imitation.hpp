#pragma once

#include <chrono>

#include "algorithm.hpp"

namespace imitation {

    std::chrono::milliseconds ImitateEasy(std::string path);

    std::chrono::milliseconds ImitateEasyWithAR(std::string path);

    std::chrono::milliseconds ImitateIterative(std::string path);

    std::chrono::milliseconds ImitateGreedy(std::string path);

    std::chrono::milliseconds ImitateHungarianUnions(std::string path);

}