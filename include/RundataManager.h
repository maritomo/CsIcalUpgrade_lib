//
// Created by Tomoo MARI on 2018/10/13.
//

#ifndef ANA_RUNDATAMANAGER_H
#define ANA_RUNDATAMANAGER_H

#include <vector>

#include "TChain.h"

struct ADCconfig {
    std::vector<int> runID;
    int crateID;
    int modID;
    int chID;
    bool operator<( const ADCconfig& right ) const {
        return runID < right.runID;
    }
};

class RundataManager {
  public:
    static const int nCSI = 2716;
    static const int nCSI_S = 2240;
    static const int nCSI_L = 476;

    static std::string path_runfile;
    static std::string path_convdata;

    static RundataManager* GetInstance();
    RundataManager();
    ~RundataManager();

    bool Init();

    void GetRunset(int runID, std::vector<int>& runset);
    void GetCsIID(int runID, std::vector<int>& csiID);

    TChain* GetTree(int runID, const char* treename);
    void GetTree(int csiID, const char* treename, std::vector<TChain*>& chain);
    
    void Clear(std::vector<TChain*>& chain);

  private:
    static RundataManager* m_runMan;
    static bool m_isInit;
    std::vector<ADCconfig> m_MPPCconf[nCSI];
    std::vector<ADCconfig> m_PMTconf[nCSI];
};


#endif //ANA_RUNDATAMANAGER_H
