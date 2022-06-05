#pragma once

#include <chrono>

#include "algorithm.hpp"
#include "models.hpp"

namespace imitation {

    models::ImitataionResult ImitateEasy(std::string path);

    models::ImitataionResult ImitateEasyWithAR(std::string path);

    models::ImitataionResult ImitateIterative(std::string path);

    models::ImitataionResult ImitateGreedy(std::string path);

    models::ImitataionResult ImitateHungarianUnions(std::string path);

}