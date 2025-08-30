#ifndef __INTERPRETER_HPP
#define __INTERPRETER_HPP
#include <cassert>
#include <deque>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "error.hpp"
#include "node.hpp"
#include "node_visitor.hpp"
#include "utils.hpp"

class CallStack {
  public:
  class Array {
public:
    typedef std::variant<int, std::shared_ptr<CallStack::Array>> ArrayData;
    typedef std::size_t SizeType;

protected:
    std::vector<ArrayData> __arr;

public:
    Array(SizeType _sz, const ArrayData &_val) {
      __arr.resize(_sz);
      if (std::holds_alternative<std::shared_ptr<CallStack::Array>>(_val)) {
        auto value = std::get<std::shared_ptr<CallStack::Array>>(_val);
        for (SizeType i = 0; i < _sz; ++i) {
          __arr[i] = std::make_shared<CallStack::Array>(*value);
        }
      } else {
        auto value = std::get<int>(_val);
        for (SizeType i = 0; i < _sz; ++i) {
          __arr[i] = value;
        }
      }
    }

    SizeType size() const { return __arr.size(); }

    ArrayData &operator[](SizeType idx) { return __arr[idx]; }
    const ArrayData &operator[](SizeType idx) const { return __arr[idx]; }
  };

  protected:
  typedef std::variant<int, FnDeclNode *, std::shared_ptr<Array>> CType;

  class VariableMap {
protected:
    std::string name;
    unsigned int level;

public:
    VariableMap(const std::string &_name, unsigned int _level) : name(_name), level(_level) {}
    std::unordered_map<std::string, CType> __internal_map;

    bool has(const std::string &__idx) { return __internal_map.find(__idx) != __internal_map.end(); }
    unsigned int get_level() { return level; }
    CType &operator[](const std::string &__idx) { return __internal_map[__idx]; }
  };
  std::deque<VariableMap> __stack;

  public:
  void add_scope(const std::string &_name, unsigned int _level) { __stack.push_front(VariableMap(_name, _level)); }
  void pop_scope() {
    assert(__stack.size() > 0);
    __stack.pop_front();
  }
  unsigned int top_level() {
    if (__stack.empty()) {
      return 0;
    }
    return __stack.front().get_level();
  }
  template <typename T>
  T &get(const std::string &_key) {
    for (auto &i : __stack) {
      if (i.has(_key)) {
        auto &res = i[_key];
        if (!std::holds_alternative<T>(res)) {
          throw std::runtime_error(get_err(ErrMsg::INV_DT_TYPE));
        }
        return std::get<T>(res);
      }
    }
    throw std::runtime_error(get_err(ErrMsg::VAR_NDEF));
  }
  void set(std::string &_key, const CType &_value) {
    for (auto &i : __stack) {
      if (i.has(_key)) {
        i[_key] = _value;
        return;
      }
    }
    throw std::runtime_error(get_err(ErrMsg::VAR_NDEF));
  }
  void register_var(const std::string &_key, const CType &_value) { __stack.front()[_key] = _value; }
};

class Interpreter : public NodeVisitor {
  protected:
  CallStack cst;

  public:
  NVRet vi_scope(const ScopeNode &program) {
    cst.add_scope("__interpreter_scoped__", cst.top_level() + 1);
    auto res = vi_block(*program.block);
    cst.pop_scope();
    return res;
  }
  NVRet vi_block(const BlockNode &block) {
    for (auto child : block.children) {
      auto res = child->accept(*this);
      if (std::holds_alternative<NVRet>(res)) {
        auto res_get = std::get<NVRet>(res);
        if (res_get.second) {
          return res_get;
        }
      }
    }

    return NVRDef;
  }
  NVRet vi_ret(const RetNode &ret) { return NVRet(vi(*ret.expr), true); }
  NVRet vi_for(const ForLoopNode &forl) {
    auto for_chk_expr = forl.cond;
    auto for_body = forl.body;

    cst.add_scope("__interpreter_for_loop__", cst.top_level() + 1);

    for (auto &i : forl.init) {
      vi(*i);
    }

    while (for_chk_expr == nullptr || vi(*for_chk_expr)) {
      auto res = vi_block(*for_body);
      if (res.second) {
        return res;
      }
      for (auto &i : forl.upd) {
        vi(*i);
      }
    }
    cst.pop_scope();

    return NVRDef;
  }
  NVRet vi_while(const WhileLoopNode &whilel) {
    auto while_chk_expr = whilel.cond;
    auto while_body = whilel.body;

    cst.add_scope("__interpreter_while_loop__", cst.top_level() + 1);

    while (vi(*while_chk_expr)) {
      auto res = vi_block(*while_body);
      if (res.second) {
        return res;
      }
    }

    cst.pop_scope();

    return NVRDef;
  }
  NVRet vi_if(const IfNode &ifn) {
    auto bl_level = cst.top_level() + 1;
    if (ifn.if_bl.first != nullptr && vi(*ifn.if_bl.first)) {
      if (ifn.if_bl.second == nullptr) {
        return NVRDef;
      }
      cst.add_scope("__interpreter_if__", bl_level);
      auto res = vi_block(*ifn.if_bl.second);
      cst.pop_scope();
      if (res.second) {
        return res;
      }
      return NVRDef;
    }
    for (auto &elif : ifn.elif_bl) {
      if (elif.first != nullptr && vi(*elif.first)) {
        if (elif.second == nullptr) {
          return NVRDef;
        }
        cst.add_scope("__interpreter_elif__", bl_level);
        auto res = vi_block(*elif.second);
        cst.pop_scope();
        if (res.second) {
          return res;
        }
        return NVRDef;
      }
    }
    if (ifn.else_bl == nullptr) {
      return NVRDef;
    }
    cst.add_scope("__interpreter_else__", bl_level);
    auto res = vi_block(*ifn.else_bl);
    cst.pop_scope();
    if (res.second) {
      return res;
    }
    return NVRDef;
  }

  int &vi_assign(const AssignNode &assign) {
    if (auto lv = std::dynamic_pointer_cast<VarNode>(assign.l)) {
      auto &lv_val = vi_var(*lv);
      auto rv = vi(*assign.r);
      lv_val = rv;
      return lv_val;
    }
    if (auto lv = std::dynamic_pointer_cast<ArrAccessNode>(assign.l)) {
      auto &lv_val = vi_arr_acc(*lv);
      auto rv = vi(*assign.r);
      lv_val = rv;
      return lv_val;
    }
    throw std::runtime_error(get_err(ErrMsg::INV_TOKEN));
  }
  int vi_num(const NumNode &num) { return num.value; }
  int vi_bin(const BinNode &bin) {
    auto lv = vi(*bin.l), rv = vi(*bin.r);
    switch (bin.op) {
      case TokenType::PLUS:
        return lv + rv;
      case TokenType::MINUS:
        return lv - rv;
      case TokenType::MUL:
        return lv * rv;
      case TokenType::DIV:
        return lv / rv;
      case TokenType::MOD:
        return lv % rv;
      case TokenType::CMP_EQU:
        return lv == rv;
      case TokenType::CMP_GRT:
        return lv > rv;
      case TokenType::CMP_GTE:
        return lv >= rv;
      case TokenType::CMP_LES:
        return lv < rv;
      case TokenType::CMP_LTE:
        return lv <= rv;
      case TokenType::CMP_NEQ:
        return lv != rv;
      case TokenType::BW_XOR:
        return lv ^ rv;
      case TokenType::AND:
        return lv && rv;
      case TokenType::OR:
        return lv || rv;
      default:
        return 0;
    }
  }
  int vi_unary(const UnaryNode &unary) {
    switch (unary.op) {
      case TokenType::PLUS:
        return +vi(*unary.expr);
      case TokenType::MINUS:
        return -vi(*unary.expr);
      case TokenType::NEGATE:
        return !vi(*unary.expr);
      default:
        return 0;
    }
  }

  int &vi_var(const VarNode &var) { return cst.get<int>(var.var_name); }
  int vi_var_decl(const VarDeclNode &var_decl) {
    cst.register_var(var_decl.var->var_name, vi(*var_decl.var_value));
    return 0;
  }
  int vi_fn_decl(FnDeclNode &fn_call) {
    cst.register_var(fn_call.name, &fn_call);
    return 0;
  }
  int vi_fn_call(const FnCallNode &fn_call) {
    auto &fn_name = fn_call.name;

    auto fn = cst.get<FnDeclNode *>(fn_name);

    cst.add_scope(fn_name, cst.top_level() + 1);

    for (std::size_t i = 0; i < fn_call.call_params.size(); ++i) {
      cst.register_var(fn->params[i]->var->var_name, vi(*fn_call.call_params[i]));
    }

    auto res = vi_block(*fn->block).first;

    cst.pop_scope();

    return res;
  }
  int vi_arr_decl(ArrDeclNode &arr_decl) {
    if (arr_decl.dimensions.empty()) {
      return 0;
    }
    std::shared_ptr<CallStack::Array> arr = std::make_shared<CallStack::Array>(vi(*arr_decl.dimensions.back()), 0);
    for (auto i = arr_decl.dimensions.rbegin() + 1; i != arr_decl.dimensions.rend(); ++i) {
      arr = std::make_shared<CallStack::Array>(vi(**i), arr);
    }
    cst.register_var(arr_decl.name, arr);
    return 0;
  }
  int &vi_arr_acc(const ArrAccessNode &arr_access) {
    auto arr = cst.get<std::shared_ptr<CallStack::Array>>(arr_access.name);
    for (auto &i : arr_access.dimensions) {
      auto &val = (*arr)[static_cast<unsigned long>(vi(*i))];
      if (std::holds_alternative<int>(val)) {
        return std::get<int>(val);
      }
      arr = std::get<std::shared_ptr<CallStack::Array>>(val);
    }
    throw std::runtime_error(get_err(ErrMsg::MISM_TYPE));
  }

  int vi_io_in(const IOInNode &io) {
    switch (io.type) {
      case IOType::CIN: {
        for (auto &i : io.body) {
          if (auto i_int = std::dynamic_pointer_cast<VarNode>(i)) {
            std::cin >> vi_var(*i_int);
          } else if (auto i_arr = std::dynamic_pointer_cast<ArrAccessNode>(i)) {
            std::cin >> vi_arr_acc(*i_arr);
          } else if (auto i_assign = std::dynamic_pointer_cast<AssignNode>(i)) {
            std::cin >> vi_assign(*i_assign);
          } else {
            throw std::runtime_error(get_err(ErrMsg::INV_TOKEN));
          }
        }
      }
      default:
        break;
    }
    return 0;
  }

  int vi_io_out(const IOOutNode &io) {
    switch (io.type) {
      case IOType::PUTCHAR: {
        int res{};
        for (auto &i : io.body) {
          res += vi(*i);
        }
        std::putchar(res);
        return res;
      }
      case IOType::COUT: {
        for (auto &i : io.body) {
          if (auto charn = std::dynamic_pointer_cast<CharNode>(i)) {
            if (charn->value == "endl") {
              std::cout << std::endl;
            } else {
              auto &char_value = charn->value;
              if (char_value == "\\n") {
                std::cout << '\n';
              } else {
                std::cout << char_value;
              }
            }
          } else {
            std::cout << vi(*i);
          }
        }
        return 0;
      }
      default:
        return 0;
    }
  }

  int vi(Node &node) {
    auto get_fn =
    overloaded{[](int x) -> int { return x; }, [](const std::reference_wrapper<int> &x) -> int { return x.get(); },
               [](const NVRet &x) -> int { return x.first; }};

    return std::visit(get_fn, node.accept(*this));
  }
};
#endif