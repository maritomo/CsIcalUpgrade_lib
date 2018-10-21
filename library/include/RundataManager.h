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
    bool operator==( const ADCconfig& right ) const {
        return crateID==right.crateID && modID==right.modID && chID==right.chID;
    }
};

struct CsIData {
    int crystalID[2716];
    int lineID[2716];
    double pos[2716][3];
    double size[2716][3];

    Bool_t isUsed[2716][2];
    Short_t data[2716][2][64];
    Float_t ped[2716][2];
    Float_t peak[2716][2];
    Float_t sumADC[2716][2];
    Float_t pt[2716][2];
    Float_t cft[2716][2];
    Bool_t eflag[2716][2];
    Float_t TD[2716];
    Float_t MT[2716];
    Bool_t isHit[2716];
    Short_t nHit;
    Float_t hitpos[2716][3];
};

struct TriggerData {
    int scintiID[12];
    double pos[12][3];
    double size[12][3];

    Short_t data[12][2][64];
    Float_t ped[12][2];
    Float_t peak[12][2];
    Float_t sumADC[12][2];
    Float_t pt[12][2];
    Float_t cft[12][2];
    Bool_t eflag[12][2];

    Float_t MT[12];
    Float_t TD[12];

    Bool_t isHit[12];
    Float_t hitpos[12][3];
    Short_t nHit[2];
    Short_t hitID[2];

    Short_t trackID;
    Float_t TOF;
    Float_t range;

    Float_t recX[12];

//    Bool_t isHit_online[12][2][64];
//    Short_t nHit_online[2][64];
//    Bool_t isTriggered_online[64];
};

struct CosmicRayData {
    Float_t track[3][2];
    Float_t csiTrack[2];
};

struct StatusData {
    UInt_t timestamp[3];
};


class RundataManager {
  public:
    static const int nCSI = 2716;
    static const int nCSI_S = 2240;
    static const int nCSI_L = 476;
    static const int nTrg = 12;

    static std::string path_runfile;
    static std::string path_convdata;

    static RundataManager* GetInstance();
    RundataManager();
    ~RundataManager();
    bool Init();

    void GetRunIDs(int runID, std::vector<int>& runset);
    void GetRunIDsFromCsIID(int csiID, std::vector<int>& runset);
    void GetCsIIDs(int runID, std::vector<int>& csiID);
    void GetOtherSummedCsIIDs(int runID, int csiID, std::vector<int>& csiIDs);

    TChain* GetTree(int runID, const char* treename);
    void GetTree(int csiID, const char* treename, std::vector<TChain*>& chain);

    void SetBranchAddress(TChain* chain);

    CsIData* GetCsIData() { return &m_csi; }
    TriggerData* GetTriggerData() { return &m_trig; }
    CosmicRayData* GetCosmicData() { return &m_cosmi; }
    StatusData* GetStatusData() { return &m_status; }

    void Clear(std::vector<TChain*>& chain);

  private:
    static RundataManager* m_runMan;
    static bool m_isInit;

    std::vector<ADCconfig> m_PMTconf[nCSI];  // [nCSI][runset]
    std::vector<ADCconfig> m_MPPCconf[nCSI]; // [nCSI][runset]

    CsIData m_csi;
    TriggerData m_trig;
    CosmicRayData m_cosmi;
    StatusData m_status;
};


#endif //ANA_RUNDATAMANAGER_H
