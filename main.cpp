#include <iostream>

#include "models.hpp"
#include "utils.hpp"
#include "imitation.hpp"

#define ITERATIONS 100

template <typename Imitation>
void Imitate(Imitation imitate, const std::string& data_file,
                                const std::string& output_file){
    Json::Value results;
    for(std::size_t i = 0; i < ITERATIONS; ++i){
        auto time = imitate(data_file);
        Json::Value run;
        run["iteration"] = std::to_string(i);
        run["time"] = std::to_string(time.count());
        results["results"].append(run);
        std::cout << "Time: " << time.count() << " ms" << std::endl;
    }
    utils::WriteFileCSV(results, output_file.c_str());
}

int main() {
    const std::string data_file = "data/data5_15_70_12.json";

    std::cout << "Easy algorithm solution: " << std::endl;
    Imitate(imitation::ImitateEasy, data_file, "results/res1.csv");

    std::cout << "Easy algorithm solution with AR: " << std::endl;
    Imitate(imitation::ImitateEasyWithAR, data_file, "results/res2.csv");

    std::cout << "\nIterative algorithm solution: " << std::endl;
    Imitate(imitation::ImitateIterative, data_file, "results/res3.csv");

    std::cout << "\nGreedy algorithm solution: " << std::endl;
    Imitate(imitation::ImitateGreedy, data_file, "results/res4.csv");

    std::cout << "\nHungarian with unions algorithm solution: " << std::endl;
    Imitate(imitation::ImitateHungarianUnions, data_file, "results/res5.csv");

    return 0;
}
