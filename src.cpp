#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <tuple>
#include <algorithm>

using namespace std;


double double_round(double input) {
    return round(input * 1000) / 1000;
};

struct EncodedSymbol {
    // Хранит все данные об абстракции символа - сам символ, его вероятность появления в тексте, а так же 
    // код, которым мы его кодируем (он ищначально равен "", и эта стркоа дописывается в процессе извлечения 
    // данных о кодируемом сообщении)

    char symbol;
    double probability;
    string code;

    EncodedSymbol(char sym, double prob) {
        symbol = sym;
        probability = prob;
        code = "";
    }
};

struct Node {
    // Абстракция "узла" символов - алгоритм Хаффмана объединяет символы в "группы" и присваивает им кодировку (0 или 1), пока 
    // групп не станет 1. У группы есть параметр "общей вероятности" появления в тексте - сначала присваиваются и сливаются с 
    // другими самые невероятные - чтобы они участвовали в наибольшем кол-ве итераций получения символа в значении (так самые 
    // вероятные символы получат наименьшую длину кода)

    vector<EncodedSymbol> symbols;
    double total_prob;

    void add_symbol(EncodedSymbol symbolObj) {
        this->symbols.push_back(symbolObj);
        this->total_prob += symbolObj.probability;
    }
};

bool CompareNodes(const Node& a, const Node& b) {
    // Компаратор для сортировки вектора узлов
    return a.total_prob < b.total_prob;
}

bool SymbolInVec(vector <EncodedSymbol>vec, char symb) {
    bool isTrue = false;
    for (auto& a : vec) {
        if (a.symbol == symb) {
            isTrue = true;
            break;
        }
    }
    return isTrue;
}

class HuffmanEncoder {
    // Ключевой класс кодировки - объединяет всю соответствующую логику
public:
    string input_text;
    map<char, double> input_dict;

    // encoding_dict - "зеркало" decoding_dict (все ключи encoding_dict - все значния decoding_dict и наоборот). Можно обойтись и 
    // без этого, но в целом декодировать легче не по значениям encoding_dict, а по ключам decoding_dict
    map<char, string> encoding_dict;
    map<string, char> decoding_dict;

    int total_bytes;

    HuffmanEncoder(string input) {
        input_text = input;
        input_dict = count_dict(input_text);

        auto dicts = encode_dict();
        encoding_dict = dicts.first;
        decoding_dict = dicts.second;

        total_bytes = count_total_bytes();
    }

    pair<map<char, string>, map<string, char>> encode_dict() {
        // Ключевой алгоритм кодировки. Из полученного сообщения находит вероятности для каждого символа, распределяет каждый 
        // символ по группам (каждому символу - одна группа). Далее объединяет каждые две самые невероятные группы, добавляя 
        // каждому символу из одной из изначальных групп "0" в коде, а из другой - "1" и перенося все символы в новую 
        // "объединенную" группу. Так продолжается, пока групп не останется 1. Потом возвращает encoding_dit и decoding_dict 
        // (зеркальные - см. инициализацию HuffmanEncoder)
    
        vector<EncodedSymbol> symbols_list;
        vector<Node> nodes_list;
        
        for (auto it = this->input_dict.begin(); it != this->input_dict.end(); it++) {
            // Не добавляем дублирующиеся значения
            if (!SymbolInVec(symbols_list, it->first)) {
                // Добавляем абстракцию символа
                struct EncodedSymbol new_symb = {it->first, it->second};
                symbols_list.push_back(new_symb);
            }
        }

        for (auto& symb : symbols_list) {
            // Группируем каждый символ по группам (одн символ на группу)
            struct Node new_node = {};
            new_node.add_symbol(symb);
            nodes_list.push_back(new_node);
        }

        while (nodes_list.size() > 1) {
            // Сортируем вектор узлов (самые невероятные будут в начале)
            sort(nodes_list.begin(), nodes_list.end(), CompareNodes);

            // Получаем две самые невероятные группы и добавляем каждому символу в них новое число в их текущий код
            Node last_node = *nodes_list.begin();
            nodes_list.erase(nodes_list.begin());

            for (auto& symb : last_node.symbols) {
                symb.code = "0" + symb.code;
            }

            // Безопасно, потому что итерация всяко запустится только если есть по крайней мере два узла
            Node second_last_node = *nodes_list.begin();
            nodes_list.erase(nodes_list.begin());

            for (auto& symb : second_last_node.symbols) {
                symb.code = "1" + symb.code;
            }

            // Добавляем все бездомные символы из вычеркнутых узлов в новый узел
            struct Node new_node = {};

            for (auto& symb : last_node.symbols) {
                new_node.add_symbol(symb);
            }
            for (auto& symb : second_last_node.symbols) {
                new_node.add_symbol(symb);
            }

            nodes_list.push_back(new_node);
        }

        // Получаем последний оставшийся узел, содержащий все символы из сообщения
        Node last_node = *nodes_list.begin();
        map <char, string> output_encoding_dict;
        map <string, char> output_decoding_dict;

        for (auto& s : last_node.symbols) {
            output_encoding_dict[s.symbol] = s.code;
            output_decoding_dict[s.code] = s.symbol;
        }

        return make_pair(output_encoding_dict, output_decoding_dict);
    }

    int count_total_bytes() {
        int output = 0;
        for (auto const& it : encoding_dict) {
            output += it.second.size();
        }

        return output;
    }

    map<char, double> count_dict(string input_str) {
        // Читает каждый символ и определяет его частоту появления в тексте. Потом считает соответственно его вероятность 
        // появления в тексте и возвращает словарь [символ]: вероятность 
        map<char, double> out_dict;

        for (char& c : input_str) {
            // Неоптимизированно, потому что считает каждый символ по столько раз, сколько он появляется в тексте (когда 
            // программировал, подумал, что остроумно будет не проверять на то, видели ли мы этот символ ранее - ведь мы просто 
            // перезапишем в словаре тот же ключ той же вероятностью!) Но конечно это глупость, надо будет добавить проверку :)
            int counter = 0;
            for (char& cc : input_str) {
                if (c==cc) counter += 1;
            }
            double probability = double(double(counter) / double(input_str.length()));
            out_dict[c] = double_round(probability);
        }

        return out_dict;
    }

    string encode_message() {
        // Для компрессора данных нет смысла иметь возможность кодировать произвольные сообщения используя 
        // текущий словарь кодировки, потому что этот словарь относится к одному конкретному сообщению и 
        // собирается исключительно для того, чтобы оптимально закодировать именно это сообщение. Для 
        // кодировки любого другого сообщения разумно будет создать новый экземпляр HuffmanEncoder.

        string output_string = "";

        for (auto& ch : input_text) {
            output_string += encoding_dict[ch];
        }

        return output_string;
    }

    string decode_message(string encoded_message) {
        // Используется для декодирования сообщения принимающим файл. В условиях такого проекта, как этот, 
        // используется, чтобы повыпендриваться тем, что алгоритм Хаффмана реализован без ошибок (потому 
        // что алгоритм Хаффмана - префиксный, что значит, что закодированное сообщение всегда можно 
        // однозначно декодировать)

        string output_string = "";
        string current_code_char = "";

        for (auto& ch : encoded_message) {
            current_code_char += ch;
            
            if (decoding_dict.find(current_code_char) != decoding_dict.end()) {
                output_string += decoding_dict[current_code_char];
                current_code_char = "";
            }
        }

        return output_string;
    }
};

