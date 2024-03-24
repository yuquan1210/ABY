#include "cost_calculator.h"

void CalculateAllCircuitCost(Circuit* ac, Circuit* bc, Circuit* yc){
    uint32_t ac_depth = ac->GetMaxDepth();
    uint32_t bc_depth = bc->GetMaxDepth();
    uint32_t yc_depth = yc->GetMaxDepth();

    uint32_t maxdepth = ac_depth;
    if(maxdepth < bc_depth){
        maxdepth = bc_depth;
    }
    if(maxdepth < yc_depth){
        maxdepth = yc_depth;
    }

    std::tuple<uint32_t, uint32_t, uint32_t> bc_cost(0, 0, 0);
    std::tuple<uint32_t, uint32_t, uint32_t> yc_cost(0, 0, 0);
    std::tuple<uint32_t, uint32_t, uint32_t> ac_cost(0, 0, 0);

    // calculate bool circuit
    if(bc->GetNumGates() > 0){
        BoolCircuit* boolCirc = new BoolCircuit(bc);
        bc_cost = boolCirc->calculateBoolCircuitCost();
        boolCirc->printBoolCircuitSummary();
    }

    // calculate arith circuit
    if(ac->GetNumGates() > 0){
        ArithCircuit* arithCirc = new ArithCircuit(ac);
        ac_cost = arithCirc->calculateArithCircuitCost();
        arithCirc->printArithCircuitSummary();
    }

    // calculate yao circuit
    if(yc->GetNumGates() > 0){
        e_role role = yc->GetRole();
        YaoCircuit* yaoCirc = new YaoCircuit(yc);
        if(role == SERVER){
            yc_cost = yaoCirc->calculateYaoServerCircuitCost();
        } else if (role == CLIENT){
            yc_cost = yaoCirc->calculateYaoClientCircuitCost();
        }
        yaoCirc->printYaoCircuitSummary();
    }

    // TODO: add ysc/ycc, need specify role
    uint32_t tt_setup = std::get<0>(bc_cost) + std::get<0>(ac_cost) + std::get<0>(yc_cost);
    uint32_t tt_online = std::get<1>(bc_cost) + std::get<1>(ac_cost) + std::get<1>(yc_cost) + 9;
    uint32_t tt_comm = tt_setup + tt_online;
    std::cout << "\n***Calculate Total Cost:" << std::endl;
    std::cout << "Summary (bc + ac + yc):" << std::endl;
    std::cout << "\tsetup_total: " << tt_setup << std::endl;
    std::cout << "\tonline_total (+9): " << tt_online << std::endl;
    std::cout << "\ttotal communication (setup+online): " << tt_setup + tt_online << std::endl;
    
    std::cout << "***Calculate Total done" << std::endl << std::endl;
    return;
}

std::tuple<uint32_t, uint32_t, uint32_t> BoolCircuit::calculateBoolCircuitCost(){
    std::cout << "\n***Calculate Bool Circuit cost:" << std::endl;
    std::cout << "Setup Phase:" << std::endl;
    uint32_t nBaseOT = 128; // defined in seclvl
    uint32_t numOT = 0;

    uint32_t sum_and = this->tt_bAnd + this->tt_bVecAnd;
    if (sum_and > 0){
        numOT = sum_and + 8 * maxdepth;
    }
    uint32_t numMT = numOT;
    std::cout << "\tnBaseOT (default 128): " << nBaseOT << std::endl;
    std::cout << "\tnumOT/numMT (nAndBit + 8 * depth):  " << numOT << std::endl;

    uint32_t otext_recer = 0;
    uint32_t otext_snder = 0;
    if (numOT > 0){
        otext_recer = nBaseOT * (PadToMultiple(numOT, 128)/8) + 25 + 9;
        otext_snder = 9;
    }
    uint32_t setup_total = otext_snder + otext_recer;
    std::cout << "\totext_snder (default 9): " << otext_snder << std::endl;
    std::cout << "\totext_recer (nBaseOT * (PadToMultiple(numMT, 128)/8) + 25 + 9):  " << otext_recer << std::endl;
    std::cout << "setup_total (otext_snder + otext_recer): " << setup_total << std::endl << std::endl;

    std::cout << "Online Phase:" << std::endl;
    uint32_t online_total = 0;
    for (int i = 0; i < maxdepth; i++)
    {
        // assume input to one out of two parties, output to all parties
        std::cout << "layer " << i << ": " << std::endl;
        uint32_t layer_cost;
        uint32_t in_cost = (PadToMultiple(this->boolCircLayers[i]->nbIn, 8) / 8) / 2;
        uint32_t out_cost = PadToMultiple(this->boolCircLayers[i]->nbOut, 8) / 8;
        uint32_t and_cost = (PadToMultiple(this->boolCircLayers[i]->nbAnd, 8) / 8) * 2;
        uint32_t vecAnd_cost = (32+1)*this->boolCircLayers[i]->nbVecAnd;

        if(in_cost > 0){
            std::cout << "\tin online ((PadToMultiple(nIn, 8) / 8) / 2): " << in_cost << std::endl;
        }
        if(out_cost > 0){
            std::cout << "\tout online (PadToMultiple(nOut, 8) / 8): " << out_cost << std::endl;
        }
        if(and_cost > 0){
            std::cout << "\tand online ((PadToMultiple(nAnd, 8) / 8) * 2): " << and_cost << std::endl;
        }
        if(vecAnd_cost > 0){
            std::cout << "\tvector and online (32+1)*nVecAnd: " << vecAnd_cost << std::endl;
        }
        
        layer_cost = and_cost + out_cost + in_cost + vecAnd_cost;
        if(layer_cost > 0) {
            layer_cost += 9; // if no interact op cost this layer, then no pkt sent, therefore no pkt header cost (9)
            std::cout << "\tlayer " << i <<  " online total (and_cost + out_cost + in_cost + vecAnd_cost + 9): " << layer_cost << std::endl;
        }
        online_total += layer_cost;
    }
    std::cout << "online_total (sum_layer_cost): " << online_total << std::endl;

    std::cout << "\nBool Circuit Summary:" << std::endl;
    std::cout << "\tsetup_total: " << setup_total << std::endl;
    std::cout << "\tonline_total: " << online_total << std::endl;
    std::cout << "\ttotal communication (sent): " << setup_total + online_total << std::endl;

    std::cout << "***Calculate Bool Circuit done ***" << std::endl;
    return std::tuple<uint32_t, uint32_t, uint32_t>(setup_total, online_total, setup_total + online_total);
}

std::tuple<uint32_t, uint32_t, uint32_t> ArithCircuit::calculateArithCircuitCost(){
    std::cout << "\n***Calculate Arith Circuit cost:" << std::endl;
    std::cout << "Setup Phase:" << std::endl;
    uint32_t nBaseOT = 128; // defined in seclvl
    uint32_t nMT = this->tt_aMul; //1 arith mul gate (regardless bitlen)-> 1 MT? need check
    uint32_t tt_nOT = 0;
    uint32_t nOTforMT = 0;
    uint32_t nOTforConv = 0;
    if(nMT > 0){
        nOTforMT += nMT * this->bitlen;
        tt_nOT += nOTforMT;
    }
    if(this->tt_aConv > 0){
        nOTforConv += this->tt_aConv * this->bitlen;
        tt_nOT += nOTforConv;
    }
    std::cout << "\tnBaseOT (default 128): " << nBaseOT << std::endl;
    std::cout << "\tnOTforMT (nMT/nMul * bitlen):  " << nOTforMT << std::endl;
    std::cout << "\tnOTforConv (nConv * bitlen):  " << nOTforConv << std::endl;
    std::cout << "\ttotal_nOT (nOTforMT + nOTforConv):  " << tt_nOT << std::endl;

    uint32_t otext_rcver = 0;
    uint32_t otext_snder = 0;
    if(nMT > 0){
        //both server & client do iknpot_snd & iknpot_rcv, so same cost
        otext_rcver += nBaseOT * (PadToMultiple(nOTforMT, 128)/8) + 25 + 9; //why 25?
        otext_snder += (nOTforMT * bitlen) / 8 + 25 + 9;
        std::cout << "\nPerform OT for MT/Mul: otext_snder ((nOTforMT * bitlen)/8 + 25 + 9): " << otext_snder << " bytes" << std::endl;
        std::cout << "Perform OT for MT/Mul: otext_rcver (nBaseOT * (PadToMultiple(nOTforMT, 128)/8) + 25 + 9): " << otext_rcver << " bytes" << std::endl;
    } 
    if(this->tt_aConv > 0){
        if(this->circ->GetRole() == SERVER){
            otext_snder += 9; 
            std::cout << "\nPerform OT for Conv: otext_snder (default 9): " << otext_snder << " bytes" << std::endl;
        } else {
            otext_rcver += nBaseOT * (PadToMultiple(nOTforConv, 128)/8) + 25 + 9; //why 25?
            std::cout << "\nPerform OT for Conv: otext_rcver (nBaseOT * (PadToMultiple(nOTforConv, 128)/8) + 25 + 9): " << otext_rcver << " bytes" << std::endl;
        }
    }
    uint32_t setup_total = otext_snder + otext_rcver;
    std::cout << "setup_total (otext_snder + otext_recer): " << setup_total << std::endl << std::endl;

    std::cout << "Online Phase:" << std::endl;
    uint32_t online_total = 0;
    for (int i = 0; i < maxdepth; i++)
    {
        // assume input to one out of two parties, output to all parties
        std::cout << "layer " << i << ": " << std::endl;
        uint32_t layer_cost;
        uint32_t in_cost = ((this->arithCircLayers[i]->naIn * this->bitlen) / 8) / 2;
        uint32_t out_cost = (this->arithCircLayers[i]->naOut * this->bitlen) / 8;
        uint32_t and_cost = ((this->arithCircLayers[i]->naMul * this->bitlen) / 8) * 2;
        uint32_t conv_cost = 0;
        if(this->circ->GetRole() == SERVER){
            conv_cost = 2 * this->arithCircLayers[i]->naConv * this->bitlen * 4;
        } else {
            conv_cost = ceil_divide(this->arithCircLayers[i]->naConv * this->bitlen, 8);
        }

        if(in_cost > 0){
            std::cout << "\tin online (((nIn * bitlen) / 8) / 2): " << in_cost << std::endl;
        }
        if(out_cost > 0){
            std::cout << "\tout online ((nOut * bitlen) / 8): " << out_cost << std::endl;
        }
        if(and_cost > 0){
            std::cout << "\tand online (((nMul * bitlen) / 8) * 2): " << and_cost << std::endl;
        }
        if(conv_cost > 0 && this->circ->GetRole() == SERVER){
            std::cout << "\tconv online (2 * nConv * bitlen * sizeof(uint32)), send in next layer: " << conv_cost << std::endl;
        } else if(conv_cost > 0 && this->circ->GetRole() == CLIENT){
            std::cout << "\tconv online (ceil_divide(nConv * bitlen, 8)): " << conv_cost << std::endl;
        }
        
        layer_cost = and_cost + out_cost + in_cost + conv_cost;
        if(layer_cost > 0) {
            layer_cost += 9; // if no interact op cost this layer, then no pkt sent, therefore no pkt header cost (9)
            std::cout << "\tlayer " << i <<  " online total (in_cost + out_cost + and_cost + conv_cost + 9): " << layer_cost << std::endl;
        }
        online_total += layer_cost;
    }
    
    std::cout << "online_total (sum_layer_cost): " << online_total << std::endl;

    std::cout << "\nArith Circuit Summary:" << std::endl;
    std::cout << "\tsetup_total: " << setup_total << std::endl;
    std::cout << "\tonline_total: " << online_total << std::endl;
    std::cout << "\ttotal communication (sent): " << setup_total + online_total << std::endl;

    std::cout << "***Calculate Arith Circuit done ***" << std::endl;
    return std::tuple<uint32_t, uint32_t, uint32_t>(setup_total, online_total, setup_total + online_total);
}

std::tuple<uint32_t, uint32_t, uint32_t> YaoCircuit::calculateYaoServerCircuitCost(){
    std::cout << "\n***Calculate Yao Server Circuit cost:" << std::endl;
    std::cout << "\tnServerInputBit: " << this->circ->GetNumInputBitsForParty(SERVER) << std::endl;
    std::cout << "\tnServerOutputBit: " << this->circ->GetNumOutputBitsForParty(SERVER) << std::endl;
    std::cout << "\tnClientInputBit: " << this->circ->GetNumInputBitsForParty(CLIENT) << std::endl;
    std::cout << "\tnClientOutputBit: " << this->circ->GetNumOutputBitsForParty(CLIENT) << std::endl;
    
    std::cout << "Setup Phase:" << std::endl;
    uint32_t nBaseOT = 128; // defined in secparam
    uint32_t secparam_byte = 16; // secparam in bytes
    uint32_t numOT = this->circ->GetNumInputBitsForParty(CLIENT) + this->circ->GetNumConvToYaoInputBits(); //+ conversion_input_bit

    // // uint32_t numMT = numOT;
    std::cout << "\tnBaseOT (default 128): " << nBaseOT << std::endl;
    std::cout << "\tA2Y+B2Y+YSwitch bit gate num: " << this->circ->GetNumConvToYaoInputBits() << std::endl;
    std::cout << "\tnumOT/numMT (nClientInputBit + (A2Y+B2Y+YSwitch)):  " << numOT << std::endl;

    uint32_t otext_snder = 0;
    if (numOT > 0){
        otext_snder = 9;
    }
    std::cout << "\totext_snder (default 9): " << otext_snder << std::endl;

    uint32_t garb_snd = 0;
    if(this->tt_yAnd > 0){
        garb_snd += this->tt_yAnd * secparam_byte * 2 + 9;
    }
    if(this->circ->GetNumOutputBitsForParty(CLIENT) > 0){
        garb_snd += (PadToMultiple(this->circ->GetNumOutputBitsForParty(CLIENT), 8) / 8) + 9;
    }
    std::cout << "\tgarb_snd (sum_and_bit * secparam_byte * 2 (+ 9) + client_output_byte (+ 9)):  " << garb_snd << std::endl;

    uint32_t setup_total = otext_snder + garb_snd;
    std::cout << "setup_total (otext_snder + garb_snd): " << setup_total << std::endl << std::endl;

    std::cout << "Online Phase:" << std::endl;
    uint32_t online_total = 0;

    std::cout << "layer 0: " << std::endl;
    uint32_t server_input_key_size = 0;
    if(this->circ->GetNumInputBitsForParty(SERVER) + this->circ->GetNumConvToYaoInputBits() > 0){
        //TODO: need differentiate b2y or a2y conversion
        server_input_key_size = (this->circ->GetNumInputBitsForParty(SERVER) + this->circ->GetNumConvToYaoInputBits()) * secparam_byte;
        online_total += server_input_key_size;
        online_total += 9;
    }
    std::cout << "\tsend sever input keys ((server_input_bits + (A2Y+B2Y+YSwitch)) * secparam_byte): " << server_input_key_size << " bytes" << std::endl; //server input only include A2Y gate? need confirm
   
    std::cout << "layer 1: " << std::endl;
    uint32_t client_input_key_size = 0;
    if(this->circ->GetNumInputBitsForParty(CLIENT) + this->circ->GetNumConvToYaoInputBits() > 0){
        client_input_key_size = (this->circ->GetNumInputBitsForParty(CLIENT) + this->circ->GetNumConvToYaoInputBits()) * secparam_byte * 2;
        online_total += client_input_key_size;
        online_total += 9;
    }
    std::cout << "\tsend client input keys ((client_input_bits + (A2Y+B2Y+YSwitch)) * secparam_byte * 2): " << client_input_key_size << " bytes" << std::endl;

    std::cout << "layer 2: " << std::endl;
    std::cout << "\tonly receive from client" << std::endl;

    // online_total += 9;
    std::cout << "online_total (sum_layer_cost): " << online_total << std::endl;

    std::cout << "\nYao Server Circuit Summary:" << std::endl;
    std::cout << "\tsetup_total: " << setup_total << std::endl;
    std::cout << "\tonline_total: " << online_total << std::endl;
    std::cout << "\ttotal communication (sent): " << setup_total + online_total << std::endl;

    std::cout << "***Calculate Yao Server Circuit done ***" << std::endl;
    return std::tuple<uint32_t, uint32_t, uint32_t>(setup_total, online_total, setup_total + online_total);
}

std::tuple<uint32_t, uint32_t, uint32_t> YaoCircuit::calculateYaoClientCircuitCost(){
    std::cout << "\n***Calculate Yao Client Circuit cost:" << std::endl;
    std::cout << "\tnServerInputBit: " << this->circ->GetNumInputBitsForParty(SERVER) << std::endl;
    std::cout << "\tnServerOutputBit: " << this->circ->GetNumOutputBitsForParty(SERVER) << std::endl;
    std::cout << "\tnClientInputBit: " << this->circ->GetNumInputBitsForParty(CLIENT) << std::endl;
    std::cout << "\tnClientOutputBit: " << this->circ->GetNumOutputBitsForParty(CLIENT) << std::endl;
    
    std::cout << "Setup Phase:" << std::endl;
    uint32_t nBaseOT = 128; // defined in secparam
    uint32_t secparam_byte = 16; // secparam in bytes
    uint32_t numOT = this->circ->GetNumInputBitsForParty(CLIENT) + this->circ->GetNumConvToYaoInputBits(); 

    // // uint32_t numMT = numOT;
    std::cout << "\tnBaseOT (default 128): " << nBaseOT << std::endl;
    std::cout << "\tA2Y+B2Y+YSwitch bit gate num: " << this->circ->GetNumConvToYaoInputBits() << std::endl;
    std::cout << "\tnumOT/numMT (nClientInputBit + (A2Y+B2Y+YSwitch)):  " << numOT << std::endl;

    uint32_t otext_rcver = 0;
    uint32_t garb_snd = 0;
    if (numOT > 0){
        otext_rcver = nBaseOT * (PadToMultiple(numOT, 128)/8)+25+9;
    }
    std::cout << "\totext_rcver (nBaseOT * (PadToMultiple(numOT, 128)/8) + 25 + 9): " << otext_rcver << std::endl;
    std::cout << "\tgarb_snd (client don't garble):  " << 0 << std::endl;

    uint32_t setup_total = otext_rcver + garb_snd;
    std::cout << "setup_total (otext_snder + garb_snd): " << setup_total << std::endl << std::endl;

    std::cout << "Online Phase:" << std::endl;
    uint32_t online_total = 0;

    std::cout << "layer 0: " << std::endl;
    uint32_t client_ot_byte = 0;
    if(numOT > 0){
        client_ot_byte = numOT / 8;
        online_total += client_ot_byte;
        online_total += 9;
    }
    std::cout << "\tsend client ot byte (numOT/8): " << client_ot_byte << " bytes" << std::endl;
   
    std::cout << "layer 1: " << std::endl;
    std::cout << "\treceive client input keys" << std::endl;

    std::cout << "layer 2: " << std::endl;
    uint32_t send_output_byte = 0;
    if(this->circ->GetNumOutputBitsForParty(SERVER) > 0){
        send_output_byte = PadToMultiple(this->circ->GetNumOutputBitsForParty(SERVER), 8) / 8;
        online_total += send_output_byte;
        online_total += 9;
    }
    std::cout << "\tsend server output shares: " << send_output_byte << " bytes" << std::endl;

    // online_total += 9;
    std::cout << "online_total (sum_layer_cost): " << online_total << std::endl;

    std::cout << "\nYao Client Circuit Summary:" << std::endl;
    std::cout << "\tsetup_total: " << setup_total << std::endl;
    std::cout << "\tonline_total: " << online_total << std::endl;
    std::cout << "\ttotal communication (sent): " << setup_total + online_total << std::endl;

    std::cout << "***Calculate Yao Client Circuit done ***" << std::endl;
    return std::tuple<uint32_t, uint32_t, uint32_t>(setup_total, online_total, setup_total + online_total);
}

void BoolCircuit::printBoolCircuitSummary(){
    std::cout << "***Total Gate Count: " << std::endl;
    std::cout << "\t" << "total Bool IN: " << this->tt_bIn << std::endl;
    std::cout << "\t" << "total Bool OUT: " << this->tt_bOut << std::endl;
    // std::cout << "\t" << "total Bool XOR: " << this->tt_bXor << std::endl;
    std::cout << "\t" << "total Bool AND(NON-LIN): " << this->tt_bAnd << std::endl;
    std::cout << "\t" << "total Bool VecAnd: " << this->tt_bVecAnd << std::endl;
}

void ArithCircuit::printArithCircuitSummary(){
    std::cout << "***Total Gate Count: " << std::endl;
    std::cout << "\t" << "total Arith IN: " << this->tt_aIn << std::endl;
    std::cout << "\t" << "total Arith OUT: " << this->tt_aOut << std::endl;
    std::cout << "\t" << "total Arith MUL(NON-LIN): " << this->tt_aMul << std::endl;
    std::cout << "\t" << "total Arith CONV: " << this->tt_aConv << std::endl;
}

void YaoCircuit::printYaoCircuitSummary(){
    std::cout << "***Total Gate Count: " << std::endl;
    std::cout << "\t" << "total Yao IN: " << this->tt_yIn << std::endl;
    std::cout << "\t" << "total Yao OUT: " << this->tt_yOut << std::endl;
    // std::cout << "\t" << "total Yao AND: " << this->tt_yAnd << std::endl;
    std::cout << "\t" << "total Yao CONV: " << this->tt_yConv << std::endl;
}

void BoolCircuit::BoolCircLayer::printBoolCircLayerLocalOps(){
    std::cout << "\tlocalops: " << std::endl;
    if (this->nbXor > 0){
        std::cout << "\t\t" << "num Bool XOR: " << this->nbXor << std::endl;
    }
    if (this->nbCons > 0){
        std::cout << "\t\t" << "num Bool CONS: " << this->nbCons << std::endl;
    }
    if (this->nbInv > 0){
        std::cout << "\t\t" << "num Bool INV: " << this->nbInv << std::endl;
    }
    if (this->nbConv > 0){
        std::cout << "\t\t" << "num Bool CONV: " << this->nbConv << std::endl;
    }
    if (this->nbSOut > 0){
        std::cout << "\t\t" << "num Bool SHARED_OUT: " << this->nbSOut << std::endl;
    }
    if (this->nbSIn > 0){
        std::cout << "\t\t" << "num Bool SHARED_IN: " << this->nbSIn << std::endl;
    }
    if (this->nbLocalCB > 0){
        std::cout << "\t\t" << "num Bool CALLBACK: " << this->nbLocalCB << std::endl;
    }
    if (this->nbPrintVal > 0){
        std::cout << "\t\t" << "num Bool PRINT_VAL: " << this->nbPrintVal << std::endl;
    }
    if (this->nbAssert > 0){
        std::cout << "\t\t" << "num Bool ASSERT: " << this->nbAssert << std::endl;
    }
    if (this->nbSIMD > 0){
        std::cout << "\t\t" << "num Bool SIMD: " << this->nbSIMD << std::endl;
    }
}

void BoolCircuit::BoolCircLayer::printBoolCircLayerInterOps(){
    std::cout << "\tinterops: " << std::endl;
    if(this->nbIn > 0){
        std::cout << "\t\t" << "num Bool IN: " << this->nbIn << std::endl;
    }
    if(this->nbOut > 0){
        std::cout << "\t\t" << "num Bool OUT: " << this->nbOut << std::endl;
    }
    if(this->nbAnd > 0){
        std::cout << "\t\t" << "num Bool AND: " << this->nbAnd << std::endl;
    }
    if(this->nbVecAnd > 0){
        std::cout << "\t\t" << "num Bool VecAnd(NON_LIN_VEC): " << this->nbVecAnd << std::endl;
    }
    if(this->nbTT > 0){
        std::cout << "\t\t" << "num Bool TT: " << this->nbTT << std::endl;
    }
    if(this->nbCB > 0){
        std::cout << "\t\t" << "num Bool CALLBACK: " << this->nbCB << std::endl;
    }
}

void ArithCircuit::ArithCircLayer::printArithCircLayerLocalOps(){
    std::cout << "\tlocalops: " << std::endl;
    if(this->naAdd > 0) {
        std::cout << "\t\t" << "num Arith ADD: " << naAdd << std::endl;
    }
    if(this->naCons > 0) {
        std::cout << "\t\t" << "num Arith CONS: " << naCons << std::endl;
    }
    if(this->naMulCons > 0) {
        std::cout << "\t\t" << "num Arith MULCONS: " << naMulCons << std::endl;
    }
    if(this->naInv > 0) {
        std::cout << "\t\t" << "num Arith INV: " << naInv << std::endl;
    }
    if(this->naSOut > 0) {
        std::cout << "\t\t" << "num Arith SHARED_OUT: " << naSOut << std::endl;
    }
    if(this->naSIn > 0) {
        std::cout << "\t\t" << "num Arith SHARED_IN: " << naSIn << std::endl;
    }
    if(this->naLocalCB > 0) {
        std::cout << "\t\t" << "num Arith Local CALLBACK: " << naLocalCB << std::endl;
    }
    if(this->naPrintVal > 0) {
        std::cout << "\t\t" << "num Arith PRINT_VAL: " << naPrintVal << std::endl;
    }
    if(this->naAssert > 0) {
        std::cout << "\t\t" << "num Arith ASSERT: " << naAssert << std::endl;
    }
    if(this->naSIMD > 0) {
        std::cout << "\t\t" << "num Arith SIMD: " << naSIMD << std::endl;
    }
}

void ArithCircuit::ArithCircLayer::printArithCircLayerInterOps(){
    std::cout << "\tinterops: " << std::endl;
    if(this->naIn > 0) {
        std::cout << "\t\t" << "num Arith IN: " << naIn << std::endl;
    }
    if(this->naOut > 0) {
        std::cout << "\t\t" << "num Arith OUT: " << naOut << std::endl;
    }
    if(this->naMul > 0) {
        std::cout << "\t\t" << "num Arith MUL: " << naMul << std::endl;
    }
    if(this->naConv > 0) {
        std::cout << "\t\t" << "num Arith CONV: " << naConv << std::endl;
    }
    if(this->naCB > 0) {
        std::cout << "\t\t" << "num Arith CALLBACK: " << naCB << std::endl;
    }
}

void YaoCircuit::YaoCircLayer::printYaoCircLayerLocalOps(){
    std::cout << "\tlocalops: " << std::endl;
    if (this->nyXor > 0){
        std::cout << "\t\t" << "num Yao XOR: " << this->nyXor << std::endl;
    }
    if (this->nyAnd > 0){
        std::cout << "\t\t" << "num Yao AND: " << this->nyAnd << std::endl;
    }
    if (this->nyCons > 0){
        std::cout << "\t\t" << "num Yao CONS: " << this->nyCons << std::endl;
    }
    if (this->nyInv > 0){
        std::cout << "\t\t" << "num Yao INV: " << this->nyInv << std::endl;
    }
    if (this->nySOut > 0){
        std::cout << "\t\t" << "num Yao SHARED_OUT: " << this->nySOut << std::endl;
    }
    if (this->nySIn > 0){
        std::cout << "\t\t" << "num Yao SHARED_IN: " << this->nySIn << std::endl;
    }
    if (this->nyLocalCB > 0){
        std::cout << "\t\t" << "num Yao CALLBACK: " << this->nyLocalCB << std::endl;
    }
    if (this->nyUNIV > 0){
        std::cout << "\t\t" << "num Yao UNIV: " << this->nyUNIV << std::endl;
    }
    if (this->nyPrintVal > 0){
        std::cout << "\t\t" << "num Yao PRINT_VAL: " << this->nyPrintVal << std::endl;
    }
    if (this->nyAssert > 0){
        std::cout << "\t\t" << "num Yao ASSERT: " << this->nyAssert << std::endl;
    }
    if (this->nySIMD > 0){
        std::cout << "\t\t" << "num Yao SIMD: " << this->nySIMD << std::endl;
    }
}


void YaoCircuit::YaoCircLayer::printYaoCircLayerInterOps(){
    std::cout << "\tinterops: " << std::endl;
    if(this->nyIn > 0){
        std::cout << "\t\t" << "num Yao IN: " << this->nyIn << std::endl;
    }
    if(this->nyOut > 0){
        std::cout << "\t\t" << "num Yao OUT: " << this->nyOut << std::endl;
    }
    if(this->nyConv > 0){
        std::cout << "\t\t" << "num Yao CONV: " << this->nyConv << std::endl;
    }
    if(this->nyCB > 0){
        std::cout << "\t\t" << "num Yao CALLBACK: " << this->nyCB << std::endl;
    }
}
