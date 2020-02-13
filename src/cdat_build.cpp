#include "Index.hpp"
#include "IndexPerm.hpp"
#include "IndexBitVector.hpp"
#include "IndexBC.hpp"
#include "IndexLookupTable.hpp"
#include "IndexFM.hpp"
#include "IndexCSA.hpp"
#include "IndexWaveletTree.hpp"
#include "IndexNC.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace cdat;
namespace po = boost::program_options;

void validate(int const size, int const shift) {
    if (size <= 0) {
        throw std::runtime_error("Size must be greater than 0.");
    }

    if (shift <= 0) {
        throw std::runtime_error("Shift must be greater than 0.");
    }

    if (shift > size) {
        throw std::runtime_error("Shift cannot be greater than size.");
    }
}

void save_to_file(Index const * const index, std::string const &file_name) {
    std::filebuf fb;
    fb.open(file_name, std::ios::out);
    std::ostream out(&fb);
    index->save_index(out);
    fb.close();
}

int main(int argc, char *argv[]) {
    std::string input_file;
    std::string output_file;
    std::string index_type;
    int size;
    int shift;

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "produce a help message")
                ("in,i", po::value<std::string>(&input_file)->required(), "input file with text")
                ("out,o", po::value<std::string>(&output_file)->required(), "output file in which index will be saved")
                ("size,s", po::value<int>(&size)->required(), "size of the words to be indexed")
                ("shift,f", po::value<int>(&shift)->required(), "shift value, 1 <= shift <= size")
                ("type,t", po::value<std::string>(&index_type)->default_value("bit"), "index type <bit | wt | perm | lut | lut2 | fm | nc | bc>")
                ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc;
            return 0;
        }

        po::notify(vm);
        validate(size, shift);
    } catch (std::exception& e) {
        std::cout << e.what() << "\n";
        return 0;
    }

    Index *index = nullptr;

    std::cerr << index_type << "\n";
    if (index_type == "perm") {
        index = new IndexPerm((size_t) size, (size_t) shift);
    } else if (index_type == "wt") {
        index = new IndexWaveletTree((size_t) size, (size_t) shift);
    } else if (index_type == "bit") {
        index = new IndexBitVector(12, (size_t) shift);
    } else if (index_type == "bit2") {
        index = new IndexBitVector(14, (size_t) shift);
    } else if (index_type == "lut") {
        std::cerr << "Lut\n";
        index = new IndexLookupTable((size_t) size, (size_t) shift, 8);
    } else if (index_type == "lut2") {
        std::cerr << "Lut\n";
        index = new IndexLookupTable((size_t) size, (size_t) shift, 7);
    } else if (index_type == "fm") {
        index = new IndexFM((size_t) size, (size_t) shift);
    } else if (index_type == "nc") {
        index = new IndexNC((size_t) size, (size_t) shift);
    } else if (index_type == "bc") {
        index = new IndexBC((size_t) size, (size_t) shift);
    } else if (index_type == "csa") {
        index = new IndexCSA((size_t) size, (size_t) shift);
    } else {
        std::cerr << "Wrong index type, available options are: bit, bit2, lut, perm, wt, fm, bc, nc, csa, lut2.\n";
        return -1;
    }

    std::cout << "Started building index.\n";
    timeval start, stop, t2;
    unsigned long time = 0;
    gettimeofday(&start, NULL);

    //std::cerr<<"aa"<<((IndexBitVector*)index)->m_text<<"\n";
    index->build<Counter<32> >(input_file.c_str());

    //std::cerr<<"aa"<<((IndexBitVector*)index)->m_text<<"\n";
    std::cerr << "po build\n";
    save_to_file(index, output_file);
    std::cerr << "po save to file\n";

    gettimeofday(&stop, NULL);
    std::cerr << "po save to file\n";
    timersub(&stop, &start, &t2);
    std::cerr << "po save to file\n";
    time += (t2.tv_sec) * 1000 + (t2.tv_usec) / 1000;
    std::cerr << "po save to file\n";

    std::cout << "Index has been built in " << time / 1000.0 << "[s] and saved to file: \'"
            << output_file << "\'.\n";
    std::cout << "Index size: " << index->get_size_in_mega_bytes() << "[mb].\n";

    return 0;
}