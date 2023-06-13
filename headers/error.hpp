#ifndef __ERROR_HPP
#define __ERROR_HPP
#include <string>

enum class ErrMsg
{
    INV_TOKEN,
    VAR_NDEF,
    VAR_ALRD,
    MISM_TYPE,
    UNS_SYNT,
    INV_DT_TYPE,
    INV_ARGS,
};

std::string get_err(const ErrMsg &_e)
{
    switch (_e)
    {
        case ErrMsg::INV_TOKEN:
            return "Syntax error: Unexpected token";
        case ErrMsg::VAR_NDEF:
            return "Type error: Variable is not defined";
        case ErrMsg::VAR_ALRD:
            return "Type error: Variable is already declared";
        case ErrMsg::MISM_TYPE:
            return "Type error: Invalid type provided";
        case ErrMsg::UNS_SYNT:
            return "Syntax error: This syntax is currently unsupported";
        case ErrMsg::INV_DT_TYPE:
            return "Type error: Unknown type";
        case ErrMsg::INV_ARGS:
            return "Type error: Invalid arguments list for function";
        default:
            return "Unknown runtime error";
    }
};
#endif