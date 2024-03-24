/**
 \file 		a2y.cpp
 \author 	ishaq@ishaq.pk
 \brief		A2Y Instruction
 */

#include "a2y.h"
#include<numeric>

int32_t test_circuit(e_role role, char* address, uint16_t port, seclvl seclvl,
		uint32_t nvals, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
		uint32_t num_tests, uint32_t seed) {

	/**
		Step 1: Create the ABYParty object which defines the basis of all the
		 	 	operations which are happening.	Operations performed are on the
		 	 	basis of the role played by this object.
	*/
	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads,
			mt_alg);


	/**
		Step 2: Get to know all the sharing types available in the program.
	*/

	std::vector<Sharing*>& sharings = party->GetSharings();

	/**
		Step 3: Create the circuit object on the basis of the sharing type
				being inputed.
	*/
	Circuit* ac = sharings[S_ARITH]->GetCircuitBuildRoutine();
    Circuit* bc = sharings[S_BOOL]->GetCircuitBuildRoutine();
	Circuit* yc =  sharings[S_YAO]->GetCircuitBuildRoutine();

	/**
		Step 4: Creating the share objects - s_alice_input, s_bob_input which
				is used as input to the computation function. Also s_out
				which stores the output.
	*/

	share **s_alice_input = new share*[num_tests];
	share **s_bob_input = new share*[num_tests];
	share **s_out1 = new share*[num_tests];
	share **s_out2 = new share*[num_tests];

	/**
		Step 5: Initialize Alice's and Bob's input with random values.
				Both parties use the same seed, to be able to verify the
				result. In a real example each party would only supply
				one input value.
	*/

	uint32_t *alice_input = new uint32_t[num_tests];
	uint32_t *bob_input = new uint32_t[num_tests];
	srand(seed);
	for(int i = 0; i < num_tests; i++) {
		alice_input[i] = rand() % 100000;
		bob_input[i] = rand() % 100000;
	}

	/**
		Step 6: Copy the randomly generated money into the respective
				share objects using the circuit object method PutINGate()
				for my inputs and PutDummyINGate() for the other parties input.
				Also mention who is sharing the object.
	*/
	//s_alice_input = circ->PutINGate(alice_input, bitlen, CLIENT);
	//s_bob_input = circ->PutINGate(bob_input, bitlen, SERVER);
	for(int i = 0; i < num_tests; i++) {
		if(role == SERVER) {
			s_alice_input[i] = ac->PutDummyINGate(bitlen);
			s_bob_input[i] = ac->PutINGate(bob_input[i], bitlen, SERVER);
		} else { //role == CLIENT
			s_alice_input[i] = ac->PutINGate(alice_input[i], bitlen, CLIENT);
			s_bob_input[i] = ac->PutDummyINGate(bitlen);
		}
	

		/**
			Step 7: Call the build method for building the circuit for the
					problem by passing the shared objects and circuit object.
					Don't forget to type cast the circuit object to type of share
		*/

		// we are only doing the conversion
		s_out1[i] = yc->PutA2YGate(s_alice_input[i]);
		s_out2[i] = yc->PutA2YGate(s_bob_input[i]);

		/**
			Step 8: Modify the output receiver based on the role played by
					the server and the client. This step writes the output to the
					shared output object based on the role.
		*/
		s_out1[i] = yc->PutOUTGate(s_out1[i], ALL);
		s_out2[i] = yc->PutOUTGate(s_out2[i], ALL);

		/**
			Step 9: Executing the circuit using the ABYParty object evaluate the
					problem.
		*/
	}

    CalculateAllCircuitCost(ac, bc, yc);
	party->ExecCircuit();

	/**
		Step 10:Type casting the value to 32 bit unsigned integer for output.
	*/
	// for(int i = 0; i < num_tests; i++) {
	// 	output[i] = s_out[i]->get_clear_value<uint32_t>();
	// }

	std::cout << "\nTesting " << num_tests << " A2Y operations ";
	// std::cout << "\nAlice's Input:\t" << alice_input;
	// std::cout << "\nBob's Input:\t" << bob_input;
	// std::cout << "\nCircuit Result:\t" << output;
	// std::cout << "\nVerify Result: \t" << (alice_input * bob_input) << "\n";
	for(int i = 0; i < num_tests; i++) {
		if(s_out1[i]->get_clear_value<uint64_t>() != alice_input[i] ||
			s_out2[i]->get_clear_value<uint64_t>() != bob_input[i]) {
			std::cerr << "\nERROR at index " << i << ": s_out1:" << s_out1[i]->get_clear_value<uint64_t>() << " != alice_input:" << alice_input[i]
				<< " || s_out2:"<< s_out2[i]->get_clear_value<uint64_t>() << " != bob_input:" << bob_input[i] << "\n"; 
			delete[] s_alice_input;
			delete[] s_bob_input;
			delete[] s_out1;
			delete[] s_out2;
			delete[] alice_input;
			delete[] bob_input;
			delete party;
			// why 57, because it's Grothendieck prime
			exit(57);
		}
	}

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
