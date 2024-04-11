#include "iddfs.h"
#include "iostream"
#include "atomic"
#include "omp.h"

std::atomic<size_t> num_threads = 1;
std::atomic<size_t> max_threads = omp_get_max_threads();


state_ptr get_best_result_weighted(state_ptr best_result, state_ptr result) {
    if (best_result == nullptr) { return result; }
    else if (result != nullptr && result->current_cost() < best_result->current_cost()) { return result;}
    else if (result != nullptr && result->current_cost() == best_result->current_cost() &&
             result->get_identifier() < best_result->get_identifier()) { return result;}
    return best_result;
}

bool can_I_start_weighted(state_ptr node, state_ptr neighbour_node) {
    return (node->get_predecessor() != nullptr && (node->get_predecessor())->get_identifier() != neighbour_node->get_identifier())
           ||  node->get_predecessor() == nullptr;
}

state_ptr limited_dfs_weighted(size_t max_depth, state_ptr node) {

    if (node->is_goal()) { return node;}
    else if (max_depth == 0) { return nullptr;}
    state_ptr best_result = nullptr;
    for (auto& neighbour_node: node->next_states()) {
        if(max_depth > 3 && can_I_start_weighted(node, neighbour_node))  {
            #pragma omp task shared(best_result)
            {
                auto result = limited_dfs_weighted(max_depth - 1, neighbour_node);
                #pragma omp critical
                {
                    best_result = get_best_result_weighted(best_result, result);
                }
            }
        }
        else if (can_I_start_weighted(node, neighbour_node)) {
            auto result = limited_dfs_weighted(max_depth - 1, neighbour_node);
            #pragma omp critical
            {
                best_result = get_best_result_weighted(best_result, result);
            }
        }
    }
    #pragma omp taskwait
    return best_result;
}

state_ptr limited_cost_dfs_weighted(size_t max_cost, state_ptr node) {
    if (node->is_goal()) { return node;}
    else if (max_cost < node->current_cost()) { return nullptr;}
    state_ptr best_result = nullptr;
    for (auto& neighbour_node: node->next_states()) {
        if(num_threads < max_threads && can_I_start_weighted(node, neighbour_node))  {
            #pragma omp task shared(best_result)
            {
                num_threads++;
                auto result = limited_cost_dfs_weighted(max_cost, neighbour_node);
                #pragma omp critical
                {
                    best_result = get_best_result_weighted(best_result, result);
                }
                num_threads--;
            }
        }
        else if (can_I_start_weighted(node, neighbour_node)) {
            auto result = limited_cost_dfs_weighted(max_cost, neighbour_node);
            #pragma omp critical
            {
                best_result = get_best_result_weighted(best_result, result);
            }
        }
    }
    #pragma omp taskwait
    return best_result;
}

state_ptr iddfs_weighted(state_ptr root) {
    state_ptr result = nullptr;
    size_t max_depth = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            while (result == nullptr) { result = limited_dfs_weighted(max_depth++, root);}
            result = limited_cost_dfs_weighted(result->current_cost(), root);
        }
    }
    return result;
}
