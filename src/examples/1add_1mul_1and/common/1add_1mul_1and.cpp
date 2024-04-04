/**
 \file 		millionaire_prob.cpp
 \author 	sreeram.sadasivam@cased.de
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
 \brief		Implementation of the millionaire problem using ABY Framework.
 */

#include "common.h"
#include "../../../abycore/circuit/booleancircuits.h"
#include "../../../abycore/sharing/sharing.h"

int32_t test_millionaire_prob_circuit(e_role role, const std::string& address, uint16_t port, seclvl seclvl,
		uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg, e_sharing sharing) {

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
	Circuit* yc = sharings[S_YAO] ->GetCircuitBuildRoutine();
	Circuit* bc = sharings[S_BOOL]->GetCircuitBuildRoutine();
	Circuit* circ = sharings[sharing]->GetCircuitBuildRoutine();


	/**
		Step 4: Creating the share objects - s_alice_money, s_bob_money which
				is used as input to the computation function. Also s_out
				which stores the output.
	*/

	share *s_alice_money, *s_bob_money, *s_out;

	/**
		Step 5: Initialize Alice's and Bob's money with random values.
				Both parties use the same seed, to be able to verify the
				result. In a real example each party would only supply
				one input value.
	*/

	uint32_t alice_money, bob_money, output;
	srand(time(NULL));
	alice_money = rand();
	bob_money = rand();

	/**
		Step 6: Copy the randomly generated money into the respective
				share objects using the circuit object method PutINGate()
				for my inputs and PutDummyINGate() for the other parties input.
				Also mention who is sharing the object.
	*/
	//s_alice_money = circ->PutINGate(alice_money, bitlen, CLIENT);
	//s_bob_money = circ->PutINGate(bob_money, bitlen, SERVER);
	if(role == SERVER) {
		s_alice_money = circ->PutDummyINGate(bitlen);
		s_bob_money = circ->PutINGate(bob_money, bitlen, SERVER);
	} else { //role == CLIENT
		s_alice_money = circ->PutINGate(alice_money, bitlen, CLIENT);
		s_bob_money = circ->PutDummyINGate(bitlen);
	}

	/**
		Step 7: Call the build method for building the circuit for the
				problem by passing the shared objects and circuit object.
				Don't forget to type cast the circuit object to type of share
	*/

	s_out = BuildMillionaireProbCircuit(s_alice_money, s_bob_money, circ);

	/**
		Step 8: Modify the output receiver based on the role played by
				the server and the client. This step writes the output to the
				shared output object based on the role.
	*/
	s_out = circ->PutOUTGate(s_out, ALL);

	/**
		Step 9: Executing the circuit using the ABYParty object evaluate the
				problem.
	*/

    CalculateAllCircuitCost(ac, bc, yc);
	party->ExecCircuit();

	/**
		Step 10:Type casting the value to 32 bit unsigned integer for output.
	*/
	output = s_out->get_clear_value<uint32_t>();

	// std::cout << "Testing Millionaire's Problem in " << get_sharing_name(sharing)
	// 			<< " sharing: " << std::endl;
	// std::cout << "\nAlice Money:\t" << alice_money;
	// std::cout << "\nBob Money:\t" << bob_money;
	// std::cout << "\nCircuit Result:\t" << (output ? ALICE : BOB);
	// std::cout << "\nVerify Result: \t" << ((alice_money > bob_money) ? ALICE : BOB)
	// 			<< "\n";
	std::vector<std::vector<double>> tt_localop_timings(5);
    std::vector<std::vector<double>> tt_interop_timings(5);
    std::vector<std::vector<double>> tt_finishlay_timings(5);
    std::vector<double> tt_interaction_timings;
    double tt_online_time = 0.0;
    double online_time;
	std::vector<std::vector<double>> localop_timings = party->GetLocalOpTimings();
	std::vector<std::vector<double>> interop_timings = party->GetInterOpTimings();
	std::vector<std::vector<double>> finishlay_timings = party->GetFinishLayerTimings();
	std::vector<double> interaction_timings = party->GetInteractionTimings();
	for(int sharing = 0; sharing < 5; sharing++){
		tt_localop_timings[sharing] = localop_timings[sharing];
		tt_interop_timings[sharing] = interop_timings[sharing];
		tt_finishlay_timings[sharing] = finishlay_timings[sharing];
	}
	tt_interaction_timings = interaction_timings;
	online_time = party->GetTiming(P_ONLINE);
	tt_online_time += online_time;
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
	std::cout << "Communication: " << std::endl;
    for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
        std::cout << tt_interaction_timings[layer] << std::endl;
    }
    std::cout << std::endl;
    std::cout << "\ntt_online_time: " << tt_online_time << " ms" << std::endl;
    // std::cout << "avg online time over " << num_tests << " reps: " << tt_online_time / ((double)num_tests) << std::endl;
	delete party;
	return 0;
}

share* BuildMillionaireProbCircuit(share *s_alice, share *s_bob, Circuit *circ) {

	share* add_out;
    share* mul_out;
	share* and_out;
	share* and2_out;

	/** Calling the greater than equal function in the Boolean circuit class.*/
	add_out = circ->PutADDGate(s_alice, s_bob);

    mul_out = circ->PutMULGate(add_out, s_bob);

	and_out = circ->PutANDGate(mul_out, s_bob);
	and2_out = circ->PutANDGate(and_out, s_bob);

	return and2_out;
}
