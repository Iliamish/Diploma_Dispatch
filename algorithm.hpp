#pragma once

#include "models.hpp"

namespace algorithm {

    std::vector<std::pair<models::Order, models::Contractor>>
        SolveEasyHungarian(const std::vector<models::Order>&,
                                    const std::vector<models::Contractor>&);

    std::vector<std::pair<models::Order, models::ContractorsUnion>>
        SolveHungarianUnions(const std::vector<models::Order>&,
                                    const std::vector<models::Contractor>&);

    std::vector<std::pair<models::Order, models::ContractorsUnion>>
        SolveGreedy(const std::vector<models::Order>&,
                                    const std::vector<models::Contractor>&);
}