/*
 * SleepingQueue.h
 *
 * This class is based on the proposal at following blog post:
 * na62://msmvps.com/blogs/vandooren/archive/2007/01/05/creating-a-thread-safe-producer-consumer-queue-in-c-without-using-locks.aspx
 *
 * A thread safe consumer-producer queue. This means this queue is only thread safe if you have only one writer-thread and only one reader-thread.
 *  Created on: Jan 5, 2012
 *      Author: Jonas Kunze (kunzej@cern.ch)
 */

#pragma once
#ifndef DEMULTIPLEXER_H_
#define DEMULTIPLEXER_H_

#include <boost/thread.hpp>

namespace na62 {

template<class T> class Demultiplexer {
private:
	int notifyCounter_;
	const int threadNum_;
	int RoundRobin_;
	boost::mutex conditionVarMutex_;
	boost::condition_variable* conditionVar_;

	volatile uint32_t* readPos_;
	volatile uint32_t* writePos_;
	volatile bool consumerSleeping_;

	uint32_t Size_;
	T** Data_;

	const unsigned int notifiyModolo_;

public:
	Demultiplexer(uint32_t size, int consumerThreadNum, unsigned int notifiyModolo, boost::condition_variable* conditonVariable =
			new boost::condition_variable()) :
			threadNum_(consumerThreadNum), conditionVar_(conditonVariable), readPos_(new uint32_t[consumerThreadNum]), writePos_(
					new uint32_t[consumerThreadNum]), consumerSleeping_(false), Size_(size / consumerThreadNum), notifiyModolo_(notifiyModolo) {
		Data_ = new T*[consumerThreadNum];
		for (int i = 0; i < consumerThreadNum; i++) {
			readPos_[i] = 0;
			writePos_[i] = 0;
			Data_[i] = new T[Size_];
		}
	}

	~Demultiplexer() {
		for (int i = 0; i < threadNum_; i++) {
			delete[] Data_[i];
		}
		delete[] Data_;
		delete[] readPos_;
		delete[] readPos_;
	}

	/*
	 * Push a new element into the circular queue. May only be called by one single thread (producer)!
	 */
	void push(T &element, const short int& thread) throw () {
		const uint32_t nextElement = (writePos_[thread] + 1) % Size_;
		while (true) {
			if (nextElement != readPos_[thread]) {
				Data_[thread][writePos_[thread]] = element;
				writePos_[thread] = nextElement;

				if (notifyCounter_++ % notifiyModolo_ == 0) {
					conditionVar_->notify_one();
				}
				return;
			}
			conditionVar_->notify_one();
			usleep(1000);
		}
	}

	bool trySinglePop(T &element) throw () {
		const short int rr = RoundRobin_++ % threadNum_;
		if (readPos_[rr] != writePos_[rr]) {
			const uint32_t nextElement = (readPos_[rr] + 1) % Size_;

			element = Data_[rr][readPos_[rr]];
			readPos_[rr] = nextElement;
			return true;
		}
		return false;
	}

	/*
	 * remove the oldest element from the circular queue. May only be called by one single thread (consumer)!
	 *
	 * @param maxSleepMicros unsigned int If this value is <0> and the queue is empty the thread will sleep until the
	 * next push. Else the thread will
	 */
	void pop(T &element) throw () {
		if (!trySinglePop(element)) {
			// Queue is full. Try another one
			bool finished = false;
			while (!finished) {
				for (int i = 0; i < threadNum_; i++) {
					finished = trySinglePop(element);
					if (finished) {
						return;
					}
				}
				sleepTillNextPush(0);
			}
		}

	}

	void sleepTillNextPush(unsigned int maxSleepMicros) {
		boost::mutex::scoped_lock lock(conditionVarMutex_);
		consumerSleeping_ = true;

		if (maxSleepMicros > 0) {
			const boost::system_time waitEnd = boost::get_system_time() + boost::posix_time::microsec(maxSleepMicros);
			conditionVar_->timed_wait(lock, waitEnd);
		} else {
			conditionVar_->wait(lock);
		}
		consumerSleeping_ = false;
	}
}
;

} /* namespace na62 */
#endif /* DEMULTIPLEXER_H_ */
