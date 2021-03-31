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

    // get all samples
    std::vector<std::string> samples;
    std::transform(
            htsHeader<bcfHeader>::dictBegin(header, htsHeader<bcfHeader>::DictType::SAMPLE),
            htsHeader<bcfHeader>::dictEnd(header, htsHeader<bcfHeader>::DictType::SAMPLE),
            std::back_inserter(samples),
            [](const auto& sample_rec) { return htsProxy(sample_rec).key(); });

    std::set<std::string> af_clusters;
    std::map<af_key_t, std::vector<float>> allele_frequencies;
    af_table_t af_table;


    std::cout<<"acquired "<<samples.size()<<" samples"<<std::endl;

    // get all clusters and allele frequencies
    std::for_each(begin(vcf, header), end(vcf, header), [&](auto &rec) {
            char *data= NULL;
            int32_t ndst = 0;
            auto ret = bcf_get_info_string(header.get(), rec.get(), "AFCLU", &data, &ndst);
            std::string afclu(data);
            if(ndst) free(data);

            af_clusters.insert(afclu);
            // get AO and RO
            int32_t *ro_dst = NULL;
            int32_t *ao_dst = NULL;
            int32_t ro_ndst = 0, ao_ndst = 0;

            bcf_get_format_int32(header.get(), rec.get(), "RO", &ro_dst, &ro_ndst);
            bcf_get_format_int32(header.get(), rec.get(), "AO", &ao_dst, &ao_ndst);
            if(ro_ndst != samples.size() || ao_ndst != samples.size()) {
                std::cout<<"skipping malformed variant"<<std::endl;
                return;
            }

            for(int32_t i=0; i<samples.size(); i++) {
                float af = float(ao_dst[i]) / float(ro_dst[i] + ao_dst[i]);
                af_key_t pa = {samples[i], afclu};
                auto af_vec = allele_frequencies.find(pa);
                if(af_vec == allele_frequencies.end()) allele_frequencies[pa] = std::vector<float>();
                allele_frequencies[pa].push_back(af);
            }
        });


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
            auto af = std::accumulate(allele_frequencies[pa].begin(), allele_frequencies[pa].end(), 0.0) / allele_frequencies[pa].size();
            af_table[pa] = af;
            std::cout<<"\t"<<af;
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


