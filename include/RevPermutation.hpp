#ifndef _REV_PERMUTATION_H
#define _REV_PERMUTATION_H

#include "Permutation.hpp"

namespace cdat {

class RevPermutation : public Permutation {
 private:
  uint *rev_permutation;

 public:

  RevPermutation(size_type const size) :
      Permutation(size) {
      rev_permutation = new uint[cds_utils::uint_len(
          Permutation::cell_size,
          Permutation::length)];
  }

  RevPermutation(uint *permutation, uint *rev_permutation,
                 size_type length, size_type cell_size) :
      Permutation(permutation, length, cell_size),
      rev_permutation(rev_permutation) {};

  ~RevPermutation() { delete[] rev_permutation; }

  void set_field(size_type const idx, value_type const value) {
      cds_utils::set_field(Permutation::permutation,
                           Permutation::cell_size, idx, value);
      cds_utils::set_field(rev_permutation,
                           Permutation::cell_size, value, idx);
  }

  value_type revpi(size_type const value) const {
      return cds_utils::get_field(rev_permutation,
                                  Permutation::cell_size, value);
  }

  double size_in_mega_bytes() const {
      return Permutation::size_in_mega_bytes() +
          (((sizeof(uint) * cds_utils::uint_len(cell_size, length)) / 1024.0) / 1024.0);
  }

  void save(std::ostream &file) const {
      Permutation::save(file);
      file.write((char *) rev_permutation,
                 cds_utils::uint_len(cell_size, length) * sizeof(uint));
  }

  static RevPermutation *load(std::istream &file) {
      size_type length, cell_size;

      file.read((char *) &length, sizeof(size_type));
      file.read((char *) &cell_size, sizeof(size_type));

      auto count = cds_utils::uint_len(cell_size, length);
      uint *permutation = new uint[count], *rev_permutation = new uint[count];

      file.read((char *) permutation, count * sizeof(uint));
      file.read((char *) rev_permutation, count * sizeof(uint));

      return new RevPermutation(permutation, rev_permutation, length, cell_size);
  }

};

}
#endif //_PERMUTATION_H
