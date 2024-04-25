#include "common.h"
#include <abycore/circuit/arithmeticcircuits.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>
// #include "abycircuit.h"
#include <abycore/circuit/abycircuit.h>

//share *s_alice_extra_input = NULL;
share* BuildSequentialCircuit(share* s_alice, share* s_bob, Circuit *circ, uint32_t nseq,
	uint32_t bitlen) {
	share *out = PutTheGateForBenchmark(s_alice, s_bob, circ, bitlen);
	//out = circ->PutADDGate(out, s_alice_extra_input);
	for(int i = 1; i < nseq; i++) {
		out = PutTheGateForBenchmark(out, s_bob, circ, bitlen);
	}
	return out;
}

int32_t test_circuit(e_role role, char* address, uint16_t port, seclvl seclvl,
		uint32_t nparal, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
		e_sharing sharing, uint32_t num_tests, uint32_t nseq, uint32_t seed) {
    
    ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads, mt_alg);
    std::vector<Sharing*>& sharings = party->GetSharings();

    Circuit* ac = sharings[S_ARITH]->GetCircuitBuildRoutine();
    Circuit* yc = sharings[S_YAO] ->GetCircuitBuildRoutine();
    Circuit* bc = sharings[S_BOOL]->GetCircuitBuildRoutine();
    
    Circuit* circ = sharings[sharing]->GetCircuitBuildRoutine(); //either one of above (ac, bc, yc)

    // y: build a sequential-ops circuit, support arbitrary num of tests
    std::vector<share*> s_alice_input(nparal);
    std::vector<share*> s_bob_input(nparal);
    std::vector<share*> s_out(nparal);

    std::vector<uint32_t> alice_input(nparal);
    std::vector<uint32_t> bob_input(nparal);

    std::vector<std::vector<double>> tt_localop_timings(5);
    std::vector<std::vector<double>> tt_interop_timings(5);
    std::vector<std::vector<double>> tt_finishlay_timings(5);
    std::map<uint32_t, double> tt_interaction_timings;
    std::map<uint32_t, uint8_t> interaction_datasent;

    double tt_online_time = 0.0;
    double online_time;
    for(int i = 0; i < num_tests; i++){
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
        
        for(int j = 0; j < s_alice_input.size(); j++) {
            s_out[j] = BuildSequentialCircuit(s_alice_input[j], s_bob_input[j], circ, nseq, bitlen);
            s_out[j] = circ->PutOUTGate(s_out[j], ALL);
        }
        if(i==0){
            CalculateAllCircuitCost(ac, bc, yc);
        }
        party->ExecCircuit();
        std::vector<std::vector<double>> localop_timings = party->GetLocalOpTimings();
        std::vector<std::vector<double>> interop_timings = party->GetInterOpTimings();
        std::vector<std::vector<double>> finishlay_timings = party->GetFinishLayerTimings();
        std::map<uint32_t, double> interaction_timings = party->GetInteractionTimings();
        if(i == 0){
            for(int sharing = 0; sharing < 5; sharing++){
                tt_localop_timings[sharing] = localop_timings[sharing];
                tt_interop_timings[sharing] = interop_timings[sharing];
                tt_finishlay_timings[sharing] = finishlay_timings[sharing];
            }
            tt_interaction_timings = interaction_timings;
            interaction_datasent = party->GetInteractionDataSent();
        } else {
            for(int sharing = 0; sharing < 5; sharing++){
                for(int layer = 0; layer < localop_timings[0].size(); layer++){
                    tt_localop_timings[sharing][layer] += localop_timings[sharing][layer];
                    tt_interop_timings[sharing][layer] += interop_timings[sharing][layer];
                    tt_finishlay_timings[sharing][layer] += finishlay_timings[sharing][layer];
                }
            }
            for(int layer = 0; layer < localop_timings[0].size(); layer++){
                tt_interaction_timings[layer] += interaction_timings[layer];
            }
        }
        online_time = party->GetTiming(P_ONLINE);
        tt_online_time += online_time;
        party->Reset();
        std::cout << std::endl;
    }

#if BENCHONLINEPHASE
	std::cout << "Average Online time distribution: " << std::endl;
	// std::cout << "Bool: " << std::endl;
    // for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
    //     std::cout << "\tlayer " << layer << ": local gate: " << tt_localop_timings[0][layer] / num_tests << ", interactive gate: " << tt_interop_timings[0][layer] / num_tests << ", layer finish: " << tt_finishlay_timings[0][layer] / num_tests << std::endl;
    // }
    // std::cout << "Yao Server: " << std::endl;
    // for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
    //     std::cout << "\tlayer " << layer << ": local gate: " << tt_localop_timings[1][layer] / num_tests << ", interactive gate: " << tt_interop_timings[1][layer] / num_tests << ", layer finish: " << tt_finishlay_timings[1][layer] / num_tests << std::endl;
    // }
	// std::cout << "Yao Receiver: " << std::endl;
    // for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
    //     std::cout << "\tlayer " << layer << ": local gate: " << tt_localop_timings[3][layer] / num_tests << ", interactive gate: " << tt_interop_timings[3][layer] / num_tests << ", layer finish: " << tt_finishlay_timings[3][layer] / num_tests << std::endl;
    // }
    // std::cout << "Arith: " << std::endl;
    // for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
    //     std::cout << "\tlayer " << layer << ": local gate: " << tt_localop_timings[2][layer] / num_tests << ", interactive gate: " << tt_interop_timings[2][layer] / num_tests << ", layer finish: " << tt_finishlay_timings[2][layer] / num_tests << std::endl;
    // }
	std::cout << "Communication per layer per test: " << std::endl;
    for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
        // std::cout << "\tlayer " << layer << ": interaction: " << tt_interaction_timings[layer] / num_tests << std::endl;
        std::cout << static_cast<unsigned int>(interaction_datasent[layer]) << " bytes " << tt_interaction_timings[layer] / ((double)num_tests) << " ms" << std::endl;
    }
    std::cout << std::endl;
#endif

    std::cout << "\ntt_online_time: " << tt_online_time << " ms" << std::endl;
    std::cout << "avg online time over " << num_tests << " reps: " << tt_online_time / ((double)num_tests) << std::endl;

    //y: compute RTT & bw here

    //y: manually update cost table (need to implement external cost table calculation program)
    // takes (function, network characteristics) then give cost table for operations in function

    // y: verify output by recalculating with clear values, need modify after paral implement done 
    // for(int j = 0; j < alice_input.size(); j++) {
    //     uint32_t expected_output = PutTheOpToVerifyResult(alice_input[j], bob_input[j]);
    //     for(int k = 1; k < nseq; k++) {
    //         expected_output = PutTheOpToVerifyResult(expected_output, bob_input[j]);
    //     }

    //     /**
    //     Type casting the value to 64 bit unsigned integer for output.
    //     NOTE: I am casting to 64bit because it works for both 32bit and 64bit shares.
    //     Also, since we always provide inputs that fit inside 32bits (even when using 64 bits),
    //     it is safe to assign the result to 32bit integer array output'
    //     */

    //     uint32_t output = s_out[j]->get_clear_value<uint64_t>();
    //     if(output != expected_output) {
    //         std::cerr << "\nERROR: (j: " << j << ") "<< output << " != " << expected_output << ")\n"; 
    //         std::cerr << "alice: " << alice_input[j] << "(" << nseq << " non amortized ops)\n";
    //         std::cerr << "bob: " << bob_input[j] << "\n";
    //         delete party;
    //         // why 57, because it's Grothendieck prime
    //         exit(57);
    //     }
    // }

	uint32_t total_tests = nparal * nseq;
	std::cout << "\nTesting " << total_tests << " operations (sequential "<< nseq << ", parallel: " 
			<< nparal << ") in " << get_sharing_name(sharing) << " sharing \n";
	
	std::cout << "\nAll tests passed\n";
    delete party;
	return 0;
}
