#include <abycore/circuit/arithmeticcircuits.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>
#include <abycore/circuit/abycircuit.h>

#ifndef BoolCircuit
class BoolCircuit
{
    //nested class
    class BoolCircLayer{
        public: 
            uint32_t layerid;
            uint32_t nbIn, nbOut, nbAnd, nbVecAnd, nbTT, nbCB; //inter
            uint32_t nbXor, nbCons, nbInv, nbConv, nbSOut, nbSIn, nbLocalCB, nbPrintVal, nbAssert, nbSIMD; //local
       
       //constructor
        BoolCircLayer(uint32_t layerid){
            this->layerid = layerid;
            this->nbIn = 0;
            this->nbOut = 0;
            this->nbAnd = 0;
            this->nbVecAnd = 0;
            this->nbTT = 0;
            this->nbCB = 0;
            this->nbXor = 0;
            this->nbCons = 0;
            this->nbInv = 0;
            this->nbConv = 0;
            this->nbSOut = 0;
            this->nbSIn = 0;
            this->nbLocalCB = 0;
            this->nbPrintVal = 0;
            this->nbAssert = 0;
            this->nbSIMD = 0;
       }

        void printBoolCircLayerLocalOps();
        void printBoolCircLayerInterOps();
    };

    //declare Boolcircuit member
    public:
        Circuit* circ;
        uint32_t maxdepth;
        uint32_t bitlen;
        std::vector<BoolCircuit::BoolCircLayer*> boolCircLayers;
        uint32_t tt_bIn, tt_bOut, tt_bAnd, tt_bVecAnd, tt_bTT, tt_bCB; //inter
        uint32_t tt_bXor, tt_bCons, tt_bInv, tt_bConv, tt_bSOut, tt_bSIn, tt_bLocalCB, tt_bPrintVal, tt_bAssert, tt_bSIMD; //local

    BoolCircuit(Circuit* bc){
        this->circ = bc;
        this->maxdepth = bc->GetMaxDepth();
        this->bitlen = bc->GetShareBitLen();

        std::vector<BoolCircuit::BoolCircLayer*> boolCircLayers(this->maxdepth);
        this->boolCircLayers = boolCircLayers;

        this->tt_bIn = 0;
        this->tt_bOut = 0;
        this->tt_bAnd = 0;
        this->tt_bVecAnd = 0;
        this->tt_bTT = 0;
        this->tt_bCB = 0;
        this->tt_bXor = 0;
        this->tt_bCons = 0;
        this->tt_bInv = 0;
        this->tt_bConv = 0;
        this->tt_bSOut = 0;
        this->tt_bSIn = 0;
        this->tt_bLocalCB = 0;
        this->tt_bPrintVal = 0;
        this->tt_bAssert = 0;
        this->tt_bSIMD = 0;

        std::cout << "\n---Bool Circuit Structure:" << std::endl;
        for (uint32_t i = 0; i < this->maxdepth; i++) {
            BoolCircuit::BoolCircLayer* currBoolLayer = new BoolCircLayer(i);
            std::deque<uint32_t> localops = this->circ->GetLocalQueueOnLvl(i);
            std::deque<uint32_t> interops = this->circ->GetInteractiveQueueOnLvl(i);
            std::cout << "Layer " << i << ": " << std::endl;
            for (uint32_t j = 0; j < localops.size(); j++) {
                e_gatetype gatetype = this->circ->GetGateType(localops[j]);
                switch (gatetype) {
                    case G_LIN:
                        currBoolLayer->nbXor++;
                        break;
                    case G_NON_LIN_CONST:
                        break;
                    case G_CONSTANT:
                        currBoolLayer->nbCons++;
                        break;
                    case G_INV:
                        currBoolLayer->nbInv++;
                        break;
                    case G_CONV:
                        currBoolLayer->nbConv++;
                        break;
                    case G_SHARED_OUT:
                        currBoolLayer->nbSOut++;
                        break;
                    case G_SHARED_IN:
                        currBoolLayer->nbSIn++;
                        break;
                    case G_CALLBACK:
                        currBoolLayer->nbLocalCB++;
                        break;
                    // case G_PRINT_VAL:
                    //     layer->nPrintVal++;
                    //     break;
                    // case G_ASSERT:
                    //     layer->nAssert++;
                    //     break;
                    default:
                        if (IsSIMDGate(gatetype)) {
                            currBoolLayer->nbSIMD++;
                        } else {
                            std::cout << "operation not recognized" << std::endl;
                        }
                        break;
                }
            }

            for (uint32_t j = 0; j < interops.size(); j++) {
                e_gatetype gatetype = circ->GetGateType(interops[j]);
                switch (gatetype) {
                    case G_IN:
                        currBoolLayer->nbIn++;
                        break;
                    case G_OUT:
                        currBoolLayer->nbOut++;
                        break;
                    case G_NON_LIN:
                        currBoolLayer->nbAnd++;
                        break;
                    case G_CONV:
                        break;
                    case G_NON_LIN_VEC:
                        currBoolLayer->nbVecAnd++;
                        break;
                    case G_TT:
                        currBoolLayer->nbTT++;
                        break;
                    case G_CALLBACK:
                        currBoolLayer->nbCB++;
                        break;
                    default:
                        std::cout << "operation not recognized" << std::endl;
                        break;
                }
            }

            this->boolCircLayers[i] = currBoolLayer;
            //print current layer gate details
            // this->boolCircLayers[i]->printBoolCircLayerLocalOps();
            this->boolCircLayers[i]->printBoolCircLayerInterOps();

            this->tt_bIn += currBoolLayer->nbIn;
            this->tt_bOut += currBoolLayer->nbOut;
            this->tt_bXor += currBoolLayer->nbXor;
            this->tt_bAnd += currBoolLayer->nbAnd;
            this->tt_bVecAnd += currBoolLayer->nbVecAnd;

            this->tt_bTT += currBoolLayer->nbTT;
            this->tt_bCB += currBoolLayer->nbCB;
            this->tt_bCons += currBoolLayer->nbCons;
            this->tt_bInv += currBoolLayer->nbInv;
            this->tt_bConv += currBoolLayer->nbConv;
            this->tt_bSOut += currBoolLayer->nbSOut;
            this->tt_bSIn += currBoolLayer->nbSIn;
            this->tt_bLocalCB += currBoolLayer->nbLocalCB;
            this->tt_bPrintVal += currBoolLayer->nbPrintVal;
            this->tt_bAssert += currBoolLayer->nbAssert;
            this->tt_bSIMD += currBoolLayer->nbSIMD;
        }
        this->printBoolCircuitSummary();
        std::cout << "---Bool Circuit Structure Done" << std::endl;
    }

    void printBoolCircuitSummary();
    std::tuple<uint32_t, uint32_t, uint32_t> calculateBoolCircuitCost();
};
#endif

#ifndef ArithCircuit
class ArithCircuit
{
    //nested class
    class ArithCircLayer{
        public: 
            uint32_t layerid;
            uint32_t naIn, naOut, naMul, naConv, naCB; //inter 
            uint32_t naAdd, naCons, naMulCons, naInv, naSOut, naSIn, naLocalCB, naPrintVal, naAssert, naSIMD; //local
       
       //constructor
        ArithCircLayer(uint32_t layerid){
            this->layerid = layerid;
            this->naIn = 0;
            this->naOut = 0;
            this->naMul = 0;
            this->naConv = 0;
            this->naCB = 0;
            this->naAdd = 0;
            this->naCons = 0;
            this->naMulCons = 0;
            this->naInv = 0;
            this->naSOut = 0;
            this->naSIn = 0;
            this->naLocalCB = 0;
            this->naPrintVal = 0;
            this->naAssert = 0;
            this->naSIMD = 0;
       }

        void printArithCircLayerLocalOps();
        void printArithCircLayerInterOps();
    };

    //declare Arithcircuit member
    public:
        Circuit* circ;
        uint32_t maxdepth;
        uint32_t bitlen;
        std::vector<ArithCircuit::ArithCircLayer*> arithCircLayers;
        uint32_t tt_aIn, tt_aOut, tt_aMul, tt_aConv, tt_aCB; //inter 
        uint32_t tt_aAdd, tt_aCons, tt_aMulCons, tt_aInv, tt_aSOut, tt_aSIn, tt_aLocalCB, tt_aPrintVal, tt_aAssert, tt_aSIMD; //local

    ArithCircuit(Circuit* ac){
        this->circ = ac;
        this->maxdepth = ac->GetMaxDepth();
        this->bitlen = ac->GetShareBitLen();

        std::vector<ArithCircuit::ArithCircLayer*> arithCircLayers(this->maxdepth);
        this->arithCircLayers = arithCircLayers;

        this->tt_aIn = 0;
        this->tt_aOut = 0;
        this->tt_aMul = 0;
        this->tt_aConv = 0;
        this->tt_aCB = 0;
        this->tt_aAdd = 0;
        this->tt_aCons = 0;
        this->tt_aMulCons = 0;
        this->tt_aInv = 0;
        this->tt_aSOut = 0;
        this->tt_aSIn = 0;
        this->tt_aLocalCB = 0;
        this->tt_aPrintVal = 0;
        this->tt_aAssert = 0;
        this->tt_aSIMD = 0;

        std::cout << "\n---Arith Circuit Structure:" << std::endl;
        for (uint32_t i = 0; i < this->maxdepth; i++) {
            ArithCircuit::ArithCircLayer* currArithLayer = new ArithCircLayer(i);
            std::deque<uint32_t> localops = this->circ->GetLocalQueueOnLvl(i);
            std::deque<uint32_t> interops = this->circ->GetInteractiveQueueOnLvl(i);
            std::cout << "Layer " << i << ": " << std::endl;
            for (uint32_t j = 0; j < localops.size(); j++) {
                e_gatetype gatetype = this->circ->GetGateType(localops[j]);
                switch (gatetype) {
                    case G_LIN:
                        currArithLayer->naAdd++;
                        break;
                    case G_INV:
                        currArithLayer->naInv++;
                        break;
                    case G_NON_LIN_CONST:
                        currArithLayer->naMulCons++;
                        break;
                    case G_CONSTANT:
                        currArithLayer->naCons++;
                        break;
                    case G_CALLBACK:
                        currArithLayer->naLocalCB++;
                        break;
                    case G_SHARED_IN:
                        currArithLayer->naSIn++;
                        break;
                    case G_SHARED_OUT:
                        currArithLayer->naSOut++;
                        break;
                    // case G_PRINT_VAL:
                    //     layer->nPrintVal++;
                    //     break;
                    // case G_ASSERT:
                    //     layer->nAssert++;
                    //     break;
                    default:
                        if (IsSIMDGate(gatetype)) {
                            currArithLayer->naSIMD++;
                        } else {
                            std::cout << "operation not recognized" << std::endl;
                        }
                        break;
                }
            }

            for (uint32_t j = 0; j < interops.size(); j++) {
                e_gatetype gatetype = circ->GetGateType(interops[j]);
                switch (gatetype) {
                    case G_IN:
                        currArithLayer->naIn++;
                        break;
                    case G_OUT:
                        currArithLayer->naOut++;
                        break;
                    case G_NON_LIN:
                        currArithLayer->naMul++;
                        break;
                    case G_CONV:
                        currArithLayer->naConv++;
                        break;
                    case G_CALLBACK:
                        currArithLayer->naCB++;
                        break;
                    default:
                        std::cout << "operation not recognized" << std::endl;
                        break;
                }
            }

            this->arithCircLayers[i] = currArithLayer;
            //print current layer gate details
            // this->arithCircLayers[i]->printArithCircLayerLocalOps();
            this->arithCircLayers[i]->printArithCircLayerInterOps();

            this->tt_aIn += currArithLayer->naIn;
            this->tt_aOut += currArithLayer->naOut;
            this->tt_aMul += currArithLayer->naMul;
            this->tt_aConv += currArithLayer->naConv;
            this->tt_aCB += currArithLayer->naCB;

            this->tt_aAdd += currArithLayer->naAdd;
            this->tt_aCons += currArithLayer->naCons;
            this->tt_aMulCons += currArithLayer->naMulCons;
            this->tt_aInv += currArithLayer->naInv;
            this->tt_aSOut += currArithLayer->naSOut;
            this->tt_aSIn += currArithLayer->naSIn;
            this->tt_aLocalCB += currArithLayer->naLocalCB;
            this->tt_aPrintVal += currArithLayer->naPrintVal;
            this->tt_aAssert += currArithLayer->naAssert;
            this->tt_aSIMD += currArithLayer->naSIMD;
        }
        this->printArithCircuitSummary();
        std::cout << "---Arith Circuit Structure Done" << std::endl;
    }

    void printArithCircuitSummary();
    std::tuple<uint32_t, uint32_t, uint32_t> calculateArithCircuitCost();
};
#endif

#ifndef YaoCircuit
class YaoCircuit
{
    //nested class
    class YaoCircLayer{
        public: 
            uint32_t layerid;
            uint32_t nyIn, nyOut, nyConv, nyCB; //inter
            uint32_t nyXor, nyAnd, nyCons, nySIMD, nyInv, nySOut, nySIn, nyLocalCB, nyPrintVal, nyAssert, nyUNIV; //local
       
       //constructor
        YaoCircLayer(uint32_t layerid){
            this->layerid = layerid;
            this->nyIn = 0;
            this->nyOut = 0;
            this->nyConv = 0;
            this->nyCB = 0;
            this->nyXor = 0;
            this->nyAnd = 0;
            this->nyCons = 0;
            this->nySIMD = 0;
            this->nyInv = 0;
            this->nySOut = 0;
            this->nySIn = 0;
            this->nyLocalCB = 0;
            this->nyPrintVal = 0;
            this->nyAssert = 0;
            this->nyUNIV = 0;
       }

        void printYaoCircLayerLocalOps();
        void printYaoCircLayerInterOps();
    };

    //declare Yaocircuit member
    public:
        Circuit* circ;
        uint32_t maxdepth;
        uint32_t bitlen;
        std::vector<YaoCircuit::YaoCircLayer*> yaoCircLayers;
        uint32_t tt_yIn, tt_yOut, tt_yConv, tt_yCB; //inter
        uint32_t tt_yXor, tt_yAnd, tt_yCons, tt_ySIMD, tt_yInv, tt_ySOut, tt_ySIn, tt_yLocalCB, tt_yPrintVal, tt_yAssert, tt_yUNIV; //local

    YaoCircuit(Circuit* yc){
        this->circ = yc;

        this->maxdepth = yc->GetMaxDepth();
        this->bitlen = yc->GetShareBitLen();

        std::vector<YaoCircuit::YaoCircLayer*> yaoCircLayers(this->maxdepth);
        this->yaoCircLayers = yaoCircLayers;

        this->tt_yIn = 0;
        this->tt_yOut = 0;
        this->tt_yConv = 0;
        this->tt_yCB = 0;
        this->tt_yXor = 0;
        this->tt_yAnd = 0;
        this->tt_yCons = 0;
        this->tt_ySIMD = 0;
        this->tt_yInv = 0;
        this->tt_ySOut = 0;
        this->tt_ySIn = 0;
        this->tt_yLocalCB = 0;
        this->tt_yPrintVal = 0;
        this->tt_yAssert = 0;
        this->tt_yUNIV = 0;

        std::cout << "\n---Yao Circuit Structure:" << std::endl;
        for (uint32_t i = 0; i < this->maxdepth; i++) {
            YaoCircuit::YaoCircLayer* currYaoLayer = new YaoCircLayer(i);
            std::deque<uint32_t> localops = this->circ->GetLocalQueueOnLvl(i);
            std::deque<uint32_t> interops = this->circ->GetInteractiveQueueOnLvl(i);
            std::cout << "Layer " << i << ": " << std::endl;
            for (uint32_t j = 0; j < localops.size(); j++) {
                e_gatetype gatetype = this->circ->GetGateType(localops[j]);
                switch (gatetype) {
                    case G_LIN:
                        currYaoLayer->nyXor++;
                        break;
                    case G_NON_LIN:
                        currYaoLayer->nyAnd++;
                        break;
                    case G_CONSTANT:
                        currYaoLayer->nyCons++;
                        break;
                    case G_INV:
                        currYaoLayer->nyInv++;
                        break;
                    case G_SHARED_OUT:
                        currYaoLayer->nySOut++;
                        break;
                    case G_SHARED_IN:
                        currYaoLayer->nySIn++;
                        break;
                    case G_CALLBACK:
                        currYaoLayer->nyLocalCB++;
                        break;
                    case G_UNIV:
                        currYaoLayer->nyUNIV++;
                        break;
                    // case G_PRINT_VAL:
                    //     layer->nPrintVal++;
                    //     break;
                    // case G_ASSERT:
                    //     layer->nAssert++;
                    //     break;
                    default:
                        if (IsSIMDGate(gatetype)) {
                            currYaoLayer->nySIMD++;
                        } else {
                            std::cout << "operation not recognized" << std::endl;
                        }
                        break;
                }
            }

            for (uint32_t j = 0; j < interops.size(); j++) {
                e_gatetype gatetype = circ->GetGateType(interops[j]);
                switch (gatetype) {
                    case G_IN:
                        currYaoLayer->nyIn++;
                        break;
                    case G_OUT:
                        currYaoLayer->nyOut++;
                        break;
                    case G_CONV:
                        currYaoLayer->nyConv++;
                        break;
                    case G_CALLBACK:
                        currYaoLayer->nyCB++;
                        break;
                    default:
                        std::cout << "operation not recognized" << std::endl;
                        break;
                }
            }

            this->yaoCircLayers[i] = currYaoLayer;
            //print current layer gate details
            // this->yaoCircLayers[i]->printYaoCircLayerLocalOps();
            this->yaoCircLayers[i]->printYaoCircLayerInterOps();

            this->tt_yIn += currYaoLayer->nyIn;
            this->tt_yOut += currYaoLayer->nyOut;
            this->tt_yConv += currYaoLayer->nyConv;
            this->tt_yCB += currYaoLayer->nyCB;

            this->tt_yXor += currYaoLayer->nyXor;
            this->tt_yAnd += currYaoLayer->nyAnd;
            this->tt_yCons += currYaoLayer->nyCons;
            this->tt_ySIMD += currYaoLayer->nySIMD;
            this->tt_yInv += currYaoLayer->nyInv;
            this->tt_ySOut += currYaoLayer->nySOut;
            this->tt_ySIn += currYaoLayer->nySIn;
            this->tt_yLocalCB += currYaoLayer->nyLocalCB;
            this->tt_yPrintVal += currYaoLayer->nyPrintVal;
            this->tt_yAssert += currYaoLayer->nyAssert;
        }
        this->printYaoCircuitSummary();
        std::cout << "---Yao Circuit Structure Done" << std::endl;
    }

    void printYaoCircuitSummary();
    std::tuple<uint32_t, uint32_t, uint32_t> calculateYaoServerCircuitCost();
    std::tuple<uint32_t, uint32_t, uint32_t> calculateYaoClientCircuitCost();
};
#endif

void CalculateAllCircuitCost(Circuit* ac, Circuit* bc, Circuit* yc);