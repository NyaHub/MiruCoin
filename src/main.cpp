#include "./blockchain.h"
#include "./ledger.h"
#include "./utils.h"
#include "tx.h"
#include <string>
#include <vector>

int main(int, char **) {
  Mirucoin::Blockchain chain{(std::string) "chain.sqlite"};

  Mirucoin::Tx tx = Mirucoin::Tx("me", "alice", 12, 0.1, "");
  std::vector<Mirucoin::Tx> txs;
  txs.push_back(std::move(tx));

  Mirucoin::Block blk{chain.getlastHash(), chain.getLastIndex(),
                      std::move(txs)};

  chain.addBlock(blk);

  chain.print();

  // Mirucoin::Ledger wallet;

  // std::string message = "Some message";

  // printf("PrivateKey: %s\nPublicKey: %s\n", wallet.getDerPrivate().c_str(),
  //        wallet.getDerPublic().c_str());

  // std::string sing = wallet.sign(message);

  // std::cout << sing << std::endl;
  // std::cout << wallet.verify(message, sing, wallet.getDerPublic()) <<
  // std::endl;

  return 0;
}