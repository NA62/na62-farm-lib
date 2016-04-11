/*
 * AExecutable2.cpp
 *
 *  Created on: Jul 22, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "AExecutable.h"

#include <stdio.h>

#include "../exceptions/NA62Error.h"


namespace na62 {
boost::thread_group AExecutable::threads_;
std::vector<AExecutable*> AExecutable::instances_;

AExecutable::AExecutable() :
		threadNum_(-1) , thread_(nullptr){
	instances_.push_back(this);
}

AExecutable::~AExecutable() {
	delete thread_;
}

void AExecutable::SetThreadAffinity(boost::thread* daThread, int threadPriority, short CPUToBind, int scheduler) {
	std::vector<short> CPUsToBind;
	CPUsToBind.push_back(CPUToBind);
	SetThreadAffinity(daThread, threadPriority, CPUsToBind, scheduler);
}

void AExecutable::SetThreadAffinity(boost::thread* daThread, int threadPriority, std::vector<short> CPUsToBind, int scheduler) {
#ifndef __APPLE__
	int policy;
	pthread_t threadID = (pthread_t) (daThread->native_handle());
	if (scheduler > 0) {

		sched_param param;
		if (pthread_getschedparam(threadID, &policy, &param) != 0) {
			perror("pthread_getschedparam");
			exit(EXIT_FAILURE);
		}

		//LOG_ERROR << "Policy " << policy << ", priority " << param.sched_priority ;
		/**
		 * Set scheduling algorithm
		 * Possible values: SCHED_FIFO(1), SCHED_RR(2), SCHED_OTHER(0)
		 */
		policy = scheduler;
		param.__sched_priority = threadPriority;
		if (pthread_setschedparam(threadID, policy, &param) != 0) {
			perror("pthread_setschedparam");
			exit(EXIT_FAILURE);
		}
	}

	if (CPUsToBind.size() > 0) {
		/**
		 * Bind the thread to CPUs from CPUsToBind
		 */
		cpu_set_t mask;
		CPU_ZERO(&mask);

		for (unsigned int i = 0; i < CPUsToBind.size(); i++) {
			if (CPUsToBind[i] == -1) {
				CPU_ZERO(&mask);
				break;
			}
			CPU_SET(CPUsToBind[i], &mask);
		}

		if (pthread_setaffinity_np(threadID, sizeof(mask), &mask) < 0) {
			throw NA62Error("Unable to bind threads to specific CPUs!");
		}
	}
#endif
}

} /* namespace na62 */
