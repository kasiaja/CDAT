// The described algorithm solves the problem of pattern searching in a text and
// was designed specifically for large DNA sequences. It scans the text once
// to create an index - a data structure that accelerates finding pattern occurrences.
// It stores data in a compressed form a direct-address table for fixed-length subwords
// regularly sampled from the text and is implemented in
// the Compressed Direct-Address Table (CDAT) library.

#include "IndexLookupTable.hpp"
#include <math.h> 

namespace cdat {

    uint const IndexLookupTable::INDEX_TYPE = 1024;

    IndexLookupTable::IndexLookupTable(size_type word_size, size_type shift, size_type text_length,
            size_type additional_text_length, sdsl::bit_vector *bit_vector,
            sdsl::bit_vector::rank_1_type *rank1,
            sdsl::bit_vector::select_1_type *select1,
            Permutation *permutation, Alphabet *alphabet, sdsl::int_vector<> *text) :

    Index(word_size, shift, text_length, additional_text_length,
    bit_vector, rank1, select1, permutation, alphabet),
    m_text(text) {
        pref_length = 5
        substring_length = pref_length * 2;
        init();
    }

    IndexLookupTable::IndexLookupTable(size_t word_size, size_t shift, uint prefix) :
    Index(word_size, shift) {
        pref_length = 5
        substring_length = pref_length * 2;
    }

    IndexLookupTable::~IndexLookupTable() {
    }

    // check if the words equal - if pattern equals the subword starting at position and ending at length
    // character by character. It's a long process but we rarely have to use it
    bool IndexLookupTable::check_word(size_type const position, size_type const length, const char *pattern) const {
        for (uint i = 0; i < length; ++i) {
            char curr_char = m_alphabet->get_char_from_value((*m_text)[position + i]);
            if (pattern[i] != curr_char)
                return false;
        }

        return true;
    }

    // returns words starting at from of length length
    IndexLookupTable::value_type IndexLookupTable::extract_value(size_type const from,
            size_type const length) const {
        value_type result = 0;
        size_type to = std::min(m_text_length, from + length);
        size_type start = from;
        while (start < to) {
            result *= m_alphabet->size();
            result += (*m_text)[start];
            ++start;
            
        }
        return result;
    }

    // returns words starting at from and ending at to
    int IndexLookupTable::extract(ulong const from, ulong const to,
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

    // returns a substring of pattern from begin of length substring_length
    std::string IndexLookupTable::read_word_from_pattern(ulong begin, ulong length) const {
        return m_alphabet->get_word_from_value(extract_value(begin, length), length);
    }

    // returns first lexicographically string of length k
    std::string IndexLookupTable::first_lexicographically(ulong k) const {
        std::string result = "";
        for (ulong i = 0; i < k; i++) {
            result += "A";
        }
        return result;
    }

    // returns last or rather after last lexicographically string of length k,
    // so basically a string consisting of letters 'T' of length k
    std::string IndexLookupTable::last_lexicographically(ulong k) const {
        std::string result = "";
        for (ulong i = 0; i < k; i++) {
            result += "T";
        }
        return result;
    }

    std::string IndexLookupTable::next_lexicographically(std::string substring) const {
        for (ulong i = pref_length - 1; i >= 0; i--) {
            switch (substring[i]) {
                case 'A': substring[i] = 'C';
                    return substring;
                case 'C':
                    substring[i] = 'G';
                    return substring;
                case 'G':
                    substring[i] = 'T';
                    return substring;
                case 'T':
                    substring[i] = 'A';
            }
        }
        // it was the last lexicographically substring of this length(T...T)
        return last_lexicographically(pref_length);
    }

    class PatternPosition {
    private:
        const IndexLookupTable& ilt;

    public:

        PatternPosition(const IndexLookupTable& pilt) : ilt(pilt) {
        }

        //  the arguments are the beginning positions of two pattern substrings of length substring_length and it returns 
        //  true if the first one is smaller lexicographically than the second
        bool operator()(ulong i, ulong j) const {
            return ilt.temp_subs.at(i) < ilt.temp_subs.at(j);
        }

        // similar to the above only one of the arguments in a string reference instead of the index
        bool operator()(const std::string& s, const ulong& i) const {
            std::string pattern_part = ilt.read_word_from_pattern(i + ilt.pref_length, ilt.substring_length - ilt.pref_length);
            for (ulong j = 0; j < ilt.substring_length - ilt.pref_length; j++) {
                if (pattern_part[j] < s[j])
                    return true;
                if (pattern_part[j] > s[j])
                    return false;
            }
            return false;
        }

        // similar to the above only one of the arguments in a string reference instead of the index
        bool operator()(const ulong& i, const std::string& s) const {
            std::string pattern_part = ilt.read_word_from_pattern(i + ilt.pref_length, ilt.substring_length - ilt.pref_length);
            for (ulong j = 0; j < ilt.substring_length - ilt.pref_length; j++) {
                if (pattern_part[j] < s[j])
                    return true;
                if (pattern_part[j] > s[j])
                    return false;
            }
            return false;
        }

    };

    class PatternPositionSearch {
    private:
        const IndexLookupTable& ilt;

    public:

        PatternPositionSearch(const IndexLookupTable& pilt) : ilt(pilt) {
        }

        // returns information if a substring of length pref_length would fit between 
        // the values in the first and the second argument
        bool operator()(std::pair<ulong, ulong> i, ulong j) const {
            return i.first < ilt.extract_value(j + ilt.pref_length, i.second);
        }
        
        // returns information if a substring of length pref_length would fit between 
        // the values in the first and the second argument
        bool operator()(ulong i, std::pair<ulong, ulong> j) const {
            return ilt.extract_value(i + ilt.pref_length, j.second) < j.first;
        }
    };

    // sorts element within a bucket
    void IndexLookupTable::sortAtPosition(ulong begin, ulong end, ulong position) {
        std::deque<ulong> buckets [m_alphabet->size()];
        for (ulong i = begin; i < end; i++) {
            if (substrings[i] + position < m_text->size())
                buckets[(*m_text)[substrings[i] + position]].push_back(substrings[i]);
            else
                buckets[0].push_back(substrings[i]);
        }
        ulong where = begin;
        for (ulong j = 0; j < m_alphabet->size(); j++) {
            for (ulong i = 0; i < buckets[j].size(); i++) {
                substrings[where] = buckets[j][i];
                where++;
            }
        }
    }

    void IndexLookupTable::radix_sort(ulong begin, ulong end) {
        for (ulong i = substring_length - 1; i >= pref_length; i--) {
            sortAtPosition(begin, end, i);
        }
    }

    void IndexLookupTable::radix_sort_init() {
        // there are two arrays so that we can use them in turns and not overwrite any data - 
        // in one round we use the first one as reference and fill in the second one
        // and then the other way round
        sdsl::int_vector<INT_ROZM> substringsPom[2];
        // we only keep each m_shift's value, hence the size
        substringsPom[1].resize(m_text->size() / m_shift);
        substringsPom[0].resize(m_text->size() / m_shift);
        lookup_table.resize(std::pow(m_alphabet->size(), pref_length));
        for (ulong i = 0; i < (ulong) m_text->size() - substring_length + 1; i += m_shift) {
            substringsPom[0][i/m_shift] = i;
        }
        // pref_length is the length of a word used in the first phase of the algorithm - 
        // it determines the number of backets (alphabet_size^pref_length).
        // substring_length is used in the second phase of the algorithm - it defines how many
        // characters we take into consideration when we sort the words
        ulong przes = (substring_length/pref_length - 1) * pref_length;
        for (ulong y = 0; y < pref_length/substring_length; y++) {
            // init
            for (ulong i =  0; i < lookup_table.size() - 1; i++) {
                lookup_table[i] = 0;
            }
            // count how many times each word appears
            for (ulong i = 0; i < (ulong) substringsPom[y % 2].size(); i++) {
                ulong current_word = extract_value(substringsPom[y%2][i]+przes, pref_length);
                lookup_table[current_word]++;
            }
            //count partial sums - where do information about each word start in the array
            ulong partial_sum = 0;
            for (ulong i = 0; i < lookup_table.size(); i++) {
                partial_sum += lookup_table[i];
                lookup_table[i] = partial_sum - lookup_table[i];
            }
           
            // put information about text's subwords in the right place of the array
            for (ulong i = 0; i < (ulong) m_text->size() - substring_length + 1; i += m_shift) {
                ulong current_word = extract_value(i + przes, pref_length);
                substringsPom[(y+1)%2][lookup_table[current_word]] = i;
                lookup_table[current_word]++;
            }
            
            // lookup_table was modified in the previous step, so we need to fix it
            for (ulong i = lookup_table.size() - 1; i > 0; i--) {
                lookup_table[i] = lookup_table[i - 1];
            }
            lookup_table[0] = 0; // first range starts at 0
            
            przes -= pref_length;
        }
        substringsPom[(substring_length / pref_length) %2].resize(0);
        substrings.resize(m_text->size() / m_shift);
        int where = 0;
        for (ulong i: substringsPom[(substring_length / pref_length + 1) %2])
        {
            substrings[where] = i;
            where++;
        }
        sdsl::util::bit_compress(substrings);
        sdsl::util::bit_compress(lookup_table);
        sdsl::util::clear(substringsPom[(substring_length / pref_length + 1) %2]);
    }
    
    void IndexLookupTable::init() {
        radix_sort_init();
    }

    ulong IndexLookupTable::number_for_letter(char c) const {
        switch (c) {
            case 'A':
                return 0;
            case 'C':
                return 1;
            case 'G':
                return 2;
            case 'T':
                return 3;
        }
        return 0;
    }

    ulong IndexLookupTable::count_where_pointer_for_prefix(std::string const &s) const {
        int result = 0;
        for (ulong i = pref_length - 1; i >= 0; i--) {
            result *= 4;
            result += number_for_letter(s[i]);
        }
        return result;
    }

    // Cut first pref_length characters from a string
    std::string IndexLookupTable::take_prefix(std::string const &pattern) const {
        return pattern.substr(0, pref_length);
    }

    // counts how many times the pattern fits fully in text. This value is in argument numocc 
    // We can decide to get the locations of these occurences (when we set argument locate to true) and we get this result in occ
    int IndexLookupTable::count_full_words(std::string const &pattern, ulong length, bool const locate, ulong *numocc, std::vector<ulong> *occ) const {
        (*numocc) = 0;
        for (ulong j = 0; j < std::min(length, m_shift); j++) {
            std::string current_pattern = pattern.substr(j);
            // we find the right bucket - the range in the array where words with the right prefix (of length pref_length) are
            ulong pointer_to_prefix_index, where_first_pointer, where_last_pointer;
            if (length > j + pref_length) {
                pointer_to_prefix_index = m_alphabet->get_word_value(current_pattern.substr(0, pref_length));
                where_first_pointer = lookup_table[pointer_to_prefix_index];
                if (pointer_to_prefix_index + 1 >= lookup_table.size())
                    where_last_pointer = substrings.size();
                else
                    where_last_pointer = lookup_table[pointer_to_prefix_index + 1];
            } else {
                const std::string extended_pattern = current_pattern + first_lexicographically(length - j);
                pointer_to_prefix_index = m_alphabet->get_word_value(extended_pattern.substr(0, pref_length));
                where_first_pointer = lookup_table[pointer_to_prefix_index];

                const std::string extended_pattern2 = next_lexicographically(current_pattern) + first_lexicographically(length - j);
                pointer_to_prefix_index = m_alphabet->get_word_value(extended_pattern2.substr(0, pref_length));
                where_last_pointer = lookup_table[pointer_to_prefix_index];
            }

            // search within the right bucket
            ulong current_pattern_ulong = m_alphabet->get_word_value(current_pattern.substr(pref_length, substring_length - pref_length));
            PatternPositionSearch patternPositionSearch(*this);
            auto begin = lower_bound(substrings.begin() + where_first_pointer, substrings.begin() + where_last_pointer, 
                                     std::make_pair(current_pattern_ulong, substring_length - pref_length), patternPositionSearch);
            auto end = upper_bound(substrings.begin() + where_first_pointer, substrings.begin() + where_last_pointer, std::make_pair(current_pattern_ulong, substring_length - pref_length), patternPositionSearch);
            ulong word_length = pattern.size(); // should equal substring_size
            for (auto i = begin; i < end; i++) {
                if (*i - j + word_length < m_text_length) {
                    if (*i >= j && check_word(*i - j, (size_type) word_length, pattern.c_str())) {
                        if (locate) {
                            occ->push_back(*i);
                        }
                        (*numocc)++;
                    }
                }
            }
        }

        // we only do that if we need occurences' locations
        if (locate) {
            std::sort(occ->begin(), occ->end());
            occ->erase(std::unique(occ->begin(), occ->end()), occ->end());
            (*numocc) = occ->size();
        }
        return 0;
    }

    void write_vector(std::ostream& out, const sdsl::int_vector<INT_ROZM>& vec) {
        uint r = vec.size();
        out.write((char *) &r, sizeof (uint));
        for (std::vector<uint>::size_type i = 0; i < vec.size(); i++) {
            uint tmp = vec[i];
            out.write((char *) &tmp, sizeof (uint));
        }
    }

    void load_vector(std::istream& in, sdsl::int_vector<INT_ROZM>* vec) {
        uint n, a;
        in.read((char *) &n, sizeof (uint));
        vec->resize(n);
        for (uint i = 0; i < n; i++) {
            in.read((char *) &a, sizeof (uint));
            (*vec)[i] = (int)a;
        }
    }

    int IndexLookupTable::save_index(std::ostream& out) const {
        out.write((char *) &IndexLookupTable::INDEX_TYPE, sizeof (uint));
        out.write((char *) &m_word_size, sizeof (size_type));
        out.write((char *) &m_shift, sizeof (size_type));
        out.write((char *) &m_text_length, sizeof (size_type));
        out.write((char *) &m_additional_text_length, sizeof (size_type));

        m_alphabet->save(out);

        m_text->serialize(out);
        write_vector(out, substrings);
        write_vector(out, lookup_table);
        out.write((char *) &substring_length, sizeof (uint));
        out.write((char *) &pref_length, sizeof (uint));
        return 0;
    }

    void IndexLookupTable::load(std::istream& in) {
        uint index_type;
        in.read((char *) &index_type, sizeof (uint));

        if (index_type != INDEX_TYPE) {
            std::cerr << "Wrong index type!\n";
        }
        in.read((char *) &m_word_size, sizeof (size_type));
        in.read((char *) &m_shift, sizeof (size_type));
        in.read((char *) &m_text_length, sizeof (size_type));
        in.read((char *) &m_additional_text_length, sizeof (size_type));

        m_alphabet = Alphabet::load(in);

        m_text = new sdsl::int_vector<>(m_text_length + m_additional_text_length, 0,
                (uint8_t) cds_utils::bits((uint) m_alphabet->size()));
        m_text->load(in);
        load_vector(in, &substrings);
        load_vector(in, &lookup_table);
        in.read((char *) &substring_length, sizeof (uint));
        in.read((char *) &pref_length, sizeof (uint));
    }
}
