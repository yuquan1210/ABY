#include "../common/common.h"

share* PutTheGateForBenchmark(share* a, share* b, Circuit* circ, 
	uint32_t bitlen) {
	BooleanCircuit* bc = dynamic_cast<BooleanCircuit*>(circ);
	assert(bc != NULL);
	std::vector<uint32_t> res = bc->PutBarrelLeftShifterGate(a->get_wires(), b->get_wires());
	std::reverse(res.begin(), res.end());
	res.erase(res.begin(), res.begin() + a->get_wires().size());
	std::reverse(res.begin(), res.end());
	return new boolshare(res, bc);
}
uint32_t PutTheOpToVerifyResult(uint32_t a, uint32_t b) {
	return a << b;
}