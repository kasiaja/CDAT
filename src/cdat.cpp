#include "Index.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace cdat;
namespace po = boost::program_options;


Index * load_from_file(std::string const &file_name) {
    std::filebuf fb;
    fb.open (file_name, std::ios::in);
    std::istream in(&fb);

    Index *index = Index::load_index(in);
    fb.close();

    return index;
}

void print_locate(std::string const &pattern, std::vector<ulong> const &occ, std::ostream &out) {
    out << "\'" <<pattern << "\' occurrences are:\n" << "[";

    for (size_t i = 0; i < occ.size(); ++i) {
        out << occ[i];

        if (i < occ.size() - 1) {
            out << ", ";
        }
    }

    out << "]\n";
}

void locate(Index * index, std::string &filename, std::ostream &out) {
    timeval start, stop, t2;
    unsigned long time = 0;

    std::ifstream file(filename);
    std::string line;
    ulong count_global = 0;
    std::vector<std::string> reads;

    while (getline(file, line)) {
        reads.push_back(line);
    }

    gettimeofday(&start, NULL);
    std::cerr<<"mainowy locate\n";
    for (int i = 0; i < 2; ++i) {
        //std::cerr<<"main locate i:"<<i<<"\n";
        for (int j = 0; j < (int)reads.size(); ++j) {
        //std::cerr<<"main locate j:"<<j<<"\n";
            ulong count = 0;
            std::vector<ulong> occurrences;
            occurrences.reserve(1 << 10);
            std::cerr<<"j:"<<j<<" reads.size():" <<reads.size()<<"\n";
            index->locate(reads[j], reads[j].size(), &occurrences, &count);
            //std::cerr<<"koniec locate\n";
            count_global += count;

        }
    }

    file.close();

    gettimeofday(&stop, NULL);
    timersub(&stop, &start, &t2);
    time += (t2.tv_sec) * 1000 + (t2.tv_usec) / 1000;

    std::cout << "Located " << count_global << " occurrences of patterns in " << time / 3000.0 << "[s].\n";
}

void count(Index * index, std::string &filename, std::ostream &out) {
    timeval start, stop, t2;
    unsigned long time = 0;

    gettimeofday(&start, NULL);
    std::ifstream file(filename);
    std::string line;
    ulong global_count = 0;

    while (getline(file, line)) {
        ulong count = 0;
        index->count(line, line.size(), &count);
        //out << "\'" << line << "\' number of occurrences = " << count << "\n";

        global_count += count;
    }

    file.close();

    gettimeofday(&stop, NULL);
    timersub(&stop, &start, &t2);
    time += (t2.tv_sec) * 1000 + (t2.tv_usec) / 1000;

    std::cout << "Found " << global_count << " occurrences of pattern in " << time / 1000.0 << "[s].\n";
}

int main(int argc, char *argv[]) {
    std::string input_file;
    std::string pattern_file;
    std::string output_file;
    bool isLocate = false;
    bool save_file = false;
    std::cerr<<"poczatek cdat\n";
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "produce a help message")
            ("index,i", po::value<std::string>(&input_file)->required(), "input file with index")
            ("pattern,p", po::value<std::string>(&pattern_file)->required(), "file with patterns to search")
            ("out,o", po::value<std::string>(&output_file), "file to write results, if not specified results will be displayed to stdout.")
            ("action,a", po::value<std::string>(&pattern_file)->required(), "locate or count")
            ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help")) {
            std::cout << desc;
            return 0;
        }

        po::notify(vm);

        {
            std::string action = vm["action"].as< std::string >();
            if (action == "locate") {
                isLocate = true;
            }
            else if (action != "count") {
                std::cerr << "Wrong action type, available options are locate and count\n";
                return -1;
            }
        }
        if (vm.count("out")) {
            save_file = true;
        }
    }
    catch (std::exception& e) {
        std::cout << e.what() << "\n";
        return 0;
    }

    try {
        std::cerr<<"przed loadem\n";
        Index *index = load_from_file(input_file);
        std::cerr<<"po loadzie\n";
        std::cout << "Loaded " << index->get_size_in_mega_bytes() << "[mb] into memory.\n";
        if (save_file) {
            std::filebuf fb;
            fb.open(output_file, std::ios::out);
            std::ostream out(&fb);

            if (isLocate) {
                locate(index, pattern_file, out);
            }
            else {
                count(index, pattern_file, out);
            }

            fb.close();
        }
        else {
            if (isLocate) {
                locate(index, pattern_file, std::cout);
            }
            else {
                count(index, pattern_file, std::cout);
            }
        }
    }
    catch (std::exception &exception) {
        std::cerr << exception.what() << "\n";
        return -1;
    }

    return 0;
}
