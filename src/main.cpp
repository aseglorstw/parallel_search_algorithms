#include <iostream>
#include <memory>
#include <random>
#include <chrono>
#include <algorithm>

#include "state.h"
#include "domains/hanoi.h"
#include "domains/maze.h"
#include "domains/sat.h"
#include "domains/sliding_puzzle.h"

#include "bfs.h"
#include "iddfs.h"
#include "iddfs_weighted.h"


using search_fn = state_ptr (*)(state_ptr);


void evaluate(state_ptr& root, search_fn search) {
    using namespace std::chrono;

    std::cout << " ****\n";
    auto begin = steady_clock::now();
    auto result = search(root);
    auto end = steady_clock::now();

    if (result != nullptr) {
        if (result->is_goal()) {
            std::cout << "Solution found. Cost=" << result->current_cost() << "\n";
        } else {
            std::cout << "Search returned a solution - but it is not a goal!\n";
        }
    } else {
        std::cout << "No solution found.\n";
    }

    std::cout << "Time: " << duration_cast<milliseconds>(end - begin).count() << "ms\n";

    // Pro snazsi ladeni zrekonstruujeme a vypiseme nalezenou cestu
    std::vector<state_ptr> path;
    while (result) {
        path.push_back(result);
        result = result->get_predecessor();
    }
    std::reverse(path.begin(), path.end());
    for (auto& s : path) {
        std::cout << s->to_string() << "\n";
    }

    std::cout << " ****\n";
}


int main() {
    //auto d = hanoi::domain<2, 1, 2>();

    auto d = sat::domain<20, 7, 3, 1, false>();

    //auto d = sliding_puzzle::domain<3, 10, 0>();

    //auto d = maze::domain<31, 21, 0, false>();

    auto root = d.get_root();

    evaluate(root, bfs);
    evaluate(root, iddfs);
    evaluate(root, iddfs_weighted);

    return 0;
}
