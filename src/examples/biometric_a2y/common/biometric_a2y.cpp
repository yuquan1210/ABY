/**
 \file 		min-euclidean-dist-circuit.cpp
 \author 	michael.zohner@ec-spride.de
 \copyright	ABY - A Framework for Efficient Mixed-protocol Secure Two-party Computation
			Copyright (C) 2019 Engineering Cryptographic Protocols Group, TU Darmstadt
			This program is free software: you can redistribute it and/or modify
            it under the terms of the GNU Lesser General Public License as published
            by the Free Software Foundation, either version 3 of the License, or
            (at your option) any later version.
            ABY is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
            GNU Lesser General Public License for more details.
            You should have received a copy of the GNU Lesser General Public License
            along with this program. If not, see <http://www.gnu.org/licenses/>.
 \brief		Implementation of Minimum Euclidean Distance Circuit
 */
#include "biometric_a2y.h"
#include "../../../abycore/circuit/booleancircuits.h"
#include "../../../abycore/sharing/sharing.h"
#include <iostream>
#include <vector>
#include <fstream>

int32_t test_min_eucliden_dist_circuit(e_role role, const std::string& address, uint16_t port, seclvl seclvl, uint32_t dbsize,
		uint32_t dim, uint32_t nthreads, e_mt_gen_alg mt_alg, e_sharing sharing) {
	// uint32_t bitlen = 8, i, j, temp, tempsum, maxbitlen=32;
    uint32_t bitlen = 32;
	uint64_t output;
	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads, mt_alg);
	std::vector<Sharing*>& sharings = party->GetSharings();

    Circuit* ac = sharings[S_ARITH]->GetCircuitBuildRoutine();
	Circuit* yc = sharings[S_YAO] ->GetCircuitBuildRoutine();
	Circuit* bc = sharings[S_BOOL]->GetCircuitBuildRoutine();

    Circuit *distcirc, *mincirc;
    distcirc = ac;
	mincirc = yc;

	// crypto* crypt = new crypto(seclvl.symbits, (uint8_t*) const_seed);
	uint32_t **serverdb, *clientquery;
	uint64_t verify;

	share ***Sshr, **Cshr, **Ssqr, *Csqr;

	std::ofstream outFile("/home/ethan/MPC/ABY/bio_a2y_circ.txt"); // Create an ofstream object for output and open "example.txt"

    if (!outFile) { // Check if the file was successfully opened
        std::cerr << "Error opening /home/ethan/MPC/ABY/bio_a2y_circ.txt" << std::endl;
        return 1; // Return an error code
    }

	srand(time(NULL));
    const uint32_t kTruncate = bitlen;

	//generate dbsize * dim * bitlen random bits as server db
	serverdb = (uint32_t**) malloc(sizeof(uint32_t*) * dbsize);
	for(uint32_t i = 0; i < dbsize; i++) {
		serverdb[i] = (uint32_t*) malloc(sizeof(uint32_t) * dim);
		for(uint32_t j = 0; j < dim; j++) {
			serverdb[i][j] = rand() % kTruncate;
		}
	}
	//generate dim * bitlen random bits as client query
	clientquery = (uint32_t*) malloc(sizeof(uint32_t) * dim);
	for(uint32_t j = 0; j < dim; j++) {
		clientquery[j] = rand() % kTruncate;
	}

	//set server input
	Sshr = (share***) malloc(sizeof(share**) * dbsize);
	for (uint32_t i = 0; i < dbsize; i++) {
		Sshr[i] = (share**) malloc(sizeof(share*) * dim);
		for (uint32_t j = 0; j < dim; j++) {
			Sshr[i][j] = distcirc->PutINGate(serverdb[i][j], bitlen, SERVER); //x
			outFile << "gate_id: " << Sshr[i][j] << ", gate_type: IN, in_left: None, in_right: None" << std::endl; 
		}
	}

	Ssqr = (share**) malloc(sizeof(share*) * dbsize);
	for (uint32_t i = 0; i < dbsize; i++) {
		uint32_t tempsum = 0;
		for (uint32_t j = 0; j < dim; j++) {
			uint32_t temp = serverdb[i][j];
			tempsum += (temp * temp);
		}
		Ssqr[i] = distcirc->PutINGate(tempsum, bitlen, SERVER); //x^2 + y^2
		outFile << "gate_id: " << Ssqr[i] << ", gate_type: IN, in_left: None, in_right: None" << std::endl; 
	}

	//set client input
	Cshr = (share**) malloc(sizeof(share*) * dim);
	uint32_t tempsum = 0;
	for (uint32_t j = 0; j < dim; j++) {
		uint32_t temp = clientquery[j];
        // input 2a, 2b
		Cshr[j] = distcirc->PutINGate(2*temp, bitlen, CLIENT);
		outFile << "gate_id: " << Cshr[j] << ", gate_type: IN, in_left: None, in_right: None" << std::endl; 
		tempsum += (temp * temp);
	}
    // input a^2 + b^2
	Csqr = distcirc->PutINGate(tempsum, bitlen, CLIENT);
	outFile << "gate_id: " << Csqr << ", gate_type: IN, in_left: None, in_right: None" << std::endl; 

    share **distance, *temp, *mindist;
	share *old_addr;
    distance = (share**) malloc(sizeof(share*) * dbsize);
    for (uint32_t i = 0; i < dbsize; i++) {
        distance[i] = distcirc->PutMULGate(Sshr[i][0], Cshr[0]);
		outFile << "gate_id: " << distance[i] << ", gate_type: MUL, in_left: " << Sshr[i][0] << ", in_right: " << Cshr[0] << std::endl; 
		for (uint32_t j = 1; j < dim; j++) {
			temp = distcirc->PutMULGate(Sshr[i][j], Cshr[j]);
			outFile << "gate_id: " << temp << ", gate_type: MUL, in_left: " << Sshr[i][j] << ", in_right: " << Cshr[j] << std::endl; 
            // distance[i] = 2ax + 2by
			old_addr = distance[i];
			distance[i] = distcirc->PutADDGate(distance[i], temp);
			outFile << "gate_id: " << distance[i] << ", gate_type: ADD, in_left: " << old_addr << ", in_right: " << temp << std::endl; 
		}
        // temp = (x^2 + y^2) + (a^2 + b^2)
		temp = distcirc->PutADDGate(Ssqr[i], Csqr);
		outFile << "gate_id: " << temp << ", gate_type: ADD, in_left: " << Ssqr[i] << ", in_right: " << Csqr << std::endl; 
        // distance[i] = (x^2 + y^2) + (a^2 + b^2) - (2ax + 2by)
		old_addr = distance[i];
        distance[i] = distcirc->PutSUBGate(temp, distance[i]);
		outFile << "gate_id: " << distance[i] << ", gate_type: SUB, in_left: " << temp << ", in_right: " << old_addr << std::endl; 
    }

	// do a2y conversion
	for (uint32_t i = 0; i < dbsize; i++)
    {
		old_addr = distance[i];
		distance[i] = mincirc->PutA2YGate(distance[i]);
		outFile << "gate_id: " << distance[i] << ", gate_type: GT, in_left: " << old_addr << ", in_right: None" << std::endl; 
	}

    //find min in distance[]
    share* cmp;
    mindist = distance[0];
    for (uint32_t i = 1; i < dbsize; i++)
    {
        cmp = mincirc->PutGTGate(mindist, distance[i]);
		outFile << "gate_id: " << cmp << ", gate_type: GT, in_left: " << mindist << ", in_right: " << distance[i] << std::endl; 
        old_addr = mindist;
		mindist = mincirc->PutMUXGate(distance[i], mindist, cmp);
		outFile << "gate_id: " << mindist << ", gate_type: MUX, in_left: " << distance[i] << ", in_right: " << old_addr << ", in_cmp: " << cmp << std::endl; 
    }

	old_addr = mindist;
	mindist = mincirc->PutOUTGate(mindist, ALL);
	outFile << "gate_id: " << mindist << ", gate_type: OUT, in_left: " << old_addr << ", in_right: None" << std::endl; 

	outFile.close();
	SaveMaxDepth(party->GetTotalDepth());
	GenerateCircSpreadSheet("biometric_a2y.csv");
    CalculateAllCircuitCost(ac, bc, yc);
	party->ExecCircuit();

	//TODO free
	for(uint32_t i = 0; i < dbsize; i++) {
		free(serverdb[i]);
		free(Sshr[i]);
	}

	free(serverdb);
	free(Sshr);
	free(Ssqr);

	free(clientquery);
	free(Cshr);

	return 0;
}

uint64_t verify_min_euclidean_dist(uint32_t** serverdb, uint32_t* clientquery, uint32_t dbsize, uint32_t dim) {
	uint32_t i, j;
	uint64_t mindist, tmpdist;

	mindist = ULLONG_MAX;
	for(i=0; i < dbsize; i++) {
		tmpdist = 0;
		for(j=0; j < dim; j++) {
			if(serverdb[i][j] > clientquery[j])
				tmpdist += pow((serverdb[i][j] - clientquery[j]), 2);
			else
				tmpdist += pow((clientquery[j] - serverdb[i][j]), 2);
		}
		if(tmpdist < mindist)
			mindist = tmpdist;
	}

	return mindist;
}
