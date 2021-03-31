#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <vector>
#include <set>
#include <map>
#include <htslib/vcf.h>
#include <htslibpp.h>
#include <htslibpp_variant.h>

#include "typedefs.h"
#include "vcf_parser.h"
#include "supertree.h"

using namespace YiCppLib::HTSLibpp;

template<class T>
class TD;

int main(int argc, char** argv) {

    auto vcf = htsOpen(argv[1], "r");
    auto header = htsHeader<bcfHeader>::read(vcf);

    // ensure vcf has "AFCLU" field
    auto has_afclu = std::any_of(
            htsHeader<bcfHeader>::dictBegin(header, htsHeader<bcfHeader>::DictType::ID),
            htsHeader<bcfHeader>::dictEnd(header, htsHeader<bcfHeader>::DictType::ID),
            [](auto id_field){ return strcmp(htsProxy(id_field).key(), "AFCLU") == 0; });

    if(!has_afclu) {
        std::cout<<"the given vcf file does not have AFCLU (allele frequency cluster annotation)"<<std::endl;
        exit(1);
    }

    // what we need here
    // sample list
    // cluster list
    // (sample, cluster) -> AF
    
    std::vector<std::string> samples;
    std::set<std::string> af_clusters;
    af_table_t af_table;

    parse_vcf(vcf, header, samples, af_clusters, af_table);


    std::sort(samples.begin(), samples.end());

    std::cout<<std::fixed;
    std::cout<<std::setprecision(2);
    
    for(auto sample : samples) {
        std::cout<<"\t"<<sample;
    }
    std::cout<<std::endl;

    for(auto cluster : af_clusters) {
        std::cout<<cluster;
        for(auto sample : samples) {
            auto pa = af_key_t{sample, cluster};
            std::cout<<"\t"<<af_table[pa];
        }
        std::cout<<std::endl;
    }

    auto trees = supertrees(samples, af_clusters, af_table);
    for(auto t : trees) {
        std::cout<<"solution:";
        for(auto edge : t) std::cout<<"\t"<<edge.first<<" -> "<<edge.second;
        std::cout<<std::endl;
    }

    return 0;
}


