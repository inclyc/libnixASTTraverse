#include "Name.h"
#include "Serialize.h"

#include <nix/eval.hh>
#include <nix/shared.hh>
#include <nix/store-api.hh>

#include <sys/mman.h>

#include <iostream>
#include <memory>
#include <unistd.h>

using namespace nixt;

int main(int argc, char *argv[]) {
  const char *filepath = "./file.txt";
  int fd = open(filepath, O_RDWR | O_CREAT, 0666);
  nix::initNix();
  nix::initGC();
  try {
    auto State = std::make_unique<nix::EvalState>(nix::SearchPath{},
                                                  nix::openStore("dummy://"));
    const auto *E = State->parseExprFromFile(
        nix::CanonPath(std::filesystem::absolute((argv[1])).string()));
    serialize::ASTWriter Writer(State->symbols, State->positions);
    auto Count = Writer.count(E);

    std::cout << "Count: " << Count << std::endl;

    lseek(fd, Count - 1, SEEK_SET);
    write(fd, "", 1);

    void *Buf = mmap(nullptr, Count, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    Writer.write(static_cast<char *>(Buf));
    munmap(Buf, Count);
    return 0;
  } catch (std::exception &E) {
    std::cout << E.what() << std::endl;
    return 1;
  } catch (...) {
    throw;
  }
  return 0;
}
