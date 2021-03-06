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
        auto wait_time =  std::chrono::milliseconds(5000 + gen() % 5000);
        std::chrono::milliseconds kNetworkDelay {10 + gen() % 40};
        std::this_thread::sleep_for(2 * (kNetworkDelay) + wait_time);
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

    std::optional<models::Edge> FindEdge(const models::Order& order, std::string contractor_id) {
        auto& edges = order.edges_to_contractors;
        auto it = std::find_if(edges.begin(), edges.end(),
        [contractor_id](const models::Edge& value){
            return contractor_id == value.to_id;
        });
        if(it != edges.end()){
            return (*it);
        }else{
            return std::nullopt;
        }
    }

    void RemoveEdge(models::Graph& graph,
                    std::string order_id, std::string contractor_id) {
        auto& order = GetOrderById(graph, order_id);
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

    template <typename Algo>
    std::size_t ImitatePairs(models::Graph& graph, Algo algo, bool with_ar){
        std::size_t iteration{0};
        std::size_t score = 0;
        while (!graph.orders.empty())
        {
            std::cout << "* Iteration " << iteration << " *"<< std::endl;

            auto result = algo(graph, with_ar);

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
                    auto order = GetOrderById(graph, pair.first.id);
                    auto edge = FindEdge(order, pair.second.id);
                    if(edge){
                        score += edge->weight;
                    }else{
                        throw std::exception();
                    }
                    RemoveOrderAndContractorFromGraph(graph, pair.first.id, pair.second.id);
                } else {
                    std::cout << "Declined" << std::endl;
                    RemoveEdge(graph, pair.first.id, pair.second.id);
                }
            }
            ++iteration;
        }
        return score;
    }

    template <typename Algo>
    std::size_t ImitateWithUnions(models::Graph& graph, Algo algo){
        std::size_t iteration{0};
        std::size_t score = 0;
        while (!graph.orders.empty())
        {
            std::cout << "* Iteration " << iteration << " *"<< std::endl;

            auto result = algo(graph);

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
                bool any_accept = false;
                for(std::size_t j = 0; j < pair.second.contractors.size(); ++j) {
                    auto candidate = pair.second.contractors[j];
                    std::cout << candidate.id;
                    if (propose_results[i][j]) {
                        std::cout << " - Accepted" << std::endl;
                        if(!any_accept){
                            auto order = GetOrderById(graph, pair.first.id);
                            auto edge = FindEdge(order, candidate.id);
                            if(edge){
                                score += edge->weight;
                            }else{
                                throw std::exception();
                            }
                            RemoveContractorFromGraph(graph, candidate.id);
                            any_accept = true;
                        }
                    } else {
                        std::cout << " - Declined" << std::endl;
                        RemoveEdge(graph, pair.first.id, candidate.id);
                    }
                }

                if (any_accept) {
                    RemoveOrderFromGraph(graph, pair.first.id);
                }
                std::cout << std::endl;
            }
            ++iteration;
        }
        return score;
    }
}

namespace imitation {

models::ImitataionResult ImitateEasy(std::string path) {
    models::Graph graph;
    BuildData(path, graph);
    models::ImitataionResult im_res;

    auto s_time = std::chrono::steady_clock::now();

    im_res.score = ImitatePairs(graph, algorithm::SolveEasyHungarian, false);

    auto elapsed_time = std::chrono::steady_clock::now() - s_time;
    im_res.time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    return im_res;
}

models::ImitataionResult ImitateEasyWithAR(std::string path) {
    models::Graph graph;
    BuildData(path, graph);
    models::ImitataionResult im_res;

    auto s_time = std::chrono::steady_clock::now();

    im_res.score = ImitatePairs(graph, algorithm::SolveEasyHungarian, true);

    auto elapsed_time = std::chrono::steady_clock::now() - s_time;
    im_res.time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    return im_res;
}

models::ImitataionResult ImitateIterative(std::string path) {
    models::Graph graph;
    BuildData(path, graph);
    models::ImitataionResult im_res;

    auto s_time = std::chrono::steady_clock::now();

    im_res.score = ImitateWithUnions(graph, algorithm::SolveHungarianIterative);

    auto elapsed_time = std::chrono::steady_clock::now() - s_time;
    im_res.time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    return im_res;
}

models::ImitataionResult ImitateGreedy(std::string path) {
    models::Graph graph;
    BuildData(path, graph);
    models::ImitataionResult im_res;

    auto s_time = std::chrono::steady_clock::now();

    im_res.score = ImitateWithUnions(graph, algorithm::SolveGreedy);

    auto elapsed_time = std::chrono::steady_clock::now() - s_time;
    im_res.time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    return im_res;
}

models::ImitataionResult ImitateHungarianUnions(std::string path) {
    models::Graph graph;
    BuildData(path, graph);
    models::ImitataionResult im_res;

    auto s_time = std::chrono::steady_clock::now();

    im_res.score = ImitateWithUnions(graph, algorithm::SolveHungarianPreprocessedUnions);

    auto elapsed_time = std::chrono::steady_clock::now() - s_time;
    im_res.time = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    return im_res;
}

}