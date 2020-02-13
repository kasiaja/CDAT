/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   IndexFM.hpp
 * Author: kasia
 *
 * Created on April 24, 2019, 5:21 PM
 */

#ifndef INDEXFM_HPP
#define INDEXFM_HPP

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/csa_wt.hpp>
#include "Index.hpp"


namespace cdat {

    class IndexFM : public Index {
    public:
        static uint const INDEX_TYPE;

        IndexFM() : Index() {};
        IndexFM(size_t word_size, size_t shift);
        IndexFM(const IndexFM& orig);
        virtual ~IndexFM();

        void init() override;

        IndexFM::value_type extract_value(size_type const from, size_type const length) const;

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
    private:
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 2, 2> fm_index2;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 4, 4> fm_index4;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 8, 8> fm_index8;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 16, 16> fm_index16;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 32, 32> fm_index32;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 64, 64> fm_index64;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 128, 128> fm_index128;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 256, 256> fm_index256;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 512, 512> fm_index512;
        sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<15> >, 1024, 1024> fm_index1024;
        sdsl::int_vector<> *m_text;
    };


inline void IndexFM::fill_text(size_type const idx, value_type const value) {
    ////std::cerr<<"fill text\n";

    (*m_text)[idx] = value;
}

inline double IndexFM::get_size_in_mega_bytes() const {
    double result = 0;
     switch(m_shift)
    {
        case 1: result += sdsl::size_in_mega_bytes(fm_index2); break;
        case 2: result += sdsl::size_in_mega_bytes(fm_index4); break;
        case 3: result += sdsl::size_in_mega_bytes(fm_index8); break;
        case 4: result += sdsl::size_in_mega_bytes(fm_index16); break;
        case 5: result += sdsl::size_in_mega_bytes(fm_index32); break;
        case 6: result += sdsl::size_in_mega_bytes(fm_index64); break;
        case 7: result += sdsl::size_in_mega_bytes(fm_index128); break;
        case 8: result += sdsl::size_in_mega_bytes(fm_index256); break;
        case 9: result += sdsl::size_in_mega_bytes(fm_index512); break;
        case 10: result += sdsl::size_in_mega_bytes(fm_index1024); break;
    }
    return result;
}

inline void IndexFM::create_text(char const *const filename) {
	std::cerr<<"create_text FM"<<m_text_length<<" "<<m_additional_text_length<<"\n";
    m_text = new sdsl::int_vector<>(m_text_length + m_additional_text_length,
                                    0, (uint8_t) cds_utils::bits((uint) (m_alphabet->size() - 1)));
    std::cerr<<"SHIFT"<<m_shift<<"\n";
    switch(m_shift)
    {
        case 1: sdsl::construct(fm_index2, filename, 1); break;
        case 2: sdsl::construct(fm_index4, filename, 1); break;
        case 3: sdsl::construct(fm_index8, filename, 1); break;
        case 4: sdsl::construct(fm_index16, filename, 1); break;
        case 5: sdsl::construct(fm_index32, filename, 1); break;
        case 6: sdsl::construct(fm_index64, filename, 1); break;
        case 7: sdsl::construct(fm_index128, filename, 1); break;
        case 8: sdsl::construct(fm_index256, filename, 1); break;
        case 9: sdsl::construct(fm_index512, filename, 1); break;
        case 10: sdsl::construct(fm_index1024, filename, 1); break;
    }
}
}
#endif /* INDEXFM_HPP */

