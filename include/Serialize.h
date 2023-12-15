#pragma once

#include <nix/nixexpr.hh>
#include <nix/symbol-table.hh>
#include <nix/value.hh>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

/// \file
/// This file contains serialization method for nix::Expr

namespace nixt::serialize {

using nix::NixFloat;
using nix::NixInt;

template <class T, class = std::enable_if_t<std::is_standard_layout_v<T>>,
          class = std::enable_if_t<std::is_trivial_v<T>>>
std::size_t encode(char *Dst, const T &Data) {
  memcpy(Dst, &Data, sizeof(T));
  return sizeof(T);
}

template <class T, class = std::enable_if_t<std::is_standard_layout_v<T>>,
          class = std::enable_if_t<std::is_trivial_v<T>>>
std::size_t getSize(const T &Data) {
  return sizeof(T);
}

template <class T> std::size_t encode(char *Dst, const std::vector<T> &Data) {
  std::size_t Ret = encode(Dst, Data.size());
  for (const auto &E : Data) {
    Ret += encode(Dst + Ret, E);
  }
  return Ret;
}

template <class T> std::size_t getSize(const std::vector<T> &Data) {
  std::size_t Ret = sizeof(Data.size());
  for (const auto &E : Data) {
    Ret += getSize(E);
  }
  return Ret;
}

/// Wrapper around std::vector that we can easily get indexes of added element.
struct IndexVector {
  template <class T> std::size_t add(T Elt) {
    std::size_t EltSize = getSize(Elt);
    Data.reserve(Data.size() + EltSize);
    Data.resize(Data.size() + EltSize);
    return encode(Data.data() + Data.size() - EltSize, Elt);
  }
  IndexVector() = default;
  std::vector<char> Data;
};

inline std::size_t encode(char *Dst, const IndexVector &Data) {
  return encode(Dst, Data.Data);
}

inline std::size_t getSize(const IndexVector &Data) {
  return getSize(Data.Data);
}

class StringTable {
public:
  using Size = std::size_t;
  Size add(std::string Str);

  [[nodiscard]] uint32_t getLength() const { return Length; }

  void write(char *Dst) const;

  StringTable() = default;

private:
  std::map<std::string, Size> Map;
  std::vector<std::string> Strings;
  uint32_t Length = 0;
};

using StringIdx = StringTable::Size;
using PosIdx = std::size_t;
using ExprIdx = std::size_t;

struct Position {
  uint32_t Line, Column;
  enum OriginKind {
    OK_None,
    OK_Stdin,
    OK_String,
    OK_SourcePath,
  } Kind;
  // For "Source Path"
  StringIdx Source;
};

struct AttrPath {
  uint32_t Size;
  StringIdx Name;
};

struct AttrDef {
  bool Inherited;
  ExprIdx E;
  PosIdx Pos;
  uint32_t Displ;
};

struct AttrDefEntry {
  StringIdx Name;
  AttrDef Def;
};

struct DynamicAttrDef {
  ExprIdx NameExpr, ValueExpr;
  PosIdx Pos;
};

enum class ExprKind : uint32_t {
  EK_Int,
  EK_Float,
  EK_String,
  EK_Path,
  EK_Var,

  // Needs custom encoding & decoding
  EK_AttrSet,
};

struct SimpleExpr {
  ExprKind Kind;

  struct Variable {
    PosIdx Pos;
    StringIdx Name;

    bool FromWidth;

    uint32_t Level;
    uint32_t Displ;
  };

  struct Select {
    PosIdx Pos;
    AttrPath Path;
    ExprIdx E;
    ExprIdx Def;
  };

  struct OpHasAttr {
    ExprIdx E;
    AttrPath Path;
  };

  union {
    NixInt Int;
    NixFloat Float;
    StringIdx String;
    StringIdx Path;
    Variable Var;
    struct Select Select;
  } Body;
};

struct ExprAttrSet {
  ExprKind Kind = ExprKind::EK_AttrSet;
  PosIdx Pos;
  std::vector<AttrDefEntry> Attrs;
  std::vector<DynamicAttrDef> DynamicAttrs;
};

std::size_t encode(char *Dst, const ExprAttrSet &E);

std::size_t getSize(const ExprAttrSet &E);

struct ASTHeader {
  unsigned char NixAST[8] = "\x7fNixAST";
  uint32_t Version;
  std::size_t ExprTableOffset;
  std::size_t PosTableOffset;
  std::size_t StringTableOffset;
};

class ASTWriter {
public:
  ASTWriter(const nix::SymbolTable &STable, const nix::PosTable &PTable)
      : STable(STable), PTable(PTable) {}

  std::size_t count(const nix::Expr *E);

  void write(char *Dst);

private:
  const nix::SymbolTable &STable;
  const nix::PosTable &PTable;
  IndexVector Exprs;
  IndexVector Pos;
  StringTable Strings;
};

} // namespace nixt::serialize
