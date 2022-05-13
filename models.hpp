#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace models {
    struct Edge
    {
        std::string to_id;
        std::size_t weight;
        double acceptance_rate;
    };

    struct Contractor
    {
        std::string id;
    };

    struct Order
    {
        std::string id;
        std::vector<Edge> edges_to_contractors;
    };
    
    struct ContractorsUnion
    {
        double acceptance_rate;
        std::vector<Contractor> contractors;
    };
}