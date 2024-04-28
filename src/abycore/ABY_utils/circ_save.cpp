
#include "circ_save.h"
#include <fstream>
#include <map>

std::vector<DetailGate> circGates;
uint32_t maxDepth;
std::string currHighLevelGateName;
uint32_t currHighLevelGateId;

std::string dir = "/home/ethan/MPC/ABY/circ_graph";

void SaveGate(uint32_t inleft, uint32_t inright, uint32_t ll_gate_id, uint32_t depth, uint32_t sharing_size){
    DetailGate gate = {GetCurrHighLevelGateName(), GetCurrHighLevelGateId(), inleft, inright, ll_gate_id, depth, sharing_size};
    circGates.push_back(gate);
}

void SaveMaxDepth(uint32_t md){
    maxDepth = md;
}

void UpdateCurrHighLevelGate(std::string hl_gate_nmae, uint32_t hl_gate_id){
    currHighLevelGateName = hl_gate_nmae;
    currHighLevelGateId = hl_gate_id;
}

std::string GetCurrHighLevelGateName(){
    return currHighLevelGateName;
}

uint32_t GetCurrHighLevelGateId(){
    return currHighLevelGateId;
}

void WriteCircForGraph(std::string fname){
    std::ofstream outputFile(dir + "/" + fname);

    if (outputFile.is_open()) { 
        outputFile << "hl_gate_name,hl_gate_id,input1_id,input2_id,ll_gate_id,depth,sharingsize\n"; 
        for(DetailGate g : circGates) {
            outputFile << g.hl_gate_nmae << ',' << g.hl_gate_id << ',' << g.inleft << ',' << g.inright << ',' <<g.ll_gate_id << ',' << g.depth << ',' << g.sharing_size << std::endl; 
        }
        outputFile.close(); 
        std::cout << "Data was written to " << dir + "/" + fname << std::endl;
    }
    else {
        std::cerr << "Error opening file\n";
    }
}

void GenerateCircGraph(std::string fname){
    WriteCircForGraph(fname);

    std::cout << "C++ program is about to call a Python script..." << std::endl;

    // Define the path to the Python script and the input for the script
    std::string pythonCommand = "python3";
    std::string scriptPath = "/home/ethan/MPC/ABY/circ_graph/circ_graph.py";
    std::string input = dir + "/" + fname;

    // Construct the command to run the Python script with an argument
    std::string command = pythonCommand + " " + scriptPath + " " + input;

    // Execute the Python script
    int result = system(command.c_str());  // Convert std::string to C-style string

    if (result != 0) {
        std::cerr << "Failed to execute Python script!" << std::endl;
    }

    std::cout << "Python script executed successfully." << std::endl;
}

void GenerateCircSpreadSheet(std::string fname){
    std::ofstream outputFile(dir + "/" + fname);
    std::map<uint32_t, std::map<uint32_t, uint32_t>> gateMapHLIdToLayerMap;
    std::map<uint32_t, std::string> gateMapHLIdToHLName;
    
    std::map<uint32_t, uint32_t> layerMap;
    for(DetailGate g : circGates) {
        gateMapHLIdToHLName[g.hl_gate_id] = g.hl_gate_nmae;
        gateMapHLIdToLayerMap[g.hl_gate_id][g.depth] += g.sharing_size;
    }

    if (outputFile.is_open()) { 
        outputFile << "Layer,";
        for (auto it = gateMapHLIdToHLName.begin(); it != gateMapHLIdToHLName.end(); ++it) {
            outputFile << it->second;
            auto nextIt = it;
            ++nextIt;
            if (nextIt != gateMapHLIdToHLName.end()) {
                outputFile << ",";
            }
        }
        outputFile << "\n"; 
        for (uint32_t layer = 0; layer < maxDepth; layer++){
            outputFile << "L" << layer << ",";
            for (auto it = gateMapHLIdToLayerMap.begin(); it != gateMapHLIdToLayerMap.end(); ++it) {
                layerMap = it->second;
                if(layerMap.find(layer) != layerMap.end()){
                    outputFile << layerMap[layer];
                } else {
                    outputFile << "0";
                }
                auto nextIt = it;
                ++nextIt;
                if (nextIt != gateMapHLIdToLayerMap.end()) {
                    outputFile << ",";
                }
            }
            outputFile << "\n"; 
        }
        outputFile.close(); 
        std::cout << "Data was written to " << dir+fname << std::endl;
    }
    else {
        std::cerr << "Error opening file\n";
    }
}

void clearCircGates(){
    circGates.clear();
    currHighLevelGateName = "";
    currHighLevelGateId = 0;
}