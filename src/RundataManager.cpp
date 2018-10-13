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
std::string RundataManager::path_runfile = "./"; // symbolic name must be "rootfile"
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
        if(dname == "rootfile") isExist = true;
    }

    if(!isExist) {
        std::cout << "[Error] Symbolic link: " << path_runfile
                  << "rootfile (-> ~/conv/rootfile/) not found\n";
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

void RundataManager::GetUsedCsIID(int runID, std::vector<int>& csiID){
    csiID.clear();
    for(int id = 0; id<nCSI; ++id) {
        for(auto itr=m_PMTconf[id].begin(); itr!=m_PMTconf[id].end(); ++itr) {
            if(runID >= (*itr).runID[0] && runID <= (*itr).runID[1]) {
                csiID.push_back(id);
                break;
            }
        }
    }
}

void RundataManager::GetEventTree(int csiID, std::vector<TChain*>& chain){
    chain.clear();
    const int nRunset = m_PMTconf[csiID].size();
    for(int set = 0; set<nRunset; ++set) {
        chain.emplace_back();
        chain[set] = new TChain("eventTree");
        for(auto itr=m_PMTconf[csiID][set].runID.begin(); itr!=m_PMTconf[csiID][set].runID.end(); ++itr) {
            TString filename = path_runfile + Form("rootfile/run%d_conv.root", *itr);
            chain[set]->Add(filename);
        }
    }
}
