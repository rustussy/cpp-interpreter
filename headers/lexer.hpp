#ifndef __LEXER_HPP
#define __LEXER_HPP
#include <stdexcept>
#include <string>

#include "error.hpp"
#include "token.hpp"

class Lexer
{
protected:
    std::string code;
    size_t pos;
    TokenType get_token_type(char c)
    {
        switch (c)
        {
            case '+':
                return TokenType::PLUS;
            case '-':
                return TokenType::MINUS;
            case '*':
                return TokenType::MUL;
            case '/':
                return TokenType::DIV;
            case '{':
                return TokenType::BRACE_OPEN;
            case '}':
                return TokenType::BRACE_CLOSE;
            case '(':
                return TokenType::PAREN_OPEN;
            case ')':
                return TokenType::PAREN_CLOSE;
            case '[':
                return TokenType::BRKET_OPEN;
            case ']':
                return TokenType::BRKET_CLOSE;
            case ';':
                return TokenType::SEMI;
            case ',':
                return TokenType::COMMA;
            case '%':
                return TokenType::MOD;
            case '^':
                return TokenType::BW_XOR;
            default:
                return TokenType::OTHERS;
        }
    }

    void skip_whitespace()
    {
        while (pos < code.length() && std::isspace(code[pos]))
        {
            pos++;
        }
    }

    bool is_valid_var_name(char x, bool is_begin)
    {
        if (is_begin)
        {
            return isalpha(x) || x == '_';
        }
        return isalnum(x) || x == '_';
    }

    Token get_identifier()
    {
        std::string value;
        while (pos < code.length() && is_valid_var_name(code[pos], false))
        {
            value += code[pos];
            pos++;
        }
        // reserved keywords
        if (value == "int" || value == "bool" || value == "char")
        {
            return {TokenType::VAR_TYPE, value};
        }
        if (value == "true")
        {
            return {TokenType::INT, "1"};
        }
        if (value == "false")
        {
            return {TokenType::INT, "0"};
        }
        if (value == "cout" || value == "cin" || value == "putchar")
        {
            return {TokenType::IO, value};
        }
        if (value == "return")
        {
            return {TokenType::RET, value};
        }
        if (value == "for")
        {
            return {TokenType::FOR, value};
        }
        if (value == "while")
        {
            return {TokenType::WHILE, value};
        }
        if (value == "if")
        {
            return {TokenType::IF, value};
        }
        if (value == "else")
        {
            return {TokenType::ELSE, value};
        }
        return {TokenType::VAR, value};
    }

    Token get_number()
    {
        std::string value;
        while (pos < code.length() && std::isdigit(code[pos]))
        {
            value += code[pos];
            pos++;
        }
        return {TokenType::INT, value};
    }

public:
    Lexer(std::string &_code) : code(std::move(_code)), pos(0) {}

    Token get_next_token()
    {
        while (pos < code.length())
        {
            char cur_char = code[pos];

            if (std::isspace(cur_char))
            {
                skip_whitespace();
                continue;
            }

            if (std::isdigit(cur_char))
            {
                return get_number();
            }

            auto cur_char_type = get_token_type(cur_char);

            if (cur_char_type != TokenType::OTHERS)
            {
                pos++;
                return {cur_char_type, std::string(1, cur_char)};
            }

            if (cur_char == '=')
            {
                auto peeked_char = peek();
                pos++;
                if (peeked_char != nullptr && *peeked_char == '=')
                {
                    pos++;
                    return {TokenType::CMP_EQU, "=="};
                }
                return {TokenType::ASSIGN, "="};
            }

            if (cur_char == '!')
            {
                auto peeked_char = peek();
                pos++;
                if (peeked_char != nullptr && *peeked_char == '=')
                {
                    pos++;
                    return {TokenType::CMP_NEQ, "!="};
                }
                return {TokenType::NEGATE, "!"};
            }

            if (cur_char == '>')
            {
                auto peeked_char = peek();
                pos++;
                if (peeked_char != nullptr)
                {
                    switch (*peeked_char)
                    {
                        case '=':
                            pos++;
                            return {TokenType::CMP_GTE, ">="};
                        case '>':
                            pos++;
                            return {TokenType::BW_SHIFTR, ">>"};
                        default:
                            break;
                    }
                }
                return {TokenType::CMP_GRT, ">"};
            }

            if (cur_char == '<')
            {
                auto peeked_char = peek();
                pos++;
                if (peeked_char != nullptr)
                {
                    switch (*peeked_char)
                    {
                        case '=':
                            pos++;
                            return {TokenType::CMP_LTE, "<="};
                        case '<':
                            pos++;
                            return {TokenType::BW_SHIFTL, "<<"};
                        default:
                            break;
                    }
                }
                return {TokenType::CMP_LES, "<"};
            }

            if (cur_char == '|')
            {
                auto peeked_char = peek();
                pos++;
                if (peeked_char != nullptr && *peeked_char == '|')
                {
                    pos++;
                    return {TokenType::OR, "||"};
                }
                throw std::runtime_error(get_err(ErrMsg::UNS_SYNT));
            }

            if (cur_char == '&')
            {
                auto peeked_char = peek();
                pos++;
                if (peeked_char != nullptr && *peeked_char == '&')
                {
                    pos++;
                    return {TokenType::AND, "&&"};
                }
                throw std::runtime_error(get_err(ErrMsg::UNS_SYNT));
            }

            if (cur_char == '\'')
            {
                pos++;
                std::string value;
                do
                {
                    value += code[pos];
                    pos++;
                } while (pos < code.length() && code[pos] != '\'');
                pos++;
                return {TokenType::CHAR, value};
            }

            if (is_valid_var_name(cur_char, true))
            {
                return get_identifier();
            }
        }
        return {TokenType::EOF_TOKEN, ""};
    }

    char *peek()
    {
        if (pos + 1 >= code.length())
        {
            return nullptr;
        }
        return &code[pos + 1];
    }
};
#endif