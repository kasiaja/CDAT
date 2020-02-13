/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   IndexCSA.hpp
 * Author: kasia
 *
 * Created on May 10, 2019, 9:31 AM
 */

#ifndef INDEXCSA_HPP
#define INDEXCSA_HPP
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/csa_sada.hpp>
#include "Index.hpp"
#include "sdsl/coder.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstring> // for strlen
#include <iomanip>
#include <iterator>


namespace cdat {

    class IndexCSA : public Index {
    public:
        static uint const INDEX_TYPE;

        IndexCSA() : Index() {};
        IndexCSA(size_t word_size, size_t shift);
        IndexCSA(const IndexCSA& orig);
        virtual ~IndexCSA();

        void init() override;

        IndexCSA::value_type extract_value(size_type const from, size_type const length) const;

        int extract(ulong const from, ulong const to,
                std::string *text, ulong *length) const;

        int count_full_words(std::string const &pattern, ulong length,
                bool const locate, ulong *numocc,
                std::vector<ulong> *occ) const;
        
        int save_index(std::ostream& out) const;

        void load(std::istream& in);



        void create_text(char const *const filename);
        void fill_text(size_type const idx, value_type const value);
        double get_size_in_mega_bytes() const;
    private: //csa_sada<enc_vector<coder::elias_delta,128>,S_SA,S_ISA,text_order_sa_sampling<sdsl::sd_vector<> > >
        sdsl::csa_sada<sdsl::enc_vector<>, 2, 2, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index2;
        sdsl::csa_sada<sdsl::enc_vector<>, 4, 4, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index4;
        sdsl::csa_sada<sdsl::enc_vector<>, 8, 8, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index8;
        sdsl::csa_sada<sdsl::enc_vector<>, 16, 16, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index16;
        sdsl::csa_sada<sdsl::enc_vector<>, 32, 32, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index32;
        sdsl::csa_sada<sdsl::enc_vector<>, 64, 64, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index64;
        sdsl::csa_sada<sdsl::enc_vector<>, 128, 128, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index128;
        sdsl::csa_sada<sdsl::enc_vector<>, 256, 256, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index256;
        sdsl::csa_sada<sdsl::enc_vector<>, 512, 512, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index512;
        sdsl::csa_sada<sdsl::enc_vector<>, 1024, 1024, sdsl::text_order_sa_sampling<sdsl::sd_vector<> >, sdsl::isa_sampling<> > csa_index1024;
        sdsl::int_vector<> *m_text;
        std::vector<int> vals = {1, 2, 4, 5, 8, 10, 16, 20, 25, 32};
    };


inline void IndexCSA::fill_text(size_type const idx, value_type const value) {
    ////std::cerr<<"fill text\n";

    (*m_text)[idx] = value;
}

inline double IndexCSA::get_size_in_mega_bytes() const {
    double result = 0;
    switch(m_shift)
    {
        case 1: result += sdsl::size_in_mega_bytes(csa_index2); break;
        case 2: result += sdsl::size_in_mega_bytes(csa_index4); break;
        case 3: result += sdsl::size_in_mega_bytes(csa_index8); break;
        case 4: result += sdsl::size_in_mega_bytes(csa_index16); break;
        case 5: result += sdsl::size_in_mega_bytes(csa_index32); break;
        case 6: result += sdsl::size_in_mega_bytes(csa_index64); break;
        case 7: result += sdsl::size_in_mega_bytes(csa_index128); break;
        case 8: result += sdsl::size_in_mega_bytes(csa_index256); break;
        case 9: result += sdsl::size_in_mega_bytes(csa_index512); break;
        case 10: result += sdsl::size_in_mega_bytes(csa_index1024); break;
    }
    return result;
}

inline void IndexCSA::create_text(char const *const filename) {
	std::cerr<<"create_text CSA"<<m_text_length<<" "<<m_additional_text_length<<"\n";
    m_text = new sdsl::int_vector<>(m_text_length + m_additional_text_length,
                                    0, (uint8_t) cds_utils::bits((uint) (m_alphabet->size() - 1)));
    std::cerr<<"SHIFT"<<m_shift<<"\n";
    switch(m_shift)
    {
        case 1: sdsl::construct(csa_index2, filename, 1); break;
        case 2: sdsl::construct(csa_index4, filename, 1); break;
        case 3: sdsl::construct(csa_index8, filename, 1); break;
        case 4: sdsl::construct(csa_index16, filename, 1); break;
        case 5: sdsl::construct(csa_index32, filename, 1); break;
        case 6: sdsl::construct(csa_index64, filename, 1); break;
        case 7: sdsl::construct(csa_index128, filename, 1); break;
        case 8: sdsl::construct(csa_index256, filename, 1); break;
        case 9: sdsl::construct(csa_index512, filename, 1); break;
        case 10: sdsl::construct(csa_index1024, filename, 1); break;
    }
}
}

#endif /* INDEXCSA_HPP */

