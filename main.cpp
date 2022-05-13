#include <iostream>

#include "models.hpp"
#include "imitation.hpp"
#include "utils.hpp"

int main() {
    imitation::ImitateEasy();
    auto value = utils::ParseFile("contractors.json");
    return 0;
}
