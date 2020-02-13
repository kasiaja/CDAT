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
        std::cerr << "duzy konstruktor!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!0"<<word_size<<"!\n";
        substring_length = 5;
        pref_length = substring_length*2;
        init();
    }

    IndexLookupTable::IndexLookupTable(size_t word_size, size_t shift, uint prefix) :
    Index(word_size, shift) {
        std::cerr << "maly konstr"<<word_size<<"\n";
        substring_length = prefix;
        pref_length = substring_length*2;
    }

    IndexLookupTable::~IndexLookupTable() {
        //std::cout<<"destruktor\n";
    }

    bool IndexLookupTable::check_word(size_type const position, size_type const length, const char *pattern) const {
        for (uint i = 0; i < length; ++i) {
            //std::cerr<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<position<<" "<<i<<" "<<pattern<<" "<<m_text_length<<"\n";
            char curr_char = m_alphabet->get_char_from_value((*m_text)[position + i]);
            if (pattern[i] != curr_char)
                return false;
        }

        return true;
    }

    /* zwraca wartość słowa zaczynającego się na from o dlugości length */
    IndexLookupTable::value_type IndexLookupTable::extract_value(size_type const from,
            size_type const length) const {
        value_type result = 0;
        size_type to = std::min(m_text_length, from + length);
        size_type start = from;
        // //std::cerr<<"extract_value from:"<<from<<" length:"<<length<<"m_text size"<<m_text_length<<"\n";
        while (start < to) {
            result *= m_alphabet->size();
            ////std::cerr<<"A"<<sizeof(m_text)/sizeof(m_text[0])<<"\n";
            result += (*m_text)[start]; //zmienione
            ////std::cerr<<"B\n";
            ++start;
            ////std::cerr<<"C\n";
        }
        // //std::cerr<<"koniec extract\n";
        return result;
    }

    /* funkcja zwraca tekst od pozycji from do pozycji to w tekscie */
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
            //assert(from + current_length < m_text_length);
            *text += m_alphabet->get_char_from_value((*m_text)[from + current_length]);
            ++current_length;
        }

        return 0;
    }

    // returns a substring of pattern from begin of length substring_length

    std::string IndexLookupTable::read_word_from_pattern(ulong begin, ulong length) const {
        //std::cout<<"read_from_pattern\n";
        return m_alphabet->get_word_from_value(extract_value(begin, length), length);
    }

    // returns first lexicographically string of length k

    std::string IndexLookupTable::first_lexicographically(ulong k) const {
        //std::cout<<"first_lexi\n";
        std::string result = "";
        for (ulong i = 0; i < k; i++) {
            result += "A";
        }
        return result;
    }

    // returns last or rather after last lexicographically string of length k

    std::string IndexLookupTable::last_lexicographically(ulong k) const {
        //std::cout<<"last_lexi\n";
        std::string result = "";
        for (ulong i = 0; i < k; i++) {
            result += "T";
        }
        return result;
    }

    std::string IndexLookupTable::next_lexicographically(std::string substring) const {
        //std::cout<<"next_lexicographically\n";
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

        //gets beginning positions of two pattern substrings of length substring_length and return 
        //true if the first one is smaller lexicographically than the second

        bool operator()(ulong i, ulong j) const {
            return ilt.temp_subs.at(i) < ilt.temp_subs.at(j); // ilt.extract_value(i + ilt.pref_length, ilt.substring_length - ilt.pref_length) < ilt.extract_value(j + ilt.pref_length, ilt.substring_length - ilt.pref_length);
        }

        bool operator()(const std::string& s, const ulong& i) const {
            ////std::cerr<<"YY\n";
            std::string pattern_part = ilt.read_word_from_pattern(i + ilt.pref_length, ilt.substring_length - ilt.pref_length);
            ////std::cerr<<"ZZ\n";
            for (ulong j = 0; j < ilt.substring_length - ilt.pref_length; j++) {
                if (pattern_part[j] < s[j])
                    return -1;
                if (pattern_part[j] > s[j])
                    return 1;
            }
            return 0;
        }

        bool operator()(const ulong& i, const std::string& s) const {
            ////std::cerr<<"Y"<<i + ilt.pref_length<<" "<<ilt.substring_length - ilt.pref_length<<"\n";
            std::string pattern_part = ilt.read_word_from_pattern(i + ilt.pref_length, ilt.substring_length - ilt.pref_length);
            ////std::cerr<<"Z\n";
            for (ulong j = 0; j < ilt.substring_length - ilt.pref_length; j++) {
                if (pattern_part[j] < s[j])
                    return 1;
                if (pattern_part[j] > s[j])
                    return -1;
            }
            return 0;
        }

    };

    class PatternPositionSearch {
    private:
        const IndexLookupTable& ilt;

    public:

        PatternPositionSearch(const IndexLookupTable& pilt) : ilt(pilt) {
        }

        //gets beginning positions of two pattern substrings of length substring_length and return 
        //true if the first one is smaller lexicographically than the second

        bool operator()(std::pair<ulong, ulong> i, ulong j) const {
            return i.first < ilt.extract_value(j + ilt.pref_length, i.second);
        }

        bool operator()(ulong i, std::pair<ulong, ulong> j) const {
            return ilt.extract_value(i + ilt.pref_length, j.second) < j.first;
        }
    };

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
        //assert(where == end);
    }

    void IndexLookupTable::radix_sort(ulong begin, ulong end) {
        for (ulong i = substring_length - 1; i >= pref_length; i--) {
            sortAtPosition(begin, end, i);
        }
    }

    void IndexLookupTable::radix_sort_init() {
        sdsl::int_vector<INT_ROZM> substringsPom[2];
        //assert(substring_length % pref_length == 0);
        
        std::cerr<<"substring length"<<substring_length<<"pref_length"<<pref_length<<"alphabet size"<<m_alphabet->size()<<" m_text size"<<m_text->size()<<"m_shift"<<m_shift<<"\n";
        //assert(false);
        //substringsPom[0].resize(m_text->size() / m_shift); // number of substrings TODO czy nie powinno byc ceil? Zostaje ten kawalek na koncu
        substringsPom[1].resize(m_text->size() / m_shift);
        substringsPom[0].resize(m_text->size() / m_shift);
        std::cerr << "ROZMIAR:" << std::pow(m_alphabet->size(), pref_length) << " " << m_text->size() / m_shift << "\n";
        lookup_table.resize(std::pow(m_alphabet->size(), pref_length)); // get_word_value(last_word), 0);
        std::cerr << "poczatek initttttt" << m_text << " " << lookup_table.size() << " " << pref_length << " " << substring_length << "\n";
        //substringsPom[0].resize((ulong) m_text->size() - substring_length + 1);
        for (ulong i = 0; i < (ulong) m_text->size() - substring_length + 1; i += m_shift) {
            substringsPom[0][i/m_shift] = i;
        }
        std::cerr<<"SP LL"<<substringsPom[0].size()<< " "<< m_text->size() / m_shift <<"\n";
        //assert(substringsPom[0].size() <= m_text->size() / m_shift + 1);
        std::cerr <<"bbb "<<lookup_table.size()<<" "<<m_alphabet->size()<<" "<<pref_length<<"\n";
        ulong przes = (substring_length/pref_length - 1) * pref_length;
        std::cerr<<"SUBSTRINGS SIZE IN MGBYTES: "<<sdsl::size_in_mega_bytes(substringsPom[0])<<" LOOKUP_TABLE: "<<sdsl::size_in_mega_bytes(lookup_table)<<
                                " TEXT: "<<sdsl::size_in_mega_bytes(*m_text)<<"\n";
        for (ulong y = 0; y < pref_length/substring_length; y++) {
            for (ulong i =  0; i < lookup_table.size() - 1; i++) {
                lookup_table[i] = 0;
            }
            std::cerr <<"ccc\n";
            for (ulong i = 0; i < (ulong) substringsPom[y % 2].size(); i++) {
                ulong current_word = extract_value(substringsPom[y%2][i]+przes, pref_length);
                std::cerr<<"AA"<<m_text->size()<<" "<<  substringsPom[y%2][i]<<" "<<current_word<<"\n";
                lookup_table[current_word]++;
            }
            //std::cerr << "init2\n";
            //uint maxRange = 0;
            ulong partial_sum = 0;
            for (ulong i = 0; i < lookup_table.size(); i++) {
                //fprintf(f, "%d", lookup_table[i]);
                //fprintf(f, ";");
                partial_sum += lookup_table[i];
                //maxRange = std::max(maxRange, lookup_table[i]);
                lookup_table[i] = partial_sum - lookup_table[i];
                std::cerr<<"LTi"<<lookup_table[i]<<"I"<<i<<"lt.size"<<lookup_table.size()<<"\n";
                assert(lookup_table[i] >= 0 && lookup_table[i] <= m_text->size());
            }
           // std::cerr << "maxRange:" << maxRange << "\n";

            //substrings.resize(m_text->size() / m_shift); // number of substrings TODO czy nie powinno byc ceil? Zostaje ten kawalek na koncu
            for (ulong i = 0; i < (ulong) m_text->size() - substring_length + 1; i += m_shift) {
                ulong current_word = extract_value(i + przes, pref_length);
                ////std::cerr<<"AA"<<m_alphabet->get_word_value(current_word)<<"\n";
                ////std::cerr<<"BB"<<lookup_table[m_alphabet->get_word_value(current_word)]<<"\n";
                std::cerr<<"QWER"<<current_word<<"\n";
                std::cerr<<"rozmiar lookup table:"<<std::pow(m_alphabet->size(), pref_length)<<"\n";
                std::cerr<<lookup_table[current_word]<<"\n";
                std::cerr<<(m_text->size() / m_shift)<<"\n";
                substringsPom[(y+1)%2][lookup_table[current_word]] = i;
                lookup_table[current_word]++;
            }
            //std::cerr << "init4\n";

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

    int IndexLookupTable::count_full_words(std::string const &pattern, ulong length, bool const locate, ulong *numocc, std::vector<ulong> *occ) const {
        /*for (ulong i=0; i<4; i++)
        {
            //std::cerr<< (m_alphabet->get_char_from_value(i)) <<" ";
        }*/
        //std::cerr << "substr_length: " << substring_length << " pref_length: " << pref_length << "\n";

        /*for (int i = 0; i < (int)substrings.size() - 1; i++) {
            std::string subst;
            ulong len;
            extract(substrings[i], substrings[i] + substring_length, &subst, &len);
            std::string substNast;
            extract(substrings[i + 1], substrings[i + 1] + substring_length, &substNast, &len);
            //std::cerr << "LLLLLLLL" << len << " " << substrings[i] << " " << subst << " " << substNast << std::endl;
            //assert(subst <= substNast);
           // if (subst > substNast) {
                //std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11\n";}
        }*/
        ///assert(false);
        std::cerr<<"TTTTTTTTTTTTT\n";
        
        (*numocc) = 0;
        //std::cerr << "poczatek count_full_words" << substring_length << " " << pref_length << "\n";
        for (ulong j = 0; j < std::min(length, m_shift); j++) {
            std::cerr << "petla" << j << "mshift:" << m_shift << " "<<pattern<< "pref_length:"<<pref_length<<"\n";
            std::string current_pattern = pattern.substr(j);
            ulong pointer_to_prefix_index, where_first_pointer, where_last_pointer;
            if (length > j + pref_length) { // TODO moze > shift + pref_length
                std::cerr << "if w count_full_wordscurrent pattern:" << current_pattern << " lookuptable.size:" << lookup_table.size() << "\n";
                pointer_to_prefix_index = m_alphabet->get_word_value(current_pattern.substr(0, pref_length));
                std::cerr << "pointer_to_prefix_index" << pointer_to_prefix_index << " lookuptable.size:" << lookup_table.size() << "\n";
                where_first_pointer = lookup_table[pointer_to_prefix_index];
                std::cerr << "if w count_full_words\n";
                if (pointer_to_prefix_index + 1 >= lookup_table.size())
                    where_last_pointer = substrings.size();
                else
                    where_last_pointer = lookup_table[pointer_to_prefix_index + 1];
            } else {
                std::cerr << "else w count_full_words\n"; // TODO dopelnianie poczatku
                const std::string extended_pattern = current_pattern + first_lexicographically(length - j);
                //std::cerr << "pat:" << current_pattern << " " << extended_pattern << "\n";
                pointer_to_prefix_index = m_alphabet->get_word_value(extended_pattern.substr(0, pref_length));
                //std::cerr << "pointer to prefix" << pointer_to_prefix_index << "\n";
                where_first_pointer = lookup_table[pointer_to_prefix_index];

                const std::string extended_pattern2 = next_lexicographically(current_pattern) + first_lexicographically(length - j);
                pointer_to_prefix_index = m_alphabet->get_word_value(extended_pattern2.substr(0, pref_length));
                where_last_pointer = lookup_table[pointer_to_prefix_index]; //TODO zmienilam first->last
            }

            ulong current_pattern_ulong = m_alphabet->get_word_value(current_pattern.substr(pref_length, substring_length - pref_length));
            //std::cerr << "po if w count_full_words" << substrings.size() << " " << where_first_pointer << " " << where_last_pointer << "\n";
            PatternPositionSearch patternPositionSearch(*this);
            auto begin = lower_bound(substrings.begin() + where_first_pointer, substrings.begin() + where_last_pointer, std::make_pair(current_pattern_ulong, substring_length - pref_length), patternPositionSearch);
            auto end = upper_bound(substrings.begin() + where_first_pointer, substrings.begin() + where_last_pointer, std::make_pair(current_pattern_ulong, substring_length - pref_length), patternPositionSearch);
            //std::cerr << "begin:" << *begin << " end: " << *end << "\n";
            ulong word_length = pattern.size(); // should equal substring_size
            //std::cerr << "przed drugim forem w count_full_words begin:" << std::distance(substrings.begin(), begin) << "<<end:" << std::distance(substrings.begin(), end) << "\n";
            for (auto i = begin; i < end; i++) {
                //std::cerr << "drugi for"<<*i<<" word_length:" <<word_length<<"\n";
                //std::string extracted_word;
                //ulong extracted_word_length;
                if (*i - j + word_length < m_text_length) {

                    //extract(*i - j, *i - j + word_length, &extracted_word, &extracted_word_length);
                    //std::cerr << "j:" << j << " pattern:" << pattern << "\n";
                    //if (extracted_word_length >= 0 && extracted_word == pattern) {
                    if (*i >= j && check_word(*i - j, (size_type) word_length, pattern.c_str())) {
                        if (locate) {
                            occ->push_back(*i);
                        }
                        (*numocc)++;
                        //std::cerr<<"LOOOOOOOOOOOOOOOOOOOOOOHOOOOOOOOOOOOOOOOOOOOOOO"<<*numocc<<"j:"<<j<<" pattern:"<<pattern<<"\n";
                    }
                }
            }
        }

        if (locate) {
            std::sort(occ->begin(), occ->end());
            occ->erase(std::unique(occ->begin(), occ->end()), occ->end());
            (*numocc) = occ->size();
        }
        //std::cerr<<"koniec count_full_words\n";
        return 0;
    }

    void write_vector(std::ostream& out, const sdsl::int_vector<INT_ROZM>& vec) {
        // out << "\n" << vec.size() << "\n";
        //std::cerr << "wypisuje" << vec.size() << "\n";
        uint r = vec.size();
        out.write((char *) &r, sizeof (uint));
        //std::cerr << "WRITE\n";
        for (std::vector<uint>::size_type i = 0; i < vec.size(); i++) {
            //std::cerr << "wypisujee" << vec.size() << "\n";
            uint tmp = vec[i];
            //std::cerr<<vec[i]<<"\n";
            out.write((char *) &tmp, sizeof (uint));

        }
        //std::cerr << "KONIEC WRITE\n";
        // out << "\n";
    }

    void load_vector(std::istream& in, sdsl::int_vector<INT_ROZM>* vec) {
        uint n, a;
        // in>>n;
        // std::cerr<<"LOAD\n";
        in.read((char *) &n, sizeof (uint));
        std::cerr << "w load\n" << n << "\n";
        vec->resize(n);
        for (uint i = 0; i < n; i++) {
            //////std::cerr<<"petla load"<<a<<"\n";
            in.read((char *) &a, sizeof (uint));
            //  std::cerr<<a<<"\n";
            (*vec)[i] = (int)a;
        }
        std::cerr << "KONIEC LOAD\n";
    }

    int IndexLookupTable::save_index(std::ostream& out) const {
        //std::cerr << "savuje index" << substrings.size() << " " << lookup_table.size() << "\n";
        out.write((char *) &IndexLookupTable::INDEX_TYPE, sizeof (uint));
        //Index::save_index(out);
        out.write((char *) &m_word_size, sizeof (size_type));
        out.write((char *) &m_shift, sizeof (size_type));
        out.write((char *) &m_text_length, sizeof (size_type));
        out.write((char *) &m_additional_text_length, sizeof (size_type));

        m_alphabet->save(out);

        //std::cerr << "aa" << m_text << "\n";
        m_text->serialize(out);
        //std::cerr << "bb\n";
        write_vector(out, substrings);
        //std::cerr << "srodek wypisywania\n";
        write_vector(out, lookup_table);
        //out << substring_length << " " << pref_length << "\n";
        out.write((char *) &substring_length, sizeof (uint));
        out.write((char *) &pref_length, sizeof (uint));
        //std::cerr << "przed wypisaniem: sub_len:" << substring_length << " pref_len:" << pref_length << "\n";
        return 0;
    }

    void IndexLookupTable::load(std::istream& in) {
        //std::cerr << "LOAD###################################################################################\n";
        uint index_type;
        in.read((char *) &index_type, sizeof (uint));

        if (index_type != INDEX_TYPE) {
            //std::cerr << "Wrong index type!\n";
        }

        //Index::load(in);
        //m_permutation = Permutation::load(in);
        in.read((char *) &m_word_size, sizeof (size_type));
        in.read((char *) &m_shift, sizeof (size_type));
        in.read((char *) &m_text_length, sizeof (size_type));
        in.read((char *) &m_additional_text_length, sizeof (size_type));

        m_alphabet = Alphabet::load(in);

        m_text = new sdsl::int_vector<>(m_text_length + m_additional_text_length, 0,
                (uint8_t) cds_utils::bits((uint) m_alphabet->size()));
        m_text->load(in);


        //std::cerr << "przed load substring\n";
        load_vector(in, &substrings);
        //std::cerr << "przed load lookup table\n";
        load_vector(in, &lookup_table);
        // in >> substring_length>>pref_length;
        in.read((char *) &substring_length, sizeof (uint));
        in.read((char *) &pref_length, sizeof (uint));
        //std::cerr << "koniec load pref length:" << pref_length << " substring_length:" << substring_length << "\n";
    }
}
