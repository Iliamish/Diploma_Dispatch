#include <iostream>

#include "models.hpp"
#include "utils.hpp"
#include "imitation.hpp"

#define ITERATIONS 10

int main() {
    std::cout << "Easy algorithm solution: " << std::endl;
    Json::Value easy_results;
    for(std::size_t i = 0; i < ITERATIONS; ++i){
        auto time = imitation::ImitateEasy("data/data5_15_70_12.json");
        Json::Value run;
        run["iteration"] = std::to_string(i);
        run["time"] = std::to_string(time.count());
        easy_results["easy"].append(run);
        std::cout << "Time: " << time.count() << " ms" << std::endl;
    }
    utils::WriteFile(easy_results, "results/res1.json");

    std::cout << "Easy algorithm solution with AR: " << std::endl;
    Json::Value easy_ar_results;
    for(std::size_t i = 0; i < ITERATIONS; ++i){
        auto time = imitation::ImitateEasyWithAR("data/data5_15_70_12.json");
        Json::Value run;
        run["iteration"] = std::to_string(i);
        run["time"] = std::to_string(time.count());
        easy_ar_results["easy"].append(run);
        std::cout << "Time: " << time.count() << " ms" << std::endl;
    }
    utils::WriteFile(easy_ar_results, "results/res2.json");

    std::cout << "\nIterative algorithm solution: " << std::endl;
    Json::Value hungarian_results;
    for(std::size_t i = 0; i < ITERATIONS; ++i){
        auto time = imitation::ImitateIterative("data/data5_15_70_12.json");
        Json::Value run;
        run["iteration"] = std::to_string(i);
        run["time"] = std::to_string(time.count());
        hungarian_results["easy"].append(run);
        std::cout << "Time: " << time.count() << " ms" << std::endl;
    }
    utils::WriteFile(hungarian_results, "results/res3.json");

    std::cout << "\nGreedy algorithm solution: " << std::endl;
    Json::Value greedy_results;
    for(std::size_t i = 0; i < ITERATIONS; ++i){
        auto time = imitation::ImitateGreedy("data/data5_15_70_12.json");
        Json::Value run;
        run["iteration"] = std::to_string(i);
        run["time"] = std::to_string(time.count());
        greedy_results["easy"].append(run);
        std::cout << "Time: " << time.count() << " ms" << std::endl;
    }
    utils::WriteFile(greedy_results, "results/res4.json");

    return 0;
}
