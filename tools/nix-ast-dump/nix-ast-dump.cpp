#include "Name.h"
#include "Visitor.h"

#include <filesystem>
#include <nix/canon-path.hh>
#include <nix/eval.hh>
#include <nix/nixexpr.hh>
#include <nix/search-path.hh>
#include <nix/shared.hh>
#include <nix/store-api.hh>

#include <iostream>
#include <memory>

using namespace nixt;

struct ASTDump : RecursiveASTVisitor<ASTDump> {

  int Depth = 0;
  std::unique_ptr<nix::EvalState> State;

  bool traverseExpr(const nix::Expr *E) {
    Depth++;
    if (!RecursiveASTVisitor<ASTDump>::traverseExpr(E))
      return false;
    Depth--;
    return true;
  }

  bool visitExpr(const nix::Expr *E) const {
    for (int I = 0; I < Depth; I++)
      std::cout << " ";
    std::cout << nameOf(E) << ": ";
    E->show(State->symbols, std::cout);
    std::cout << "\n";
    return true;
  }
};

void printUsage() { std::cout << "Usage: nix-ast-dump <filename>\n"; }

int main(int argc, char *argv[]) {
  if (argc != 2 || (argc == 2 && std::string(argv[1]) == "--help")) {
    printUsage();
    return 0;
  }

  nix::initNix();
  nix::initGC();
  try {
    auto State = std::make_unique<nix::EvalState>(nix::SearchPath{},
                                                  nix::openStore("dummy://"));
    const auto *E = State->parseExprFromFile(
        nix::CanonPath(std::filesystem::absolute((argv[1])).string()));
    ASTDump A;
    A.State = std::move(State);
    A.traverseExpr(E);
  } catch (std::exception &E) {
    std::cout << E.what() << std::endl;
    return 1;
  } catch (...) {
    throw;
  }
  return 0;
}
