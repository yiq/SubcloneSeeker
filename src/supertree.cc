#include "supertree.h"

#include <algorithm>
#include <numeric>

constexpr float epsilon = 0.05;

edge_set_t find_viable_moves(
        const std::vector<std::string>& samples,
        const std::set<std::string>& af_clusters,
        const af_table_t& af_table);

void enumerate_tree(
        std::set<std::string> free_nodes,
        edge_set_t current_tree,
        solution_t& candidate_set,
        solution_t& solution_set,
        const edge_set_t& viable_moves,
        const std::vector<std::string>& samples,
        const af_table_t& afs);

solution_t supertrees(
        const std::vector<std::string>& samples,
        const std::set<std::string>& af_clusters,
        const af_table_t& af_table) {

    solution_t solution_set;
    solution_t candidate_set;
    auto viable_moves = find_viable_moves(samples, af_clusters, af_table);

    enumerate_tree(af_clusters, edge_set_t(),
            candidate_set, solution_set,
            viable_moves, samples, af_table);

    return solution_set;



}

/* private implementations */

edge_set_t find_viable_moves(
        const std::vector<std::string>& samples,
        const std::set<std::string>& af_clusters,
        const af_table_t& af_table) {

    edge_set_t viable_moves;
    // normal can be the parent of all other nodes
    for(auto cluster : af_clusters) viable_moves.insert({"n", cluster});

    for(auto parent: af_clusters) {
        for(auto child : af_clusters) {
            if(child == parent) continue;
            bool feasible = std::all_of(samples.begin(), samples.end(), [&](auto sample) {
                    auto pa1 = af_key_t{sample, parent};
                    auto pa2 = af_key_t{sample, child};
                    return af_table.at(pa1) - af_table.at(pa2) > -epsilon;
                    });
            
            if(feasible) viable_moves.insert({parent, child});
        }
    }

    return viable_moves;
}

void enumerate_tree(
        std::set<std::string> free_nodes,
        edge_set_t current_tree,
        solution_t& candidate_set,
        solution_t& solution_set,
        const edge_set_t& viable_moves,
        const std::vector<std::string>& samples,
        const af_table_t& afs) {

    // step 1. find all nodes on current tree
    std::set<std::string> nodes;
    if(current_tree.size() == 0) nodes = {"n"};
    else {
        for(auto edge : current_tree) {
            nodes.insert(edge.first);
            nodes.insert(edge.second);
        }
    }

    const float epsilon = 0.05;

    if(free_nodes.size() == 0) {
        if(candidate_set.find(current_tree) != candidate_set.end()) return;
        candidate_set.insert(current_tree);

        // validate
        for(auto n : nodes) {
            std::set<std::string> children;
            for(auto edge : current_tree) {
                if(edge.first == n) children.insert(edge.second);
            }
            for(auto s : samples) {
                float parent_af = n == "n" ? 0.6 : afs.at({s, n});
                float total_children_af = 0.0;
                for(auto c : children) total_children_af += afs.at({s, c});
                if(parent_af - total_children_af < -epsilon) return;
            }
        }

        solution_set.insert(current_tree);

        return;
    }

    

    // step 2. iterate over free nodes and try their placements
    for(auto fn : free_nodes) {
        // step 2.1 on each existing nodes
        for(auto en : nodes) {
            // if en->fn is possible, try it
            auto new_pair = edge_t{en, fn};
            if(viable_moves.find(new_pair) != viable_moves.end()) {
                auto new_tree = current_tree; new_tree.insert(new_pair);
                auto new_free_nodes = free_nodes; new_free_nodes.erase(fn);
                enumerate_tree(new_free_nodes, new_tree, candidate_set, solution_set, viable_moves, samples, afs);
            }
        }
    }

    return;
}

