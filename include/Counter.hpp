#ifndef _COUNTER_H
#define _COUNTER_H

#include "Alphabet.hpp"

#include <unordered_map>
#include <stdint.h>

#include <libcds/libcdsBasics.h>
#include <sdsl/bit_vectors.hpp>

namespace cdat {

template<uint8_t t_width>
class Counter {
 public:
  static_assert(t_width <= 32 && t_width > 0,
                "Counter: width must be at most 32 bits and at least 1bits");
  typedef uint64_t size_type;
  typedef uint64_t value_type;

  Counter(size_type const length);
  virtual ~Counter();
  virtual value_type get(size_type const idx) const;
  virtual void set(size_type const idx, value_type const value);
  virtual void inc(size_type const idx);
  size_type size() const;
  virtual value_type get_and_inc(size_type const idx);

  virtual size_type build_counter(std::ifstream &file, size_type word_size,
                                  size_type const shift, Alphabet const *alphabet,
                                  size_type &text_length, size_type &additional_length);

  void prepare_for_permutation();

  //make template parameter accessible
  enum { fixed_int_width = t_width };

 protected:

  size_type build(std::ifstream &file, size_type const word_size,
                  size_type const shift, Alphabet const *alphabet,
                  size_type &text_length, size_type &additional_text_length);

 private:

  unsigned int *m_counter;
  size_type m_length;

};

/************** IMPLEMENTATION ******************/

template<uint8_t t_width>
inline Counter<t_width>::Counter(size_type const length) :
    m_length(length) {
    m_counter = new unsigned int[cds_utils::uint_len(t_width, length)];
}

template<uint8_t t_width>
inline Counter<t_width>::~Counter() {
    delete[] m_counter;
}

template<uint8_t t_width>
inline typename Counter<t_width>::value_type Counter<t_width>::get(size_type const idx) const {
    return cds_utils::get_field(m_counter, t_width, idx);
}

template<uint8_t t_width>
inline void Counter<t_width>::set(size_type const idx, value_type const value) {
    cds_utils::set_field(m_counter, t_width, idx, value);
}

template<uint8_t t_width>
inline void Counter<t_width>::inc(size_type const idx) {
    set(idx, get(idx) + 1);
}

template<uint8_t t_width>
inline typename Counter<t_width>::size_type Counter<t_width>::size() const {
    return m_length;
}

template<uint8_t t_width>
inline typename Counter<t_width>::value_type Counter<t_width>::get_and_inc(size_type const idx) {
    unsigned int result = get(idx);
    set(idx, result + 1);

    return result;
}

template<uint8_t t_width>
typename Counter<t_width>::value_type Counter<t_width>::build(std::ifstream &file, size_type const word_size,
                                                              size_type const shift, Alphabet const *alphabet,
                                                              size_type &text_length, size_type &additional_text_length) {
																  
	std::cerr<<"build_counter\n";
    size_type words_number = 0;
    size_type word_length = 0, word_value = 0;
    size_type divisor = pow(alphabet->size(), word_size - shift);
    additional_text_length = 0;
    text_length = 0;

    bool additional_word = false;
    const size_t BUFFER_SIZE = 16 * 1024;
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    do {
        file.read(buffer, BUFFER_SIZE);
        bytes_read = (size_t) file.gcount();
        text_length += bytes_read;
		//std::cerr<<"text_length:"<<text_length<<"\n";
        for (size_t i = 0; i < bytes_read; ++i) {
            word_value *= alphabet->size();
            word_value += alphabet->get_char_value(buffer[i]);
			//std::cerr<<"buffer[i]"<<buffer[i]<<"\n";
            ++word_length;
            if (!additional_word)
                additional_word = true;
            if (word_length == word_size) {
                inc(word_value);
                ++words_number;
                word_value %= divisor;
                word_length -= shift;
                additional_word = false;
            }
        }
    }
    while (bytes_read == BUFFER_SIZE);

    if (additional_word) {
        while (word_length != word_size) {
            word_value *= alphabet->size();
            ++word_length;
            additional_text_length++;
        }

        inc(word_value);
        ++words_number;
    }

    return words_number;
}

template<uint8_t t_width>
typename Counter<t_width>::value_type Counter<t_width>::build_counter(std::ifstream &file, size_type word_size,
                                                                      size_type const shift, Alphabet const *alphabet,
                                                                      size_type &text_length, size_type &additional_length) {
    for (size_type i = 0; i < m_length; ++i)
        set(i, 0);

    return Counter<t_width>::build(file, word_size, shift, alphabet,
                                   text_length, additional_length);
}

template<uint8_t t_width>
inline void Counter<t_width>::prepare_for_permutation() {
    for (uint i = 0; i < m_length; ++i)
        set(i, 0);
}

/************** TEMPLATE SPECIALIZATION ************/

template<>
inline Counter<32>::Counter(size_type const length) : m_length(length) {
    m_counter = new unsigned int[length];
}

template<>
inline void Counter<32>::set(size_type const idx, value_type const value) {
    m_counter[idx] = value;
}

template<>
inline void Counter<32>::inc(size_type const idx) {
    ++m_counter[idx];
}

template<>
inline typename Counter<32>::size_type Counter<32>::size() const {
    return m_length;
}

template<>
inline typename Counter<32>::value_type Counter<32>::get_and_inc(size_type const idx) {
    return m_counter[idx]++;
}

template<>
inline void Counter<32>::prepare_for_permutation() {
    value_type sum = 0;
    for (size_type i = 0; i < size(); ++i) {
        value_type temp = m_counter[i];
        m_counter[i] = sum;
        sum += temp;
    }
}

/*********** EXTENDED COUNTERS **************/

template<uint8_t t_width>
class CounterHashMap : public Counter<t_width> {
 public:
  static_assert(t_width <= 31 && t_width > 0,
                "Counter: width must be at most 31 bits and at least 1 bits");
  typedef typename Counter<t_width>::size_type size_type;
  typedef typename Counter<t_width>::value_type value_type;

  CounterHashMap(size_type const length);
  ~CounterHashMap();
  value_type get(size_type const idx) const;
  void set(size_type const idx, value_type const value);
  void inc(size_type const idx);
  value_type get_and_inc(size_type const idx);

  //make template parameter accessible
  enum { fixed_int_width = t_width };

 private:
  const uint MAX_VALUE;
  std::unordered_map<uint, uint> hash_map;

};

template<uint8_t t_width>
CounterHashMap<t_width>::CounterHashMap(size_type const length) :
    Counter<t_width>(length), MAX_VALUE((1 << t_width) - 1) {
    hash_map.reserve(1 << 14);
}

template<uint8_t t_width>
CounterHashMap<t_width>::~CounterHashMap() {

}

template<uint8_t t_width>
inline typename CounterHashMap<t_width>::value_type CounterHashMap<t_width>::get(size_type const idx) const {
    value_type result = Counter<t_width>::get(idx);
    if (result < MAX_VALUE)
        return result;
    else {
        return hash_map.find(idx)->second;
    }
}

template<uint8_t t_width>
inline void CounterHashMap<t_width>::set(size_type const idx, value_type const value) {
    if (value >= MAX_VALUE) {
        Counter<t_width>::set(idx, MAX_VALUE);
        hash_map[idx] = value;
    }
    else
        Counter<t_width>::set(idx, value);
}

template<uint8_t t_width>
inline void CounterHashMap<t_width>::inc(size_type const idx) {
    value_type value = Counter<t_width>::get(idx);
    if (value < MAX_VALUE) {
        Counter<t_width>::set(idx, value + 1);

        if (value + 1 == MAX_VALUE)
            hash_map[idx] = MAX_VALUE;
    }
    else {
        ++hash_map.find(idx)->second;
    }
}

template<uint8_t t_width>
inline typename CounterHashMap<t_width>::value_type CounterHashMap<t_width>::get_and_inc(size_type const idx) {
    value_type result = Counter<t_width>::get(idx);

    if (result < MAX_VALUE) {
        Counter<t_width>::set(idx, result + 1);
        if (result + 1 == MAX_VALUE)
            hash_map[idx] = MAX_VALUE;
    }
    else
        result = hash_map.find(idx)->second++;

    return result;
}

template<uint8_t t_width>
class CounterBitVector : public Counter<t_width> {
 public:
  static_assert(t_width <= 31 && t_width > 0,
                "Counter: width must be at most 31 bits and at least 1 bits");
  typedef typename Counter<t_width>::size_type size_type;
  typedef typename Counter<t_width>::value_type value_type;

  CounterBitVector(size_type const length);
  ~CounterBitVector();
  value_type get(size_type const idx) const;
  void set(size_type const idx, value_type const value);
  void inc(size_type const idx);
  value_type get_and_inc(size_type const idx);

  size_type build_counter(std::ifstream &file, size_type word_size,
                          size_type const shift, Alphabet const *alphabet,
                          size_type &text_length, size_type &additional_length);

  //make template parameter accessible
  enum { fixed_int_width = t_width };

 private:
  const value_type MAX_VALUE;
  bool m_extended;
  uint *m_extended_counter;
  sdsl::bit_vector *m_bitvector;
  sdsl::bit_vector::rank_1_type *m_rank1;
};

template<uint8_t t_width>
inline CounterBitVector<t_width>::CounterBitVector(size_type const length) :
    Counter<t_width>(length), MAX_VALUE((1 << t_width) - 1), m_extended(false) {
    m_extended_counter = NULL;
}

template<uint8_t t_width>
CounterBitVector<t_width>::~CounterBitVector() {
    if (m_extended) {
        delete[] m_extended_counter;
        delete m_rank1;
        delete m_bitvector;
    }
}

template<uint8_t t_width>
inline typename CounterBitVector<t_width>::value_type CounterBitVector<t_width>::get(size_type const idx) const {
    size_type result = Counter<t_width>::get(idx);
    if (m_extended && result == MAX_VALUE)
        return m_extended_counter[m_rank1->rank(idx)];

    return result;
}

template<uint8_t t_width>
inline void CounterBitVector<t_width>::set(size_type const idx, value_type const value) {
    if (value < MAX_VALUE)
        Counter<t_width>::set(idx, value);
    else {
        Counter<t_width>::set(idx, MAX_VALUE);

        if (m_extended)
            m_extended_counter[m_rank1->rank(idx)] = value;
    }
}

template<uint8_t t_width>
inline void CounterBitVector<t_width>::inc(size_type const idx) {
    value_type value = Counter<t_width>::get(idx);
    if (!m_extended && value < MAX_VALUE) {
        Counter<t_width>::set(idx, value + 1);
    }
    else if (m_extended && value == MAX_VALUE) {
        ++m_extended_counter[m_rank1->rank(idx)];
    }

}

template<uint8_t t_width>
inline typename CounterBitVector<t_width>::value_type CounterBitVector<t_width>::get_and_inc(size_type const idx) {
    value_type result = Counter<t_width>::get(idx);
    if (result < MAX_VALUE) {
        Counter<t_width>::set(idx, result + 1);
        if (result + 1 == MAX_VALUE)
            m_extended_counter[m_rank1->rank(idx)] = MAX_VALUE;
    }
    else
        result = m_extended_counter[m_rank1->rank(idx)]++;

    return result;

}

}
#endif
