#include "vcf_parser.h"
#include <algorithm>
#include <numeric>

using namespace YiCppLib::HTSLibpp;

void parse_vcf(
        YiCppLib::HTSLibpp::htsFile& vcf,
        YiCppLib::HTSLibpp::bcfHeader& header,
        std::vector<std::string>& samples,
        std::set<std::string>& af_clusters,
        af_table_t& af_table) {

    // get all samples
    std::transform(
            htsHeader<bcfHeader>::dictBegin(header, htsHeader<bcfHeader>::DictType::SAMPLE),
            htsHeader<bcfHeader>::dictEnd(header, htsHeader<bcfHeader>::DictType::SAMPLE),
            std::back_inserter(samples),
            [](const auto& sample_rec) { return htsProxy(sample_rec).key(); });

    std::map<af_key_t, std::vector<float>> allele_frequencies;

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
                // std::cout<<"skipping malformed variant"<<std::endl;
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

    for(auto cluster : af_clusters) {
        for(auto sample : samples) {
            auto pa = af_key_t{sample, cluster};
            auto af = std::accumulate(allele_frequencies[pa].begin(), allele_frequencies[pa].end(), 0.0) / allele_frequencies[pa].size();
            af_table[pa] = af;
        }
    }
}

