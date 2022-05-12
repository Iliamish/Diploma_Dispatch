#include <unordered_map>

#include "algorithm.hpp"
#define INF 1000000

namespace algorithm {

namespace {
void AddZeros(std::vector<std::vector<int>>& matrix) {
    for(int i = 0; i < matrix.size(); ++i){
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
    std::vector<int> pairs(n);
    std::vector<int> path(m);

    for(int i = 1; i < matrix.size(); ++i){
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
        ans[pairs[j]] = j;

    return ans;
}

std::vector<models::ContractorsUnion> PreprocessUnions(std::string order_id);


std::vector<std::vector<int>> PreprocessMatrix(
    const std::vector<models::Order>& orders,
    const std::vector<models::Contractor>& contractors) {
    std::unordered_map<std::string, std::size_t> contractors_map;
    const std::size_t N = orders.size();
    const std::size_t M = contractors.size();

    for (std::size_t j = 0; j < M; ++j) {
        contractors_map.emplace(contractors[j].id, j);
    }

    std::vector<std::vector<int>> result(N, std::vector<int>(M));
    for (std::size_t i = 0; i < N; ++i) {
        for(const auto& edge_to_contractor : orders[i].edges_to_contractors) {
            const std::size_t j = contractors_map.at(edge_to_contractor.to_id);
            result[i][j] = edge_to_contractor.weight;
        }
    }

    return result;
}

}


std::vector<std::pair<models::Order, models::Contractor>>
    SolveEasyHungarian(const std::vector<models::Order>&,
                                const std::vector<models::Contractor>&){
    
}

std::vector<std::pair<models::Order, models::ContractorsUnion>>
    SolveHungarianUnions(const std::vector<models::Order>&,
                                const std::vector<models::Contractor>&);

std::vector<std::pair<models::Order, models::ContractorsUnion>>
    SolveGreedy(const std::vector<models::Order>&,
                                const std::vector<models::Contractor>&);
}