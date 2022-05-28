#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

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
        std::string id;
        std::vector<Contractor> contractors;
    };

    struct Graph {
        std::vector<Order> orders;
        std::vector<Contractor> contractors;
    };

    struct GraphWithUnions {
        std::vector<Order> orders;
        std::vector<ContractorsUnion> contractors;
    };
}