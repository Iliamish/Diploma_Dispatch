#include <iostream>
#include <algorithm>
#include <optional>

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
    int n = matrix.size();
    int m = matrix[0].size();
    int real_border = m;
    if (n > m) {
        for(int i = 0; i < n; ++i) {
            for(int j = 0; j < n - m; ++j) {
                matrix[i].push_back(INF);
            }
        }
    }
    AddZeros(matrix);
    n = matrix.size();
    m = matrix[0].size();
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
    for (int j = 0; j < m ; ++j) {
        if (j <= real_border) {
            ans[pairs[j]] = j - 1;
        } else {
            ans[pairs[j]] = -1;
        }
    }

    ans.erase(ans.begin());
    return ans;
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

models::GraphWithUnions PreprocessUnions(const models::Graph& graph) {
    const std::size_t N = graph.orders.size();

    std::vector<models::ContractorsUnion> unions(N);

    std::size_t counter = 0;
    for(const auto& contractor: graph.contractors) {
        unions[counter].contractors.push_back(contractor);
        if(unions[counter].id.empty()){
            unions[counter].id = std::to_string(counter);
        }
        ++counter;
        counter %= N;
    }

    std::vector<models::Order> orders_with_new_edges;

    for(const auto& order : graph.orders) {
        models::Order new_order;
        new_order.id = order.id;
        for(const auto& co_union : unions) {
            models::Edge new_edge;
            new_edge.to_id = co_union.id;
            new_edge.acceptance_rate = 1;
            for(const auto& contractor: co_union.contractors) {
                auto old_edge = FindEdge(order, contractor.id);
                if(old_edge){
                    new_edge.acceptance_rate *= (1 - old_edge->acceptance_rate);
                    new_edge.weight +=  (1 / old_edge->acceptance_rate) * old_edge->weight;
                }
            }
            new_edge.acceptance_rate = 1 - new_edge.acceptance_rate;
            new_edge.weight /= co_union.contractors.size();
            new_order.edges_to_contractors.push_back(new_edge);
        }
        orders_with_new_edges.push_back(new_order);
    }

    models::GraphWithUnions new_graph;
    new_graph.orders = orders_with_new_edges;
    new_graph.contractors = unions;

    return new_graph;
}

template <typename Graph>
std::vector<std::vector<int>> PreprocessMatrix(const Graph& graph) {
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

std::vector<std::vector<int>> PreprocessMatrixWithAR(const models::Graph& graph) {
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
            result[i][j] = edge_to_contractor.weight * (1 / edge_to_contractor.acceptance_rate);
        }
    }

    return result;
}

models::ContractorsUnion SolveOneOrder(models::Order order){
    std::sort(order.edges_to_contractors.begin(), order.edges_to_contractors.end(),
            [](const models::Edge& edge1, const models::Edge& edge2){
                return edge1.weight < edge2.weight;
            });
    models::ContractorsUnion contractors_union;
    for(auto edge: order.edges_to_contractors){
        contractors_union.contractors.push_back(models::Contractor{edge.to_id});
    }
    return contractors_union;
}

}

namespace algorithm {

std::vector<std::pair<models::Order, models::Contractor>>
    SolveEasyHungarian(const models::Graph& graph, bool use_ar){
    std::vector<std::vector<int>> preproced_matrix;

    if (use_ar) {
        preproced_matrix = PreprocessMatrixWithAR(graph);
    } else {
        preproced_matrix = PreprocessMatrix(graph);
    }
    const auto result_pairs = HungarianImpl(preproced_matrix);

    std::vector<std::pair<models::Order, models::Contractor>> orders_contractors;
    for(std::size_t i = 0; i < result_pairs.size(); ++i){
        orders_contractors.push_back({graph.orders[i], graph.contractors[result_pairs[i]]});
    }
    return orders_contractors;
}

std::vector<std::pair<models::Order, models::ContractorsUnion>>
    SolveHungarianIterative(models::Graph graph) {
    std::vector<models::Order>& orders = graph.orders;
    std::vector<models::Contractor>& contractors = graph.contractors;
    const std::size_t N = orders.size();
    const std::size_t M = contractors.size();
    if (M < 2 * N) {
        std::cerr << "This solution are not optimized for current set of orders and contractors";
    }

    const std::size_t iterations = M / N + (M % N ? 1 : 0);

    std::unordered_map<std::string, models::ContractorsUnion> orders_map;

    if(N == 1){
        orders_map.emplace(orders.back().id, SolveOneOrder(orders.back()));
    } else {
        for (std::size_t i = 0; i < iterations; ++i) {
            const auto preproced_matrix = PreprocessMatrix(graph);
            const auto result_pairs = HungarianImpl(preproced_matrix);
            std::unordered_set<std::string> matched_candidates;
            for(std::size_t j = 0; j < result_pairs.size(); ++j){
                auto& order = orders[j];
                if(result_pairs[j] == -1)
                    continue;
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
    }

    std::vector<std::pair<models::Order, models::ContractorsUnion>> orders_contractors;
    for(const auto& order : orders){
        orders_contractors.push_back({order, orders_map.at(order.id)});
    }
    return orders_contractors;
}

std::vector<std::pair<models::Order, models::ContractorsUnion>>
    SolveHungarianPreprocessedUnions(models::Graph graph) {
    auto new_graph = PreprocessUnions(graph);

    std::vector<std::vector<int>> preproced_matrix;

    preproced_matrix = PreprocessMatrix(new_graph);

    const auto result_pairs = HungarianImpl(preproced_matrix);

    std::vector<std::pair<models::Order, models::ContractorsUnion>> orders_contractors;
    for(std::size_t i = 0; i < result_pairs.size(); ++i){
        orders_contractors.push_back({new_graph.orders[i], new_graph.contractors[result_pairs[i]]});
    }
    return orders_contractors;
}

std::vector<std::pair<models::Order, models::ContractorsUnion>>
    SolveGreedy(models::Graph graph) {
    std::vector<models::Order>& orders = graph.orders;
    std::vector<models::Contractor>& contractors = graph.contractors;

    std::unordered_map<std::string, models::ContractorsUnion> orders_map;

    if(orders.size() == 1){
        orders_map.emplace(orders.back().id, SolveOneOrder(orders.back()));
    } else {
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
    }
    std::vector<std::pair<models::Order, models::ContractorsUnion>> orders_contractors;
    for(const auto& order : orders){
        orders_contractors.push_back({order, orders_map.at(order.id)});
    }
    return orders_contractors;
}
}