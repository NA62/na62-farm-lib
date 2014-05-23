/*
 * SleepingQueue.h
 *
 * This class is based on the proposal at following blog post:
 * na62://msmvps.com/blogs/vandooren/archive/2007/01/05/creating-a-thread-safe-producer-consumer-queue-in-c-without-using-locks.aspx
 *
 * A thread safe consumer-producer queue. This means this queue is only thread safe if you have only one writer-thread and only one reader-thread.
 *  Created on: Jan 5, 2012
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#pragma once
#ifndef MULTIPLEXER_H_
#define MULTIPLEXER_H_

#include <boost/thread.hpp>

namespace na62 {

template<class T> class Multiplexer {
private:
	int* notifyCounter_;
	const int threadNum_;
	int RoundRobin_;
	boost::mutex* conditionVarMutex_;
	boost::condition_variable* conditionVars_;

	volatile uint32_t* readPos_;
	volatile uint32_t* writePos_;
	volatile bool* consumerSleeping_;

	uint32_t Size_;
	T** Data_;

	const unsigned int notifiyModolo_;

public:
	Multiplexer(uint32_t size, int consumerThreadNum, unsigned int notifiyModolo) :
			notifyCounter_(new int[consumerThreadNum]), threadNum_(consumerThreadNum), conditionVarMutex_(new boost::mutex[consumerThreadNum]), conditionVars_(
					new boost::condition_variable[consumerThreadNum]), readPos_(new uint32_t[consumerThreadNum]), writePos_(
					new uint32_t[consumerThreadNum]), consumerSleeping_(new bool[consumerThreadNum]), Size_(size / consumerThreadNum), notifiyModolo_(
					notifiyModolo) {
		Data_ = new T*[consumerThreadNum];
		for (int i = 0; i < consumerThreadNum; i++) {
			readPos_[i] = 0;
			writePos_[i] = 0;
			Data_[i] = new T[Size_];
			consumerSleeping_[i] = false;
		}
	}

	~Multiplexer() {
		delete[] notifyCounter_;
		delete[] conditionVarMutex_;
		delete[] conditionVars_;

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
	void push(T &element) throw () {
		if (!trySinglePush(element)) {
			// Queue is full. Try another one
			bool finished = false;
			while (!finished) {
				for (int i = 0; i < threadNum_; i++) {
					finished = trySinglePush(element);
					if (finished) {
						return;
					}
				}
				usleep(1000);
			}
		}
	}

	/*
	 * Push a new element into the circular queue. May only be called by one single thread (producer)!
	 */
	bool trySinglePush(T &element) throw () {
		const short int rr = RoundRobin_++ % threadNum_;
		const uint32_t nextElement = (writePos_[rr] + 1) % Size_;
		if (nextElement != readPos_[rr]) {
			Data_[rr][writePos_[rr]] = element;
			writePos_[rr] = nextElement;

			if (consumerSleeping_[rr] && notifyCounter_[rr]++ % notifiyModolo_ == 0) {
				conditionVars_[rr].notify_one();
			}
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
	void pop(T &element, const short int& thread) throw () {
		while (readPos_[thread] == writePos_[thread]) {
			sleepTillNextPush(10000, thread);
		}

		const uint32_t nextElement = (readPos_[thread] + 1) % Size_;

		element = Data_[thread][readPos_[thread]];
		readPos_[thread] = nextElement;
	}

	void sleepTillNextPush(unsigned int maxSleepMicros, const short int& thread) {
		boost::mutex::scoped_lock lock(conditionVarMutex_[thread]);
		consumerSleeping_[thread] = true;

		if (maxSleepMicros > 0) {
			const boost::system_time waitEnd = boost::get_system_time() + boost::posix_time::microsec(maxSleepMicros);
			conditionVars_[thread].timed_wait(lock, waitEnd);
		} else {
			conditionVars_[thread].wait(lock);
		}
		consumerSleeping_[thread] = false;
	}
};

} /* namespace na62 */
#endif /* MULTIPLEXER_H_ */
