#include <iostream>

#include "imitation.hpp"
#include "models.hpp"

namespace {
    
}

namespace imitation {

    std::chrono::milliseconds ImitateEasy() {
        models::Contractor contractor1{"driver1"};
        models::Contractor contractor2{"driver2"};
        models::Contractor contractor3{"driver3"};

        models::Edge edge1_1{"driver1", 3, 0.8};
        models::Edge edge1_2{"driver2", 1, 0.9};
        models::Edge edge1_3{"driver3", 2, 0.7};
        models::Order order1{"order1", std::vector<models::Edge>{edge1_1, edge1_2, edge1_3}};

        models::Edge edge2_1{"driver1", 3, 0.5};
        models::Edge edge2_2{"driver2", 2, 0.9};
        models::Edge edge2_3{"driver3", 1, 0.6};
        models::Order order2{"order2", std::vector<models::Edge>{edge2_1, edge2_2, edge2_3}};

        models::Edge edge3_1{"driver1", 1, 0.8};
        models::Edge edge3_2{"driver2", 2, 0.9};
        models::Edge edge3_3{"driver3", 3, 0.7};
        models::Order order3{"order3", std::vector<models::Edge>{edge3_1, edge3_2, edge3_3}};

        std::vector<models::Contractor> contractors{contractor1, contractor2, contractor3};
        std::vector<models::Order> orders{order1, order2, order3};

        auto s_time = std::chrono::steady_clock::now();
        auto result = algorithm::SolveEasyHungarian(orders, contractors);
        auto elapsed_time = std::chrono::steady_clock::now() - s_time;

        for (auto pair : result)
        {
            std::cout << pair.first.id << " -- " << pair.second.id << std::endl;
        }
        
        return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time);
    }

}