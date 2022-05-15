#pragma once

#include "models.hpp"

namespace algorithm {

    std::vector<std::pair<models::Order, models::Contractor>>
        SolveEasyHungarian(const models::Graph& graph);

    std::vector<std::pair<models::Order, models::ContractorsUnion>>
        SolveHungarianUnions(models::Graph graph);

    std::vector<std::pair<models::Order, models::ContractorsUnion>>
        SolveGreedy(models::Graph graph);
}