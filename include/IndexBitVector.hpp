#ifndef _INDEXBITVECTOR_H
#define _INDEXBITVECTOR_H

#include "Index.hpp"

namespace cdat {

class IndexBitVector : public Index {

 public:
  static uint const INDEX_TYPE;

  IndexBitVector() : Index() {};
  IndexBitVector(size_type word_size, size_type shift);
  IndexBitVector(size_type word_size, size_type shift, size_type text_length,
                 size_type additional_text_length, sdsl::bit_vector *bit_vector,
                 sdsl::bit_vector::rank_1_type *m_rank1,
                 sdsl::bit_vector::select_1_type *m_select1,
                 Permutation *permutation, Alphabet *alphabet, sdsl::int_vector<> *text);
  ~IndexBitVector();

  int extract(const ulong from, const ulong to, std::string *text, ulong *length) const;
  int save_index(std::ostream& out) const;
  void load(std::istream& in);

  double get_size_in_mega_bytes() const;

 private:

  /*********   FUNCTIONS  ********/

  value_type extract_value(size_type const from, size_type const length) const;

  int count_full_words(std::string const &pattern, ulong length,
                       bool const locate, ulong *numocc, std::vector<ulong> *occ) const;

  bool check_word(size_type const position, size_type const length, const char *pattern) const;

  void create_text(char const *const filename);
  void fill_text(size_type const idx, value_type const value);

  /**********  FIELDS  ***********/
  sdsl::int_vector<> *m_text;
};

inline double IndexBitVector::get_size_in_mega_bytes() const {
	std::cerr<<"get size\n";
    double result = Index::get_size_in_mega_bytes();
    result += sdsl::size_in_mega_bytes(*m_text);

    return result;
}

inline void IndexBitVector::create_text(char const *const) {
	std::cerr<<"create_text\n";
    m_text = new sdsl::int_vector<>(m_text_length + m_additional_text_length,
                                    0, (uint8_t) cds_utils::bits((uint) (m_alphabet->size() - 1)));
}

inline void IndexBitVector::fill_text(size_type const idx, value_type const value) {
    //std::cerr<<"fill text\n";
	(*m_text)[idx] = value;
}

}
#endif
