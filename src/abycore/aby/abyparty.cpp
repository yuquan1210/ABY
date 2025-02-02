/**
 \file 		abyparty.cpp
 \author	michael.zohner@ec-spride.de
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

 \brief		ABYParty class implementation.
 */

#include "abyparty.h"
#include "abysetup.h"
#include "../circuit/abycircuit.h"
#include "../sharing/arithsharing.h"
#include "../sharing/boolsharing.h"
#include "../sharing/sharing.h"
#include "../sharing/splut.h"
#include "../sharing/yaoclientsharing.h"
#include "../sharing/yaoserversharing.h"
#include <ENCRYPTO_utils/crypto/crypto.h>
#include <ENCRYPTO_utils/connection.h>
#include <ENCRYPTO_utils/thread.h>

#include <cstdlib>
#include <mutex>
#include <sstream>

#ifdef _DEBUG
#include <cassert>
#endif

class ABYParty::CPartyWorkerThread: public CThread {
public:
	CPartyWorkerThread(uint32_t id, ABYParty* callback) :
			threadid(id), m_pCallback(callback) {
		m_eJob = e_Party_Undefined;
	};

	void PutJob(EPartyJobType e) {
		std::lock_guard<std::mutex> lock(m_eJob_mutex_);
		m_eJob = e;
		m_evt.Set();
	}

	CEvent* GetEvent() {
		return &m_evt;
	}
private:
	void ThreadMain();
	uint32_t threadid;
	ABYParty* m_pCallback;
	CEvent m_evt;
	EPartyJobType m_eJob;
	std::mutex m_eJob_mutex_;
};

ABYParty::ABYParty(e_role pid, const std::string& addr, uint16_t port, seclvl seclvl,
	uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mg_algo,
	uint32_t reservegates, const std::string& abycircdir)
	: m_cCrypt(std::make_unique<crypto>(seclvl.symbits)), glock(std::make_unique<CLock>()),
	m_eMTGenAlg(mg_algo), m_eRole(pid), m_nNumOTThreads(nthreads),
	m_tComm(std::make_unique<comm_ctx>()),
	m_pSetup(std::make_unique<ABYSetup>(m_cCrypt.get(), m_nNumOTThreads, m_eRole, m_eMTGenAlg)),
	m_nPort(port), m_sSecLvl(seclvl),
	m_cAddress(addr) {

	StartWatch("Initialization", P_INIT);

#if BENCH_HARDWARE
	bench_aes();
#endif

	//m_aSeed = (uint8_t*) malloc(sizeof(uint8_t) * m_cCrypt->get_hash_bytes());

#ifdef DEBUGABYPARTY
	std::cout << "Performing Init" << std::endl;
#endif

	m_evt = std::make_unique<CEvent>();
	m_lock = std::make_unique<CLock>();

	Init();

	m_pCircuit = NULL;
	StopWatch("Time for initiatlization: ", P_INIT);

#ifdef DEBUGABYPARTY
	std::cout << "Generating circuit" << std::endl;
#endif
	StartWatch("Generating circuit", P_CIRCUIT);
	if (!InitCircuit(bitlen, reservegates, abycircdir)) {
		std::cout << "There was an while initializing the circuit, ending! " << std::endl;
		std::exit(EXIT_FAILURE);
	}
	StopWatch("Time for circuit generation: ", P_CIRCUIT);
}

void ABYParty::ConnectAndBaseOTs() {
	if (!is_online) {
#ifdef DEBUGABYPARTY
		std::cout << "Establishing network connection" << std::endl;
#endif
		//Establish network connection
		StartWatch("Establishing network connection: ", P_NETWORK);
		if (!EstablishConnection()) {
			std::cout << "There was an error during establish connection, ending! " << std::endl;
			std::exit(EXIT_FAILURE);
		}
		StopWatch("Time for network connect: ", P_NETWORK);

#ifdef DEBUGABYPARTY
		std::cout << "Performing base OTs" << std::endl;
#endif
		/* Pre-Compute Naor-Pinkas base OTs by starting two threads */

        uint64_t actual_snd_begin_0 = this->m_vSockets[0]->getSndCnt();
        uint64_t actual_snd_begin_1 = this->m_vSockets[1]->getSndCnt();
		StartRecording("Starting NP OT", P_BASE_OT, m_vSockets);
		m_pSetup->PrepareSetupPhase(m_tComm.get());
		StopRecording("Time for NP OT: ", P_BASE_OT, m_vSockets);
        uint64_t actual_snd_end_0 = this->m_vSockets[0]->getSndCnt();
        uint64_t actual_snd_end_1 = this->m_vSockets[1]->getSndCnt();
#ifdef DEBUGABYPARTY       
		std::cout << "[ABYParty::ConnectAndBaseOT] actual sent during m_pSetup->PrepareSetupPhase() thru sock0: " << (actual_snd_end_0 - actual_snd_begin_0) << " byte" << std::endl;
        std::cout << "[ABYParty::ConnectAndBaseOT] actual sent during m_pSetup->PrepareSetupPhase() thru sock1: " << (actual_snd_end_1 - actual_snd_begin_1) << " byte" << std::endl;
#endif
		is_online = true;
	}
}

ABYParty::~ABYParty() {
	m_vSharings[S_BOOL]->PreCompFileDelete();
	Cleanup();
}

std::vector<Sharing*>& ABYParty::GetSharings() {
	return m_vSharings;
}

BOOL ABYParty::Init() {
	//Threads that support execution by e.g. concurrent sending / receiving
	m_nHelperThreads = 2;

	//m_vSockets.resize(m_nNumOTThreads * 2);
	m_vSockets.resize(2);

	m_vThreads.resize(m_nHelperThreads);
	for (uint32_t i = 0; i < m_nHelperThreads; i++) {
		m_vThreads[i] = new CPartyWorkerThread(i, this); //First thread is started as receiver, second as sender
		m_vThreads[i]->Start();
	}

	m_nMyNumInBits = 0;

	return TRUE;
}

void ABYParty::Cleanup() {
	// free any gates that are still instantiated
	for(size_t i = 0; i < m_pCircuit->GetGateHead(); i++) {
		if((*m_vGates)[i].instantiated) {
			m_vSharings[0]->FreeGate(&(*m_vGates)[i]);
		}
	}
	for(uint32_t i = 0; i < S_LAST; i++) {
		if(m_vSharings[i]) {
			delete m_vSharings[i];
		}
	}

	// clean circuit after sharings because sharing destructors need
	// access to the circuit structure.
	if (m_pCircuit) {
		delete m_pCircuit;
	}

	for (uint32_t i = 0; i < m_nHelperThreads; i++) {
		m_vThreads[i]->PutJob(e_Party_Stop);
		m_vThreads[i]->Wait();
		delete m_vThreads[i];
	}
}

void ABYParty::ExecCircuit() {

#ifdef DEBUGABYPARTY
	std::cout << "Finishing circuit generation" << std::endl;
#endif

	ConnectAndBaseOTs();

	StartRecording("Starting execution", P_TOTAL, m_vSockets);

	//Setup phase
	StartRecording("Starting setup phase: ", P_SETUP, m_vSockets);
	for (uint32_t i = 0; i < m_vSharings.size(); i++) {
#ifdef DEBUGABYPARTY
		std::cout << "Preparing setup phase for " << m_vSharings[i]->sharing_type() << " sharing" << std::endl;
#endif
		m_vSharings[i]->PrepareSetupPhase(m_pSetup.get());
	}

#ifdef DEBUGABYPARTY
	std::cout << "Preforming OT extension" << std::endl;
#endif
    uint64_t performSetup_actual_snd_begin_0 = m_vSockets[0]->getSndCnt();
    uint64_t performSetup_actual_snd_begin_1 = m_vSockets[1]->getSndCnt();
	StartRecording("Starting OT Extension", P_OT_EXT, m_vSockets);
	m_pSetup->PerformSetupPhase();
	StopRecording("Time for OT Extension phase: ", P_OT_EXT, m_vSockets);
    uint64_t performSetup_actual_snd_end_0 = m_vSockets[0]->getSndCnt();
    uint64_t performSetup_actual_snd_end_1 = m_vSockets[1]->getSndCnt();
#ifdef DEBUGABYPARTY    
	std::cout << "Actual sent during m_pSetup->PrepareSetupPhase() thru sock0: " << (performSetup_actual_snd_end_0 - performSetup_actual_snd_begin_0) << " byte" << std::endl;
    std::cout << "Actual sent during m_pSetup->PrepareSetupPhase() thru sock1: " << (performSetup_actual_snd_end_1 - performSetup_actual_snd_begin_1) << " byte" << std::endl;
#endif

	for (uint32_t i = 0; i < m_vSharings.size(); i++) {
#ifdef DEBUGABYPARTY
		std::cout << "Performing setup phase for " << m_vSharings[i]->sharing_type() << " sharing" << std::endl;
#endif
		if(i == S_YAO) {
			StartRecording("Starting Circuit Garbling", P_GARBLE, m_vSockets);
			if(m_eRole == SERVER) {
				m_vSharings[S_YAO]->PerformSetupPhase(m_pSetup.get());
				m_vSharings[S_YAO_REV]->PerformSetupPhase(m_pSetup.get());
			} else {
				m_vSharings[S_YAO_REV]->PerformSetupPhase(m_pSetup.get());
				m_vSharings[S_YAO]->PerformSetupPhase(m_pSetup.get());
			}
			/*m_vSharings[S_YAO]->PerformSetupPhase(m_pSetup.get());
			m_vSharings[S_YAO_REV]->PerformSetupPhase(m_pSetup.get());*/
			m_vSharings[S_YAO]->FinishSetupPhase(m_pSetup.get());
			m_vSharings[S_YAO_REV]->FinishSetupPhase(m_pSetup.get());
			StopRecording("Time for Circuit garbling: ", P_GARBLE, m_vSockets);
		} else if (i == S_YAO_REV) {
			//Do nothing, was done in parallel to Yao
		} else {
			m_vSharings[i]->PerformSetupPhase(m_pSetup.get());
			m_vSharings[i]->FinishSetupPhase(m_pSetup.get());
		}

	}
	StopRecording("Time for setup phase: ", P_SETUP, m_vSockets);

#ifdef DEBUGABYPARTY
	std::cout << "Evaluating circuit" << std::endl;
#endif

	//Online phase
	if(m_vSharings[S_BOOL]->GetPreCompPhaseValue() != ePreCompStore) {
		StartRecording("Starting online phase: ", P_ONLINE, m_vSockets);
		EvaluateCircuit();
		StopRecording("Time for online phase: ", P_ONLINE, m_vSockets);
	}

	StopRecording("Total Time: ", P_TOTAL, m_vSockets);

#ifdef PRINT_OUTPUT
	//Print input and output gates
	PrintInput();
	PrintOutput();
#endif


#if PRINT_PERFORMANCE_STATS
	PrintPerformanceStatistics();
#endif

#if PRINT_COMMUNICATION_STATS
	PrintCommunication();
#endif
}


BOOL ABYParty::InitCircuit(uint32_t bitlen, uint32_t reservegates, const std::string& abycircdir) {
	// Default reserved gates in abyparty.h constructur
	m_pCircuit = new ABYCircuit(reservegates);

	m_vSharings.resize(S_LAST);
	m_vSharings[S_BOOL] = new BoolSharing(S_BOOL, m_eRole, 1, m_pCircuit, m_cCrypt.get(), abycircdir);
	if (m_eRole == SERVER) {
		m_vSharings[S_YAO] = new YaoServerSharing(S_YAO, SERVER, m_sSecLvl.symbits, m_pCircuit, m_cCrypt.get(), abycircdir);
		m_vSharings[S_YAO_REV] = new YaoClientSharing(S_YAO_REV, CLIENT, m_sSecLvl.symbits, m_pCircuit, m_cCrypt.get(), abycircdir);
	}
	else {
		m_vSharings[S_YAO] = new YaoClientSharing(S_YAO, CLIENT, m_sSecLvl.symbits, m_pCircuit, m_cCrypt.get(), abycircdir);
		m_vSharings[S_YAO_REV] = new YaoServerSharing(S_YAO_REV, SERVER, m_sSecLvl.symbits, m_pCircuit, m_cCrypt.get(), abycircdir);
	}
	switch (bitlen) {
	case 8:
		m_vSharings[S_ARITH] = new ArithSharing<uint8_t>(S_ARITH, m_eRole, 1, m_pCircuit, m_cCrypt.get(), m_eMTGenAlg);
		break;
	case 16:
		m_vSharings[S_ARITH] = new ArithSharing<uint16_t>(S_ARITH, m_eRole, 1, m_pCircuit, m_cCrypt.get(), m_eMTGenAlg);
		break;
	case 32:
		m_vSharings[S_ARITH] = new ArithSharing<uint32_t>(S_ARITH, m_eRole, 1, m_pCircuit, m_cCrypt.get(), m_eMTGenAlg);
		break;
	case 64:
		m_vSharings[S_ARITH] = new ArithSharing<uint64_t>(S_ARITH, m_eRole, 1, m_pCircuit, m_cCrypt.get(), m_eMTGenAlg);
		break;
	default:
		m_vSharings[S_ARITH] = new ArithSharing<uint32_t>(S_ARITH, m_eRole, 1, m_pCircuit, m_cCrypt.get(), m_eMTGenAlg);
		break;
	}
	m_vSharings[S_SPLUT] = new SetupLUT(S_SPLUT, m_eRole, 1, m_pCircuit, m_cCrypt.get(), abycircdir);

	m_vGates = &(m_pCircuit->GatesVec());

#ifdef DEBUGABYPARTY
	std::cout << " circuit initialized..." << std::endl;
#endif

	return TRUE;
}

BOOL ABYParty::EvaluateCircuit() {
#if BENCHONLINEPHASE
	timespec tstart, tend;
	uint32_t num_sharings = m_vSharings.size();
	double interaction = 0;
	double currInteraction = 0;
	std::vector<double> localops(num_sharings,0);
	std::vector<double> interactiveops(num_sharings,0);
	std::vector<double> fincirclayer(num_sharings,0);

	std::vector<double> currLocalops(num_sharings,0);
	std::vector<double> currInteractiveops(num_sharings,0);
	std::vector<double> currFincirclayer(num_sharings,0);
#endif
	m_nDepth = 0;

	m_tPartyChan = new channel(ABY_PARTY_CHANNEL, m_tComm->rcv_std.get(), m_tComm->snd_std.get());

	for (uint32_t i = 0; i < m_vSharings.size(); i++) {
		m_vSharings[i]->PrepareOnlinePhase();
	}

	uint32_t maxdepth = 0;

	for (uint32_t i = 0; i < m_vSharings.size(); i++) {
		maxdepth = std::max(maxdepth, m_vSharings[i]->GetMaxCommunicationRounds());
	}
#ifdef DEBUGABYPARTY
	std::cout << "[ABYParty::EvaluateCircuit] Starting online evaluation with maxdepth = " << maxdepth << std::endl;
#endif
	//Evaluate Circuit layerwise;
	for (uint32_t depth = 0; depth < maxdepth; depth++, m_nDepth++) {
#ifdef DEBUGABYPARTY
        std::cout << "\n******************************************************************" << std::endl;
		std::cout << "[ABYParty::EvaluateCircuit] Starting evaluation on depth " << depth << std::endl << std::flush;
#endif
		for (uint32_t i = 0; i < m_vSharings.size(); i++) {
#ifdef DEBUGABYPARTY
			std::cout << "[ABYParty::EvaluateCircuit] Evaluating local operations of sharing " << i << " on depth " << depth << std::endl;
#endif
#if BENCHONLINEPHASE
			clock_gettime(CLOCK_MONOTONIC, &tstart);
#endif
			m_vSharings[i]->EvaluateLocalOperations(depth);
#if BENCHONLINEPHASE
			clock_gettime(CLOCK_MONOTONIC, &tend);
			SaveLocalOpTiming(getMillies(tstart, tend), i);
			currLocalops[i] = getMillies(tstart, tend);
			localops[i] += getMillies(tstart, tend);
			clock_gettime(CLOCK_MONOTONIC, &tstart);
			// StartWatch("start inter op", sharingInterops[i]);
#endif
#ifdef DEBUGABYPARTY
			std::cout << "[ABYParty::EvaluateCircuit] Evaluating interactive operations of sharing " << i << std::endl;
#endif
			m_vSharings[i]->EvaluateInteractiveOperations(depth);
#if BENCHONLINEPHASE
			clock_gettime(CLOCK_MONOTONIC, &tend);
			SaveInterOpTiming(getMillies(tstart, tend), i);
			interactiveops[i] += getMillies(tstart, tend);
			currInteractiveops[i] = getMillies(tstart, tend);
#endif
		}
#ifdef DEBUGABYPARTY
		std::cout << "[ABYParty::EvaluateCircuit] Finished with evaluating operations on depth = " << depth << ", continuing with interactions" << std::endl;
#endif
#if BENCHONLINEPHASE
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		// StartWatch("start interact", sharingInterops[i]);
#endif
		PerformInteraction();
#if BENCHONLINEPHASE
		clock_gettime(CLOCK_MONOTONIC, &tend);
		// SaveInteractionTiming(getMillies(tstart, tend));
		interaction += getMillies(tstart, tend);
		currInteraction = getMillies(tstart, tend);
#endif
#ifdef DEBUGABYPARTY
		std::cout << "[ABYParty::EvaluateCircuit] Done performing interaction, having sharings wrap up this circuit layer" << std::endl;
#endif
		for (uint32_t i = 0; i < m_vSharings.size(); i++) {
#if BENCHONLINEPHASE
			clock_gettime(CLOCK_MONOTONIC, &tstart);
#endif
			//std::cout << "Finishing circuit layer for sharing "<< i << std::endl;
			m_vSharings[i]->FinishCircuitLayer();
#if BENCHONLINEPHASE
			clock_gettime(CLOCK_MONOTONIC, &tend);
			SaveFinishLayerTiming(getMillies(tstart, tend), i);
			fincirclayer[i] += getMillies(tstart, tend);
			currFincirclayer[i] = getMillies(tstart, tend);
#endif
		}
#if BENCHONLINEPHASE
		// std::cout << "\nOnline time at layer " << depth << ": " << std::endl;
		// std::cout << "Bool: local gates: " << currLocalops[S_BOOL] << ", interactive gates: " << currInteractiveops[S_BOOL] << ", layer finish: " << currFincirclayer[S_BOOL] << std::endl;
		// std::cout << "Yao: local gates: " << currLocalops[S_YAO] << ", interactive gates: " << currInteractiveops[S_YAO] << ", layer finish: " << currFincirclayer[S_YAO] << std::endl;
		// std::cout << "Yao Rev: local gates: " << currLocalops[S_YAO_REV] << ", interactive gates: " << currInteractiveops[S_YAO_REV] << ", layer finish: " << currFincirclayer[S_YAO_REV] << std::endl;
		// std::cout << "Arith: local gates: " << currLocalops[S_ARITH] << ", interactive gates: " << currInteractiveops[S_ARITH] << ", layer finish: " << currFincirclayer[S_ARITH] << std::endl;
		// std::cout << "SPLUT: local gates: " << currLocalops[S_SPLUT] << ", interactive gates: " << currInteractiveops[S_SPLUT] << ", layer finish: " << currFincirclayer[S_SPLUT] << std::endl;
		// std::cout << "Communication: " << currInteraction << std::endl << std::endl;
#endif
	}
#ifdef DEBUGABYPARTY
		std::cout << "[ABYParty::EvaluateCircuit] Done with online phase; synchronizing "<< std::endl;
#endif
	m_tPartyChan->synchronize_end();
	delete m_tPartyChan;

#if BENCHONLINEPHASE
	// std::cout << "Online time is distributed as follows: " << std::endl;
	// std::cout << "Bool: local gates: " << localops[S_BOOL] << ", interactive gates: " << interactiveops[S_BOOL] << ", layer finish: " << fincirclayer[S_BOOL] << std::endl;
	// std::cout << "Yao: local gates: " << localops[S_YAO] << ", interactive gates: " << interactiveops[S_YAO] << ", layer finish: " << fincirclayer[S_YAO] << std::endl;
	// std::cout << "Yao Rev: local gates: " << localops[S_YAO_REV] << ", interactive gates: " << interactiveops[S_YAO_REV] << ", layer finish: " << fincirclayer[S_YAO_REV] << std::endl;
	// std::cout << "Arith: local gates: " << localops[S_ARITH] << ", interactive gates: " << interactiveops[S_ARITH] << ", layer finish: " << fincirclayer[S_ARITH] << std::endl;
	// std::cout << "SPLUT: local gates: " << localops[S_SPLUT] << ", interactive gates: " << interactiveops[S_SPLUT] << ", layer finish: " << fincirclayer[S_SPLUT] << std::endl;
	// std::cout << "Communication: " << interaction << std::endl << std::endl;
#endif

	double RTT = 0;
	double bw = 0;
	estimate_network(GetSendTimingsFromTimer(), GetSendDataSizeFromTimer(), GetRcvTimingsFromTimer(), GetRcvDataSizeFromTimer(), maxdepth, RTT, bw);
	std::cout << "\n***Estimating over " << maxdepth << " communications" << std::endl;
	std::cout << "RTT: " << RTT << " ms" << std::endl;
	std::cout << "bw: " << bw << " bits/ms" << std::endl << std::endl;
	return true;
}

BOOL ABYParty::PerformInteraction() {
	WakeupWorkerThreads(e_Party_Comm);
	BOOL success = WaitWorkerThreads();
	return success;
}

BOOL ABYParty::ThreadSendValues(uint32_t id) {
	std::vector<std::vector<BYTE*> >sendbuf(m_vSharings.size());
	std::vector<std::vector<uint64_t> >sndbytes(m_vSharings.size());

	uint64_t snd_buf_size_total = 0, ctr = 0;
	for (uint32_t j = 0; j < m_vSharings.size(); j++) {
		m_vSharings[j]->GetDataToSend(sendbuf[j], sndbytes[j]);
		for (uint32_t i = 0; i < sendbuf[j].size(); i++) {
			snd_buf_size_total += sndbytes[j][i];
			//m_tPartyChan->send(sendbuf[j][i], sndbytes[j][i]);
#ifdef DEBUGCOMM
			cout_mutex.lock();
			std::cout << "[PerformInter->ABYParty::ThreadSendVal] (depth: " << m_nDepth << ") Sending " << sndbytes[j][i] << " bytes on socket " << m_eRole << " for sharing " << j << std::endl;
			cout_mutex.unlock();
#endif
		}
		//sendbuf[j].clear();
		//sndbytes[j].clear();
	}
	uint8_t* snd_buf_total = (uint8_t*) malloc(snd_buf_size_total);
	for (uint32_t j = 0; j < m_vSharings.size(); j++) {
		for (uint32_t i = 0; i < sendbuf[j].size(); i++) {
			if(sndbytes[j][i] > 0) {
				memcpy(snd_buf_total+ctr, sendbuf[j][i], sndbytes[j][i]);
				ctr+= sndbytes[j][i];
			}
		}
	}
	//gettimeofday(&tstart, NULL);
    // y: get socket start count 
	timespec start, end;
    uint64_t actual_snd_begin = this->m_vSockets[0]->getSndCnt();
	double sent_time = 0;
	uint64_t sent_data_size = 0;
	if(snd_buf_size_total > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		//m_vSockets[2]->Send(snd_buf_total, snd_buf_size_total);
		m_tPartyChan->blocking_send(m_vThreads[id]->GetEvent(), snd_buf_total, snd_buf_size_total);
		clock_gettime(CLOCK_MONOTONIC, &end);
		sent_time = getMillies(start, end);
		sent_data_size = snd_buf_size_total+9;
	}
	std::cout << "\nSEND \nData sent: " << sent_data_size << " bytes" << std::endl;
	std::cout << "Time spent: " << sent_time << " ms" << std::endl;
	std::cout << "Throughput: " << sent_data_size / sent_time << " Byte/ms" << std::endl  << std::endl;
	SaveSendTiming(m_nDepth, sent_time);
	SaveSendDataSize(m_nDepth, sent_data_size);
    // y: print actual sent vs. sent buf total here
    uint64_t actual_snd_end = this->m_vSockets[0]->getSndCnt();
#ifdef DEBUGCOMM
    std::cout << "[PerformInter->ABYParty::ThreadSendVal] snd_buf_size_total: " << snd_buf_size_total << " byte" << std::endl;
    std::cout << "[PerformInter->ABYParty::ThreadSendVal] actual sent thru sock0: " << (actual_snd_end - actual_snd_begin) << " byte" << std::endl;
#endif	
	free(snd_buf_total);

	return true;
}

BOOL ABYParty::ThreadReceiveValues() {
	std::vector<std::vector<BYTE*> > rcvbuf(m_vSharings.size());
	std::vector<std::vector<uint64_t> > rcvbytes(m_vSharings.size());

//	timeval tstart, tend;

	uint64_t rcvbytestotal = 0;
	for (uint32_t j = 0; j < m_vSharings.size(); j++) {
		m_vSharings[j]->GetBuffersToReceive(rcvbuf[j], rcvbytes[j]);
		for (uint32_t i = 0; i < rcvbuf[j].size(); i++) {
			rcvbytestotal += rcvbytes[j][i];
			//	m_tPartyChan->blocking_receive(sendbuf[j][i], sndbytes[j][i]);
#ifdef DEBUGCOMM
			cout_mutex.lock();
			std::cout << "[PerformInter->ABYParty::ThreadRcvVal] (depth: " << m_nDepth << ") Receiving " << rcvbytes[j][i] << " bytes on socket " << (m_eRole^1) << " for sharing " << j << std::endl;
			cout_mutex.unlock();
#endif
		}
	}
	uint8_t* rcvbuftotal = (uint8_t*) malloc(rcvbytestotal);
	assert(rcvbuftotal != NULL);
	//gettimeofday(&tstart, NULL);
	timespec start, end;
	double rcv_time = 0;
	uint64_t rcv_datasize = 0;
	if (rcvbytestotal > 0) {
		//m_vSockets[2]->Receive(rcvbuftotal, rcvbytestotal);
		clock_gettime(CLOCK_MONOTONIC, &start);
		m_tPartyChan->blocking_receive(rcvbuftotal, rcvbytestotal);
		clock_gettime(CLOCK_MONOTONIC, &end);
		rcv_time = getMillies(start, end);
		rcv_datasize = rcvbytestotal+9;
	}
	std::cout << "\nRCV \nData to receive: " << rcv_datasize << " bytes" << std::endl;
	std::cout << "Time spent: " << rcv_time << " ms" << std::endl;
	std::cout << "Throughput: " << rcv_datasize / rcv_time << " Byte/ms" << std::endl;
	SaveRcvTiming(m_nDepth, rcv_time);
	SaveRcvDataSize(m_nDepth, rcv_datasize);
	//gettimeofday(&tend, NULL);
	//std::cout << "(" << m_nDepth << ") Time taken for receiving " << rcvbytestotal << " bytes: " << getMillies(tstart, tend) << std::endl;

	for (uint32_t j = 0, ctr = 0; j < m_vSharings.size(); j++) {
		for (uint32_t i = 0; i < rcvbuf[j].size(); i++) {
			if (rcvbytes[j][i] > 0) {
				memcpy(rcvbuf[j][i], rcvbuftotal + ctr, rcvbytes[j][i]);
				ctr += rcvbytes[j][i];
			}
		}
	}
	free(rcvbuftotal);

	for (uint32_t j = 0; j < m_vSharings.size(); j++) {
		rcvbuf[j].clear();
		rcvbytes[j].clear();
	}
	rcvbuf.clear();
	rcvbytes.clear();

	return true;
}


void ABYParty::PrintPerformanceStatistics() {
	std::cout << "Complexities: " << std::endl;
	m_vSharings[S_BOOL]->PrintPerformanceStatistics();
	m_vSharings[S_YAO]->PrintPerformanceStatistics();
	m_vSharings[S_YAO_REV]->PrintPerformanceStatistics();
	m_vSharings[S_ARITH]->PrintPerformanceStatistics();
	m_vSharings[S_SPLUT]->PrintPerformanceStatistics();
	std::cout << "Total number of gates: " << m_pCircuit->GetGateHead() << " Total depth: " << m_pCircuit->GetTotalDepth() << std::endl;
	PrintTimings();
	std::cout << std::endl;
}

void ABYParty::bench_aes() const {
	timespec bench_start, bench_end;
	AES_KEY_CTX kGarble;
	m_cCrypt->init_aes_key(&kGarble, (uint8_t*) m_vFixedKeyAESSeed);

	uint64_t bench_aes_len = 128 * 1024 * 1024; // 128 MiB blocks
	uint32_t bench_aes_rounds = 8;

	std::vector<BYTE> bench_outp(bench_aes_len + AES_BYTES);

	clock_gettime(CLOCK_MONOTONIC, &bench_start);

	for (uint32_t ctr = 0; ctr < bench_aes_rounds; ++ctr) {
		m_cCrypt->encrypt(&kGarble, bench_outp.data(), bench_outp.data(), bench_aes_len);
	}

	clock_gettime(CLOCK_MONOTONIC, &bench_end);

	double bench_time = getMillies(bench_start, bench_end);
	std::cout << "AES performance: " << ((bench_aes_len >> 20) / (bench_time / 1000)) * bench_aes_rounds << " MiB/sec" << std::endl;
}

//=========================================================
// Connection Routines
BOOL ABYParty::EstablishConnection() {
	BOOL success = false;
	if (m_eRole == SERVER) {
		 #ifdef DEBUGABYPARTY
		 std::cout << "Server starting to listen" << std::endl;
		 #endif
		success = ABYPartyListen();
	} else { //CLIENT
		success = ABYPartyConnect();

	}
	if (!success)
		return false;

	m_tComm->snd_std = std::make_unique<SndThread>(m_vSockets[0].get(), glock.get());
	m_tComm->rcv_std = std::make_unique<RcvThread>(m_vSockets[0].get(), glock.get());

	m_tComm->snd_inv = std::make_unique<SndThread>(m_vSockets[1].get(), glock.get());
	m_tComm->rcv_inv = std::make_unique<RcvThread>(m_vSockets[1].get(), glock.get());

	m_tComm->snd_std->Start();
	m_tComm->snd_inv->Start();

	m_tComm->rcv_std->Start();
	m_tComm->rcv_inv->Start();
	return true;
}

//Interface to the connection method
BOOL ABYParty::ABYPartyConnect() {
	//Will open m_vSockets.size new sockets to
	return Connect(m_cAddress, m_nPort, m_vSockets, static_cast<uint32_t>(m_eRole));
}

//Interface to the listening method
BOOL ABYParty::ABYPartyListen() {
	std::vector<std::vector<std::unique_ptr<CSocket>> > tempsocks(2);

	for(uint32_t i = 0; i < 2; i++) {
		tempsocks[i].resize(m_vSockets.size());
	}

	bool success = Listen(m_cAddress, m_nPort, tempsocks, m_vSockets.size(), static_cast<uint32_t>(m_eRole));
	for(uint32_t i = 0; i < m_vSockets.size(); i++) {
		m_vSockets[i] = std::move(tempsocks[1][i]);
	}
	return success;
}

// TODO: are InstantiateGate and UsedGate needed in ABYParty? They don't
// seem to get used anywhere
void ABYParty::InstantiateGate(uint32_t gateid) {
	(*m_vGates)[gateid].gs.val = (UGATE_T*) malloc(sizeof(UGATE_T) * (ceil_divide((*m_vGates)[gateid].nvals, GATE_T_BITS)));
}

void ABYParty::UsedGate(uint32_t gateid) {
	//Decrease the number of further uses of the gate
	(*m_vGates)[gateid].nused--;
	//If the gate is needed in another subsequent gate, delete it
	if (!(*m_vGates)[gateid].nused) {
		free((*m_vGates)[gateid].gs.val);

	}
}

void ABYParty::Reset() {
	m_pSetup->Reset();
	m_nDepth = 0;
	m_nMyNumInBits = 0;

	// free any gates that are still instantiated
	for(size_t i = 0; i < m_pCircuit->GetGateHead(); i++) {
		if((*m_vGates)[i].instantiated) {
			m_vSharings[0]->FreeGate(&(*m_vGates)[i]);
		}
	}
	for (uint32_t i = 0; i < m_vSharings.size(); i++) {
		m_vSharings[i]->Reset();
	}

	clearTimings();

	m_pCircuit->Reset();
}

double ABYParty::GetTiming(ABYPHASE phase) {
	return GetTimeForPhase(phase);
}

std::vector<std::vector<double>> ABYParty::GetLocalOpTimings() {
	return GetLocalOpTimingsFromTimer();
}

std::vector<std::vector<double>> ABYParty::GetInterOpTimings() {
	return GetInterOpTimingsFromTimer();
}

std::vector<std::vector<double>> ABYParty::GetFinishLayerTimings() {
	return GetFinishLayerTimingsFromTimer();
}

std::map<uint32_t, double> ABYParty::GetSendTimings() {
	return GetSendTimingsFromTimer();
}

std::map<uint32_t, uint64_t> ABYParty::GetSendDataSize() {
	return GetSendDataSizeFromTimer();
}

std::map<uint32_t, double> ABYParty::GetRcvTimings() {
	return GetRcvTimingsFromTimer();
}

std::map<uint32_t, uint64_t> ABYParty::GetRcvDataSize() {
	return GetRcvDataSizeFromTimer();
}

uint64_t ABYParty::GetSentData(ABYPHASE phase) {
	return GetSentDataForPhase(phase);
}

uint64_t ABYParty::GetReceivedData(ABYPHASE phase) {
	return GetReceivedDataForPhase(phase);
}

uint32_t ABYParty::GetTotalGates() {
	return m_pCircuit->GetGateHead();
}

uint32_t ABYParty::GetTotalDepth() {
	return m_pCircuit->GetTotalDepth();
}

//===========================================================================
// Thread Management
BOOL ABYParty::WakeupWorkerThreads(EPartyJobType e) {
	m_bWorkerThreadSuccess = TRUE;

	m_nWorkingThreads = 2;
	uint32_t n = m_nWorkingThreads;

	for (uint32_t i = 0; i < n; i++)
		m_vThreads[i]->PutJob(e);

	return TRUE;
}

BOOL ABYParty::WaitWorkerThreads() {
	{
		std::lock_guard<CLock> lock(*m_lock);
		if (!m_nWorkingThreads)
			return TRUE;
	}

	for (;;) {
		m_lock->Lock();
		uint32_t n = m_nWorkingThreads;
		m_lock->Unlock();
		if (!n)
			return m_bWorkerThreadSuccess;
		m_evt->Wait();
	}
	return m_bWorkerThreadSuccess;
}

BOOL ABYParty::ThreadNotifyTaskDone(BOOL bSuccess) {
	m_lock->Lock();
	uint32_t n = --m_nWorkingThreads;
	if (!bSuccess)
		m_bWorkerThreadSuccess = FALSE;
	m_lock->Unlock();

	if (!n)
		m_evt->Set();
	return TRUE;
}

void ABYParty::CPartyWorkerThread::ThreadMain() {
	BOOL bSuccess = FALSE;
	for (;;) {
		m_evt.Wait();

		EPartyJobType job;
		{
			std::lock_guard<std::mutex> lock(m_eJob_mutex_);
			job = m_eJob;
		}

		switch (job) {
		case e_Party_Stop:
			return;
		case e_Party_Comm:
			if (threadid == 0){
				bSuccess = m_pCallback->ThreadSendValues(threadid);
			}
			else{
				bSuccess = m_pCallback->ThreadReceiveValues();
			}
			break;
		case e_Party_Undefined:
		default:
			std::cerr << "Error: Unhandled Thread Job!" << std::endl;
		}

		m_pCallback->ThreadNotifyTaskDone(bSuccess);
	}
}
