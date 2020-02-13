#ifndef _INDEX_H
#define _INDEX_H

#include "Alphabet.hpp"
#include "config/Config.h"
#include "Counter.hpp"
#include "Permutation.hpp"

#include <string.h>
#include <fstream>

#include <sdsl/bit_vectors.hpp>

namespace cdat {

    class Index {
    public:
        typedef uint64_t size_type;
        typedef uint64_t value_type;

        Index() : m_word_size(0), m_shift(0), m_text_length(0), m_additional_text_length(0) {
        };

        Index(size_type word_size, size_type transition) : m_word_size(word_size),
        m_shift(transition), m_additional_text_length(0) {
            std::cerr<<"M_WORD_SIZE"<<m_word_size<<" shift: "<<m_shift<<"\n";
        }
        Index(size_type word_size, size_type transition, size_type text_length,
                size_type additional_text_length, sdsl::bit_vector *bit_vector,
                sdsl::bit_vector::rank_1_type *m_rank1,
                sdsl::bit_vector::select_1_type *m_select1,
                Permutation *permutation, Alphabet *alphabet);

        virtual ~Index();

        template<typename Counter>
        int build(char const *filename);
        int count_index(std::string const &pattern, ulong const from, ulong *numocc) const;
        int count(std::string const &pattern, ulong const length, ulong *numocc) const;
        int locate_index(std::string const &pattern, ulong const from, std::vector<ulong> *occ, ulong *numocc) const;
        int locate(std::string const &pattern, ulong const length, std::vector<ulong> *occ, ulong *numocc) const;
        virtual int extract(ulong const from, ulong const to, std::string *text, ulong *length) const = 0;

        static Index* load_index(std::istream& in);
        virtual void load(std::istream& in);
        virtual int save_index(std::ostream &out) const;
        virtual double get_size_in_mega_bytes() const;
        virtual void init(){};

    protected:

        /*********   FUNCTIONS  ********/

        size_type rank_0(size_type const idx) const;
        virtual value_type extract_value(size_type const from, size_type const length) const = 0;

        int count_short(std::string const &pattern, ulong length,
                bool const locate, ulong *numocc, std::vector<ulong> *occ) const;
        int count_short_right(std::string const &pattern, bool const locate, ulong *numocc,
                std::vector<ulong> *occ) const;
        int count_short_left(std::string const &pattern, bool const locate, ulong *numocc,
                std::vector<ulong> *occ) const;

        virtual int count_full_words(std::string const &pattern, ulong length,
                bool const locate, ulong *numocc,
                std::vector<ulong> *occ) const = 0;

        int count_exact_size_word(std::string const &pattern, size_type length,
                ulong *numocc, bool const locate, std::vector<ulong> *occ) const;

        void verify_occurrences(size_type const start, size_type const end, size_type const length,
                size_type const offset, ulong *numocc, std::vector<ulong> *occ) const;

        size_type get_position_in_permutation(value_type const word_value) const;

        void add_last_occ(std::string const &pattern, bool const locate,
                ulong *numocc, std::vector<ulong> *occ) const;
        void check_occ_end(std::string const &pattern, ulong *numocc) const;

        template<typename Counter>
        void create_bit_vector(Counter const &counter,
                size_type const words_number,
                size_type const genome_words_number);
        virtual void create_bit_vector_support();

        template<typename Counter>
        void create_permutation(std::ifstream &file, Counter &counter);
        void create_permutation(std::ifstream &file, size_type const genome_words_number);
        virtual Permutation *create_permutation(size_type const size) const;
        value_type perm_binary_search(size_type const word_value, size_type const genome_words_number) const;

        template<uint8_t t_width>
        value_type inc_counter(Counter<t_width> &counter, value_type const word_value);

        virtual void create_text(char const *const filename) {
        };

        virtual void fill_text(size_type const idx, value_type const value) {
        };

#ifdef DEBUG
        void print_time(timeval &start, char const *const msg);
#endif

        /**********  FIELDS  ***********/
        size_type m_word_size;
        size_type m_shift;
        size_type m_text_length;
        size_type m_additional_text_length;

        sdsl::bit_vector * m_bit_vector;
        sdsl::bit_vector::rank_1_type *m_rank1;
        sdsl::bit_vector::select_1_type *m_select1;
        Permutation *m_permutation;
        Alphabet *m_alphabet;

    };

    template<typename Counter>
    void Index::create_bit_vector(Counter const &counter, size_type const words_number,
            size_type const genome_words_number) {
#ifdef DEBUG
        timeval start_now;
        gettimeofday(&start_now, NULL);
#endif

        m_bit_vector = new sdsl::bit_vector(genome_words_number + words_number + 1, 0);
        size_type position = 0;
        (*m_bit_vector)[0] = 1;

        for (size_type i = 0; i < words_number - 1; ++i) {
            position += counter.get(i) + 1;
            (*m_bit_vector)[position] = 1;
        }
        (*m_bit_vector)[genome_words_number + words_number] = 1;
        create_bit_vector_support();

#ifdef DEBUG
        print_time(start_now, "bit vector time: ");
#endif
    }

    template<typename Counter>
    int Index::build(char const *filename) {
        std::cerr << "index.build\n";
#ifdef DEBUG
        timeval start_now, start;
        gettimeofday(&start, NULL);
        gettimeofday(&start_now, NULL);
#endif

        m_alphabet = new Alphabet();
        if (m_alphabet->build_from_text(filename) != 0) {
            return -1;
        }
#ifdef DEBUG
        print_time(start_now, "alphabet time: ");
#endif

        size_type words_number = m_alphabet->pow_wsize(m_word_size);
        size_type genome_words_number = 0;

        std::ifstream file(filename);

        if (file.is_open()) {
            Counter counter(words_number);

            genome_words_number = counter.build_counter(file, m_word_size,
                    m_shift,
                    m_alphabet,
                    m_text_length,
                    m_additional_text_length);
            std::cerr << "po build_counter\n";
#ifdef DEBUG
            print_time(start_now, "word counter time: ");
#endif

            file.clear();
            file.seekg(0, std::ios::beg);
            std::cerr << "przed create bit vector\n";
            create_bit_vector(counter, words_number, genome_words_number);
            std::cerr << "przed create_text\n";
            create_text(filename);
            std::cerr << "pp create_text\n";

            m_permutation = create_permutation(genome_words_number);
            std::cerr << "pp create_perm1\n";
            counter.prepare_for_permutation();
            std::cerr << "pp prepare_perm\n";
            create_permutation(file, counter);
            std::cerr << "pp create_perm2\n";
            init();
            //std::cout<<"RANK:"<<m_rank1->size()<<"\n";
            //std::cout<<"SELECT:"<<m_select1-><<"\n";
#ifdef DEBUG
            print_time(start_now, "perm and text time: ");
            print_time(start, "index build time: ");
#endif
        } else {
            std::cerr << "Error: unable to open file\n";
            return -1;
        }

        return 0;
    }

    template<uint8_t t_width>
    inline Index::value_type Index::inc_counter(Counter<t_width> &counter, value_type const word_value) {
        auto sel = m_select1->select(word_value + 1);
        auto curr_position = rank_0(sel);

        return counter.get_and_inc(word_value) + curr_position;
    }

    template<>
    inline Index::value_type Index::inc_counter(Counter<32> &counter, value_type const word_value) {
        return counter.get_and_inc(word_value);
    }

    /*
     * tworzy wektor bitów na permutacji.
     * Dla każdego słowa sprawdzam, ile wystąpiło w tekście mniejszych słów, 
     * do tej wartości dodaje ile razy wystąpiło dane słowo w obecnym przejściu
     * pętli i wówczas wiem na jakiej pozycji w permutacji je wstawić.
     */
    template<typename Counter>
    void Index::create_permutation(std::ifstream &file, Counter &counter) {

        std::cerr << "create_perm\n";
        file.clear();
        file.seekg(0, std::ios::beg);

        value_type divisor = m_alphabet->pow_wsize(m_word_size - m_shift);
        bool additional_word = false;
        size_type position = 0;
        size_type word_length = 0;
        value_type word_value = 0;

        size_type char_counter = 0;
        const size_t BUFFER_SIZE = 16 * 1024;
        char buffer[BUFFER_SIZE];
        size_t bytes_read;
        do {
            file.read(buffer, BUFFER_SIZE);
            bytes_read = (size_t) file.gcount();
            //std::cerr<<"crea_perm petla"<<bytes_read<<" "<<BUFFER_SIZE<<"\n";

            for (size_t i = 0; i < bytes_read; ++i) {
                //std::cerr<<"w petli"<<m_word_size<<"\n";
                value_type char_value = m_alphabet->get_char_value(buffer[i]);
                word_value *= m_alphabet->size();
                word_value += char_value;
                fill_text(char_counter++, char_value);
                ++word_length;

                if (!additional_word)
                    additional_word = true;
                if (word_length == m_word_size) {
                    //std::cerr<<"w ifie w petli\n";
                    value_type perm_value = inc_counter(counter, word_value);
                    m_permutation->set_field(perm_value, position++);
                    word_value %= divisor;
                    word_length -= m_shift;
                    additional_word = false;
                }
            }
        } while (bytes_read == BUFFER_SIZE);

        if (additional_word) {
            while (word_length != m_word_size) {
                fill_text(char_counter++, 0);
                word_value *= m_alphabet->size();
                word_length++;
            }

            value_type perm_value = inc_counter(counter, word_value);
            m_permutation->set_field(perm_value, position);
        }
    }

}
#endif