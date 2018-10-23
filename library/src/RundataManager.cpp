//
// Created by Tomoo MARI on 2018/10/13.
//

#include <fstream>
#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <sstream>

#include "RundataManager.h"

RundataManager* RundataManager::m_runMan = nullptr;
bool RundataManager::m_isInit = 0;
std::string RundataManager::path_runfile = "./"; // symbolic name must be "runfile"
std::string RundataManager::path_convdata = "./"; // symbolic name must be "conv_data"

RundataManager* RundataManager::GetInstance() {
    if(!m_isInit) {
        m_runMan = new RundataManager();
    }
    return m_runMan;
}

RundataManager::RundataManager() {
    if(!Init()) {
        std::cout << "[Error] RundataManager initialization failed\n";
    }
}

RundataManager::~RundataManager() {}

bool RundataManager::Init() {
    // Check if both RundataManager::path_runfile and RundataManager::path_convdir exist
    DIR* dir = opendir(path_runfile.c_str());
    bool isExist = false;

    while(1) {
        dirent* entry = readdir(dir);
        if(entry==NULL) break;
        std::string dname = entry->d_name;
        if(dname=="runfile") isExist = true;
    }

    if(!isExist) {
        std::cout << "[Error] Symbolic link: " << path_runfile
                  << "runfile (-> ~/conv/runfile/) not found\n";
        return false;
    }

    dir = opendir(path_runfile.c_str());
    isExist = false;

    while(1) {
        dirent* entry = readdir(dir);
        if(entry==NULL) break;
        std::string dname = entry->d_name;
        if(dname=="conv_data") isExist = true;
    }

    if(!isExist) {
        std::cout << "[Error] Symbolic link: " << path_runfile
                  << "conv_data (-> ~/conv/source/XXXX/data/) not found\n";
        return false;
    }

    // Initialization
    std::string path = path_convdata + "conv_data/ADCchMap/";
    dir = opendir(path.c_str());
    if(dir==NULL) {
        std::cout << "[Error] " << path << " not found\n";
        return false;
    }

    while(1) {
        dirent* entry = readdir(dir);
        if(entry==NULL) break;

        std::string filename = entry->d_name;
        int len = filename.length();
        if(len<8) continue;

        // if PMT configuration file
        if(filename.substr(len - 7, 3)=="pmt") {
            int runID = stoi(filename.substr(3, 4));
            int csiID, crate, mod, ch;

            filename.insert(0, path);
            std::ifstream ifs(filename.c_str());

            while(ifs >> csiID >> crate >> mod >> ch) {
                m_PMTconf[csiID].emplace_back();
                int index = m_PMTconf[csiID].size() - 1;

                GetRunIDs(runID, m_PMTconf[csiID][index].runID);
                m_PMTconf[csiID][index].crateID = crate;
                m_PMTconf[csiID][index].modID = mod;
                m_PMTconf[csiID][index].chID = ch;
                sort(m_PMTconf[csiID].begin(), m_PMTconf[csiID].end());
            }
        }
        // if MPPC configuration file
        if(filename.substr(len - 8, 4)=="mppc") {
            int runID = stoi(filename.substr(3, 4));
            int csiID, crate, mod, ch;

            filename.insert(0, path);
            std::ifstream ifs(filename.c_str());

            while(ifs >> csiID >> crate >> mod >> ch) {
                m_MPPCconf[csiID].emplace_back();
                int index = m_MPPCconf[csiID].size() - 1;

                GetRunIDs(runID, m_MPPCconf[csiID][index].runID);
                m_MPPCconf[csiID][index].crateID = crate;
                m_MPPCconf[csiID][index].modID = mod;
                m_MPPCconf[csiID][index].chID = ch;
                sort(m_MPPCconf[csiID].begin(), m_MPPCconf[csiID].end());
            }
        }

    }

    // Location maps
    // CsI
    std::string filename = path_convdata + "conv_data/map_csi.txt";
    std::ifstream ifs(filename.c_str());
    if(!ifs) {
        std::cout << filename << " not found\n";
        return false;
    }

    int csiID, lineID, crysID;
    double posx, posy, size;
    while(ifs >> lineID >> csiID >> crysID >> posx >> posy >> size) {
        m_csi.crystalID[csiID] = crysID;
        m_csi.lineID[csiID] = lineID;
        m_csi.pos[csiID][0] = posx;
        m_csi.pos[csiID][1] = posy;
        m_csi.pos[csiID][2] = 0;
        m_csi.size[csiID][0] = size;
        m_csi.size[csiID][1] = size;
        m_csi.size[csiID][2] = 500;
    }
    ifs.close();
    ifs.clear();

    // Trigger counter
    filename = path_convdata + "conv_data/map_crc.txt";
    ifs.open(filename.c_str());
    if(!ifs) {
        std::cout << filename << " not found\n";
        return false;
    }

    int scintiID, direction;
    double pos[3];
    for(int id = 0; id<nTrg; ++id) {
        ifs >> scintiID >> direction >> pos[0] >> pos[1] >> pos[2];
        m_trig.scintiID[id] = scintiID;
        m_trig.pos[id][0] = pos[0];
        m_trig.pos[id][1] = pos[1];
        m_trig.pos[id][2] = pos[2];
        m_trig.size[id][0] = 2000;
        m_trig.size[id][1] = 100;
        m_trig.size[id][2] = 50;
    }
    ifs.close();


    std::cout << "######################################\n";
    std::cout << "##### RundataManager initialized #####\n";
    std::cout << "######################################\n";

    m_isInit = 1;
    return true;
}

void RundataManager::GetRunIDs(int runID, std::vector<int>& runset) {

    std::string filename = path_convdata + "conv_data/runset.txt";
    std::ifstream ifs(filename.c_str());
    if(!ifs) {
        std::cout << filename << " not found\n";
        return;
    }

    // read line-by-line
    // If current runID found, sort and return
    std::string line;
    while(std::getline(ifs, line)) {
        std::stringstream ss {line};

        bool endflag = false;
        std::string buf;
        while(std::getline(ss, buf, ' ')) {
            runset.push_back(stoi(buf));
            if(runID==stoi(buf)) {
                endflag = true;
            }
        }
        if(endflag) {
            sort(runset.begin(), runset.end());
            return;
        }

        runset.clear();
    }

    std::cout << "[Error] in GetRunIDs(int runID, std::vector<int> runset):\t";
    std::cout << "runID not found\n";
}

void RundataManager::GetRunIDsFromCsIID(int csiID, std::vector<int>& runset) {
    runset.clear();
    for(int set = 0; set<m_PMTconf[csiID].size(); ++set) {
        for(auto itr = m_PMTconf[csiID][set].runID.begin(); itr!=m_PMTconf[csiID][set].runID.end(); ++itr) {
            runset.push_back(*itr);
        }
    }
}

void RundataManager::GetCsIIDs(int runID, std::vector<int>& csiIDs) {
    csiIDs.clear();
    for(int id = 0; id<nCSI; ++id) {
        for(auto itr = m_PMTconf[id].begin(); itr!=m_PMTconf[id].end(); ++itr) {
            if(runID>=*((*itr).runID.begin()) && runID<=*(--((*itr).runID.end()))) {
                csiIDs.push_back(id);
                break;
            }
        }
    }
}

void RundataManager::GetSummedCsIIDs(int runID, int csiID, std::vector<int>& sumCsIIDs) {
    std::vector<int> runIDs, csiIDs;
    GetRunIDs(runID, runIDs);
    GetCsIIDs(runID, csiIDs);

    // if csiID not found
    if(!IsIncluded(csiID, csiIDs)) {
        std::cout << "[Error] CsI " << csiID << " was not found in run" << runID << "\n";
        return;
    }

    // Search ADC configuration of this MPPC
    ADCconfig thisMPPC;
    for(auto mppc: m_MPPCconf[csiID]) {
        if(IsIncluded(runID, mppc.runID)) {
            thisMPPC = mppc;
            break;
        }
    }

    // Search summed MPPCs
    sumCsIIDs.clear();
    for(auto id: csiIDs) {
        for(auto mppc: m_MPPCconf[id]) {
            // if this run
            if(IsIncluded(runID, mppc.runID)) {
                if(mppc==thisMPPC) {
                    sumCsIIDs.push_back(id);
                }
            }
        }
    }

}

TChain* RundataManager::GetTree(int runID, const char* treename) {
    TChain* chain = new TChain(treename, "");
    std::vector<int> runset;
    GetRunIDs(runID, runset);
    for(auto itr = runset.begin(); itr!=runset.end(); ++itr) {
        TString filename = path_runfile + Form("runfile/run%d_conv.root", *itr);
        chain->Add(filename);
    }
    SetBranchAddress(chain);
    return chain;
}

void RundataManager::GetTree(int csiID, const char* treename, std::vector<TChain*>& chain) {
    Clear(chain);
    for(int set = 0; set<m_PMTconf[csiID].size(); ++set) {
        chain.emplace_back();
        chain[set] = new TChain(treename, "");
        for(auto itr = m_PMTconf[csiID][set].runID.begin(); itr!=m_PMTconf[csiID][set].runID.end(); ++itr) {
            TString filename = path_runfile + Form("runfile/run%d_conv.root", *itr);
            chain[set]->Add(filename);
        }
    }
}

void RundataManager::SetBranchAddress(TChain* chain) {
    std::string treename = chain->GetName();
    if(treename!="eventTree" && treename!="statusTree") {
        std::cout << "[Error] tree name must be \"eventTree\" or \"statusTree\"\n";
        return;
    }

    if(treename=="eventTree") {
        // cosmic ray data
        chain->SetBranchAddress("cosmi.track", m_cosmi.track);

        // CsI data
        chain->SetBranchAddress("csi.isUsed", m_csi.isUsed);
        chain->SetBranchAddress("csi.data", m_csi.data);
        chain->SetBranchAddress("csi.ped", m_csi.ped);
        chain->SetBranchAddress("csi.peak", m_csi.peak);
        chain->SetBranchAddress("csi.sumADC", m_csi.sumADC);
        chain->SetBranchAddress("csi.pt", m_csi.pt);
        chain->SetBranchAddress("csi.cft", m_csi.cft);
        chain->SetBranchAddress("csi.eflag", m_csi.eflag);
        chain->SetBranchAddress("csi.TD", m_csi.TD);
        chain->SetBranchAddress("csi.MT", m_csi.MT);
        chain->SetBranchAddress("csi.isHit", m_csi.isHit);
        chain->SetBranchAddress("csi.nHit", &m_csi.nHit);
        chain->SetBranchAddress("csi.hitpos", m_csi.hitpos);
        chain->SetBranchAddress("csi.Edep", m_csi.Edep);

        // Trigger data
        chain->SetBranchAddress("trig.data", m_trig.data);
        chain->SetBranchAddress("trig.ped", m_trig.ped);
        chain->SetBranchAddress("trig.peak", m_trig.peak);
        chain->SetBranchAddress("trig.sumADC", m_trig.sumADC);
        chain->SetBranchAddress("trig.pt", m_trig.pt);
        chain->SetBranchAddress("trig.cft", m_trig.cft);
        chain->SetBranchAddress("trig.eflag", m_trig.eflag);
        chain->SetBranchAddress("trig.TD", m_trig.TD);
        chain->SetBranchAddress("trig.MT", m_trig.MT);
        chain->SetBranchAddress("trig.isHit", m_trig.isHit);
        chain->SetBranchAddress("trig.nHit", &m_trig.nHit);
        chain->SetBranchAddress("trig.hitpos", m_trig.hitpos);
        chain->SetBranchAddress("trig.hitID", m_trig.hitID);
        chain->SetBranchAddress("trig.trackID", &m_trig.trackID);
        chain->SetBranchAddress("trig.hitpos", &m_trig.TOF);
        chain->SetBranchAddress("trig.TOF", &m_trig.range);
        chain->SetBranchAddress("trig.range", m_trig.recX);
    }

    if(treename=="statusTree") {
        chain->SetBranchAddress("timestamp", m_status.timestamp);
    }
}

void RundataManager::Clear(std::vector<TChain*>& chain) {
    for(auto itr = chain.begin(); itr!=chain.end(); ++itr) {
        delete *itr;
    }
    chain.clear();
}

bool RundataManager::IsIncluded(int i, std::vector<int>& v) {
    for(auto v_i: v) {
        if(i==v_i) return true;
    }
    return false;
}

TCut RundataManager::SingleHitCut(int runID, int csiID) {
    std::vector<int> sumCsIIDs;
    GetSummedCsIIDs(runID, csiID, sumCsIIDs);
    TCut cut;
    for(auto id: sumCsIIDs) {
        static int count = 0;
        if(count>0) cut += "&&";
        if(id == csiID) {
            cut += Form("csi.isHit[%d]", id);
        } else {
            cut += Form("!csi.isHit[%d]", id);
        }
    }
    return cut;
}