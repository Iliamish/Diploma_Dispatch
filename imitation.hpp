#pragma once

#include <chrono>

#include "algorithm.hpp"

namespace imitation {

    std::chrono::milliseconds ImitateEasy();

    std::chrono::milliseconds ImitateIterative();

    std::chrono::milliseconds ImitateGreedy();

}