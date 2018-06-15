//
//  parser.hpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/20.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#ifndef lexer_parser_hpp
#define lexer_parser_hpp
#include <stdlib.h> // for size_t
#include <list>
#include <memory>
#include <set>
#include <string>

using std::list;
using std::set;
using std::string;
using std::shared_ptr;

enum TokenType{
    NUMBER,
    OPERATOR,
    ASSIGNER,
    TOKEN_DOT,
    TOKEN_EOL,
    TOKEN_EOF,
    IDENTIFIER,
    LPARENTHESES,
    RPARENTHESES,
    COLON,
    LPAREN,
    RPAREN,
    COMMA,
    BLOCK_BEGIN,
    BLOCK_END,
    TOKEN_COMPARE,
    EQ,
    RESERVED_WORD,
    TOKEN_STRING,
    DOUBLE_DOT,
    UNKNOWN_TOKEN
};

class NoncopyAble {
    NoncopyAble(const NoncopyAble&);
    NoncopyAble& operator=(const NoncopyAble&);
public:
    NoncopyAble() {}
};

class ReservedWords: private NoncopyAble{
private:
    std::set<std::string> m_words;
    ReservedWords() {
        m_words.insert("class");
        m_words.insert("import");
        m_words.insert("if");
        m_words.insert("elseif");
        m_words.insert("else");
        m_words.insert("while");
        m_words.insert("for");
        m_words.insert("in");
        m_words.insert("break");
        m_words.insert("continue");
        m_words.insert("func");
        m_words.insert("return");
        m_words.insert("True");
        m_words.insert("False");
        m_words.insert("Null");
    }
public:
    bool is_reserved(const char* word) const {
        std::set<std::string>::const_iterator it = m_words.find(word);
        return it != m_words.end();
    }
    static ReservedWords* instance() {
        static ReservedWords reservedWords;
        return &reservedWords;
    }
};

class InputStream: private NoncopyAble {
public:
    virtual int readChar()= 0;
    virtual void close() = 0;
    virtual ~InputStream(){}
};


class Lexer;

class FeedBackInputStream: public InputStream {
private:
    InputStream *m_targetInputStream;
    Lexer *m_lexer;
    char m_feebackCharacters[16];
    int m_pos;
    bool try_pop(char *c);
    void read_char_for_num(char c);
public:
    FeedBackInputStream(InputStream *inputStream, Lexer *lexer):
    m_targetInputStream(inputStream),m_lexer(lexer), m_pos(-1){}
    void push_back(char c);
    int peekChar(int n = 1);
    virtual int readChar();
    virtual void close(){
        m_targetInputStream->close();
    }
};

class Token: private NoncopyAble {
private:
    TokenType m_tokenType;
    char* m_data;
    size_t m_data_length;
    Token(const Token&);
    Token& operator=(const Token&);
public:
    size_t m_line;
    size_t m_column;
    Token(TokenType type, const char* token);
    Token();
    void set(TokenType type, const char* token);
    void setType(TokenType type) {
        m_tokenType = type;
    }
    bool is_operator() const;
    ~Token();
    const char* getString() const {
        if (m_tokenType == TOKEN_EOL) {
            return "<EOL>";
        }
        if (m_tokenType == TOKEN_EOF) {
            return "<EOF>";
        }
        return m_data;
    }
    TokenType getType() const {
        return m_tokenType;
    }
};

class Lexer: private NoncopyAble {
public:
    Lexer(InputStream *inputStream);
    ~Lexer();
    bool readNext(std::shared_ptr<Token> outToken);
    bool peek(std::shared_ptr<Token> outToken, int n = 1);
private:
    bool readImpl(std::shared_ptr<Token> outToken);
    bool nextTokenImpl(std::shared_ptr<Token> outToken);
    FeedBackInputStream *m_inputStream;
    std::list<std::shared_ptr<Token>> m_token_queue;
    bool m_eof_returned;
public:
    size_t m_line;
    size_t m_column;
};

class StringInputStream: public InputStream {
private:
    char *m_string_data;
    size_t m_pos;
public:
    StringInputStream(const char* s);
    virtual int readChar()
    {
        char c = m_string_data[m_pos];
        if (c == '\0') {
            return -1;
        }
        m_pos++;
        return c;
    }
    virtual void close(){}
    ~StringInputStream() {
        if (m_string_data) {
            delete[] m_string_data;
            m_string_data = NULL;
        }
    }
};

class FileInputStream : public InputStream {
private:
    FILE* m_file;
    bool m_eof;
public:
    FileInputStream(const char* filename): m_eof(false)
    {
        m_file = fopen(filename, "r");
    }
    FileInputStream(FILE* file): m_file(file), m_eof(false)
    {
    }
    virtual int readChar()
    {
        if (m_eof) {
            return -1;
        }
        if (!m_file) {
            throw std::string("file not exist.");
        }
        int c = fgetc(m_file);
        if (c < 0) {
            m_eof = c;
        }
        return c;
    }
    virtual void close(){
        if (m_file) {
            fclose(m_file);
            m_file = NULL;
        }
    }
    ~FileInputStream() {
        close();
    }
};

#endif /* parser_hpp */

