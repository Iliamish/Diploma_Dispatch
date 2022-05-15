#include <iostream>
#include <stdlib.h>

#include "imitation.hpp"
#include "models.hpp"
#include "utils.hpp"


namespace {
    void BuildData(std::string path, models::Graph& graph) {
        auto json_value = utils::ParseFile(path.c_str());
        std::unordered_set<std::string> candidates_set;
        for(auto json_order : json_value["orders"]) {
            models::Order order;
            order.id = json_order["id"].asString();
            for(auto json_candidate : json_order["candidates"]) {
                models::Edge edge;
                candidates_set.emplace(json_candidate["id"].asString());
                edge.to_id = json_candidate["id"].asString();
                edge.weight = json_candidate["score"].asInt();
                order.edges_to_contractors.push_back(edge);
            }
            graph.orders.push_back(order);
        }
        for(auto& candidate_id : candidates_set) {
            graph.contractors.push_back(models::Contractor{candidate_id});
        }
    }
}

namespace imitation {

    std::chrono::milliseconds ImitateEasy() {
        models::Graph graph;
        BuildData("data1.json", graph);

        auto s_time = std::chrono::steady_clock::now();
        auto result = algorithm::SolveEasyHungarian(graph);
        auto elapsed_time = std::chrono::steady_clock::now() - s_time;

        for (auto pair : result)
        {
            std::cout << pair.first.id << " -- " << pair.second.id << std::endl;
        }
        
        return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    }

    std::chrono::milliseconds ImitateIterative() {
        models::Graph graph;
        BuildData("data1.json", graph);

        auto s_time = std::chrono::steady_clock::now();
        auto result = algorithm::SolveHungarianUnions(graph);
        auto elapsed_time = std::chrono::steady_clock::now() - s_time;

        for (auto pair : result)
        {
            for(auto cand : pair.second.contractors)
                std::cout << pair.first.id << " -- " << cand.id << std::endl;

            std::cout << "\n" << std::endl;
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    }

    std::chrono::milliseconds ImitateGreedy() {
        models::Graph graph;
        BuildData("data1.json", graph);

        auto s_time = std::chrono::steady_clock::now();
        auto result = algorithm::SolveGreedy(graph);
        auto elapsed_time = std::chrono::steady_clock::now() - s_time;

        for (auto pair : result)
        {
            for(auto cand : pair.second.contractors)
                std::cout << pair.first.id << " -- " << cand.id << std::endl;

            std::cout << "\n" << std::endl;
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    }

}