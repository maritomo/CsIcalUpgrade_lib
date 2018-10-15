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
        if(dname == "runfile") isExist = true;
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
        if(dname == "conv_data") isExist = true;
    }

    if(!isExist) {
        std::cout << "[Error] Symbolic link: " << path_runfile
                  << "conv_data (-> ~/conv/source/XXXX/data/) not found\n";
        return false;
    }

    // Initialize
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

                GetRunset(runID, m_PMTconf[csiID][index].runID);
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

                GetRunset(runID, m_MPPCconf[csiID][index].runID);
                m_MPPCconf[csiID][index].crateID = crate;
                m_MPPCconf[csiID][index].modID = mod;
                m_MPPCconf[csiID][index].chID = ch;
                sort(m_MPPCconf[csiID].begin(), m_MPPCconf[csiID].end());
            }
        }

    }

    std::cout << "##### RundataManager initialized #####\n";
    m_isInit = 1;
    return true;
}

void RundataManager::GetRunset(int runID, std::vector<int>& runset) {

    std::string filename = path_convdata + "conv_data/runset.txt";
    std::ifstream ifs(filename.c_str());
    if(!ifs) {
        std::cout << filename << " not found\n";
        return;
    }

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

    std::cout << "[Error] in GetRunset(int runID, std::vector<int> runset):\t";
    std::cout << "runID not found\n";
}

void RundataManager::GetCsIID(int runID, std::vector<int>& csiID){
    csiID.clear();
    for(int id = 0; id<nCSI; ++id) {
        for(auto itr=m_PMTconf[id].begin(); itr!=m_PMTconf[id].end(); ++itr) {
	  if(runID >= *((*itr).runID.begin()) && runID <= *(--((*itr).runID.end()))) {
                csiID.push_back(id);
                break;
            }
        }
    }
}

TChain* RundataManager::GetTree(int runID, const char* treename){
    TChain* chain = new TChain(treename, "");
    std::vector<int> runset;
    GetRunset(runID, runset);
    for(auto itr=runset.begin(); itr!=runset.end(); ++itr) {
        TString filename = path_runfile + Form("runfile/run%d_conv.root", *itr);
        chain->Add(filename);
    }
    return chain;
}

void RundataManager::GetTree(int csiID, const char* treename, std::vector<TChain*>& chain){
    Clear(chain);
    const int nRunset = m_PMTconf[csiID].size();
    for(int set = 0; set<nRunset; ++set) {
        chain.emplace_back();
        chain[set] = new TChain(treename, "");
        for(auto itr=m_PMTconf[csiID][set].runID.begin(); itr!=m_PMTconf[csiID][set].runID.end(); ++itr) {
            TString filename = path_runfile + Form("runfile/run%d_conv.root", *itr);
            chain[set]->Add(filename);
        }
    }
}

void RundataManager::SetBranchAddress(TChain* chain) {
    if(chain->GetName()!="eventTree" && chain->GetName()!="statusTree") {
        std::cout << "[Error] tree should be \"eventTree\" or \"statusTree\"\n";
        return;
    }

    if(chain->GetName()!="eventTree") {
        // cosmic ray data
        chain->SetBranchAddress("cosmi.track", m_cosmi.track);
        chain->SetBranchAddress("cosmi.csiTrack", m_cosmi.csiTrack);

        // CsI data
        chain->SetBranchAddress("csi.isUsed", m_csi.isUsed);
        chain->SetBranchAddress("csi.data", m_csi.data);
        chain->SetBranchAddress("csi.ped", m_csi.ped);
        chain->SetBranchAddress("csi.peak", m_csi.peak);
        chain->SetBranchAddress("csi.integ", m_csi.integ);
        chain->SetBranchAddress("csi.pt", m_csi.pt);
        chain->SetBranchAddress("csi.cft", m_csi.cft);
        chain->SetBranchAddress("csi.eflag", m_csi.eflag);
        chain->SetBranchAddress("csi.TD", m_csi.TD);
        chain->SetBranchAddress("csi.MT", m_csi.MT);
        chain->SetBranchAddress("csi.isHit", m_csi.isHit);
        chain->SetBranchAddress("csi.nHit", &m_csi.nHit);
        chain->SetBranchAddress("csi.hitpos", m_csi.hitpos);

        // Trigger data
        chain->SetBranchAddress("trig.data", m_trig.data);
        chain->SetBranchAddress("trig.ped", m_trig.ped);
        chain->SetBranchAddress("trig.peak", m_trig.peak);
        chain->SetBranchAddress("trig.integ", m_trig.integ);
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
}

void RundataManager::Clear(std::vector<TChain*>& chain){
    for(auto itr=chain.begin(); itr!=chain.end(); ++itr) {
        delete *itr;
    }
    chain.clear();
}
