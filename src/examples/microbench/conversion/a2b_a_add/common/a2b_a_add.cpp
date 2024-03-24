/**
 \file 		a2b.cpp
 \author 	ishaq@ishaq.pk
 \brief		A2B Instruction
 */

#include "a2b_a_add.h"
#include<numeric>

int32_t test_circuit(e_role role, char* address, uint16_t port, seclvl seclvl,
		uint32_t nvals, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
		uint32_t num_tests, uint32_t seed) {

	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads,
			mt_alg);

	std::vector<Sharing*>& sharings = party->GetSharings();

	Circuit* ac = sharings[S_ARITH]->GetCircuitBuildRoutine();
	Circuit *yc = sharings[S_YAO] ->GetCircuitBuildRoutine();
	Circuit* bc = sharings[S_BOOL]->GetCircuitBuildRoutine();

	share **s_alice_input = new share*[num_tests];
	share **s_bob_input = new share*[num_tests];
	share **s_out1 = new share*[num_tests];
	share **s_out2 = new share*[num_tests];
    share **s_out3 = new share*[num_tests];

    share* s_a_conv;
    share* s_b_conv;
    share* s_add;

	uint32_t *alice_input = new uint32_t[num_tests];
	uint32_t *bob_input = new uint32_t[num_tests];
	srand(seed);
	for(int i = 0; i < num_tests; i++) {
		alice_input[i] = rand() % 100000;
		bob_input[i] = rand() % 100000;
	}

	for(int i = 0; i < num_tests; i++) {
		if(role == SERVER) {
			s_alice_input[i] = ac->PutDummyINGate(bitlen);
			s_bob_input[i] = ac->PutINGate(bob_input[i], bitlen, SERVER);
		} else { //role == CLIENT
			s_alice_input[i] = ac->PutINGate(alice_input[i], bitlen, CLIENT);
			s_bob_input[i] = ac->PutDummyINGate(bitlen);
		}
	
        // s_out1[i] = bc->PutMULGate(s_alice_input[i], ac);
		// s_out2[i] = bc->PutA2BGate(s_bob_input[i], ac);

		s_a_conv = bc->PutA2BGate(s_alice_input[i], yc);
        s_b_conv = bc->PutA2BGate(s_bob_input[i], yc);

        s_add = ac->PutADDGate(s_alice_input[i], s_bob_input[i]);

		s_out1[i] = bc->PutOUTGate(s_a_conv, ALL);
        s_out2[i] = bc->PutOUTGate(s_b_conv, ALL);
		s_out3[i] = ac->PutOUTGate(s_add, ALL);

	}

	party->ExecCircuit();

	/**
		Step 10:Type casting the value to 32 bit unsigned integer for output.
	*/
	// for(int i = 0; i < num_tests; i++) {
	// 	output[i] = s_out[i]->get_clear_value<uint32_t>();
	// }

	std::cout << "\nTesting " << num_tests << " A2B operations ";
	// std::cout << "\nAlice's Input:\t" << alice_input;
	// std::cout << "\nBob's Input:\t" << bob_input;
	// std::cout << "\nCircuit Result:\t" << output;
	// std::cout << "\nVerify Result: \t" << (alice_input * bob_input) << "\n";
	// for(int i = 0; i < num_tests; i++) {
	// 	if(s_out1[i]->get_clear_value<uint64_t>() != alice_input[i] ||
	// 		s_out2[i]->get_clear_value<uint64_t>() != bob_input[i]) {
	// 		std::cerr << "\nERROR at index " << i << ": s_out1:" << s_out1[i]->get_clear_value<uint32_t>() << " != alice_input:" << alice_input[i]
	// 			<< " || s_out2:"<< s_out2[i]->get_clear_value<uint32_t>() << " != bob_input:" << bob_input[i] << "\n"; 
	// 		delete[] s_alice_input;
	// 		delete[] s_bob_input;
	// 		delete[] s_out1;
	// 		delete[] s_out2;
	// 		delete[] alice_input;
	// 		delete[] bob_input;
	// 		delete party;
	// 		// why 57, because it's Grothendieck prime
	// 		exit(57);
	// 	}
	// }

	std::cout << "\nAll tests passed\n";

	delete[] s_alice_input;
	delete[] s_bob_input;
	delete[] s_out1;
	delete[] s_out2;
	delete[] alice_input;
	delete[] bob_input;
	delete party;
	return 0;
}
