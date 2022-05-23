#include <iostream>
#include <algorithm>
#include <random>
#include <thread>
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
                edge.acceptance_rate = json_candidate["ar"].asDouble();
                order.edges_to_contractors.push_back(edge);
            }
            graph.orders.push_back(order);
        }
        for(auto& candidate_id : candidates_set) {
            graph.contractors.push_back(models::Contractor{candidate_id});
        }
    }

    bool ProposeOrderToDriver(const models::Order& order, const models::Contractor& contractor) {
        auto it = std::find_if(order.edges_to_contractors.begin(),
                               order.edges_to_contractors.end(),
                               [id = contractor.id](const models::Edge& edge){
                                   return id == edge.to_id;
                               });
        if (it == order.edges_to_contractors.end()) {
            std::cerr << "Algorithm error. Founded contractor can`t take this order" << std::endl;
            return false;
        }
        static std::random_device rd{};
        static std::mt19937 gen{rd()};
        auto p = gen() % 100;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return it->acceptance_rate * 100 > p;
    }

    models::Order& GetOrderById(models::Graph& graph, std::string order_id) {
        auto order = std::find_if(graph.orders.begin(),
                                  graph.orders.end(),
                                  [order_id](const models::Order& value){
                                    return order_id == value.id;
                                  });
        if (order == graph.orders.end()) {
            std::cerr << "Algorithm error. No order with that id in graph" << std::endl;
            throw std::exception();
        }
        return (*order);
    }

    void RemoveEdge(models::Graph& graph,
                    std::string order_id, std::string contractor_id) {
        auto order = GetOrderById(graph, order_id);
        auto& edges = order.edges_to_contractors;
        edges.erase(std::remove_if(edges.begin(), edges.end(),
        [contractor_id](const models::Edge& value){
            return contractor_id == value.to_id;
        }), edges.end());
    }

    void RemoveContractorFromGraph(models::Graph& graph, std::string contractor_id) {
        std::vector<models::Order>& orders = graph.orders;
        std::vector<models::Contractor>& contractors = graph.contractors;
        for(auto& order : orders) {
            auto& edges = order.edges_to_contractors;
            edges.erase(std::remove_if(edges.begin(), edges.end(),
            [contractor_id](const models::Edge& value){
                return contractor_id == value.to_id;
            }), edges.end());
        }
        contractors.erase(std::remove_if(contractors.begin(), contractors.end(),
            [contractor_id](const models::Contractor& value){
                return contractor_id == value.id;
            }), contractors.end());
    }

    void RemoveOrderFromGraph(models::Graph& graph, std::string order_id) {
        std::vector<models::Order>& orders = graph.orders;
        orders.erase(std::remove_if(orders.begin(), orders.end(),
            [order_id](const models::Order& value){
                return order_id == value.id;
            }), orders.end());
    }

    void RemoveOrderAndContractorFromGraph(models::Graph& graph,
                                           std::string order_id, std::string contractor_id) {
        RemoveContractorFromGraph(graph, contractor_id);
        RemoveOrderFromGraph(graph, order_id);
    }
}

namespace imitation {

std::chrono::milliseconds ImitateEasy() {
    models::Graph graph;
    BuildData("data2.json", graph);

    std::size_t iteration{0};

    auto s_time = std::chrono::steady_clock::now();

    while (!graph.orders.empty())
    {
        std::cout << "* Iteration " << iteration << " *"<< std::endl;

        auto result = algorithm::SolveEasyHungarian(graph);

        std::vector<bool> propose_results(result.size());
        std::vector<std::thread> proposes;
        for (std::size_t i = 0; i < result.size(); ++i) {
            auto& pair = result[i];
            proposes.emplace_back(std::thread([&, i]{
                if(ProposeOrderToDriver(pair.first, pair.second)){
                    propose_results[i] = true;
                } else {
                    propose_results[i] = false;
                }
            }));
        }

        for(auto& propose : proposes){
                propose.join();
        }

        for (std::size_t i = 0; i < result.size(); ++i) {
            auto& pair = result[i];
            std::cout << pair.first.id << " -- " << pair.second.id << std::endl;
            if (propose_results[i]) {
                std::cout << "Accepted" << std::endl;
                RemoveOrderAndContractorFromGraph(graph, pair.first.id, pair.second.id);
            } else {
                std::cout << "Declined" << std::endl;
                RemoveEdge(graph, pair.first.id, pair.second.id);
            }
        }
        ++iteration;
    }

    auto elapsed_time = std::chrono::steady_clock::now() - s_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
}

std::chrono::milliseconds ImitateIterative() {
    models::Graph graph;
    BuildData("data2.json", graph);

    std::size_t iteration{0};
    auto s_time = std::chrono::steady_clock::now();

    while (!graph.orders.empty())
    {
        std::cout << "* Iteration " << iteration << " *"<< std::endl;

        auto result = algorithm::SolveHungarianUnions(graph);

        std::vector<std::thread> proposes;
        std::vector<std::vector<bool>> propose_results(result.size());
        for (std::size_t i = 0; i < result.size(); ++i) {
            auto& pair = result[i];
            propose_results[i] = (std::vector<bool>(pair.second.contractors.size()));
            for(std::size_t j = 0; j < pair.second.contractors.size(); ++j) {
                auto candidate = pair.second.contractors[j];
                auto order = GetOrderById(graph, pair.first.id);
                proposes.emplace_back(std::thread([=, &propose_results]{
                    if(ProposeOrderToDriver(order, candidate)){
                        propose_results[i][j] = true;
                    } else {
                        propose_results[i][j] = false;
                    }
                }));
            }
        }
        for(auto& propose : proposes){
                propose.join();
        }

        for (std::size_t i = 0; i < result.size(); ++i) {
            auto& pair = result[i];
            std::cout << "Order: " << pair.first.id << std::endl;
            for(std::size_t j = 0; j < pair.second.contractors.size(); ++j) {
                auto candidate = pair.second.contractors[j];
                std::cout << candidate.id;
                if (propose_results[i][j]) {
                    std::cout << " - Accepted" << std::endl;
                    RemoveContractorFromGraph(graph, candidate.id);
                } else {
                    std::cout << " - Declined" << std::endl;
                    RemoveEdge(graph, pair.first.id, candidate.id);
                }
            }
            bool any_accept =
                    std::any_of(std::begin(propose_results[i]), std::end(propose_results[i]),
                            [](bool i) { return i;});
            if (any_accept) {
                RemoveOrderFromGraph(graph, pair.first.id);
            }
            std::cout << std::endl;
        }
        ++iteration;
    }

    auto elapsed_time = std::chrono::steady_clock::now() - s_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
}

std::chrono::milliseconds ImitateGreedy() {
    models::Graph graph;
    BuildData("data2.json", graph);

    std::size_t iteration{0};
    auto s_time = std::chrono::steady_clock::now();

    while (!graph.orders.empty())
    {
        std::cout << "* Iteration " << iteration << " *"<< std::endl;

        auto result = algorithm::SolveGreedy(graph);

        std::vector<std::thread> proposes;
        std::vector<std::vector<bool>> propose_results(result.size());
        for (std::size_t i = 0; i < result.size(); ++i) {
            auto& pair = result[i];
            propose_results[i] = (std::vector<bool>(pair.second.contractors.size()));
            for(std::size_t j = 0; j < pair.second.contractors.size(); ++j) {
                auto candidate = pair.second.contractors[j];
                auto order = GetOrderById(graph, pair.first.id);
                proposes.emplace_back(std::thread([=, &propose_results]{
                    if(ProposeOrderToDriver(order, candidate)){
                        propose_results[i][j] = true;
                    } else {
                        propose_results[i][j] = false;
                    }
                }));
            }
        }

        for(auto& propose : proposes){
                propose.join();
        }

        for (std::size_t i = 0; i < result.size(); ++i) {
            auto& pair = result[i];
            std::cout << "Order: " << pair.first.id << std::endl;
            for(std::size_t j = 0; j < pair.second.contractors.size(); ++j) {
                auto candidate = pair.second.contractors[j];
                std::cout << candidate.id;
                if (propose_results[i][j]) {
                    std::cout << " - Accepted" << std::endl;
                    RemoveContractorFromGraph(graph, candidate.id);
                } else {
                    std::cout << " - Declined" << std::endl;
                    RemoveEdge(graph, pair.first.id, candidate.id);
                }
            }
            bool any_accept =
                    std::any_of(std::begin(propose_results[i]), std::end(propose_results[i]),
                            [](bool i) { return i;});
            if (any_accept) {
                RemoveOrderFromGraph(graph, pair.first.id);
            }
            std::cout << std::endl;
        }
        ++iteration;
    }

    auto elapsed_time = std::chrono::steady_clock::now() - s_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
}

}