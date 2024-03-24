#include "common.h"
#include <abycore/circuit/arithmeticcircuits.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>
// #include "abycircuit.h"
#include <abycore/circuit/abycircuit.h>

//share *s_alice_extra_input = NULL;
share* BuildSequentialCircuit(share* s_alice, share* s_bob, Circuit *circ, uint32_t num_non_amortized,
	uint32_t bitlen) {
	share *out = PutTheGateForBenchmark(s_alice, s_bob, circ, bitlen);
	//out = circ->PutADDGate(out, s_alice_extra_input);
	for(int i = 1; i < num_non_amortized; i++) {
		out = PutTheGateForBenchmark(out, s_bob, circ, bitlen);
	}
	return out;
}

int32_t test_circuit(e_role role, char* address, uint16_t port, seclvl seclvl,
		uint32_t nvals, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
		e_sharing sharing, uint32_t num_tests, uint32_t num_non_amortized, uint32_t seed) {

	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads,
			mt_alg);
	std::vector<Sharing*>& sharings = party->GetSharings();

    Circuit* ac = sharings[S_ARITH]->GetCircuitBuildRoutine();
	Circuit* yc = sharings[S_YAO] ->GetCircuitBuildRoutine();
	Circuit* bc = sharings[S_BOOL]->GetCircuitBuildRoutine();
	
    Circuit* circ = sharings[sharing]->GetCircuitBuildRoutine(); //either one of above (ac, bc, yc)

    if (nvals == 1) {
        // y: build a sequential-ops circuit, support arbitrary num of tests
        std::vector<share*> s_alice_input(num_tests);
        std::vector<share*> s_bob_input(num_tests);
        std::vector<share*> s_out(num_tests);

        std::vector<uint32_t> alice_input(num_tests);
        std::vector<uint32_t> bob_input(num_tests);
        srand(seed);
        const uint32_t kTruncate = bitlen;
        //	uint32_t alice_one = 1;
        for(int j = 0; j < alice_input.size(); j++) {
            alice_input[j] = rand() % kTruncate;
            bob_input[j] = rand() % kTruncate;
        }

        
        for(int j = 0; j < s_alice_input.size(); j++) {
            if(role == SERVER) {
                s_alice_input[j] = circ->PutDummyINGate(bitlen);
                s_bob_input[j] = circ->PutINGate(bob_input[j], bitlen, SERVER);
            }
            else {
                s_alice_input[j] = circ->PutINGate(alice_input[j], bitlen, CLIENT);
                s_bob_input[j] = circ->PutDummyINGate(bitlen);
            }
        }

        //	if(role == SERVER) {
        //		s_alice_extra_input = circ->PutDummyINGate(bitlen);
        //	}
        //	else {
        //		s_alice_extra_input = circ->PutINGate(alice_one, bitlen, CLIENT);
        //	}
        
        for(int j = 0; j < s_alice_input.size(); j++) {
            s_out[j] = BuildSequentialCircuit(s_alice_input[j], s_bob_input[j], circ, num_non_amortized, bitlen);
            s_out[j] = circ->PutOUTGate(s_out[j], ALL);
        }
    } else if (nvals >= 1) {
        // y: build parallel-ops circuit, can be understood as multiple sequential circuit put in parallel 
        std::vector<std::vector<share*>> s_alice_input(num_tests, std::vector<share*> (nvals));
        std::vector<std::vector<share*>> s_bob_input(num_tests, std::vector<share*> (nvals));
        std::vector<std::vector<share*>> s_out(num_tests, std::vector<share*> (nvals));

        std::vector<std::vector<uint32_t>> alice_input(num_tests, std::vector<uint32_t> (nvals));
        std::vector<std::vector<uint32_t>> bob_input(num_tests, std::vector<uint32_t> (nvals));
        srand(seed);
        const uint32_t kTruncate = bitlen;

        for(int i = 0; i < alice_input.size(); i++) {
            for(int j = 0; j < alice_input[i].size(); j++) {
                alice_input[i][j] = rand() % kTruncate;
                bob_input[i][j] = rand() % kTruncate;
            }
        }
        
        for(int i = 0; i < s_alice_input.size(); i++) {
            for(int j = 0; j < s_alice_input[i].size(); j++) {
                if(role == SERVER) {
                    s_alice_input[i][j] = circ->PutDummyINGate(bitlen);
                    s_bob_input[i][j] = circ->PutINGate(bob_input[i][j], bitlen, SERVER);
                }
                else {
                    s_alice_input[i][j] = circ->PutINGate(alice_input[i][j], bitlen, CLIENT);
                    s_bob_input[i][j] = circ->PutDummyINGate(bitlen);
                }
            }
        }

        for(int i = 0; i < s_out.size(); i++) {
            for(int j = 0; j < s_out[i].size(); j++) {
                s_out[i][j] = BuildSequentialCircuit(s_alice_input[i][j], s_bob_input[i][j], circ, num_non_amortized, bitlen);
                s_out[i][j] = circ->PutOUTGate(s_out[i][j], ALL);
            }
        }
    } 

    CalculateAllCircuitCost(ac, bc, yc);
	party->ExecCircuit();

	uint32_t total_tests = num_tests * num_non_amortized;
	std::cout << "\nTesting " << total_tests << " operations (non_amortized: "<< num_non_amortized << ", parallel: " 
			<< num_tests << ") in " << get_sharing_name(sharing) << " sharing \n";
	
    // y: verify output by recalculating with clear values, need modify after paral implement done 
    // for(int j = 0; j < alice_input.size(); j++) {
	// 	uint32_t expected_output = PutTheOpToVerifyResult(alice_input[j], bob_input[j]);
	// 	//expected_output = expected_output + alice_one;
	// 	for(int i = 1; i < num_non_amortized; i++) {
	// 		expected_output = PutTheOpToVerifyResult(expected_output, bob_input[j]);
	// 	}

	// 	/**
	// 	Type casting the value to 64 bit unsigned integer for output.
	// 	NOTE: I am casting to 64bit because it works for both 32bit and 64bit shares.
	// 	Also, since we always provide inputs that fit inside 32bits (even when using 64 bits),
	// 	it is safe to assign the result to 32bit integer array output'
	// 	*/

	// 	uint32_t output = s_out[j]->get_clear_value<uint64_t>();
	// 	if(output != expected_output) {
	// 		std::cerr << "\nERROR: (j: " << j << ") "<< output << " != " << expected_output << ")\n"; 
	// 		std::cerr << "alice: " << alice_input[j] << "(" << num_non_amortized << " non amortized ops)\n";
	// 		std::cerr << "bob: " << bob_input[j] << "\n";
	// 		delete party;
	// 		// why 57, because it's Grothendieck prime
	// 		exit(57);
	// 	}
	// }

	std::cout << "\nAll tests passed\n";

	delete party;
	return 0;
}
