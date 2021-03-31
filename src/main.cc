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

using namespace YiCppLib::HTSLibpp;

template<class T>
class TD;


using edge_set_t = std::set<std::pair<std::string, std::string>>;
using s_c_pair_t = std::pair<std::string, std::string>;

std::set<edge_set_t> candidate_set;

void enumerate_tree(std::set<std::string> free_nodes, edge_set_t viable_moves, edge_set_t current_tree, const std::vector<std::string>& samples, const std::map<s_c_pair_t, float>& afs);

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
    std::map<s_c_pair_t, std::vector<float>> allele_frequencies;
    std::map<s_c_pair_t, float> allele_frequencies_reduced;

    std::cout<<"acquired "<<samples.size()<<" samples"<<std::endl;

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
                s_c_pair_t pa = {samples[i], afclu};
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
            auto pa = s_c_pair_t{sample, cluster};
            auto af = std::accumulate(allele_frequencies[pa].begin(), allele_frequencies[pa].end(), 0.0) / allele_frequencies[pa].size();
            allele_frequencies_reduced[pa] = af;
            std::cout<<"\t"<<af;
        }
        std::cout<<std::endl;
    }

    // find all parental relationship
    edge_set_t viable_moves;
    for(auto cluster : af_clusters) viable_moves.insert({"n", cluster});

    float epsilon = 0.05;
    for(auto parent: af_clusters) {
        for(auto child : af_clusters) {
            if(child == parent) continue;
            bool feasible = std::all_of(samples.begin(), samples.end(), [&](auto sample) {
                    auto pa1 = s_c_pair_t{sample, parent};
                    auto pa2 = s_c_pair_t{sample, child};
                    return allele_frequencies_reduced[pa1] - allele_frequencies_reduced[pa2] > -epsilon;
                    });
            
            if(feasible) viable_moves.insert({parent, child});
        }
    }

    for(auto vip : viable_moves) std::cout<<vip.first<<"->"<<vip.second<<std::endl;

    enumerate_tree(af_clusters, viable_moves, edge_set_t(), samples, allele_frequencies_reduced);

    return 0;
}

void enumerate_tree(std::set<std::string> free_nodes, edge_set_t viable_moves, edge_set_t current_tree, const std::vector<std::string>& samples, const std::map<s_c_pair_t, float>& afs){

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
        std::cout<<"candidate";
        for(auto edge : current_tree) std::cout<<"\t"<<edge.first<<"->"<<edge.second;
        std::cout<<"\t";


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
                if(parent_af - total_children_af < -epsilon) {
                    //std::cout<<"in sample "<<s<<", parent node "<<n<<" has af "<<parent_af<<std::endl;
                    //std::cout<<"in sample "<<s<<", children nodes have total af "<<total_children_af<<std::endl;
                    std::cout<<"non-solution: in sample "<<s<<", children af "<<total_children_af<<" >> parent "<<n<< " af "<<parent_af<<std::endl;
                    return;
                }
            }
            
        }

        std::cout<<"solution"<<std::endl;
        return;
    }

    

    // step 2. iterate over free nodes and try their placements
    for(auto fn : free_nodes) {
        // step 2.1 on each existing nodes
        for(auto en : nodes) {
            // if en->fn is possible, try it
            auto new_pair = s_c_pair_t{en, fn};
            if(viable_moves.find(new_pair) != viable_moves.end()) {
                auto new_tree = current_tree; new_tree.insert(new_pair);
                auto new_free_nodes = free_nodes; new_free_nodes.erase(fn);
                enumerate_tree(new_free_nodes, viable_moves, new_tree, samples, afs);
            }
        }
    }

    return;
}
