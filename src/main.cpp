#include "./blockchain.h"
#include "./ledger.h"
#include "./utils.h"
#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>

int main(int, char **) {
  // Mirucoin::Blockchain chain{"chain.sqlite"};

  // Mirucoin::Block blk{chain.getlastHash(), chain.getLastIndex(), "first
  // block"};

  // chain.addBlock(&blk);

  // chain.print();

  Mirucoin::Ledger wallet;

  std::string message = "Some message";

  printf("PrivateKey: %s\nPublicKey: %s\n", wallet.getDerPrivate().c_str(),
         wallet.getDerPublic().c_str());

  std::string sing = wallet.sign(message);

  std::cout << sing << std::endl;
  std::cout << wallet.verify(message, sing, wallet.getDerPublic()) << std::endl;

  return 0;
}