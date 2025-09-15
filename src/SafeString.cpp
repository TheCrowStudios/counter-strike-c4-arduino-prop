#include "SafeString.h"

SafeString::SafeString(int size) {
    capacity = size;
    data = new char[capacity];
    data[0] = '\0';
    len = 0;
}

SafeString::~SafeString() {
    delete[] data;
}

void SafeString::appendChar(char c) {
    if (len < capacity - 1) {
        data[len] = c;
        data[len + 1] = '\0';
        len++;
    }
}

void SafeString::deleteLast() {
    if (len > 0) {
        len--;
        data[len] = '\0';
    }
}

void SafeString::clear() {
    data[0] = '\0';
    len = 0;
}

int SafeString::length() {
    return len;
}

const char* SafeString::getData() {
    return data;
}