#include <iostream>
#include "TChain.h"

#include "RundataManager.h"

int main() {
    std::vector<TChain*> chain;
    std::vector<int> runset;

    auto man = RundataManager::GetInstance();
    man->GetRunIDsFromCsIID(2000, runset);

    for(int runID: runset) {
        std::cout << runID << "\n";
    }

    return 0;
}