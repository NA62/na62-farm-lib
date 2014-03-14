/*
 * AExecutable.h
 *
 *  Created on: Jul 22, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef AEXECUTABLE_H_
#define AEXECUTABLE_H_

#include <vector>

#include <boost/thread.hpp>

//#include "../options/Options.h"
namespace na62 {

class AExecutable {
public:
	AExecutable();
	virtual ~AExecutable();

	void startThread() {
		threadNum_ = 0;
		thread_ = threads_.create_thread(
				boost::bind(&AExecutable::thread, this));
	}

	void startThread(unsigned short threadNum,
			std::vector<unsigned short> CPUMask, unsigned threadPrio) {
		threadNum_ = threadNum;
		thread_ = threads_.create_thread(
				boost::bind(&AExecutable::thread, this));

//		SetThreadAffinity(thread_, threadPrio, CPUMask, Options::Instance()->SCHEDULER);
	}

	void startThread(unsigned short threadNum, unsigned short CPUMask = -1,
			unsigned threadPrio = 15) {
		threadNum_ = threadNum;
		thread_ = threads_.create_thread(
				boost::bind(&AExecutable::thread, this));

//		SetThreadAffinity(thread_, 15, CPUMask, Options::Instance()->SCHEDULER);
	}

	static void SetThreadAffinity(boost::thread& daThread,
			unsigned short threadPriority, short unsigned CPUToBind,
			int scheduler);

	static void SetThreadAffinity(boost::thread& daThread,
			unsigned short threadPriority,
			std::vector<unsigned short> CPUsToBind, int scheduler);

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

		std::cout << "Interrupting " << instances_.size() << " threads"
				<< std::endl;
		threads_.interrupt_all();
	}

	static void JoinAll() {
		std::cout << "Joining " << instances_.size() << " threads" << std::endl;
		threads_.join_all();
	}

protected:
	short threadNum_;

private:
	virtual void thread() {
	}

	virtual void onInterruption() {
	}

	boost::thread* thread_;
	static boost::thread_group threads_;
	static std::vector<AExecutable*> instances_;
};

} /* namespace na62 */
#endif /* AEXECUTABLE_H_ */
