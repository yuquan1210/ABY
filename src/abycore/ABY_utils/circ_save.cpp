
#include "circ_save.h"
#include <fstream>
#include <map>

std::vector<Gate> circGates;
std::string currHighLevelGateName;
uint32_t currHighLevelGateId;

std::string dir = "/home/ethan/MPC/ABY/circ_graph";

void SaveGate(uint32_t inleft, uint32_t inright, uint32_t ll_gate_id, uint32_t depth){
    Gate gate = {GetCurrHighLevelGateName(), GetCurrHighLevelGateId(), inleft, inright, ll_gate_id, depth};
    circGates.push_back(gate);
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
    std::ofstream outputFile(dir + fname);

    if (outputFile.is_open()) { 
        outputFile << "hl_gate_name,hl_gate_id,input1_id,input2_id,ll_gate_id,depth\n"; 
        for(Gate g : circGates) {
            outputFile << g.hl_gate_nmae << ',' << g.hl_gate_id << ',' << g.inleft << ',' << g.inright << ',' <<g.ll_gate_id << ',' << g.depth << std::endl; 
        }
        outputFile.close(); 
        std::cout << "Data was written to " << dir+fname << std::endl;
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
    std::string input = dir + fname;

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
    std::ofstream outputFile(dir + fname);
    std::map<std::string, std::map<uint32_t, uint32_t>> gateMapHLIdToLayer;
    std::map<std::string, std::string> gateMapHLIdToHLName;
    
    std::vector<uint32_t, uint32_t> layerMap;
    for(Gate g : circGates) {
        //add to corresponding hl_gate_id
        if(gateMapHLIdToLayer.count(g.hl_gate_id) == 0){
            layerMap = gateMap[g.hl_gate_id];
            // layerVec.
        }
        gateMapHLIdToHLName[g.hl_gate_id] = g.hl_gate_nmae;



        //increment its ll_gate_num at specific depth
        outputFile << g.hl_gate_nmae << ',' << g.hl_gate_id << ',' << g.inleft << ',' << g.inright << ',' <<g.ll_gate_id << ',' << g.depth << std::endl; 
    }

    if (outputFile.is_open()) { 
        outputFile << "\n"; 
        for(Gate g : circGates) {
            outputFile << g.hl_gate_nmae << ',' << g.hl_gate_id << ',' << g.inleft << ',' << g.inright << ',' <<g.ll_gate_id << ',' << g.depth << std::endl; 
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