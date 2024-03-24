/**
 \file 		shl.h
 \author 	ishaq@ishaq.pk
 \brief     SHL Instruction 
 */

#ifndef __SHL_CLEAR_H_
#define __SHL_CLEAR_H_

#include "../../../abycore/circuit/booleancircuits.h"
#include "../../../abycore/circuit/arithmeticcircuits.h"
#include "../../../abycore/circuit/circuit.h"
#include "../../../abycore/aby/abyparty.h"
#include <math.h>
#include <cassert>

/**
 \param		role 		role played by the program which can be server or client part.
 \param 	andress 	IP Address
 \param 	seclvl 		Security level
 \param 	nvals		Number of values
 \param 	bitlen		Bit length of the inputs
 \param 	nthreads	Number of threads
 \param		mt_alg		The algorithm for generation of multiplication triples
 \param 	sharing		Sharing type object
 \brief		This function is used for running a testing environment the instruction
 */
int32_t test_circuit(e_role role, char* andress, uint16_t port, seclvl seclvl,
		uint32_t nvals, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
		e_sharing sharing, uint32_t num_tests, uint32_t seed);

/**
 \param		s_alice		shared object of alice's input;
 \param		s_bob 		shared object of bob's input.
 \param		bc	 		boolean circuit object.
 \brief		This function is used to build and solve gcd;
 */
share* BuildCircuit(share *s_alice, share* s_bob, BooleanCircuit *bc, uint32_t bitlen);


#endif /* __SHL_CLEAR_H_ */
