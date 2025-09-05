
#pragma once

#include <iostream>
#include <vector>

using std::string;
using std::vector;
using std::cout;
using std::endl;

template<typename T>
void print_debug(const string& message, T& val) {
    cout << message << " " << val << endl;
}

template<typename T>
void print_vector(const string& message, const vector<T>& v) {
    cout << message << " ";
    for (int i = 0; i < v.size(); i++) {
        cout << v.at(i) << " ";
    }
    cout << endl;
}