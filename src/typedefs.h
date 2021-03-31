#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <string>
#include <vector>
#include <set>
#include <map>

using af_key_t = std::pair<std::string /* sample */, std::string /* cluster */>;
using af_table_t = std::map<af_key_t, float>;

using edge_t = std::pair<std::string /* from node */, std::string /* to node */>;
using edge_set_t = std::set<edge_t>;

using solution_t = std::set<edge_set_t>;

#endif
