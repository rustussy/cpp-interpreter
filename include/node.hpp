#ifndef __NODE_HPP
#define __NODE_HPP
#include <memory>
#include <vector>

#include "node_visitor.hpp"
#include "token.hpp"

class Node {
  public:
  virtual ~Node() = default;
  virtual Accept accept(NodeVisitor &) { return 0; };
};

class BinNode : public Node {
  public:
  std::shared_ptr<Node> l, r;
  TokenType op;
  BinNode() = default;
  BinNode(const std::shared_ptr<Node> &_l, const std::shared_ptr<Node> &_r, const TokenType &_op)
      : l(_l), r(_r), op(_op) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_bin(*this); }
};
class UnaryNode : public Node {
  public:
  std::shared_ptr<Node> expr;
  TokenType op;
  UnaryNode() = default;
  UnaryNode(const std::shared_ptr<Node> &_expr, const TokenType &_token_type) : expr(_expr), op(_token_type) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_unary(*this); }
};
class AssignNode : public Node {
  public:
  std::shared_ptr<Node> l, r;
  AssignNode() = default;
  AssignNode(const std::shared_ptr<Node> &_l, const std::shared_ptr<Node> &_r) : l(_l), r(_r) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_assign(*this); }
};

class NumNode : public Node {
  public:
  int value;
  NumNode() = default;
  NumNode(const std::string &_token_value) : value(std::stoi(_token_value)) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_num(*this); }
};

class VarNode : public Node {
  public:
  std::string var_name;
  VarNode() = default;
  VarNode(std::string &_var_name) : var_name(std::move(_var_name)) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_var(*this); }
};
class VarDeclNode : public Node {
  public:
  std::shared_ptr<VarNode> var;
  std::string var_type;
  std::shared_ptr<Node> var_value;
  VarDeclNode() = default;
  VarDeclNode(const decltype(var) &_var, decltype(var_type) &_type, const decltype(var_value) &_value)
      : var(_var), var_type(std::move(_type)), var_value(_value) {}
  Accept accept(NodeVisitor &nv) { return nv.vi_var_decl(*this); }
};

class ParamsDeclNode : public Node {
  public:
  std::shared_ptr<VarNode> var;
  std::string var_type;
  ParamsDeclNode() = default;
  ParamsDeclNode(const decltype(var) &_var, decltype(var_type) &_type) : var(_var), var_type(std::move(_type)) {}
};
class FnDeclNode : public Node {
  public:
  std::string return_type, name;
  std::vector<std::shared_ptr<ParamsDeclNode>> params;
  std::shared_ptr<BlockNode> block;
  FnDeclNode() = default;
  FnDeclNode(decltype(return_type) &_ret_type, std::string &_name)
      : return_type(std::move(_ret_type)), name(std::move(_name)) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_fn_decl(*this); }
};
class FnCallNode : public Node {
  public:
  std::string name;
  std::vector<std::shared_ptr<Node>> call_params;
  FnCallNode() = default;
  FnCallNode(decltype(name) &_name) : name(std::move(_name)) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_fn_call(*this); }
};

class ArrDeclNode : public Node {
  public:
  std::string type, name;
  std::vector<std::shared_ptr<Node>> dimensions;
  ArrDeclNode(decltype(type) &_typ, std::string &_name) : type(std::move(_typ)), name(std::move(_name)) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_arr_decl(*this); }
};
class ArrAccessNode : public Node {
  public:
  std::string name;
  std::vector<std::shared_ptr<Node>> dimensions;
  ArrAccessNode(std::string &_name) : name(std::move(_name)) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_arr_acc(*this); }
};

class BlockNode : public Node {
  public:
  std::vector<std::shared_ptr<Node>> children;
  Accept accept(NodeVisitor &nv) override { return nv.vi_block(*this); }
};
class ScopeNode : public Node {
  public:
  std::shared_ptr<BlockNode> block;
  ScopeNode() = default;
  ScopeNode(const decltype(block) &_block) : block(_block) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_scope(*this); }
};
class ForLoopNode : public Node {
  public:
  std::vector<std::shared_ptr<Node>> init;
  std::shared_ptr<Node> cond;
  std::vector<std::shared_ptr<Node>> upd;
  std::shared_ptr<BlockNode> body;
  Accept accept(NodeVisitor &nv) override { return nv.vi_for(*this); }
};
class WhileLoopNode : public Node {
  public:
  std::shared_ptr<Node> cond;
  std::shared_ptr<BlockNode> body;
  Accept accept(NodeVisitor &nv) override { return nv.vi_while(*this); }
};
class IfNode : public Node {
  public:
  typedef std::pair<std::shared_ptr<Node>, std::shared_ptr<BlockNode>> IfBlock;
  typedef std::shared_ptr<BlockNode> ElseBlock;
  IfBlock if_bl;
  std::vector<IfBlock> elif_bl;
  std::shared_ptr<BlockNode> else_bl;
  Accept accept(NodeVisitor &nv) override { return nv.vi_if(*this); }
};

class RetNode : public Node {
  public:
  std::shared_ptr<Node> expr;
  RetNode() = default;
  RetNode(const decltype(expr) &_exp) : expr(_exp) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_ret(*this); }
};

enum class IOType {
  PUTCHAR,
  CIN,
  COUT,
};

inline IOType get_io_type(const std::string &_iostr) {
  if (_iostr == "putchar") {
    return IOType::PUTCHAR;
  }
  if (_iostr == "cin") {
    return IOType::CIN;
  }
  return IOType::COUT;
}

class IOOutNode : public Node {
  public:
  IOType type;
  std::vector<std::shared_ptr<Node>> body;
  IOOutNode() = default;
  IOOutNode(const IOType &_type) : type(_type) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_io_out(*this); }
};

class IOInNode : public Node {
  public:
  IOType type;
  std::vector<std::shared_ptr<Node>> body;
  IOInNode() = default;
  IOInNode(const IOType &_type) : type(_type) {}
  Accept accept(NodeVisitor &nv) override { return nv.vi_io_in(*this); }
};

class CharNode : public Node {
  public:
  std::string value;
  CharNode() = default;
  CharNode(std::string &_value) : value(std::move(_value)) {}
};
#endif