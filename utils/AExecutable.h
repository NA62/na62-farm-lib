/*
 * AExecutable.h
 *
 *  Created on: Jul 22, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef AEXECUTABLE_H_
#define AEXECUTABLE_H_

#include <vector>

#include <boost/thread.hpp>
#include <pthread.h>
#include <iostream>

#include "../options/Logging.h"
namespace na62 {

class AExecutable {
public:
	AExecutable();
	virtual ~AExecutable();

	void startThread(const std::string threadName) {
		threadNum_ = 0;
		thread_ = threads_.create_thread(
				boost::bind(&AExecutable::runThread, this));
		threadName_ = threadName;

//		pthread_setname_np(thread_->native_handle(), threadName.c_str());
	}

	void startThread(unsigned short threadNum, const std::string threadName,
			std::vector<short> CPUMask, int threadPrio,
			int scheduler = 1) {
		threadNum_ = threadNum;
		thread_ = threads_.create_thread(
				boost::bind(&AExecutable::runThread, this));

		SetThreadAffinity(thread_, threadPrio, CPUMask, scheduler);
		threadName_ = threadName;
//		pthread_setname_np(thread_->native_handle(), threadName.c_str());
	}

	void startThread(unsigned short threadNum, const std::string threadName,
			short CPUMask = -1, int threadPrio = 15, int scheduler =
					1) {
		threadNum_ = threadNum;
		thread_ = threads_.create_thread(
				boost::bind(&AExecutable::runThread, this));

		SetThreadAffinity(thread_, threadPrio, CPUMask, scheduler);
		threadName_ = threadName;
//		pthread_setname_np(thread_->native_handle(), threadName.c_str());
	}

	static void SetThreadAffinity(boost::thread* daThread,
			int threadPriority, short CPUToBind, int scheduler);

	static void SetThreadAffinity(boost::thread* daThread,
			int threadPriority, std::vector<short> CPUsToBind,
			int scheduler);

	void join() {
		thread_->join();
	}

	virtual void interrupt() {
		onInterruption();
		thread_->interrupt();
	}

	static void InterruptAll() {
		for (unsigned int i = 0; i < instances_.size(); i++) {
			instances_[i]->onInterruption();
		}

		LOG_INFO << "Interrupting " << instances_.size() << " threads"
				<< ENDL;
		threads_.interrupt_all();
	}

	static void JoinAll() {
		LOG_INFO << "Joining " << instances_.size() << " threads" << ENDL;
		threads_.join_all();
	}

protected:
	short threadNum_;

private:
	void runThread() {
		thread();
	}

	virtual void thread() {
	}

	virtual void onInterruption() {
	}

	boost::thread* thread_;
	std::string threadName_;

	static boost::thread_group threads_;
	static std::vector<AExecutable*> instances_;
};

} /* namespace na62 */
#endif /* AEXECUTABLE_H_ */
