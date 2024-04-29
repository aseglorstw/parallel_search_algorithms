#include "bfs.h"
#include <iostream>
#include <unordered_set>
#include <queue>
#include <omp.h>

bool is_visited(std::unordered_set<size_t>& visited, size_t id) {
    bool found;
    #pragma omp critical(visited_section)
    {
        found = (visited.find(id) != visited.end());
    }
    return found;
}

void add_to_visited(std::unordered_set<size_t>& visited, size_t id) {
    #pragma omp critical(visited_section)
    {
        visited.insert(id);
    }
}

void add_to_results(std::vector<state_ptr>& results, state_ptr neighbour_node) {
    #pragma omp critical
    {
        results.push_back(neighbour_node);
    }
}

void atomic_vector_merge(std::vector<state_ptr>& secondary_vector, std::vector<state_ptr>& local_vector) {
    #pragma omp critical
    {
        secondary_vector.insert(secondary_vector.end(), local_vector.begin(),
                                local_vector.end());
    }
    local_vector.clear();
}

void swap_vectors( std::vector<state_ptr>& main_vector, std::vector<state_ptr>& secondary_vector) {
    main_vector.swap(secondary_vector);
    secondary_vector.clear();
}

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
        std::vector<state_ptr> local_vector;
        while (!main_vector.empty() || results.size() <= 0) {
            for (size_t i = thread_id; i < main_vector.size(); i += threads) {
                auto current_node = main_vector[i];
                for (auto neighbour_node: current_node->next_states()) {
                    if (!is_visited(visited, neighbour_node->get_identifier())) {
                        add_to_visited(visited, neighbour_node->get_identifier());
                        local_vector.push_back(neighbour_node);
                        if(neighbour_node->is_goal()) { add_to_results(results, neighbour_node);}
                    }
                }
            }
            atomic_vector_merge(secondary_vector, local_vector);
            #pragma omp barrier
            #pragma omp single
            swap_vectors(main_vector, secondary_vector);
        }
    }
    return get_best_result_bfs(results);
}
