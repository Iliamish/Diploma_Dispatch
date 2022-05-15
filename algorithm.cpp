#include <iostream>
#include <algorithm>

#include "algorithm.hpp"
#define INF 1000000

namespace {

void AddZeros(std::vector<std::vector<int>>& matrix) {
    for(std::size_t i = 0; i < matrix.size(); ++i){
        matrix[i].insert(matrix[i].begin(),0);
    }
    matrix.insert(matrix.begin(), std::vector<int>(matrix[0].size(), 0));
}

std::vector<int> HungarianImpl(std::vector<std::vector<int>> matrix){
    AddZeros(matrix);
    int n = matrix.size();
    int m = matrix[0].size();
    std::vector<int> u_potential(n);
    std::vector<int> v_potential(m);
    std::vector<int> pairs(m);
    std::vector<int> path(m);

    for(std::size_t i = 1; i < matrix.size(); ++i){
        pairs[0] = i;
        int j0 = 0;
        std::vector<int> minj(m, INF);
        std::vector<bool> used(m, false);
        do {
            used[j0] = true;
            int i0 = pairs[j0], delta = INF, j1;
            for(int j = 1; j < m; ++j){
                if(!used[j]){
                    int cur = matrix[i0][j]-u_potential[i0]-v_potential[j];
                    if(cur < minj[j]){
                        minj[j] = cur;
                        path[j] = j0;
                    }
                    if(minj[j] < delta){
                        delta = minj[j];
                        j1 = j;
                    }
                }
            }

            for (int j = 0; j < m; ++j){
                if(used[j]){
                    u_potential[path[j]] += delta;
                    v_potential[j] -= delta;
                }
                else{
                    minj[j] -= delta;
                }
            }
            j0 = j1;
        } while(pairs[j0] != 0);

        do {
            int j1 = path[j0];
            pairs[j0] = pairs[j1];
            j0 = j1;
        } while (j0);
    }

    std::vector<int> ans (n);
    for (int j = 0; j < m ; ++j)
        ans[pairs[j]] = j - 1;

    ans.erase(ans.begin());
    return ans;
}

// std::vector<models::ContractorsUnion> PreprocessUnions(std::string order_id);


std::vector<std::vector<int>> PreprocessMatrix(const models::Graph& graph) {
    std::unordered_map<std::string, std::size_t> contractors_map;
    const std::size_t N = graph.orders.size();
    const std::size_t M = graph.contractors.size();

    for (std::size_t j = 0; j < M; ++j) {
        contractors_map.emplace(graph.contractors[j].id, j);
    }

    std::vector<std::vector<int>> result(N, std::vector<int>(M, INF));
    for (std::size_t i = 0; i < N; ++i) {
        for(const auto& edge_to_contractor : graph.orders[i].edges_to_contractors) {
            const std::size_t j = contractors_map.at(edge_to_contractor.to_id);
            result[i][j] = edge_to_contractor.weight;
        }
    }

    return result;
}

}

namespace algorithm {

std::vector<std::pair<models::Order, models::Contractor>>
    SolveEasyHungarian(const models::Graph& graph){
    const auto preproced_matrix = PreprocessMatrix(graph);
    const auto result_pairs = HungarianImpl(preproced_matrix);

    std::vector<std::pair<models::Order, models::Contractor>> orders_contractors;
    for(std::size_t i = 0; i < result_pairs.size(); ++i){
        orders_contractors.push_back({graph.orders[i], graph.contractors[result_pairs[i]]});
    }
    return orders_contractors;
}

std::vector<std::pair<models::Order, models::ContractorsUnion>>
    SolveHungarianUnions(models::Graph graph) {
    std::vector<models::Order>& orders = graph.orders;
    std::vector<models::Contractor>& contractors = graph.contractors;
    const std::size_t N = orders.size();
    const std::size_t M = contractors.size();
    if (M < 2 * N) {
        std::cerr << "This solution are not optimized for current set of orders and contractors";
    }

    const std::size_t iterations = M / N;

    std::unordered_map<std::string, models::ContractorsUnion> orders_map;
    for (std::size_t i = 0; i < iterations; ++i) {
        const auto preproced_matrix = PreprocessMatrix(graph);
        const auto result_pairs = HungarianImpl(preproced_matrix);
        std::unordered_set<std::string> matched_candidates;
        for(std::size_t j = 0; j < result_pairs.size(); ++j){
            auto& order = orders[j];
            auto& contractor = contractors[result_pairs[j]];
            matched_candidates.emplace(contractor.id);
            auto it = orders_map.find(order.id);
            if (it != orders_map.end()) {
                orders_map[order.id].contractors.push_back(contractor);
            }else{
                models::ContractorsUnion contractors_union;
                contractors_union.contractors.push_back(contractor);
                orders_map.emplace(order.id, contractors_union);
            }
        }
        for(auto& order : orders) {
            auto& edges = order.edges_to_contractors;
            edges.erase(std::remove_if(edges.begin(), edges.end(),
            [&matched_candidates](const models::Edge& value){
                return matched_candidates.find(value.to_id) != matched_candidates.end();
            }), edges.end());
        }
        contractors.erase(std::remove_if(contractors.begin(), contractors.end(),
            [&matched_candidates](const models::Contractor& value){
                return matched_candidates.find(value.id) != matched_candidates.end();
            }), contractors.end());
    }

    std::vector<std::pair<models::Order, models::ContractorsUnion>> orders_contractors;
    for(const auto& order : orders){
        orders_contractors.push_back({order, orders_map.at(order.id)});
    }
    return orders_contractors;
}

std::vector<std::pair<models::Order, models::ContractorsUnion>>
    SolveGreedy(models::Graph graph) {
    std::vector<models::Order>& orders = graph.orders;
    std::vector<models::Contractor>& contractors = graph.contractors;

    std::unordered_map<std::string, models::ContractorsUnion> orders_map;
    for(auto& order : orders) {
        std::sort(order.edges_to_contractors.begin(), order.edges_to_contractors.end(),
            [](const models::Edge& edge1, const models::Edge& edge2){
                return edge1.weight > edge2.weight;
            });
        models::ContractorsUnion contractors_union;
        orders_map.emplace(order.id, contractors_union);
    }

    std::unordered_set<std::string> free_candidates;
    for(auto& contractor : contractors) {
        free_candidates.emplace(contractor.id);
    }

    bool at_least_one = true;
    while(free_candidates.size() > 0 && at_least_one) {
        at_least_one = false;
        for(auto& order : orders) {
            if(!order.edges_to_contractors.empty()) {
                at_least_one = true;
                auto candidate = order.edges_to_contractors.back();
                bool found = true;
                while(free_candidates.find(candidate.to_id) == free_candidates.end()){
                    order.edges_to_contractors.pop_back();
                    if(order.edges_to_contractors.empty()){
                        found = false;
                        break;
                    }
                    candidate = order.edges_to_contractors.back();
                }
                if(found){
                    orders_map[order.id].contractors.push_back(models::Contractor{candidate.to_id});
                    order.edges_to_contractors.pop_back();
                    free_candidates.erase(candidate.to_id);
                }
            }
        }
    }

    std::vector<std::pair<models::Order, models::ContractorsUnion>> orders_contractors;
    for(const auto& order : orders){
        orders_contractors.push_back({order, orders_map.at(order.id)});
    }
    return orders_contractors;
}
}