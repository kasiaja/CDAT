#ifndef _ALPHABET_H
#define _ALPHABET_H

#include <iostream>
#include <fstream>
#include <math.h>

namespace cdat {

#define ALPHABET 99
#define ASCII 256

class Alphabet {
  typedef unsigned char uchar;

 public:
  typedef uint64_t size_type;
  typedef uint64_t value_type;

  Alphabet();
  Alphabet(size_type const _size);
  virtual ~Alphabet();

  int build_from_text(char const *filename);
  int build(char const *filename);

  value_type get_word_value(std::string const &word) const;
  value_type get_word_value(std::string const &word, size_type const start,
                        size_type const end) const;
  std::string get_word_from_value(size_type const value, size_type const word_size) const;

  static Alphabet *load(std::istream &file);
  void save(std::ostream &file) const;

  unsigned long long pow_wsize(size_type const power) const;
  bool validate_word(std::string const &word) const;

  size_type size() const {
      return m_size;
  }

  size_type get_char_value(char const sign) const {
      return m_alphabet[static_cast<uchar>(sign)];
  }

  char get_char_from_value(value_type const value) const {
      return m_reverse_alphabet[value];
  }

 private:
  void count_ifpower2();

  size_type m_size;
  uint  m_alphabet[ASCII];
  uchar * m_reverse_alphabet;

  bool m_if_power2;
  uint m_power2;
};

inline unsigned long long Alphabet::pow_wsize(size_type const power) const {
    return m_if_power2 ? ((unsigned long long) 1) << (m_power2 * power) :
           (unsigned long long) pow(m_size, power);
}

}

#endif
