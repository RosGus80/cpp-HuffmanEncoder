#include <iostream>
#include <string>
#include "src.cpp"

int main() {
    string input_txt;
    cout << "Введите текст, который хотите закодировать (не вводите текст с разделением строк - консоль думает, что вы закончили писать): ";
    getline(cin, input_txt);

    HuffmanEncoder huff(input_txt);
    string encoded_msg = huff.encode_message();

    cout << endl << "Закодированный текст: " << encoded_msg << endl;

    string decoded_msg = huff.decode_message(encoded_msg);

    if (decoded_msg == input_txt) {
        cout << "Декодированный текст (выводит то же, что вы и ввели. Значит, все работает хорошо!): " << decoded_msg;
    } else {
        cout << "Судя по всему, кодировщик не смог однозначно декодировать обратно в текст :( Значит, что-то в нем не работает. Можете открыть issue на гитхабе проекта и скинуть, какой текст вы ввели (а так же что вам вывелось): " << decoded_msg;
    }
    
    return 0;
}