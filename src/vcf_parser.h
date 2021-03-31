#ifndef VCF_PARSER_H
#define VCF_PARSER_H

#include "typedefs.h"
#include <htslibpp.h>
#include <htslibpp_variant.h>

void parse_vcf(
        YiCppLib::HTSLibpp::htsFile& vcf,
        YiCppLib::HTSLibpp::bcfHeader& header,
        std::vector<std::string>& samples,
        std::set<std::string>& af_clusters,
        af_table_t& af_table);

#endif
