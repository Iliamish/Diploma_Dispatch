#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <cmath>

#include "utils.hpp"


namespace utils {

    Json::Value ParseFile(const char* inputFile) {
        Json::Value val;
        std::ifstream ifs(inputFile);
        Json::Reader reader;
        reader.parse(ifs, val);
        return val;
    }

    void WriteFile(Json::Value value, const char* outputFile) {
        std::ofstream myfile(outputFile);
        Json::StyledWriter styledWriter;
        myfile << styledWriter.write(value);
        myfile.close();
    }

    void WriteFileCSV(Json::Value value, const char* outputFile) {
        std::ofstream myfile(outputFile);
        for(auto val : value["results"]){
            myfile << std::stoi(val["time"].asString()) << "\n";
        }
        myfile.close();
    }

    void FillData() {
        std::vector<std::string> drivers {
            "0515a586277747af94e55d027a2a640c_eab1b2c65d0513d1df7370f26cf87a62",
            "7f71e1b5eae640b083b4250ad57fc52a_3e7c8a064cc9da55fe357485794c63a1",
            "550083f9dc574f728b20cacd08549238_7d62434b385ce3561cbf082f20933d4b",
            "a7d726b045774b0abb864064fa975877_7ede917467c542b6bd7f7b71222f5dd5",
            "115c501601de43418a701d359a0932ce_a9ff21e644a43c8568847adc5e52b2a4",
            "017f1842b1764b60b2a9c622dc640d8e_e463c54e7f4a4e8a8d0a19f7a472ca6a",
            "4a0e7ef7e9ee401daf17a3d16c166756_f04a64b8db204f94aa7d4ad783f6b8d6",
            "d2dea28cd0244aad845ca1541d2c9dbe_81c201023a963e154b6896bbfc03dac7",
            "b56aad316b9047c38d952cc91a113fb6_499bd8d6e737c6d6817598aee6b9f28e",
            "73c9572289ed4afcba87689c72c6b3ac_e6dbaa5f1488590cddce412e0fef657a",
            "4ef8b56cbd7d442e89fbdc51ce4678f8_e9a10fe5ba3217a5f5e22c59523f0337",
            "6b7644a8ae0b44359833b6b824dd7757_e7d7749866e2460bb2f4a2dd2bed38ee",
            "ed493a841e19472db5319b8a146ef512_239bffb70015c587bada0530205c5324",
            "af3e25736e8d44099559eefaaa1bc789_51a62291cc964aa6acf5850fd1912e3f",
            "01856312487c4465b9fbeaad5dfed477_e43102c8222102d902f0ae93eb316290",
            "f069835e915e40ebae701ec65e5afc47_4d3bd6da03607edc1c42ed53d87b1444",
            "9e4904327b14426f8c1076c72992c057_b6789f5c639b208832916ddb6bce3041",
            "6ef85b43ed3748b68187a558b727b136_634deb73829f5b6edad9eaf5f2b20135",
            "3997096520c74f18b56f74dba9f0b33b_dc0a89bcad984e05b1e4e669ae60f23e",
            "a7d162c788d4414db852152da0ae2af6_4b72fa8b3d5346aca18f55e449baf53b"
        };

        std::random_device rd{};
        std::mt19937 gen{rd()};
    
        std::normal_distribution<> d{70,12};

        auto json_value = utils::ParseFile("data/data3.json");
        for(auto& json_order : json_value["orders"]) {

            std::size_t counter = 0;
            for(auto& json_candidate : json_order["candidates"]) {
                if(counter < 20) {
                    json_candidate["id"] = drivers[counter++];
                }
                json_candidate.removeMember("rd");
                json_candidate.removeMember("rt");
                json_candidate.removeMember("winner");
                json_candidate.removeMember("winner_applied");
                auto ar = std::round(d(gen));
                while(ar >= 100) {
                    ar = std::round(d(gen));
                }
                json_candidate["ar"] = ar / 100;

            }

            std::cout << "Order: " << json_order["id"].asString() << " candidates: "
                << json_order["candidates"].size() << std::endl;
        }

        std::ofstream file_id;
        file_id.open("data/data5_15_70_12.json");

        Json::StyledWriter styledWriter;
        file_id << styledWriter.write(json_value);

        file_id.close();
    }

}