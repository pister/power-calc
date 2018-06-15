//
//  parser.cpp
//  power_calc
//
//  Created by Songli Huang on 2017/12/20.
//  Copyright © 2017年 Songli Huang. All rights reserved.
//

#include "lexer_parser.hpp"
#include <string.h>

#define MAX_TOKEN_SIZE 128

Token::Token(TokenType type, const char* token):
m_tokenType(type)
{
    m_data_length = strlen(token);
    m_data = new char[m_data_length+1];
    m_data[m_data_length] = '\0';
    strncpy(m_data, token, m_data_length);
}

void Token::set(TokenType type, const char* token)
{
    // clear old
    if (m_data) {
        delete[] m_data;
        m_data = NULL;
        m_data_length = 0;
    }
    // set new
    m_tokenType = type;
    m_data_length = token == NULL ? 0: strlen(token);
    m_data = new char[m_data_length+1];
    m_data[m_data_length] = '\0';
    strncpy(m_data, token, m_data_length);
}

bool Token::is_operator() const
{
    switch (m_tokenType) {
        case OPERATOR:
        case TOKEN_COMPARE:
        case EQ:
            return true;
        default:
            return false;
    }
}

Token::Token():
m_data_length(0),
m_data(NULL),
m_line(0),
m_column(0)
{
}
Token::~Token()
{
    if (m_data) {
        delete[] m_data;
        m_data = NULL;
        m_data_length = 0;
    }
}


////========================================================
enum Token_Status {
    ST_Init,
    ST_Number,
    ST_DOT,
    ST_Operator,
    ST_Assigner,
    ST_COMPARE,
    ST_EXCL_MARK,
    ST_EOL,
    ST_LINE_COMMENT,
    ST_STRING,
    ST_Identifier
};

///==============
static TokenType findTokenType(Token_Status state) {
    switch (state) {
        case ST_Init:
            return UNKNOWN_TOKEN;
        case ST_Number:
            return NUMBER;
        case ST_Operator:
            return OPERATOR;
        case ST_Assigner:
            return ASSIGNER;
        case ST_Identifier:
            return IDENTIFIER;
        case ST_EOL:
            return TOKEN_EOL;
        case ST_COMPARE:
            return TOKEN_COMPARE;
        default:
            return UNKNOWN_TOKEN;
    }
}

Lexer::Lexer(InputStream *inputStream):
m_inputStream(new FeedBackInputStream(inputStream, this)), m_eof_returned(false),
m_line(1), m_column(1)
{}

///
class TokenBuf {
private:
    char m_token_buf[MAX_TOKEN_SIZE];
    size_t m_token_pos;
public:
    TokenBuf():m_token_pos(0){
        memset(m_token_buf, 0, sizeof(m_token_buf));
    }
    void add_char(char c) {
        m_token_buf[m_token_pos] = c;
        m_token_pos++;
    }
    char peek_last(size_t n = 1) const {
        if (m_token_pos <= n) {
            return 0;
        }
        return m_token_buf[m_token_pos - n];
    }
    const char* getTokenAsString() const {
        return m_token_buf;
    }
    int find_char(char c) const {
        int pos = 0;
        while (pos < m_token_pos) {
            char pos_c = m_token_buf[pos];
            if (pos_c == c) {
                return pos;
            }
            pos++;
        }
        return -1;
    }
};

///
static bool handle_single_char(TokenType targetTokenType, FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    if (st == ST_Init) {
        tokenBuff->add_char(c);
        outToken->set(targetTokenType, tokenBuff->getTokenAsString());
        return true;
    } else {
        inputStream->push_back(c);
        TokenType type = findTokenType(st);
        outToken->set(type, tokenBuff->getTokenAsString());
        return false;
    }
}

static void handle_operator(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(OPERATOR, inputStream, st, c, tokenBuff, outToken);
}

static void handle_block_begin(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(BLOCK_BEGIN, inputStream, st, c, tokenBuff, outToken);
}

static void handle_block_end(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(BLOCK_END, inputStream, st, c, tokenBuff, outToken);
}

static void handle_lparen(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(LPAREN, inputStream, st, c, tokenBuff, outToken);
}

static void handle_rparen(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(RPAREN, inputStream, st, c, tokenBuff, outToken);
}

static void handle_comma(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(COMMA, inputStream, st, c, tokenBuff, outToken);
}

static void handle_lparetheres(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(LPARENTHESES, inputStream, st, c, tokenBuff, outToken);
}

static void handle_rparetheres(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(RPARENTHESES, inputStream, st, c, tokenBuff, outToken);
}

static void handle_colon(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(COLON, inputStream, st, c, tokenBuff, outToken);
}

static bool handle_assigner(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    switch (*st) {
        case ST_Init:
            tokenBuff->add_char(c);
            *st = ST_Assigner;
            return false;
        case ST_COMPARE:
        case ST_Assigner:
        case ST_EXCL_MARK:
            tokenBuff->add_char(c);
            outToken->set(TOKEN_COMPARE, tokenBuff->getTokenAsString());
            break;
        default:
            handle_single_char(ASSIGNER, inputStream, *st, c, tokenBuff, outToken);
            break;
    }
    return true;
}

static void handle_eol(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    handle_single_char(TOKEN_EOL, inputStream, st, c, tokenBuff, outToken);
}

static bool handle_space(FeedBackInputStream *inputStream, Token_Status st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    if (st == ST_Init) {
        return false;
    }
    TokenType type = findTokenType(st);
    outToken->set(type, tokenBuff->getTokenAsString());
    return true;
}


static bool handle_dot(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    int next_c;
    switch (*st) {
        case ST_Init:
        {
            next_c = inputStream->peekChar(1);
            if (next_c < 0) {
                tokenBuff->add_char(c);
                outToken->set(TOKEN_DOT, tokenBuff->getTokenAsString());
                return true;
            } else {
                if (next_c != '.') {
                    tokenBuff->add_char(c);
                    outToken->set(TOKEN_DOT, tokenBuff->getTokenAsString());
                    return true;
                } else {
                    tokenBuff->add_char(c);
                    *st = ST_DOT;
                    return false;
                }
            }
        }
        case ST_DOT:
            tokenBuff->add_char(c);
            outToken->set(DOUBLE_DOT, tokenBuff->getTokenAsString());
            return true;
        case ST_Number:
            next_c = inputStream->peekChar(1);
            if (next_c == '.') {
                inputStream->push_back(c);
                outToken->set(NUMBER, tokenBuff->getTokenAsString());
                return true;
            }
            if (tokenBuff->find_char(c) >= 0) {
                inputStream->push_back(c);
                outToken->set(NUMBER, tokenBuff->getTokenAsString());
                return true;
            } else {
                tokenBuff->add_char(c);
                return false;
            }
            break;
        case ST_Identifier:
            inputStream->push_back(c);
            outToken->set(IDENTIFIER, tokenBuff->getTokenAsString());
            return true;
        default:
            tokenBuff->add_char(c);
            outToken->set(UNKNOWN_TOKEN, tokenBuff->getTokenAsString());
            return true;
    }
}

static bool handle_number(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    switch (*st) {
        case ST_Init:
            tokenBuff->add_char(c);
            *st = ST_Number;
            return false;
        case ST_Number:
            tokenBuff->add_char(c);
            return false;
        case ST_Identifier:
            tokenBuff->add_char(c);
            return false;
        default:
            inputStream->push_back(c);
            TokenType type = findTokenType(*st);
            outToken->set(type, tokenBuff->getTokenAsString());
            return true;
    }
}

static bool handle_identifier(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    switch (*st) {
        case ST_Init:
            tokenBuff->add_char(c);
            *st = ST_Identifier;
            return false;
        case ST_Number:
            tokenBuff->add_char(c);
            return false;
        case ST_Identifier:
            tokenBuff->add_char(c);
            return false;
        default:
            inputStream->push_back(c);
            TokenType type = findTokenType(*st);
            outToken->set(type, tokenBuff->getTokenAsString());
            return true;
    }
    return true;
}

static bool handle_compare(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    if (*st == ST_Init) {
        tokenBuff->add_char(c);
        *st = ST_COMPARE;
        return false;
    } else {
        inputStream->push_back(c);
        TokenType type = findTokenType(*st);
        outToken->set(type, tokenBuff->getTokenAsString());
        return true;
    }
}

static bool handle_excl_mark(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    if (*st == ST_Init) {
        tokenBuff->add_char(c);
        *st = ST_EXCL_MARK;
        return false;
    } else {
        inputStream->push_back(c);
        TokenType type = findTokenType(*st);
        outToken->set(type, tokenBuff->getTokenAsString());
        return true;
    }
}


static void handle_line_comments(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    if (*st == ST_Init) {
        *st = ST_LINE_COMMENT;
    } else {
        inputStream->push_back(c);
        TokenType type = findTokenType(*st);
        outToken->set(type, tokenBuff->getTokenAsString());
    }
}

static bool handle_string(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    switch (*st ) {
        case ST_Init:
            // not include quot
            //tokenBuff->add_char(c);
            *st = ST_STRING;
            return false;
        case ST_STRING:
            // may be not here any time!
            tokenBuff->add_char(c);
            outToken->set(TOKEN_STRING, tokenBuff->getTokenAsString());
            return true;
        default:
            inputStream->push_back(c);
            TokenType type = findTokenType(*st);
            outToken->set(type, tokenBuff->getTokenAsString());
    }
    return true;
}

static bool handle_char_to_string(FeedBackInputStream *inputStream, Token_Status *st, char c, TokenBuf *tokenBuff, std::shared_ptr<Token> outToken) {
    switch (c) {
        case '\n':
        case '\r':
            throw std::string("string not allow multi lines.");
        case '\"':
        {
            char before_c = tokenBuff->peek_last();
            if (before_c == '\\') {
                tokenBuff->add_char(c);
                return false;
            } else {
                outToken->set(TOKEN_STRING, tokenBuff->getTokenAsString());
                return true;
            }
        }
        default:
            tokenBuff->add_char(c);
            break;
    }
    return false;
}

bool Lexer::peek(std::shared_ptr<Token> outToken, int n) {
    if (n <= 1) {
        n = 1;
    }
    for (std::list<std::shared_ptr<Token>>::const_iterator it = m_token_queue.begin(), end = m_token_queue.end(); it != end; ++it) {
        if (n == 1) {
            std::shared_ptr<Token> token(*it);
            outToken->set(token->getType(), token->getString());
            return true;
        }
        n--;
    }
    while (n > 0) {
        std::shared_ptr<Token> token(new Token);
        if (!readImpl(token)) {
            return false;
        }
        m_token_queue.push_back(token);
        n--;
    }
    std::shared_ptr<Token> last_item(m_token_queue.back());
    outToken->set(last_item->getType(), last_item->getString());
    return true;
}

Lexer::~Lexer(){
}

bool Lexer::readNext(std::shared_ptr<Token> outToken)
{
    if (m_token_queue.empty()) {
        return readImpl(outToken);
    } else {
        std::shared_ptr<Token> token(m_token_queue.front());
        m_token_queue.pop_front();
        outToken->set(token->getType(), token->getString());
        return true;
    }
}

bool Lexer::readImpl(std::shared_ptr<Token> outToken)
{
    if (!nextTokenImpl(outToken)) {
        return false;
    }
    if (outToken->getType() == IDENTIFIER) {
        ReservedWords* words = ReservedWords::instance();
        if (words->is_reserved(outToken->getString())) {
            outToken->setType(RESERVED_WORD);
        }
    }
    return true;
}


bool Lexer::nextTokenImpl(std::shared_ptr<Token> outToken)
{
    outToken->m_column = this->m_column;
    outToken->m_line = this->m_line;
    TokenBuf tokenBuff;
    Token_Status st = ST_Init;
    while (true) {
        int c = m_inputStream->readChar();
        if (c < 0) {
            if (st == ST_Init) {
                if (m_eof_returned) {
                    return false;
                } else {
                    outToken->set(TOKEN_EOF, "");
                    m_eof_returned = true;
                    return true;
                }
            } else {
                TokenType type = findTokenType(st);
                outToken->set(type, tokenBuff.getTokenAsString());
                m_inputStream->close();
                return true;
            }
        }
        if (ST_LINE_COMMENT == st) {
            if ('\n' == c || '\r' == c) {
                st = ST_Init;
            }
            continue;
        }
        if (ST_STRING == st) {
            if (handle_char_to_string(m_inputStream, &st, c, &tokenBuff, outToken)) {
                return true;
            } else {
                continue;
            }
        }
        switch (c) {
            case '\"':
                if (handle_string(m_inputStream, &st, c, &tokenBuff, outToken)) {
                    return true;
                } else {
                    continue;
                }
            case '#':
                handle_line_comments(m_inputStream, &st, c, &tokenBuff, outToken);
                continue;
            case '<':
            case '>':
                if (handle_compare(m_inputStream, &st, c, &tokenBuff, outToken)) {
                    return true;
                } else {
                    continue;
                }
            case '!':
                if (handle_excl_mark(m_inputStream, &st, c, &tokenBuff, outToken)) {
                    return true;
                } else {
                    continue;
                }
            case '+':
            case '/':
            case '*':
            case '-':
            case '%':
                handle_operator(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case '=':
                if (handle_assigner(m_inputStream, &st, c, &tokenBuff, outToken)) {
                    return true;
                } else {
                    continue;
                }
            case '(':
                handle_lparen(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case ')':
                handle_rparen(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case '{':
                handle_block_begin(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case '}':
                handle_block_end(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case ',':
                handle_comma(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case '[':
                handle_lparetheres(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case ']':
                handle_rparetheres(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case ':':
                handle_colon(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case '\r':
            case '\n':
                handle_eol(m_inputStream, st, c, &tokenBuff, outToken);
                return true;
            case ' ':
            case '\t':
                if(handle_space(m_inputStream, st, c, &tokenBuff, outToken)) {
                    return true;
                } else {
                    // or ignore
                    continue;
                }
            case '.':
                if (handle_dot(m_inputStream, &st, c, &tokenBuff, outToken)) {
                    return true;
                } else {
                    continue;
                }
            default:
                if ((c >= '0' && c <= '9')) {
                    if (handle_number(m_inputStream, &st, c, &tokenBuff, outToken)) {
                        return true;
                    } else {
                        continue;
                    }
                } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                    if (handle_identifier(m_inputStream, &st, c, &tokenBuff, outToken)) {
                        return true;
                    } else {
                        continue;
                    }
                } else {
                    tokenBuff.add_char(c);
                    outToken->set(UNKNOWN_TOKEN, tokenBuff.getTokenAsString());
                    return true;
                }
                break;
        }
        
    }
}


void FeedBackInputStream::read_char_for_num(char c)
{
    if (c == '\n') {
        m_lexer->m_line += 1;
        m_lexer->m_column = 1;
    } else {
        m_lexer->m_column += 1;
    }
}

bool FeedBackInputStream::try_pop(char *c) {
    if (m_pos < 0) {
        return false;
    }
    *c = m_feebackCharacters[m_pos];
    m_pos--;
    return true;
}

void FeedBackInputStream::push_back(char c) {
    if (m_pos >= 16-1) {
        throw "feedback is full.";
    }
    m_feebackCharacters[++m_pos] = c;
}

int FeedBackInputStream::peekChar(int n) {
    if (n <= 1) {
        n = 1;
    }
    for (int i = 0; i <= m_pos; ++i) {
        if (n == 1) {
            return m_feebackCharacters[i];
        }
        n--;
    }
    while (n > 0) {
        std::shared_ptr<Token> token(new Token);
        int c = readChar();
        if (c < 0) {
            return false;
        }
        push_back(c);
        n--;
    }
    return m_feebackCharacters[m_pos];
}

int FeedBackInputStream::readChar() {
    char c;
    if (try_pop(&c)) {
        return c;
    }
    int v = m_targetInputStream->readChar();
    read_char_for_num((char)v);
    return v;
}

/// ===========
StringInputStream::StringInputStream(const char* s): m_pos(0) {
    size_t s_len = strlen(s);
    m_string_data = new char[s_len + 1];
    m_string_data[s_len] = '\0';
    strncpy(m_string_data, s, s_len);
}
