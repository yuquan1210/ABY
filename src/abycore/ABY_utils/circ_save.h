#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

struct DetailGate{
    std::string hl_gate_nmae;
    uint32_t hl_gate_id;
    uint32_t inleft; 
    uint32_t inright; 
    uint32_t ll_gate_id; 
    uint32_t depth;
    uint32_t sharing_size;
};

extern std::vector<DetailGate> circGates;
extern uint32_t maxDepth;
extern std::string currHighLevelGateName;
extern uint32_t currHighLevelGateId;
extern std::string dir;

void SaveGate(uint32_t inleft, uint32_t inright, uint32_t ll_gate_id, uint32_t depth, uint32_t sharing_size);
void SaveMaxDepth(uint32_t md);
void UpdateCurrHighLevelGate(std::string hl_gate_nmae, uint32_t hl_gate_id);
std::string GetCurrHighLevelGateName();
uint32_t GetCurrHighLevelGateId();
void clearCircGates();
void WriteCircForGraph(std::string fname);
void GenerateCircGraph(std::string fname);
void GenerateCircSpreadSheet(std::string fname);