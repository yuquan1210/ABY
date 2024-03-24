#include "../common/common.h"

/**
 * Greater than or equal to. implemented in Boolean Circuit since 
 * GT, EQ and OR are implemented in Boolean Sharing only
 */
share* PutGEGate(share* a, share* b, BooleanCircuit *bc) {
	share *gt = bc->PutGTGate(a, b);
	share *eq = bc->PutEQGate(a, b);
	share *ge = bc->PutORGate(gt, eq);
	return ge;
}

/**
 * remainder. implemented in Boolean Circuit since MUX is
 * implemented in Boolean Sharing only
 */
/*share* PutRemGate(share* val, share* mod, BooleanCircuit *bc) {
	share* rem = NULL;
	int i = 0;
	share *zero = bc->PutCONSGate((UGATE_T)0, val->get_bitlength());
	rem = bc->PutCONSGate((UGATE_T)0, val->get_bitlength());
	for(i = val->get_bitlength()-1; i >= 0; i--) {
		// rem = rem << 1
		rem = bc->PutLeftShifterGate(rem, 1);
		// rem[0] = val[i];
		rem->set_wire_id(0, val->get_wire_id(i));
		// // if (rem >= mod) rem = rem - mod;
		share* ge = PutGEGate(rem, mod, bc);
		share* diff = bc->PutSUBGate(rem, mod);
		rem = bc->PutMUXGate(diff, rem, ge);
	}
	return rem;
}*/

share* PutRemGate(share* val, share* mod, uint32_t bitlen, BooleanCircuit *circ) {
	share* rem = NULL;
	int i = 0;
	share *zero = circ->PutCONSGate((UGATE_T)0, bitlen);
	rem = circ->PutCONSGate((UGATE_T)0, bitlen);

	// MI: bug fix, for cases where val may not be as big as bitlen
	// this is needed because not all gates in ABY respect the bitlen passed
	// to ABYParty at creation
	int val_bitlen = val->get_bitlength();
	if(val_bitlen < bitlen) {
		if(bitlen > val->get_max_bitlength()) val->set_max_bitlength(bitlen);
		val->set_bitlength(bitlen);
		share *zero = circ->PutCONSGate((UGATE_T)0, (bitlen-val_bitlen));

		for(int k = val_bitlen; k < bitlen; k++) {
			val->set_wire_id(k, zero->get_wire_id(k - val_bitlen));
		}
	}

	for(i = bitlen-1; i >= 0; i--) {
		// rem = rem << 1
		rem = circ->PutLeftShifterGate(rem, 1);
		// rem[0] = val[i];
		rem->set_wire_id(0, val->get_wire_id(i));
		// // if (rem >= mod) rem = rem - mod;
		share* ge = PutGEGate(rem, mod, circ); // CUSTOM GATE
		share* diff = circ->PutSUBGate(rem, mod);
		rem = circ->PutMUXGate(diff, rem, ge);
	}
	return rem;
}

share* PutTheGateForBenchmark(share* a, share* b, Circuit* circ, 
	uint32_t bitlen) {
	BooleanCircuit* bc = dynamic_cast<BooleanCircuit*>(circ);
	assert(bc != NULL);
	share *out = PutRemGate(a, b, bitlen, bc);
	return out;
}
uint32_t PutTheOpToVerifyResult(uint32_t a, uint32_t b) {
	return a % b;
}