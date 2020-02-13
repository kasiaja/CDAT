#include "Index.hpp"
#include "IndexBitVector.hpp"
#include "IndexFM.hpp"
#include "IndexNC.hpp"
#include "IndexBC.hpp"
#include "IndexLookupTable.hpp"
#include "IndexPerm.hpp"
#include "IndexWaveletTree.hpp"
#include "IndexCSA.hpp"

namespace cdat {

    Index::Index(size_type word_size, size_type shift, size_type text_length,
            size_type additional_text_length, sdsl::bit_vector *bit_vector,
            sdsl::bit_vector::rank_1_type *m_rank1,
            sdsl::bit_vector::select_1_type *m_select1,
            Permutation *permutation, Alphabet *alphabet) :
    m_word_size(word_size),
    m_shift(shift), m_text_length(text_length),
    m_additional_text_length(additional_text_length), m_bit_vector(bit_vector),
    m_rank1(m_rank1), m_select1(m_select1),
    m_permutation(permutation), m_alphabet(alphabet) {

        std::cerr << "konstr index!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n";
    }

    Index::~Index() {
        delete m_permutation;
        delete m_bit_vector;
        delete m_select1;
        delete m_rank1;
        delete m_alphabet;
    }

    void Index::create_bit_vector_support() {
        m_select1 = new sdsl::bit_vector::select_1_type(m_bit_vector);
        m_rank1 = new sdsl::bit_vector::rank_1_type(m_bit_vector);
    }

    double Index::get_size_in_mega_bytes() const {
        double result = sdsl::size_in_mega_bytes(*m_bit_vector);
        result += sdsl::size_in_mega_bytes(*m_rank1);
        result += sdsl::size_in_mega_bytes(*m_select1);
        result += m_permutation->size_in_mega_bytes();

        return result;
    }

    Index::size_type Index::rank_0(const Index::size_type idx) const {
        return idx - m_rank1->rank(idx);
    }

    Permutation *Index::create_permutation(size_type const size) const {
        return new Permutation(size);
    }

#ifdef DEBUG

    void Index::print_time(timeval &start, char const *const msg) {
        timeval stop, t2;

        gettimeofday(&stop, NULL);
        timersub(&stop, &start, &t2);

        const uint min_msg_length = 25;
        size_type msg_length = strlen(msg);
        std::string space = "";
        if (msg_length < min_msg_length) {
            for (size_type i = 0; i < min_msg_length - msg_length; ++i) {
                space += " ";
            }
        }

        double time = (t2.tv_sec) * 1000 + (t2.tv_usec) / 1000;
        time = std::round(time * 1000.0) / 1000.0;
        std::cout << msg << space << (time / 1000.0) << "[s]\n";
        gettimeofday(&start, NULL);
    }
#endif

    // Watch out with word_value + 1

    Index::size_type Index::get_position_in_permutation(value_type const word_value) const {
        size_type position = m_select1->select(word_value + 1);
        return (position - word_value);
    }

    Index::value_type Index::perm_binary_search(size_type const word_value,
            size_type const genome_words_number) const {
        auto sel1 = m_select1->select(word_value + 1);
        auto sel2 = m_select1->select(word_value + 2);
        auto start = rank_0(sel1);
        auto end = rank_0(sel2) - 1;

        while (start < end) {
            auto middle = (start + end) / 2;
            if (m_permutation->pi(middle) != genome_words_number) {
                start = middle + 1;
            } else {
                end = middle;
            }
        }

        return start;
    }

    void Index::create_permutation(std::ifstream &file, size_type const genome_words_number) {
        std::cerr << "create_perm&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n";
        for (size_type i = 0; i < genome_words_number; ++i) {
            m_permutation->set_field(i, genome_words_number);
        }

        file.clear();
        file.seekg(0, std::ios::beg);

        size_t divisor = m_alphabet->pow_wsize(m_word_size - m_shift);
        bool additional_word = false;
        size_type position = 0;
        size_type word_value = 0;
        size_type word_length = 0;
        unsigned long long char_counter = 0;

        const size_t BUFFER_SIZE = 16 * 1024;
        char buffer[BUFFER_SIZE];
        size_t bytes_read;
        do {
            file.read(buffer, BUFFER_SIZE);
            bytes_read = (size_t) file.gcount();

            for (size_t i = 0; i < bytes_read; ++i) {
                value_type char_value = m_alphabet->get_char_value(buffer[i]);
                word_value *= m_alphabet->size();
                word_value += char_value;
                fill_text(char_counter++, char_value);
                ++word_length;

                if (!additional_word)
                    additional_word = true;
                if (word_length == m_word_size) {
                    auto start = perm_binary_search(word_value, genome_words_number);
                    m_permutation->set_field(start, position++);
                    word_value %= divisor;
                    word_length -= m_shift;
                    additional_word = false;
                }
            }
        } while (bytes_read == BUFFER_SIZE);

        if (additional_word) {
            while (word_length != m_word_size) {
                fill_text(char_counter++, 0);
                word_value *= this->m_alphabet->size();
                word_length++;
            }
            auto start = perm_binary_search(word_value, genome_words_number);
            m_permutation->set_field(start, position);
        }
    }

    int Index::count_index(std::string const &pattern, ulong const from, ulong *numocc) const {
        if (from + m_word_size > pattern.size()) {
            return -1;
        }

        auto word_value = m_alphabet->get_word_value(pattern, from, from + m_word_size) + 1;
        auto position = m_select1->select(word_value);
        auto next_position = m_select1->select(word_value + 1);

        *numocc = next_position - position - 1;

        return 0;
    }

    /* Funkcja obliczająca ilość wystąpień słowa w indeksie */
    int Index::count(std::string const &pattern, ulong const length, ulong *numocc) const {
        if (pattern.length() < m_word_size + m_shift - 1)
            return count_short(pattern, length, false, numocc, NULL);
        else
            return count_full_words(pattern, length, false, numocc, NULL);
    }

    int Index::locate_index(std::string const &pattern, ulong const from, std::vector<ulong> *occ, ulong *numocc) const {
        if (from + m_word_size > pattern.size()) {
            return -1;
        }

        auto word_value = m_alphabet->get_word_value(pattern, from, from + m_word_size) + 1;
        auto position = m_select1->select(word_value) - word_value + 1;
        auto next_position = m_select1->select(word_value + 1) - word_value;

        for (ulong i = position; i < next_position; ++i) {
            occ->push_back(m_permutation->pi(i) * m_shift);
        }

        *numocc = next_position - position + 1;

        return 0;
    }

    int Index::locate(std::string const &pattern, ulong const length, std::vector<ulong> *occ,
            ulong *numocc) const {
        //std::cerr << "locate w index" << pattern.length() << " " << m_word_size << " " << m_shift << "\n";
        if (pattern.length() < m_word_size + m_shift - 1) {
            return count_short(pattern, length, true, numocc, occ);
        } else {
            return count_full_words(pattern, length, true, numocc, occ);
        }
    }

    void Index::verify_occurrences(size_type const start, size_type const end, size_type const length,
            size_type const offset, ulong *numocc, std::vector<ulong> *occ) const {
        for (ulong i = start; i < end; ++i) {
            size_type word_position = m_permutation->pi(i) * m_shift;

            if (word_position + offset + length <= m_text_length) {
                occ->push_back(word_position + offset);
            } else {
                --(*numocc);
            }
        }
    }

    int Index::count_exact_size_word(std::string const &pattern, size_type, ulong *numocc,
            bool const locate, std::vector<ulong> *occ) const {
        size_type word_value = m_alphabet->get_word_value(pattern, 0,
                m_word_size) + 1;
        size_type position = m_select1->select(word_value) - word_value + 1;
        size_type next_position = m_select1->select(word_value + 1) - word_value;
        *numocc = next_position - position;

        if (locate) {
            verify_occurrences(position, next_position, pattern.length(),
                    0, numocc, occ);
        } else if (m_additional_text_length > 0) {
            /** TODO  **/
        }

        return 0;
    }

    void Index::check_occ_end(std::string const &pattern, ulong *numocc) const {
        if (m_additional_text_length == 0)
            return;

        value_type pattern_value = m_alphabet->get_word_value(pattern);
        size_type length;
        std::string extracted_text;
        extract(m_text_length - pattern.length() + 1, m_text_length + m_additional_text_length,
                &extracted_text, &length);

        value_type current_word_value = m_alphabet->get_word_value(extracted_text, 0,
                pattern.length());
        value_type divisor = m_alphabet->pow_wsize(pattern.length() - 1);

        for (size_type i = 0; i < m_additional_text_length; ++i) {
            if (pattern_value == current_word_value) {
                --(*numocc);
            }

            current_word_value %= divisor;
            current_word_value *= m_alphabet->size();
        }
    }

    void Index::add_last_occ(std::string const &pattern, bool const locate,
            ulong *numocc, std::vector<ulong> *occ) const {
        if ((int) (m_word_size - m_shift) < (int) pattern.length())
            return;

        size_type word_value = m_alphabet->get_word_value(pattern);
        size_type word_position_index = m_permutation->get_size() * m_shift;
        value_type extracted_value = extract_value(word_position_index,
                m_word_size - m_shift);

        value_type modulo = m_alphabet->pow_wsize(m_word_size - m_shift);
        value_type divisor = m_alphabet->pow_wsize(m_word_size - m_shift - pattern.length());

        for (size_type i = m_shift; i + pattern.length() <= m_word_size; ++i) {
            ulong current_value = extracted_value % modulo;
            current_value /= divisor;

            if (word_value == current_value &&
                    word_position_index + i - m_shift < m_text_length) {
                (*numocc)++;

                if (locate) {
                    occ->push_back(word_position_index + i - m_shift);
                }
            }

            divisor /= m_alphabet->size();
            modulo /= m_alphabet->size();
        }
    }

    int Index::count_short_left(std::string const &pattern, bool const locate, ulong *numocc,
            std::vector<ulong> *occ) const {

        size_type start = (ulong) std::max(0, (int) (pattern.length() - m_word_size + 1));

        size_type left_index = 1;
        // value of the word pattern[-left_index..(-left_index + m_word_size)]
        // negative indices are any of the charachters from m_alphabet
        value_type left_chunk_value = m_alphabet->get_word_value(pattern, 0,
                std::min(m_word_size - 1, pattern.length()));
        if (pattern.length() < m_word_size - 1)
            left_chunk_value *= m_alphabet->pow_wsize(m_word_size - pattern.length() - 1);

        // value of the word pattern[m_word_size - 1], used for verification in left case
        value_type right_chunk_value = 0;
        if (m_word_size <= pattern.length())
            right_chunk_value = m_alphabet->get_word_value(pattern, m_word_size - left_index,
                pattern.length());

        value_type divisor = m_alphabet->pow_wsize(m_word_size - 1);
        // value used in left case, multiplication of this value is added to
        // current_left_value
        value_type lowest_divisor = divisor;
        // value used in loop in left case
        value_type multiplier = this->m_alphabet->size();

        value_type limit = (m_shift - start) / 2;
        if ((m_shift - start) % 2)
            ++limit;
        limit = m_shift - std::min((ulong) limit + start, (ulong) pattern.length());

        for (size_type i = 0; i < limit; ++i) {
            for (ulong k = 0; k < multiplier; ++k) {
                value_type lower_bound = get_position_in_permutation(left_chunk_value +
                        k * lowest_divisor);

                // there is chunk on the right which must be verified
                if (m_word_size - left_index < pattern.length()) {
                    value_type upper_bound = get_position_in_permutation(left_chunk_value +
                            k * lowest_divisor + 1);

                    for (ulong j = lower_bound; j < upper_bound; ++j) {
                        size_type word_position = m_permutation->pi(j);
                        size_type word_position_index = word_position * m_shift;
                        size_type right_chunk_length = pattern.length() - m_word_size + left_index;

                        if (right_chunk_length > 0 &&
                                word_position_index + m_word_size + right_chunk_length <= m_text_length + 1 &&
                                right_chunk_value == extract_value(word_position_index + m_word_size,
                                right_chunk_length)) {
                            (*numocc)++;

                            if (locate) {
                                occ->push_back(word_position_index + left_index);
                            }
                        }
                    }
                } else {
                    value_type upper_bound = get_position_in_permutation((left_chunk_value +
                            k * lowest_divisor) + m_alphabet->pow_wsize(m_word_size - left_index -
                            pattern.length()));

                    *numocc += upper_bound - lower_bound;

                    if (locate) {
                        verify_occurrences(lower_bound, upper_bound, pattern.length(),
                                left_index, numocc, occ);
                    }
                }
            }

            if (m_word_size - left_index - 1 < pattern.length())
                right_chunk_value += m_alphabet->get_char_value(pattern[m_word_size - left_index - 1]) *
                m_alphabet->pow_wsize(pattern.length() - m_word_size + left_index);

            left_chunk_value /= this->m_alphabet->size();
            multiplier *= this->m_alphabet->size();
            ++left_index;
            lowest_divisor /= m_alphabet->size();
        }

        return 0;
    }

    int Index::count_short_right(std::string const &pattern, bool const locate, ulong *numocc,
            std::vector<ulong> *occ) const {
        size_type start = (ulong) std::max(0, (int) (pattern.length() - m_word_size + 1));
        value_type left_side_value = m_alphabet->get_word_value(pattern, 0, start);
        value_type right_window_value = m_alphabet->get_word_value(pattern, start,
                pattern.length());

        if (start + m_word_size > pattern.length()) {
            right_window_value *= m_alphabet->pow_wsize(start + m_word_size -
                    pattern.length());
        }

        value_type limit = (m_shift - start) / 2;
        if ((m_shift - start) % 2)
            ++limit;
        limit = std::min((ulong) limit + start, (ulong) pattern.length());

        value_type divisor = m_alphabet->pow_wsize(m_word_size - 1);

        for (size_type i = start; i < limit; ++i) {
            value_type right_window_value_bound = right_window_value +
                    m_alphabet->pow_wsize(i + m_word_size - pattern.length());

            value_type lower_bound = get_position_in_permutation(right_window_value);
            value_type upper_bound = get_position_in_permutation(right_window_value_bound);

            if (i == 0) {
                *numocc += upper_bound - lower_bound;

                if (locate) {
                    verify_occurrences(lower_bound, upper_bound, pattern.length(),
                            0, numocc, occ);
                }
            } else {
                for (ulong j = lower_bound; j < upper_bound; ++j) {
                    size_type word_position_index = m_permutation->pi(j) * m_shift;

                    if (word_position_index >= i &&
                            word_position_index + i <= m_text_length + 1 &&
                            left_side_value == extract_value(word_position_index - i, i)) {
                        ++(*numocc);

                        if (locate) {
                            occ->push_back(word_position_index - i);
                        }
                    }

                }
            }

            right_window_value %= divisor;
            right_window_value *= m_alphabet->size();
            left_side_value *= m_alphabet->size();
            left_side_value += m_alphabet->get_char_value(pattern[i]);
        }

        return 0;
    }

    int Index::count_short(std::string const &pattern, ulong length,
            bool const locate, ulong *numocc, std::vector<ulong> *occ) const {
        length = std::min(m_text_length, length);

        if (pattern.length() >= m_word_size)
            count_full_words(pattern, length, locate, numocc, occ);

        count_short_right(pattern, locate, numocc, occ);
        count_short_left(pattern, locate, numocc, occ);

        add_last_occ(pattern, locate, numocc, occ);
        if (!locate) {
            check_occ_end(pattern, numocc);
        }

        return 0;
    }

    int Index::save_index(std::ostream& out) const {
        out.write((char *) &m_word_size, sizeof (size_type));
        out.write((char *) &m_shift, sizeof (size_type));
        out.write((char *) &m_text_length, sizeof (size_type));
        out.write((char *) &m_additional_text_length, sizeof (size_type));

        m_alphabet->save(out);
        m_bit_vector->serialize(out);
        m_rank1->serialize(out);
        m_select1->serialize(out);
        m_permutation->save(out);

        return 0;
    }

    void Index::load(std::istream &in) {
        std::cerr << "w indexie^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^6\n";
        in.read((char *) &m_word_size, sizeof (size_type));
        in.read((char *) &m_shift, sizeof (size_type));
        in.read((char *) &m_text_length, sizeof (size_type));
        in.read((char *) &m_additional_text_length, sizeof (size_type));

        m_alphabet = Alphabet::load(in);

        m_bit_vector = new sdsl::bit_vector();
        m_bit_vector->load(in);

        m_rank1 = new sdsl::bit_vector::rank_1_type();
        m_rank1->load(in, m_bit_vector);

        m_select1 = new sdsl::bit_vector::select_1_type;
        m_select1->load(in, m_bit_vector);
    }

    Index *Index::load_index(std::istream &in) {
        uint index_type;
        std::cerr << "load index\n";
        in.read((char *) &index_type, sizeof (uint));
        if (index_type != IndexFM::INDEX_TYPE && index_type != IndexNC::INDEX_TYPE && index_type != IndexCSA::INDEX_TYPE && index_type != IndexBC::INDEX_TYPE) {
            int position = (int) in.tellg() - sizeof (uint);
            in.seekg(position);
        }
        Index *result = nullptr;
        if (index_type == IndexBitVector::INDEX_TYPE) {
            result = new IndexBitVector();
            result->load(in);
        } else if (index_type == IndexPerm::INDEX_TYPE) {
            result = new IndexPerm();
            result->load(in);
        } else if (index_type == IndexLookupTable::INDEX_TYPE) {
            result = new IndexLookupTable();
            result->load(in);
        } else if (index_type == IndexWaveletTree::INDEX_TYPE) {
            result = new IndexWaveletTree();
            result->load(in);
        } else if (index_type == IndexFM::INDEX_TYPE) {
            result = new IndexFM();
            result->load(in);
        } else if (index_type == IndexNC::INDEX_TYPE) {
            result = new IndexNC();
            result->load(in);
        } else if (index_type == IndexCSA::INDEX_TYPE) {
            result = new IndexCSA();
            result->load(in);
        } else if (index_type == IndexBC::INDEX_TYPE) {
            result = new IndexBC();
            result->load(in);
        } else {
            std::cerr << "Couldn't load index from file, wrong format.";
            throw std::runtime_error("Wrong file.");
        }

        return result;
    }

}
