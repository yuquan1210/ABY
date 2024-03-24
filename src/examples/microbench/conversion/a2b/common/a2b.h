/**
 \file 		a2b.h
 \author 	ishaq@ishaq.pk
 \brief     A2B
 */

#ifndef __A2B_H_
#define __A2B_H_

#include <abycore/circuit/arithmeticcircuits.h>
#include <abycore/circuit/booleancircuits.h>
#include <abycore/sharing/sharing.h>
#include <abycore/circuit/circuit.h>
#include <abycore/aby/abyparty.h>
#include <math.h>
#include <cassert>

#include "../../../common/cost_calculator.h"

/**
 \param		role 		role played by the program which can be server or client part.
 \param 	andress 	IP Address
 \param 	seclvl 		Security level
 \param 	nvals		Number of values
 \param 	bitlen		Bit length of the inputs
 \param 	nthreads	Number of threads
 \param		mt_alg		The algorithm for generation of multiplication triples
 \brief		This function is used for running a testing environment the instruction
 */
int32_t test_circuit(e_role role, char* andress, uint16_t port, seclvl seclvl,
		uint32_t nvals, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
         uint32_t num_tests, uint32_t seed);

#endif /* __A2B_H_ */
