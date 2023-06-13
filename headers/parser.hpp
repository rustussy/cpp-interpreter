#ifndef __PARSER_HPP
#define __PARSER_HPP
#include <memory>

#include "lexer.hpp"
#include "node.hpp"

class Parser
{
protected:
    Lexer &lexer;
    Token cur_token;
    bool eat(const TokenType &token_type, bool safe = false)
    {
        if (cur_token.type == token_type)
        {
            cur_token = lexer.get_next_token();
            return true;
        }
        if (safe)
        {
            return false;
        }
        throw std::runtime_error(get_err(ErrMsg::INV_TOKEN));
    }
    std::shared_ptr<ScopeNode> scoped() { return std::make_shared<ScopeNode>(block()); }
    std::shared_ptr<BlockNode> block(bool single_statement = false)
    {
        auto res = std::make_shared<BlockNode>();
        if (single_statement)
        {
            bool should_eat_token = true;
            res->children.push_back(stment(true, true, should_eat_token));
            if (should_eat_token)
            {
                eat(TokenType::SEMI, true);
            }
        }
        else
        {
            stments(res->children, TokenType::SEMI, true, true);
        }
        return res;
    }

    std::shared_ptr<Node> var_decl(std::string &name, std::string &type)
    {
        if (cur_token.type == TokenType::BRKET_OPEN)
        {
            auto arr_node = std::make_shared<ArrDeclNode>(type, name);
            while (true)
            {
                if (!eat(TokenType::BRKET_OPEN, true))
                {
                    break;
                }
                arr_node->dimensions.push_back(expr());
                eat(TokenType::BRKET_CLOSE);
            }
            return arr_node;
        }

        if (eat(TokenType::ASSIGN, true))
        {
            return std::make_shared<VarDeclNode>(std::make_shared<VarNode>(name), type, expr());
        }

        return std::make_shared<VarDeclNode>(std::make_shared<VarNode>(name), type, std::make_shared<NumNode>("0"));
    }
    std::shared_ptr<VarNode> var()
    {
        auto node = std::make_shared<VarNode>(cur_token.value);
        eat(TokenType::VAR);
        return node;
    }

    std::shared_ptr<Node> var_stment()
    {
        auto name = cur_token.value;
        eat(TokenType::VAR);

        if (eat(TokenType::ASSIGN, true))
        {
            return std::make_shared<AssignNode>(std::make_shared<VarNode>(name), expr());
        }

        if (eat(TokenType::PAREN_OPEN, true))
        {
            auto fn_call_node = std::make_shared<FnCallNode>(name);
            if (cur_token.type != TokenType::PAREN_CLOSE)
            {
                while (true)
                {
                    fn_call_node->call_params.push_back(expr());
                    if (!eat(TokenType::COMMA, true))
                    {
                        break;
                    }
                }
            }
            eat(TokenType::PAREN_CLOSE);
            return fn_call_node;
        }

        if (cur_token.type == TokenType::BRKET_OPEN)
        {
            auto arr_access_node = std::make_shared<ArrAccessNode>(name);
            while (eat(TokenType::BRKET_OPEN, true))
            {
                arr_access_node->dimensions.push_back(expr());
                eat(TokenType::BRKET_CLOSE);
            }
            if (eat(TokenType::ASSIGN, true))
            {
                return std::make_shared<AssignNode>(arr_access_node, expr());
            }
            return arr_access_node;
        }

        return std::make_shared<VarNode>(name);
    }

    std::shared_ptr<Node> fn_decl(std::string &name, std::string &type)
    {
        eat(TokenType::PAREN_OPEN);
        auto fn_node = std::make_shared<FnDeclNode>(type, name);
        while (cur_token.type != TokenType::PAREN_CLOSE)
        {
            auto params_type = cur_token.value;
            eat(TokenType::VAR_TYPE);
            auto params = cur_token.value;
            eat(TokenType::VAR);
            fn_node->params.push_back(std::make_shared<ParamsDeclNode>(std::make_shared<VarNode>(params), params_type));
            if (cur_token.type == TokenType::PAREN_CLOSE)
            {
                break;
            }
            eat(TokenType::COMMA);
        }
        eat(TokenType::PAREN_CLOSE);
        eat(TokenType::BRACE_OPEN);
        fn_node->block = block();
        eat(TokenType::BRACE_CLOSE);
        return fn_node;
    }

    void init_var_stment(std::vector<std::shared_ptr<Node>> &result, bool allow_func_decl, bool &should_eat_token)
    {
        auto var_type = cur_token.value;
        eat(TokenType::VAR_TYPE);
        auto var_name = cur_token.value;
        eat(TokenType::VAR);

        if (cur_token.type == TokenType::PAREN_OPEN)
        {
            if (!allow_func_decl)
            {
                throw std::runtime_error(get_err(ErrMsg::INV_TOKEN));
            }
            result.push_back(fn_decl(var_name, var_type));
            should_eat_token = false;
        }
        else
        {
            result.push_back(var_decl(var_name, var_type));

            while (cur_token.type == TokenType::COMMA)
            {
                eat(TokenType::COMMA);
                auto cur_var_name = cur_token.value;
                eat(TokenType::VAR);
                result.push_back(var_decl(cur_var_name, var_type));
            }
        }
    }

    std::shared_ptr<Node> stment(bool allow_block, bool allow_ret, bool &should_eat_token)
    {
        if (cur_token.type == TokenType::BRACE_OPEN)
        {
            eat(TokenType::BRACE_OPEN);
            auto scope_node = scoped();
            eat(TokenType::BRACE_CLOSE);
            should_eat_token = false;
            return scope_node;
        }
        if (cur_token.type == TokenType::VAR_TYPE)
        {
            auto block_node = std::make_shared<BlockNode>();
            init_var_stment(block_node->children, allow_block, should_eat_token);
            return block_node;
        }
        if (cur_token.type == TokenType::VAR)
        {
            return var_stment();
        }
        if (allow_ret && cur_token.type == TokenType::RET)
        {
            eat(TokenType::RET);
            return std::make_shared<RetNode>(expr());
        }
        if (allow_block && cur_token.type == TokenType::FOR)
        {
            auto for_node = std::make_shared<ForLoopNode>();

            eat(TokenType::FOR);

            eat(TokenType::PAREN_OPEN);

            stments(for_node->init, TokenType::COMMA, false, false);

            eat(TokenType::SEMI);

            for_node->cond = expr();

            eat(TokenType::SEMI);

            exprs(for_node->upd);

            eat(TokenType::PAREN_CLOSE);

            auto should_eat_brace = eat(TokenType::BRACE_OPEN, true);
            for_node->body = block(!should_eat_brace);
            should_eat_token = false;
            if (should_eat_brace)
            {
                eat(TokenType::BRACE_CLOSE);
            }

            return for_node;
        }
        if (allow_block && cur_token.type == TokenType::WHILE)
        {
            auto while_node = std::make_shared<WhileLoopNode>();
            eat(TokenType::WHILE);
            eat(TokenType::PAREN_OPEN);
            while_node->cond = expr();
            eat(TokenType::PAREN_CLOSE);
            auto should_eat_brace = eat(TokenType::BRACE_OPEN, true);
            while_node->body = block(!should_eat_brace);
            should_eat_token = false;
            if (should_eat_brace)
            {
                eat(TokenType::BRACE_CLOSE);
            }
            return while_node;
        }
        if (allow_block && cur_token.type == TokenType::IF)
        {
            auto if_node = std::make_shared<IfNode>();

            eat(TokenType::IF);

            eat(TokenType::PAREN_OPEN);
            if_node->if_bl.first = expr();
            eat(TokenType::PAREN_CLOSE);

            auto should_eat_brace_if = eat(TokenType::BRACE_OPEN, true);
            if_node->if_bl.second = block(!should_eat_brace_if);
            should_eat_token = false;
            if (should_eat_brace_if)
            {
                eat(TokenType::BRACE_CLOSE);
            }

            while (eat(TokenType::ELSE, true))
            {
                if (eat(TokenType::IF, true))
                {
                    eat(TokenType::PAREN_OPEN);
                    auto elif_expr = expr();
                    eat(TokenType::PAREN_CLOSE);
                    auto should_eat_brace_elif = eat(TokenType::BRACE_OPEN, true);
                    if_node->elif_bl.emplace_back(elif_expr, block(!should_eat_brace_elif));
                    should_eat_token = false;
                    if (should_eat_brace_elif)
                    {
                        eat(TokenType::BRACE_CLOSE);
                    }
                }
                else
                {
                    auto should_eat_brace_else = eat(TokenType::BRACE_OPEN, true);
                    if_node->else_bl = block(!should_eat_brace_else);
                    should_eat_token = false;
                    if (should_eat_brace_else)
                    {
                        eat(TokenType::BRACE_CLOSE);
                    }
                    break;
                }
            }

            return if_node;
        }
        if (cur_token.type == TokenType::IO)
        {
            auto io_type = get_io_type(cur_token.value);
            eat(TokenType::IO);
            if (io_type == IOType::PUTCHAR)
            {
                auto putchar_node = std::make_shared<IOOutNode>(io_type);
                eat(TokenType::PAREN_OPEN);
                putchar_node->body.push_back(expr());
                eat(TokenType::PAREN_CLOSE);
                return putchar_node;
            }
            else if (io_type == IOType::CIN)
            {
                auto cin_node = std::make_shared<IOInNode>(io_type);
                while (eat(TokenType::BW_SHIFTR, true))
                {
                    cin_node->body.push_back(var_stment());
                }
                return cin_node;
            }
            else
            {
                auto cout_node = std::make_shared<IOOutNode>(io_type);
                while (eat(TokenType::BW_SHIFTL, true))
                {
                    if (cur_token.type == TokenType::CHAR)
                    {
                        cout_node->body.push_back(std::make_shared<CharNode>(cur_token.value));
                        eat(TokenType::CHAR);
                    }
                    else if (cur_token.type == TokenType::VAR && cur_token.value == "endl")
                    {
                        cout_node->body.push_back(std::make_shared<CharNode>(cur_token.value));
                        eat(TokenType::VAR);
                    }
                    else
                    {
                        cout_node->body.push_back(expr());
                    }
                }
                return cout_node;
            }
        }
        return nullptr;
    }
    void stments(std::vector<std::shared_ptr<Node>> &res, const TokenType &eat_token, bool allow_block, bool allow_ret)
    {
        while (true)
        {
            bool should_eat_token = true;
            auto stm = stment(allow_block, allow_ret, should_eat_token);
            if (stm != nullptr)
            {
                res.push_back(stm);
            }
            if (should_eat_token && !eat(eat_token, true))
            {
                break;
            }
        }
    }

    void exprs(std::vector<std::shared_ptr<Node>> &result)
    {
        while (true)
        {
            result.push_back(expr());

            if (!eat(TokenType::COMMA, true))
            {
                break;
            }
        }
    }

    std::shared_ptr<Node> expr()
    {
        auto node = or_expr();

        while (cur_token.type == TokenType::ASSIGN)
        {
            eat(TokenType::ASSIGN);
            node = std::make_shared<AssignNode>(node, or_expr());
        }

        return node;
    }
    std::shared_ptr<Node> or_expr()
    {
        auto node = and_expr();

        while (cur_token.type == TokenType::OR)
        {
            auto token_type = cur_token.type;
            eat(token_type);
            node = std::make_shared<BinNode>(node, and_expr(), token_type);
        }

        return node;
    }
    std::shared_ptr<Node> and_expr()
    {
        auto node = bitwise();

        while (cur_token.type == TokenType::AND)
        {
            auto token_type = cur_token.type;
            eat(token_type);
            node = std::make_shared<BinNode>(node, bitwise(), token_type);
        }

        return node;
    }
    std::shared_ptr<Node> bitwise()
    {
        auto node = cmp_eq();

        while (cur_token.type == TokenType::BW_XOR)
        {
            auto token_type = cur_token.type;
            eat(token_type);
            node = std::make_shared<BinNode>(node, cmp_eq(), token_type);
        }

        return node;
    }
    std::shared_ptr<Node> cmp_eq()
    {
        auto node = cmp_neq();

        while (cur_token.type == TokenType::CMP_EQU || cur_token.type == TokenType::CMP_NEQ)
        {
            auto token_type = cur_token.type;
            eat(token_type);
            node = std::make_shared<BinNode>(node, cmp_neq(), token_type);
        }

        return node;
    }
    std::shared_ptr<Node> cmp_neq()
    {
        auto node = add_sub_expr();

        while (cur_token.type == TokenType::CMP_LTE || cur_token.type == TokenType::CMP_LES ||
               cur_token.type == TokenType::CMP_GTE || cur_token.type == TokenType::CMP_GRT)
        {
            auto token_type = cur_token.type;
            eat(token_type);
            node = std::make_shared<BinNode>(node, add_sub_expr(), token_type);
        }

        return node;
    }

    std::shared_ptr<Node> add_sub_expr()
    {
        auto node = mul_div_expr();

        while (cur_token.type == TokenType::PLUS || cur_token.type == TokenType::MINUS)
        {
            auto token_type = cur_token.type;
            eat(token_type);
            node = std::make_shared<BinNode>(node, mul_div_expr(), token_type);
        }

        return node;
    }
    std::shared_ptr<Node> mul_div_expr()
    {
        auto node = factor();

        while (cur_token.type == TokenType::MUL || cur_token.type == TokenType::DIV || cur_token.type == TokenType::MOD)
        {
            auto token_type = cur_token.type;
            eat(token_type);
            node = std::make_shared<BinNode>(node, factor(), token_type);
        }

        return node;
    }
    std::shared_ptr<Node> factor()
    {
        auto token_type = cur_token.type;
        switch (token_type)
        {
            case TokenType::INT:
            {
                auto int_node = std::make_shared<NumNode>(cur_token.value);
                eat(TokenType::INT);
                return int_node;
            }
            case TokenType::PLUS:
            case TokenType::MINUS:
            case TokenType::NEGATE:
                eat(token_type);
                return std::make_shared<UnaryNode>(factor(), token_type);
            case TokenType::PAREN_OPEN:
            {
                eat(TokenType::PAREN_OPEN);
                auto node = expr();
                eat(TokenType::PAREN_CLOSE);
                return node;
            }
            case TokenType::IO:
            {
                if (cur_token.value == "putchar")
                {
                    eat(TokenType::IO);
                    auto putchar_node = std::make_shared<IOOutNode>(IOType::PUTCHAR);
                    eat(TokenType::PAREN_OPEN);
                    putchar_node->body.push_back(expr());
                    eat(TokenType::PAREN_CLOSE);
                    return putchar_node;
                }
                throw std::runtime_error(get_err(ErrMsg::INV_TOKEN));
            }
            case TokenType::VAR:
                return var_stment();
            default:
                return nullptr;
        }
    }

public:
    Parser(Lexer &_lexer) : lexer(_lexer), cur_token(lexer.get_next_token()) {}
    std::shared_ptr<ScopeNode> parse() { return scoped(); }
};
#endif