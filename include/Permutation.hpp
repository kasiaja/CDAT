#ifndef _PERMUTATION_H
#define _PERMUTATION_H

#include <libcds/libcdsBasics.h>

namespace cdat {

class Permutation {
 public:
  typedef uint64_t size_type;
  typedef uint64_t value_type;
 protected:
  uint *permutation;
  size_type length;
  size_type cell_size;

 public:

  Permutation(size_type const size) : length(size) {
      using namespace cds_utils;

      cell_size = bits(size - 1);
      auto count = uint_len(cell_size, length);

      permutation = new uint[count];
  };

  Permutation(uint *permutation, size_type length, size_type cell_size) :
      permutation(permutation), length(length), cell_size(cell_size) {};

  virtual ~Permutation() { delete[] permutation; }

  virtual void set_field(size_type const idx, value_type const value) {
      cds_utils::set_field(permutation, cell_size, idx, value);
  };

  value_type pi(size_type const idx) const {
      return cds_utils::get_field(permutation, cell_size, idx);
  };

  virtual value_type revpi(size_type const value) const {
      for (unsigned long long i = 0; i < length; ++i) {
          if (pi(i) == value)
              return i;
      }

      return -1;
  }

  virtual double size_in_mega_bytes() const {
      return ((sizeof(uint) * (cds_utils::uint_len(cell_size, length) + 2)) /
          1024.0) / 1024.0;
  }

  size_type get_size() {
      return length;
  }

  virtual void save(std::ostream &file) const {
      file.write((char *) &length, sizeof(size_type));
      file.write((char *) &cell_size, sizeof(size_type));
      file.write((char *) permutation,
                 cds_utils::uint_len(cell_size, length) * sizeof(uint));
  }

  static Permutation *load(std::istream &file) {
      size_type length, cell_size;

      file.read((char *) &length, sizeof(size_type));
      file.read((char *) &cell_size, sizeof(size_type));

      auto count = cds_utils::uint_len(cell_size, length);
      uint *permutation = new uint[count];
      file.read((char *) permutation, count * sizeof(uint));

      return new Permutation(permutation, length, cell_size);
  }
};

}
#endif //_PERMUTATION_H
