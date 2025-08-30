#ifndef __NODE_VISITOR_HPP
#define __NODE_VISITOR_HPP
#include <utility>
#include <variant>

typedef std::pair<int, bool> NVRet;
#define NVRDef NVRet(0, false);
typedef std::variant<int, std::reference_wrapper<int>, NVRet> Accept;

class ScopeNode;
class BlockNode;
class RetNode;
class ForLoopNode;
class WhileLoopNode;
class IfNode;
class AssignNode;
class NumNode;
class BinNode;
class UnaryNode;
class FnDeclNode;
class FnCallNode;
class ArrDeclNode;
class ArrAccessNode;
class VarNode;
class VarDeclNode;
class IOInNode;
class IOOutNode;
class NodeVisitor {
  public:
  virtual NVRet vi_scope(const ScopeNode &) = 0;
  virtual NVRet vi_block(const BlockNode &) = 0;
  virtual NVRet vi_ret(const RetNode &) = 0;
  virtual NVRet vi_for(const ForLoopNode &) = 0;
  virtual NVRet vi_while(const WhileLoopNode &) = 0;
  virtual NVRet vi_if(const IfNode &) = 0;

  virtual int &vi_assign(const AssignNode &) = 0;
  virtual int vi_num(const NumNode &) = 0;
  virtual int vi_bin(const BinNode &) = 0;
  virtual int vi_unary(const UnaryNode &) = 0;

  virtual int vi_fn_decl(FnDeclNode &) = 0;
  virtual int vi_fn_call(const FnCallNode &) = 0;
  virtual int vi_arr_decl(ArrDeclNode &) = 0;
  virtual int &vi_arr_acc(const ArrAccessNode &) = 0;
  virtual int &vi_var(const VarNode &) = 0;
  virtual int vi_var_decl(const VarDeclNode &) = 0;

  virtual int vi_io_in(const IOInNode &) = 0;
  virtual int vi_io_out(const IOOutNode &) = 0;
};
#endif