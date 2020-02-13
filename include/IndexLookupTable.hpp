#ifndef _INDEXLOOKUPTABLE_H
#define _INDEXLOOKUPTABLE_H

#include <fstream>

#include "Index.hpp"
//#include "RevPermutation.h"
#include <string>
#include <vector>
#define INT_ROZM 35

namespace cdat {

    class IndexLookupTable : public Index {
    public:
        static uint const INDEX_TYPE;

        IndexLookupTable() : Index() {
        };
        IndexLookupTable(size_t word_size, size_t transition, uint prefix);
        IndexLookupTable(size_type word_size, size_type shift, size_type text_length,
                size_type additional_text_length, sdsl::bit_vector *bit_vector,
                sdsl::bit_vector::rank_1_type *m_rank1,
                sdsl::bit_vector::select_1_type *m_select1,
                Permutation *permutation, Alphabet *alphabet, sdsl::int_vector<> *text);

        ~IndexLookupTable();

        int extract(const ulong from, const ulong to, std::string *text, ulong *length) const;
        int save_index(std::ostream& out) const;
        void load(std::istream& in);

        IndexLookupTable::value_type extract_value(size_type const from, size_type const length) const;
        // template<typename Counter>
        // int build(char const * filename);
        void init() override;



    private:

        int count_full_words(std::string const &pattern, ulong length,
                bool const locate, ulong * numocc, std::vector<ulong> * occ) const;

        std::string get_word_from_pattern(ulong const start, size_t const length) const;

        double get_size_in_mega_bytes() const;
        void create_text(char const *const filename);
        void fill_text(size_type const idx, value_type const value);

        std::string read_word_from_pattern(ulong begin, ulong length) const;
        std::string first_lexicographically(ulong k) const;
        std::string last_lexicographically(ulong k) const;
        std::string next_lexicographically(std::string substring) const;
        ulong number_for_letter(char c) const;
        ulong count_where_pointer_for_prefix(std::string const &s) const;
        std::string take_prefix(std::string const &pattern) const;
        bool check_word(size_type const position, size_type const length, const char *pattern) const;
        void radix_sort(ulong begin, ulong end);
        void sortAtPosition(ulong begin, ulong end, ulong position);
        void radix_sort_init ();
        void init_bex_radixa();

        /**********  FIELDS  ***********/
        sdsl::int_vector<> *m_text;

        uint substring_length; //length of substrings
        uint pref_length; // length of prefixes (for each prefix of this length we have a pointer in lookup_table) < substring_length
        sdsl::int_vector<INT_ROZM> substrings;
        sdsl::int_vector<INT_ROZM> lookup_table;
        uint possible_word_size; // number of letters which can fit into one ulong (2^32 / alphabet_size)
        std::map<int, int> temp_subs; //pomocnicze tymczasowe (dla inita) spamietane wartosci podslow
        friend class PatternPosition;
        friend class PatternPositionSearch;
    };

    inline double IndexLookupTable::get_size_in_mega_bytes() const {
        double result = 0;
        result += sdsl::size_in_mega_bytes(*m_text);
        result += sdsl::size_in_mega_bytes(lookup_table);
        result += sdsl::size_in_mega_bytes(substrings);
      //  result += sdsl::size_in_mega_bytes(*m_bit_vector); TODO wydaje mi się że niepotrzebne
       // std::cerr<<"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"<<m_permutation->size_in_mega_bytes()<<"\n";
        return result;
    }

    inline void IndexLookupTable::create_text(char const *const) {
        //std::cerr<<"create text lut\n";
        m_text = new sdsl::int_vector<>(m_text_length + m_additional_text_length,
                0, cds_utils::bits(m_alphabet->size() - 1));
    }

    inline void IndexLookupTable::fill_text(size_type const idx, value_type const value) {
        ////std::cerr<<"fill text\n";

        (*m_text)[idx] = value;
    }

    /*template <typename Counter>
    inline int IndexLookupTable::build(char const * filename)
    {
            //std::cout<<"build\n";
      return build_index<Counter>(filename);
    }*/

}
#endif
