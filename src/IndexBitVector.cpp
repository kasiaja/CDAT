#include "IndexBitVector.hpp"

namespace cdat {

uint const IndexBitVector::INDEX_TYPE = 128;

IndexBitVector::IndexBitVector(size_type word_size, size_type shift) :
    Index(word_size, shift) {
		std::cerr<<"maly konstr\n";}

IndexBitVector::IndexBitVector(size_type word_size, size_type shift, size_type text_length,
                               size_type additional_text_length, sdsl::bit_vector *bit_vector,
                               sdsl::bit_vector::rank_1_type *rank1,
                               sdsl::bit_vector::select_1_type *select1,
                               Permutation *permutation, Alphabet *alphabet, sdsl::int_vector<> *text) :

    Index(word_size, shift, text_length, additional_text_length,
          bit_vector, rank1, select1, permutation, alphabet),
    m_text(text) {
		
		std::cerr<<"konstr bitvector\n";}

IndexBitVector::~IndexBitVector() {
    delete m_text;
}

bool IndexBitVector::check_word(size_type const position, size_type const length, const char *pattern) const {
    for (uint i = 0; i < length; ++i) {
        char curr_char = m_alphabet->get_char_from_value((*m_text)[position + i]);
        if (pattern[i] != curr_char)
            return false;
    }

    return true;
}

/* Fukcja oblicza ilość słów, które są dostatecznie długie */
int IndexBitVector::count_full_words(std::string const &pattern, ulong length,
                                     bool const locate, ulong *numocc,
                                     std::vector<ulong> *occ) const {
    if (pattern.length() == m_word_size) {
        return count_exact_size_word(pattern, length, numocc, locate, occ);
    }

    /* jeżeli słowo jest dłuższe niż rozmiar okna, to będziemy przechodzić oknem po
     * słowie dokładnie shift razy */
    size_type start = 0;
    size_type limit = m_shift;
    /* jeśli wywołane z count_short */
    if (m_word_size + m_shift - 1 > pattern.length())
        limit = pattern.length() - m_word_size + 1;

    while (start < limit) {
        /* wartość słowa w oknie */
        size_type word_value = m_alphabet->get_word_value(pattern, start,
                                                          start + m_word_size) + 1;

        /* wyliczanie miejsca wystąpienia słowa w permutacji, od position do next_position */
        size_t position = m_select1->select(word_value) - word_value + 1;
        size_t next_position = m_select1->select(word_value + 1) - word_value;

        for (ulong i = position; i < next_position; ++i) {
            size_type right_end = start + m_word_size;
            /* pobranie pozycji w tekście słowa, które znajduje się akutalnie w oknie */
            size_type curr_word_position = m_permutation->pi(i);
            size_type word_index_position = curr_word_position * m_shift;
            /* z lewej lub z prawej strony okna nie ma wystarczająco liter w tekście */
            if ((word_index_position < start) ||
                (m_text_length < (word_index_position + m_word_size) +
                    (pattern.length() - right_end)))
                continue;

            /* jeżeli okno nie znajduje się na początku wzorca to sprawdzamy czy lewa
             * strona wzorca i tekstu się zgadzają, jeśli nie przechodzimy do
             * następnego obrotu */
            if (start != 0) {
                if (!check_word(word_index_position - start, start,
                                pattern.c_str())) {
                    continue;
                }
            }

            /* sprawdzenie tekstu po prawej stronie od okna */
            if (start + m_word_size < pattern.length()) {
                if (!check_word(word_index_position + m_word_size,
                                pattern.length() - start - m_word_size,
                                pattern.c_str() + m_word_size + start)) {
                    continue;
                }
            }

            if (locate) {
                occ->push_back(word_index_position - start);
            }

            (*numocc)++;
        }

        ++start;
    }

    return 0;
}

/* zwraca wartość słowa zaczynającego się na from o dlugości length */
IndexBitVector::value_type IndexBitVector::extract_value(size_type const from,
                                                         size_type const length) const{
    value_type result = 0;
    size_type to = from + length;
    size_type start = from;

    while (start < to) {
        result *= m_alphabet->size();
        result += (*m_text)[start];
        ++start;
    }

    return result;
}

/* funkcja zwraca tekst od pozycji from do pozycji to w tekscie */
int IndexBitVector::extract(ulong const from, ulong const to,
                            std::string *text, ulong *length) const {
    ulong _to = std::min(to, m_text_length);
    if (_to < from) {
        *length = 0;
        return -1;
    }

    *length = to - from;
    text->reserve(*length);

    ulong current_length = 0;

    while (current_length < *length) {
        *text += m_alphabet->get_char_from_value((*m_text)[from + current_length]);
        ++current_length;
    }

    return 0;
}

int IndexBitVector::save_index(std::ostream& out) const {
	std::cerr<<"save_index  aa"<<m_text<<"\n";
    out.write((char *) &IndexBitVector::INDEX_TYPE, sizeof(uint));
    Index::save_index(out);
	std::cerr<<"aa"<<m_text<<"\n";
    m_text->serialize(out);

    return 0;
}

void IndexBitVector::load(std::istream& in) {
	std::cerr<<"load\n";
    uint index_type;
    in.read((char *) &index_type, sizeof(uint));

    if (index_type != INDEX_TYPE) {
        std::cerr << "Wrong index type!\n";
    }

    Index::load(in);
    m_permutation = Permutation::load(in);

    m_text = new sdsl::int_vector<>(m_text_length + m_additional_text_length, 0,
                                    (uint8_t) cds_utils::bits((uint) m_alphabet->size()));
    m_text->load(in);
}

}
