/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   IndexBC.cpp
 * Author: kasia
 * 
 * Created on May 18, 2019, 12:16 AM
 */

#include "IndexBC.hpp"

namespace cdat {

    uint const IndexBC::INDEX_TYPE = 16384;

    IndexBC::~IndexBC() {
    }

    void IndexBC::init() {
    }

    IndexBC::value_type IndexBC::extract_value(size_type const from, size_type const length) const {
        return 0;
    }
    
    IndexBC::IndexBC(size_t word_size, size_t shift) :
    Index(word_size, shift) {
        std::cerr << "maly konstr\n";
    };

    int IndexBC::extract(ulong const from, ulong const to,
            std::string *text, ulong *length) const {

    }

    int IndexBC::count_full_words(std::string const &pattern, ulong length,
            bool const locate, ulong *numocc,
            std::vector<ulong> *occ) const {
       // std::cerr<<"count full words NC\n";
        *numocc =  sdsl::count(fm_index, pattern.begin(), pattern.end());
        if (locate)
        {
            //std::cerr<<"aaa\n";
            sdsl::int_vector<64> res = sdsl::locate(fm_index, pattern.begin(), pattern.end());
            //std::cerr<<"bb\n";
            for (auto a: res)
            {
                
                occ->push_back(a);
            }
        }

        //std::cerr<<"koniec count full words NC\n";
    }

    int IndexBC::save_index(std::ostream& out) const {
        //out<<"kasia";
        out.write((char *) &IndexBC::INDEX_TYPE, sizeof(uint));
        out.write((char *) &m_word_size, sizeof (size_type));
        out.write((char *) &m_shift, sizeof (size_type));
        out.write((char *) &m_text_length, sizeof (size_type));
        out.write((char *) &m_additional_text_length, sizeof (size_type));
        //out<<"drugakasia";
        fm_index.serialize(out);
        //out<<"trzeciakasia";
    }

    void IndexBC::load(std::istream& in) {
        //std::cerr<<"poczatek NC load\n";
        in.read((char *) &m_word_size, sizeof (size_type));
        in.read((char *) &m_shift, sizeof (size_type));
        in.read((char *) &m_text_length, sizeof (size_type));
        in.read((char *) &m_additional_text_length, sizeof (size_type));
        fm_index.load(in);
        std::cerr<<"koniec NC load\n";
    }
}
