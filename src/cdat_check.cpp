#include"Alphabet.hpp"

#include <cmath>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace cdat;
namespace po = boost::program_options;

double log(size_t const value, size_t const base) {
    return log10(value) / log10(base);
}

int check(std::string const &filename) {
    bool characters[ASCII];
    std::fill(characters, characters + ASCII, false);
    size_t alphabet_size = 0;
    size_t text_length = 0;

    std::ifstream file(filename);
    if (file.is_open()) {
        const size_t BUFFER_SIZE = 16 * 1024;
        char buffer[BUFFER_SIZE];
        size_t bytes_read;
        do {
            file.read(buffer, BUFFER_SIZE);
            bytes_read = (size_t) file.gcount();

            for (size_t i = 0; i < bytes_read; ++i) {
                unsigned char uc = static_cast<unsigned char>(buffer[i]);
                if (!characters[uc]) {
                    characters[uc] = true;
                    ++alphabet_size;
                }
            }

            text_length += bytes_read;
        }
        while (bytes_read == BUFFER_SIZE);
    }
    else {
        std::cerr << "Error: could not open file\"" << filename << "\n";
        return -1;
    }

    std::cout << "Text length          = " << text_length << "\n";
    std::cout << "Alphabet size        = " << alphabet_size << "\n";
    std::cout << "Preferable word size = " << (int) log(text_length, alphabet_size) << "\n";

    return 0;
}

int main(int argc, char *argv[]) {
    std::string input_file;

    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce a help message")
            ("in,i", po::value<std::string>(&input_file)->required(), "input file with text")
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc;
            return 0;
        }

        po::notify(vm);
    }
    catch (std::exception& e) {
        std::cout << e.what() << "\n";
        return -1;
    }

    check(input_file);
}