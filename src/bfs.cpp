#include "bfs.h"
#include <iostream>
#include <unordered_set>
#include <queue>
#include <omp.h>

state_ptr get_best_result_bfs(std::vector<state_ptr> results) {
    state_ptr best_result = nullptr;
    for(auto result : results) {
        if(best_result == nullptr) { best_result = result;}
        else if (result->get_identifier() < best_result->get_identifier()) {
            best_result = result;
        }
    }
    return best_result;
}

state_ptr bfs(state_ptr root) {
    std::unordered_set<size_t> visited;
    visited.insert(root->get_identifier());
    std::vector<state_ptr> main_vector; main_vector.push_back(root);
    std::vector<state_ptr> secondary_vector;
    std::vector<state_ptr> results;
    #pragma omp parallel
    {
        size_t threads = omp_get_num_threads();
        size_t thread_id = omp_get_thread_num();
        while (!main_vector.empty() || results.size() <= 0) {
            for (size_t i = thread_id; i < main_vector.size(); i += threads) {
                std::vector<state_ptr> local_secondary_vector;
                auto current_node = main_vector[i];
                for (auto neighbour_node: current_node->next_states()) {

                    if (visited.find(neighbour_node->get_identifier()) == visited.end()) {
                        #pragma omp critical
                        {
                            if (visited.find(neighbour_node->get_identifier()) == visited.end()) {
                                visited.insert(neighbour_node->get_identifier());
                                local_secondary_vector.push_back(neighbour_node);
                            }
                        }
                        if(neighbour_node->is_goal()) {
                            #pragma omp critical
                            results.push_back(neighbour_node);
                        }
                    }
                }
                #pragma omp critical
                {
                    secondary_vector.insert(secondary_vector.end(), local_secondary_vector.begin(),
                                            local_secondary_vector.end());
                }
                local_secondary_vector.clear();
            }
            #pragma omp barrier
            #pragma omp single
            {
                main_vector.swap(secondary_vector);
                secondary_vector.clear();
            }
        }
    }
    return get_best_result_bfs(results);
}