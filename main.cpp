#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>

#include "models.hpp"
#include "utils.hpp"
#include "imitation.hpp"

#define ITERATIONS 50

template <typename Imitation>
void Imitate(Imitation imitate, const std::string& data_file,
                                const std::string& output_file){
    Json::Value results;
    auto res = imitate(data_file);
    Json::Value run;
    run["score"] = std::to_string(res.score);
    run["time"] = std::to_string(res.time.count());
    results["results"].append(run);
    std::cout << "Time: " << res.time.count() << " ms, Score: " << res.score << std::endl;
    utils::WriteFileCSV(results, output_file.c_str());
}

void RandomizeAcceptanceRate(const std::string& data_file, double mu, double sig) {
    std::random_device rd{};
    std::mt19937 gen{rd()};
 
    std::normal_distribution<> d{mu, sig};

    auto json_value = utils::ParseFile(data_file.c_str());
    for(auto& json_order : json_value["orders"]) {
        for(auto& json_candidate : json_order["candidates"]) {
            auto ar = std::round(d(gen));
            while(ar >= 100 || ar <= 50) {
                ar = std::round(d(gen));
            }
            json_candidate["ar"] = ar / 100;
        }
    }

    std::ofstream file_id;
    file_id.open(data_file.c_str());

    Json::StyledWriter styledWriter;
    file_id << styledWriter.write(json_value);

    file_id.close();
}

int main() {
    const std::string data_file = "data/data5_15_70_12.json";

    for(std::size_t i = 0; i < ITERATIONS; ++i){
        RandomizeAcceptanceRate(data_file, 70, 12);
        // std::cout << "Easy algorithm solution: " << std::endl;
        Imitate(imitation::ImitateEasy, data_file, "results/res1.csv");

        // std::cout << "Easy algorithm solution with AR: " << std::endl;
        Imitate(imitation::ImitateEasyWithAR, data_file, "results/res2.csv");

        // std::cout << "\nIterative algorithm solution: " << std::endl;
        Imitate(imitation::ImitateIterative, data_file, "results/res3.csv");

        // std::cout << "\nGreedy algorithm solution: " << std::endl;
        Imitate(imitation::ImitateGreedy, data_file, "results/res4.csv");

        // std::cout << "\nHungarian with unions algorithm solution: " << std::endl;
        Imitate(imitation::ImitateHungarianUnions, data_file, "results/res5.csv");
    }
    return 0;
}
