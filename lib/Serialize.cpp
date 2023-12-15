#include "Serialize.h"
#include "Visitor.h"
#include <nix/input-accessor.hh>
#include <nix/nixexpr.hh>
#include <nix/symbol-table.hh>

#include <cstring>
#include <type_traits>

namespace nixt::serialize {

StringIdx StringTable::add(std::string Str) {
  if (Map.contains(Str)) {
    return Map[std::move(Str)];
  }
  Length += Str.size() + 1; // size of string + null terminator
  StringIdx Ret = Map[Str] = Length;
  Strings.push_back(std::move(Str));
  return Ret;
}

void StringTable::write(char *Dst) const {
  for (const auto &S : Strings) {
    memcpy(Dst, S.c_str(), S.size() + 1);
    Dst = static_cast<char *>(Dst) + S.size() + 1;
  }
}

std::size_t ASTWriter::count(const nix::Expr *E) {
  struct ASTWriteVisitor : public RecursiveASTVisitor<ASTWriteVisitor> {
    IndexVector &Exprs;
    IndexVector &Pos;
    StringTable &Strings;
    const nix::SymbolTable &STable;
    const nix::PosTable &PTable;
    std::map<const nix::Expr *, std::size_t> ExprMap;

    ASTWriteVisitor(IndexVector &Exprs, IndexVector &Pos, StringTable &Strings,
                    const nix::SymbolTable &STable, const nix::PosTable &PTable)
        : Exprs(Exprs), Pos(Pos), Strings(Strings), STable(STable),
          PTable(PTable) {}

    Position getPos(const nix::PosIdx P) {
      if (P == nix::noPos) {
        return Position{};
      }
      const auto &Pos = PTable[P];
      Position Ret;
      Ret.Column = Pos.column;
      Ret.Line = Pos.line;
      Position::OriginKind KindTable[] = {Position::OK_None, Position::OK_Stdin,
                                          Position::OK_String,
                                          Position::OK_SourcePath};
      Ret.Kind = KindTable[Pos.origin.index()];
      if (Ret.Kind == Position::OK_SourcePath)
        Ret.Source =
            Strings.add(std::get<nix::SourcePath>(Pos.origin).to_string());
      return Ret;
    }
    bool visitExprAttrs(const nix::ExprAttrs *E) {
      ExprAttrSet Expr;
      Expr.Pos = Pos.add(getPos(E->pos));
      for (const auto &[K, V] : E->attrs) {
        Expr.Attrs.emplace_back(AttrDefEntry{
            Strings.add(STable[K]),
            {V.inherited, ExprMap[V.e], Pos.add(getPos(V.pos)), V.displ}});
      }
      ExprMap[E] = Exprs.add(Expr);
      return true;
    }
    bool visitExprVar(const nix::ExprVar *E) {
      SimpleExpr Expr;
      Expr.Kind = ExprKind::EK_Var;
      Expr.Body.Var.Displ = E->displ;
      Expr.Body.Var.FromWidth = E->fromWith;
      Expr.Body.Var.Name = Strings.add(STable[E->name]);
      Expr.Body.Var.Pos = Pos.add(getPos(E->pos));
      ExprMap[E] = Exprs.add(Expr);
      return true;
    }

  } Writer(Exprs, Pos, Strings, STable, PTable);
  Writer.traverseExpr(E);

  return sizeof(ASTHeader) + getSize(Exprs) + getSize(Pos) +
         Strings.getLength();
}

void ASTWriter::write(char *Dst) {
  ASTHeader Header;
  Header.Version = 1;
  Header.ExprTableOffset = sizeof(ASTHeader);
  Header.PosTableOffset = Header.ExprTableOffset + getSize(Exprs);
  Header.StringTableOffset = Header.PosTableOffset + getSize(Pos);
  memcpy(Dst, &Header, sizeof(ASTHeader));
  Dst += sizeof(ASTHeader);
  Dst += encode(Dst, Exprs);
  Dst += encode(Dst, Pos);
  Strings.write(Dst);
}

std::size_t encode(char *Dst, const ExprAttrSet &E) {
  std::size_t Ret = encode(Dst, E.Kind);
  Ret += encode(Dst + Ret, E.Pos);
  Ret += encode(Dst + Ret, E.Attrs);
  Ret += encode(Dst + Ret, E.DynamicAttrs);
  return Ret;
}

std::size_t getSize(const ExprAttrSet &E) {
  return sizeof(E.Kind) + getSize(E.Pos) + getSize(E.Attrs) +
         getSize(E.DynamicAttrs);
}

} // namespace nixt::serialize
