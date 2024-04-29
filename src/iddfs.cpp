#include "iddfs.h"
#include "iostream"

state_ptr get_best_result(state_ptr best_result, state_ptr result) {
    if (best_result == nullptr) { return result; }
    else if (result != nullptr && result->current_cost() < best_result->current_cost()) { return result;}
    else if (result != nullptr && result->current_cost() == best_result->current_cost() &&
             result->get_identifier() < best_result->get_identifier()) { return result;}
    return best_result;
}

bool can_I_start(state_ptr node, state_ptr neighbour_node) {
    return (node->get_predecessor() != nullptr && (node->get_predecessor())->get_identifier() != neighbour_node->get_identifier())
           ||  node->get_predecessor() == nullptr;
}

state_ptr get_updated_best_result(state_ptr best_result, state_ptr result) {
    state_ptr updated_best_result;
    #pragma omp critical
    {
        updated_best_result = get_best_result(best_result, result);
    }
    return updated_best_result;
}

state_ptr limited_dfs(size_t max_depth, state_ptr node) {
    if (node->is_goal()) { return node;}
    else if (max_depth == 0) { return nullptr;}
    state_ptr best_result = nullptr;
    for (auto& neighbour_node: node->next_states()) {
        if(max_depth > 3 && can_I_start(node, neighbour_node))  {
            #pragma omp task shared(best_result)
            {
                auto result = limited_dfs(max_depth - 1, neighbour_node);
                best_result = get_updated_best_result(best_result, result);
            }
        }
        else if (can_I_start(node, neighbour_node)) {
            auto result = limited_dfs(max_depth - 1, neighbour_node);
            best_result = get_updated_best_result(best_result, result);
        }
    }
    #pragma omp taskwait
    return best_result;
}

state_ptr iddfs(state_ptr root) {
    state_ptr result = nullptr;
    size_t max_depth = 0;
    #pragma omp parallel
    {
        #pragma omp single
        while (result == nullptr) { result = limited_dfs(max_depth++, root);}
    }
    return result;
}
