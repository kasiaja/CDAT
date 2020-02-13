#include "Alphabet.hpp"

#include <vector>
#include <algorithm>

namespace cdat {

Alphabet::Alphabet() : m_size(0) {
}

Alphabet::Alphabet(size_type const _size) : m_size(_size) {
}

Alphabet::~Alphabet() {
    delete[] m_reverse_alphabet;
}

void Alphabet::count_ifpower2() {
    auto alphabet_size = m_size;
    if (alphabet_size && !(alphabet_size & (alphabet_size - 1))) {
        m_if_power2 = true;
        m_power2 = 0;
        for (; alphabet_size > 1; ++m_power2, alphabet_size /= 2);
    }
    else
        m_if_power2 = false;
}

int Alphabet::build(char const *filename) {
    std::ifstream file(filename);
    std::string line;
    std::vector<uchar> characters;

    if (file.is_open()) {
        while (getline(file, line)) {
            for (unsigned int i = 0; i < line.length(); ++i) {
                characters.push_back(static_cast<uchar>(line[i]));
            }
        }
    }
    else {
        std::cerr << "Error: could not open file to read m_alphabet\n";
        return -1;
    }
    std::sort(characters.begin(), characters.end());

    this->m_size = characters.size();
    std::fill(m_alphabet, m_alphabet + ASCII, m_size + 1);
    this->m_reverse_alphabet = new uchar[this->m_size];

    for (unsigned int i = 0; i < this->m_size; ++i) {
        m_alphabet[characters[i]] = i;
        m_reverse_alphabet[i] = characters[i];
    }

    file.close();
    count_ifpower2();

    return 0;
}

int Alphabet::build_from_text(char const *filename) {
    bool characters[ASCII];
    std::fill(characters, characters + ASCII, false);
    this->m_size = 0;

    std::ifstream file(filename);
    if (file.is_open()) {
        const size_t BUFFER_SIZE = 16 * 1024;
        char buffer[BUFFER_SIZE];
        size_t bytes_read;
        do {
            file.read(buffer, BUFFER_SIZE);
            bytes_read = (size_t) file.gcount();

            for (size_t i = 0; i < bytes_read; ++i) {
                uchar uc = static_cast<uchar>(buffer[i]);
                if (!characters[uc]) {
                    characters[uc] = true;
                    ++this->m_size;
                }
            }
        }
        while (bytes_read == BUFFER_SIZE);
    }
    else {
        std::cerr << "Error: could not open file\"" << filename << "\" to read m_alphabet\n";
        return -1;
    }

    std::fill(m_alphabet, m_alphabet + ASCII, m_size + 1);

    this->m_reverse_alphabet = new uchar[this->m_size];
    uint counter = 0;
    for (unsigned int i = 0; i < ASCII; ++i) {
        if (characters[i]) {
            m_alphabet[i] = counter;
            m_reverse_alphabet[counter++] = (uchar) i;
        }
    }

    file.close();
    count_ifpower2();

    return 0;
}

Alphabet::value_type Alphabet::get_word_value(std::string const &word) const {
    value_type result = 0;
    //std::cerr<<"poczatek get_word_value\n";
    for (uint i = 0; i < word.length(); ++i) {
        //std::cerr<<result<<"<m_size"<<this->m_size<<" get_char_value(word[i])"<<get_char_value(word[i])<<" word[i]:"<<word[i]<<"\n";
        result *= this->m_size;
        result += get_char_value(word[i]);
    }
	//std::cerr<<"koniec get_word_value\n";

    return result;
}

Alphabet::value_type Alphabet::get_word_value(std::string const &word, size_type const start,
                                size_type const end) const {
    value_type result = 0;

    for (size_type i = start; i < end; ++i) {
		//std::cout<<result<<"\n";
        result *= this->m_size;
        result += get_char_value(word[i]);
    }
	//std::cout<<"koniec get_word_value\n";
    return result;
}

std::string Alphabet::get_word_from_value(size_type const value, size_type const word_size) const {
    std::string result = "";
    size_t word_value = value;
    size_t multiplier = (size_t) pow(this->m_size, word_size - 1);

    for (; multiplier > 0; multiplier /= this->m_size) {
        result += m_reverse_alphabet[word_value / multiplier];
        word_value -= (word_value / multiplier) * multiplier;
    }

    return result;
}

bool Alphabet::validate_word(std::string const &word) const {
    for (uint i = 0; word.size(); ++i) {
        if (get_char_value(word[i]) > m_size) {
            return false;
        }
    }

    return true;
}

Alphabet *Alphabet::load(std::istream &file) {
    uint alphabet_header;
    file.read((char *) &alphabet_header, sizeof(uint));
    if (alphabet_header != ALPHABET) {
        std::cerr << "Wrong file header\n";
        return NULL;
    }

    Alphabet *alphabet = new Alphabet();

    file.read((char *) &alphabet->m_size, sizeof(size_t));
    file.read((char *) &alphabet->m_if_power2, sizeof(bool));
    file.read((char *) &alphabet->m_power2, sizeof(uint));

    alphabet->m_reverse_alphabet = new uchar[alphabet->m_size];

    for (uint i = 0; i < alphabet->m_size; ++i) {
        uint value;
        uchar character;
        file.read((char *) &character, sizeof(char));
        file.read((char *) &value, sizeof(uint));

        alphabet->m_alphabet[(uint) character] = value;
        alphabet->m_reverse_alphabet[value] = character;
    }

    return alphabet;
}

void Alphabet::save(std::ostream &file) const {
    uint alphabet_header = ALPHABET;
    file.write((char *) &alphabet_header, sizeof(uint));

    file.write((char *) &this->m_size, sizeof(size_type));
    file.write((char *) &m_if_power2, sizeof(bool));
    file.write((char *) &m_power2, sizeof(uint));

    for (size_t i = 0; i < this->m_size; ++i) {
        file.write((char *) &m_reverse_alphabet[i], sizeof(uchar));
        file.write((char *) &m_alphabet[(uint) m_reverse_alphabet[i]], sizeof(uint));
    }
}

}
