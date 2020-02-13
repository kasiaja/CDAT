#include "IndexWaveletTree.hpp"

namespace cdat {

uint const IndexWaveletTree::INDEX_TYPE = 512;

IndexWaveletTree::IndexWaveletTree(size_type word_size, size_type shift) :
    Index(word_size, shift) {}

IndexWaveletTree::IndexWaveletTree(size_type word_size, size_type shift, size_type text_length,
                                   size_type additional_text_length, sdsl::bit_vector *bit_vector,
                                   sdsl::bit_vector::rank_1_type *rank1,
                                   sdsl::bit_vector::select_1_type *select1,
                                   Permutation *permutation, Alphabet *alphabet,
                                   sdsl::wt_huff<sdsl::bit_vector, sdsl::rank_support_v<>,
                                   sdsl::select_support_scan<>, sdsl::select_support_scan<> > *text) :

    Index(word_size, shift, text_length, additional_text_length,
          bit_vector, rank1, select1, permutation, alphabet),
    m_text(text) {}

IndexWaveletTree::~IndexWaveletTree() {
    delete m_text;
}

bool IndexWaveletTree::check_word(size_type const position, size_type const length, const char *pattern) const {
    for (size_type i = 0; i < length; ++i) {
        if (pattern[i] != (*m_text)[i + position])
            return false;
    }

    return true;
}

/* Fukcja oblicza ilość słów, które są dostatecznie długie */
int IndexWaveletTree::count_full_words(std::string const &pattern, ulong length,
                                       bool const locate, ulong *numocc,
                                       std::vector<ulong> *occ) const {
    /* jeżeli wzorzec jest długości naszego okna to wystarczy policzyć ilość zer
     * w wektorze bitów */
    if (pattern.length() == m_word_size) {
        return count_exact_size_word(pattern, length, numocc, locate, occ);
    }

    size_type limit = m_shift;
    /* jeśli wywołane z count_short */
    if (m_word_size + m_shift - 1 > pattern.length())
        limit = pattern.length() - m_word_size + 1;

    uint start = 0;
    while (start < limit) {
        /* wartość słowa w oknie */
        size_t word_value = m_alphabet->get_word_value(pattern, start,
                                                       start + m_word_size) + 1;

        /* wyliczanie miejsca wystąpienia słowa w permutacji, od position do next_position */
        size_t position = m_select1->select(word_value);
        size_t next_position = m_select1->select(word_value + 1) - word_value;

        position = position - word_value + 1;
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
IndexWaveletTree::value_type IndexWaveletTree::extract_value(size_type const from, size_type const length) const{
    value_type result = 0;
    size_type to = from + length;
    size_type start = from;

    while (start < to) {
        result *= m_alphabet->size();
        if (start < m_text->size())
            result += m_alphabet->get_char_value((*m_text)[start]);
        ++start;
    }

    return result;
}

/* funkcja zwraca tekst od pozycji from do pozycji to w tekscie */
int IndexWaveletTree::extract(ulong const from, ulong const to,
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
        if (from + current_length < m_text->size())
            *text += (*m_text)[from + current_length];
        else
            *text += m_alphabet->get_char_from_value(0);
        ++current_length;
    }

    return 0;
}

int IndexWaveletTree::save_index(std::ostream& out) const {
    out.write((char *) &IndexWaveletTree::INDEX_TYPE, sizeof(uint));
    Index::save_index(out);
    m_text->serialize(out);

    return 0;
}

void IndexWaveletTree::load(std::istream& in) {
    uint index_type;
    in.read((char *) &index_type, sizeof(uint));

    if (index_type != INDEX_TYPE) {
        std::cerr << "Wrong index type!\n";
    }

    Index::load(in);
    m_permutation = Permutation::load(in);

    typedef sdsl::wt_huff<sdsl::bit_vector, sdsl::rank_support_v<>,
                          sdsl::select_support_scan<>, sdsl::select_support_scan<> > wavelet_tree;

    m_text = new wavelet_tree();
    m_text->load(in);
}

}
