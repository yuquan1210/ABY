/**
 \file 		SHL.cpp
 \author 	ishaq@ishaq.pk
 \brief		SHL Instruction
 */

#include "shl.h"
#include "../../../abycore/circuit/arithmeticcircuits.h"
#include "../../../abycore/circuit/booleancircuits.h"
#include "../../../abycore/sharing/sharing.h"
#include<numeric>

share* BuildCircuit(share *s_alice, uint32_t n, BooleanCircuit *circ, uint32_t bitlen) {
	assert(n > 0 && n < bitlen);
	share* out = circ->PutLeftShifterGate(s_alice, n);
	return out;
}

int32_t test_circuit(e_role role, char* address, uint16_t port, seclvl seclvl,
		uint32_t nvals, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
		e_sharing sharing, uint32_t num_tests, uint32_t seed) {

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
	Circuit* circ = sharings[sharing]->GetCircuitBuildRoutine();


	/**
		Step 4: Creating the share objects - s_alice_input, s_bob_input which
				is used as input to the computation function. Also s_out
				which stores the output.
	*/

	share **s_alice_input = new share*[num_tests];
	share **s_bob_input = new share*[num_tests];
	share **s_out = new share*[num_tests];

	/**
		Step 5: Initialize Alice's and Bob's input with random values.
				Both parties use the same seed, to be able to verify the
				result. In a real example each party would only supply
				one input value.
	*/

	uint32_t *alice_input = new uint32_t[num_tests];
	uint32_t *bob_input = new uint32_t[num_tests];
	uint32_t *output = new uint32_t[num_tests];
	srand(seed);
	for(int i = 0; i < num_tests; i++) {
		// truncating at 10000 to prevent overflow
		alice_input[i] = rand() % 10000;
		bob_input[i] = (rand() % (bitlen - 1)) + 1;
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
			s_alice_input[i] = circ->PutDummyINGate(bitlen);
			s_bob_input[i] = circ->PutINGate(bob_input[i], bitlen, SERVER);
		} else { //role == CLIENT
			s_alice_input[i] = circ->PutINGate(alice_input[i], bitlen, CLIENT);
			s_bob_input[i] = circ->PutDummyINGate(bitlen);
		}
	

		/**
			Step 7: Call the build method for building the circuit for the
					problem by passing the shared objects and circuit object.
					Don't forget to type cast the circuit object to type of share
		*/

		s_out[i] = BuildCircuit(s_alice_input[i], bob_input[i], (BooleanCircuit*) circ, bitlen);

		/**
			Step 8: Modify the output receiver based on the role played by
					the server and the client. This step writes the output to the
					shared output object based on the role.
		*/
		s_out[i] = circ->PutOUTGate(s_out[i], ALL);

		/**
			Step 9: Executing the circuit using the ABYParty object evaluate the
					problem.
		*/
	}
	party->ExecCircuit();

	/**
		Step 10:Type casting the value to 64 bit unsigned integer for output.
		NOTE: I am casting to 64bit because it works for both 32bit and 64bit shares.
		Also, since we always provide inputs that fit inside 32bits (even when using 64 bits),
		it is safe to assign the result to 32bit integer array output'
	*/
	for(int i = 0; i < num_tests; i++) {
		output[i] = s_out[i]->get_clear_value<uint64_t>();
	}

	std::cout << "\nTesting " << num_tests << " operations in " << get_sharing_name(sharing)
				<< " sharing: ";
	// std::cout << "\nAlice's Input:\t" << alice_input;
	// std::cout << "\nBob's Input:\t" << bob_input;
	// std::cout << "\nCircuit Result:\t" << output;
	// std::cout << "\nVerify Result: \t" << (alice_input * bob_input) << "\n";
	for(int i = 0; i < num_tests; i++) {
		if(output[i] != (alice_input[i] << bob_input[i])) {
			std::cerr << "\nERROR at index " << i << ": " << output[i] << " != " << (alice_input[i] << bob_input[i]) 
				<< " ("<< alice_input[i] << " << " << bob_input[i] << ")\n"; 
			delete[] s_alice_input;
			delete[] s_bob_input;
			delete[] s_out;
			delete[] alice_input;
			delete[] bob_input;
			delete[] output;
			delete party;
			// why 57, because it's Grothendieck prime
			exit(57);
		}
	}

	std::cout << "\nAll tests passed\n";

	delete[] s_alice_input;
	delete[] s_bob_input;
	delete[] s_out;
	delete[] alice_input;
	delete[] bob_input;
	delete[] output;
	delete party;
	return 0;
}
