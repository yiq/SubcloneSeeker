#ifndef SUPERTREE_H
#define SUPERTREE_H

#include <string>
#include <vector>
#include <set>
#include <map>

#include "typedefs.h"

solution_t supertrees(
        const std::vector<std::string>& samples,
        const std::set<std::string>& af_clusters,
        const af_table_t& af_table);

#endif
