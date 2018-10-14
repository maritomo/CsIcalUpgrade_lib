#include <iostream>
#include "TChain.h"

#include "RundataManager.h"

int main() {
    std::vector<TChain*> chain;

    RundataManager::GetInstance()->GetEventTree(2714, chain);
    chain[0]->Print();
    RundataManager::GetInstance()->GetEventTree(2715, chain);
    chain[0]->Print();

    return 0;
}