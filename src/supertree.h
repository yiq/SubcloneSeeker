#ifndef SUPERTREE_H
#define SUPERTREE_H

#include <string>
#include <vector>
#include <set>
#include <map>

using edge_t = std::pair<std::string /* from node */, std::string /* to node */>;
using af_key_t = std::pair<std::string /* sample */, std::string /* cluster */>;
using edge_set_t = std::set<edge_t>;
using af_table_t = std::map<af_key_t, float>;

using solution_t = std::set<edge_set_t>;

solution_t supertrees(
        const std::vector<std::string>& samples,
        const std::set<std::string>& af_clusters,
        const af_table_t& af_table);


#endif
