#include <iostream>
#include "TChain.h"

#include "RundataManager.h"

int main() {
    int csiID = 2260;
    std::vector<TChain*> chain;

    RundataManager::GetInstance()->GetEventTree(csiID, chain);
    chain[0]->Print();

    return 0;
}