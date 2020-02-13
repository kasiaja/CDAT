#include "IndexFM.hpp"
namespace cdat {

    uint const IndexFM::INDEX_TYPE = 2048;

    IndexFM::~IndexFM() {
    }

    void IndexFM::init() {
    }

    IndexFM::value_type IndexFM::extract_value(size_type const from, size_type const length) const {

    }

    IndexFM::IndexFM(size_t word_size, size_t shift) :
    Index(word_size, shift) {
        std::cerr << "maly konstr\n";
    };

    int IndexFM::extract(ulong const from, ulong const to,
            std::string *text, ulong *length) const {

    }

    int IndexFM::count_full_words(std::string const &pattern, ulong length,
            bool const locate, ulong *numocc,
            std::vector<ulong> *occ) const {
        // std::cerr<<"count full words FM\n";
          switch (m_shift) {
            case 1: *numocc = sdsl::count(fm_index2, pattern.begin(), pattern.end());
                break;
            case 2: *numocc = sdsl::count(fm_index4, pattern.begin(), pattern.end());
                break;
            case 3: *numocc = sdsl::count(fm_index8, pattern.begin(), pattern.end());
                break;
            case 4: *numocc = sdsl::count(fm_index16, pattern.begin(), pattern.end());
                break;
            case 5: *numocc = sdsl::count(fm_index32, pattern.begin(), pattern.end());
                break;
            case 6: *numocc = sdsl::count(fm_index64, pattern.begin(), pattern.end());
                break;
            case 7: *numocc = sdsl::count(fm_index128, pattern.begin(), pattern.end());
                break;
            case 8: *numocc = sdsl::count(fm_index256, pattern.begin(), pattern.end());
                break;
            case 9: *numocc = sdsl::count(fm_index512, pattern.begin(), pattern.end());
                break;
            case 10: *numocc = sdsl::count(fm_index1024, pattern.begin(), pattern.end());
                break;
        }if (locate) {
            //std::cerr<<"aaa\n";
            sdsl::int_vector<64> res;
            switch (m_shift) {
                case 1: res = sdsl::locate(fm_index2, pattern.begin(), pattern.end());
                    break;
                case 2: res = sdsl::locate(fm_index4, pattern.begin(), pattern.end());
                    break;
                case 3: res = sdsl::locate(fm_index8, pattern.begin(), pattern.end());
                    break;
                case 4: res = sdsl::locate(fm_index16, pattern.begin(), pattern.end());
                    break;
                case 5: res = sdsl::locate(fm_index32, pattern.begin(), pattern.end());
                    break;
                case 6: res = sdsl::locate(fm_index64, pattern.begin(), pattern.end());
                    break;
                case 7: res = sdsl::locate(fm_index128, pattern.begin(), pattern.end());
                    break;
                case 8: res = sdsl::locate(fm_index256, pattern.begin(), pattern.end());
                    break;
                case 9: res = sdsl::locate(fm_index512, pattern.begin(), pattern.end());
                    break;
                case 10: res = sdsl::locate(fm_index1024, pattern.begin(), pattern.end());
                    break;
            }//std::cerr<<"bb\n";
            for (auto a : res) {

                occ->push_back(a);
            }
        }

        //std::cerr<<"koniec count full words FM\n";
    }

    int IndexFM::save_index(std::ostream& out) const {
        //out<<"kasia";
        out.write((char *) &IndexFM::INDEX_TYPE, sizeof (uint));
        out.write((char *) &m_word_size, sizeof (size_type));
        out.write((char *) &m_shift, sizeof (size_type));
        out.write((char *) &m_text_length, sizeof (size_type));
        out.write((char *) &m_additional_text_length, sizeof (size_type));
        //out<<"drugakasia";
        switch (m_shift) {
            case 1: fm_index2.serialize(out);
                break;
            case 2: fm_index4.serialize(out);
                break;
            case 3: fm_index8.serialize(out);
                break;
            case 4: fm_index16.serialize(out);
                break;
            case 5: fm_index32.serialize(out);
                break;
            case 6: fm_index64.serialize(out);
                break;
            case 7: fm_index128.serialize(out);
                break;
            case 8: fm_index256.serialize(out);
                break;
            case 9: fm_index512.serialize(out);
                break;
            case 10: fm_index1024.serialize(out);
                break;
        }
        //out<<"trzeciakasia";
    }

    void IndexFM::load(std::istream& in) {
        //std::cerr<<"poczatek FM load\n";
        in.read((char *) &m_word_size, sizeof (size_type));
        in.read((char *) &m_shift, sizeof (size_type));
        in.read((char *) &m_text_length, sizeof (size_type));
        in.read((char *) &m_additional_text_length, sizeof (size_type));
        switch (m_shift) {
            case 1: fm_index2.load(in);
                break;
            case 2: fm_index4.load(in);
                break;
            case 3: fm_index8.load(in);
                break;
            case 4: fm_index16.load(in);
                break;
            case 5: fm_index32.load(in);
                break;
            case 6: fm_index64.load(in);
                break;
            case 7: fm_index128.load(in);
                break;
            case 8: fm_index256.load(in);
                break;
            case 9: fm_index512.load(in);
                break;
            case 10: fm_index1024.load(in);
                break;
        }
        std::cerr << "koniec FM load\n";
    }

}

