/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   IndexBC.hpp
 * Author: kasia
 *
 * Created on May 18, 2019, 12:16 AM
 */

#ifndef INDEXBC_HPP
#define INDEXBC_HPP

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/csa_wt.hpp>
#include "Index.hpp"


namespace cdat {

    class IndexBC : public Index {
    public:
        static uint const INDEX_TYPE;

        IndexBC() : Index() {};
        IndexBC(size_t word_size, size_t shift);
        IndexBC(const IndexBC& orig);
        virtual ~IndexBC();

        void init() override;

        IndexBC::value_type extract_value(size_type const from, size_type const length) const;

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
        
        sdsl::csa_bitcompressed<> fm_index;
        sdsl::int_vector<> *m_text;
        std::vector<int> vals = {1, 2, 4, 5, 8, 10, 16, 20, 25, 32};
    };


inline void IndexBC::fill_text(size_type const idx, value_type const value) {
    ////std::cerr<<"fill text\n";

    (*m_text)[idx] = value;
}

inline double IndexBC::get_size_in_mega_bytes() const {
    double result = 0;
    result += sdsl::size_in_mega_bytes(fm_index);

    return result;
}

inline void IndexBC::create_text(char const *const filename) {
	std::cerr<<"create_text NC"<<m_text_length<<" "<<m_additional_text_length<<"\n";
    m_text = new sdsl::int_vector<>(m_text_length + m_additional_text_length,
                                    0, (uint8_t) cds_utils::bits((uint) (m_alphabet->size() - 1)));
    std::cerr<<"SHIFT"<<m_shift<<"\n";
    sdsl::construct(fm_index, filename, vals[m_shift - 1]);
}
}
#endif /* INDEXBC_HPP */

