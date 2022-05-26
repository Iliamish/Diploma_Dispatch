#pragma once

#include "models.hpp"

namespace algorithm {

    std::vector<std::pair<models::Order, models::Contractor>>
        SolveEasyHungarian(const models::Graph& graph, bool use_ar);

    std::vector<std::pair<models::Order, models::ContractorsUnion>>
        SolveHungarianUnions(models::Graph graph);
    
    std::vector<std::pair<models::Order, models::ContractorsUnion>>
        SolveHungarianPreprocessedUnions(models::Graph graph);

    std::vector<std::pair<models::Order, models::ContractorsUnion>>
        SolveGreedy(models::Graph graph);
}