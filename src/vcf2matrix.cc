#include <iostream>
#include <htslibpp.h>
#include <htslibpp_variant.h>

using namespace YiCppLib::HTSLibpp;

int main(int argc, char** argv) {

    auto vcf = htsOpen(argv[1], "r");
    auto header = htsHeader<bcfHeader>::read(vcf);

    for(auto it = htsHeader<bcfHeader>::dictBegin(header, htsHeader<bcfHeader>::DictType::ID); it != htsHeader<bcfHeader>::dictEnd(header, htsHeader<bcfHeader>::DictType::ID); it++) {
        const auto proxy = htsProxy(*it);
        std::cout<<proxy.key()<<std::endl;
    }

    return 0;
}
