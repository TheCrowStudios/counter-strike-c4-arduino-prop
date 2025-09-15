#ifndef SAFESTRING_H
#define SAFESTRING_H

class SafeString {
    private:
        char* data;
        int capacity;
        int len;

    public:
    SafeString(int size);
    ~SafeString();

    void appendChar(char c);
    void deleteLast();
    void clear();
    int length();
    const char* getData();
};

#endif