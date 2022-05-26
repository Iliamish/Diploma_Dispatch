#include <iostream>

#include "models.hpp"
#include "imitation.hpp"


int main() {
    std::cout << "Easy algorithm solution: " << std::endl;
    auto time = imitation::ImitateEasy();
    std::cout << "Time: " << time.count() << " ms" << std::endl;

    std::cout << "Easy algorithm solution: " << std::endl;
    auto time = imitation::ImitateEasyWithAR("data/data5_15_70_12.json");
    std::cout << "Time: " << time.count() << " ms" << std::endl;

    std::cout << "\nIterative algorithm solution: " << std::endl;
    time = imitation::ImitateIterative();
    std::cout << "Time: " << time.count() << " ms" << std::endl;

    std::cout << "\nGreedy algorithm solution: " << std::endl;
    time = imitation::ImitateGreedy();
    std::cout << "Time: " << time.count() << " ms" << std::endl;
    return 0;
}
