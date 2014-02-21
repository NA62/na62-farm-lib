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
#ifndef SLEEPINGQUEUE_H_
#define SLEEPINGQUEUE_H_

#include <boost/thread.hpp>

namespace na62 {

template<class T> class SleepingQueue {
private:
	bool consumerIsSleeping;
	int notifyCounter;
	boost::mutex conditionVarMutex;

	boost::condition_variable* conditionVar_;
	volatile uint32_t readPos_;
	volatile uint32_t writePos_;
	uint32_t Size_;
	T* Data;

	const unsigned int notifiyModolo_;

public:
	SleepingQueue(uint32_t size, unsigned int notifiyModolo) :
			consumerIsSleeping(false), conditionVar_(new boost::condition_variable()), Size_(size), notifiyModolo_(notifiyModolo) {
		readPos_ = 0;
		writePos_ = 0;
		Data = new T[Size_];
	}

	SleepingQueue(uint32_t size, boost::condition_variable* conditionVar, unsigned int notifiyModolo) :
			consumerIsSleeping(false), conditionVar_(conditionVar), Size_(size), notifiyModolo_(notifiyModolo) {
		readPos_ = 0;
		writePos_ = 0;
		Data = new T[Size_];
	}

	~SleepingQueue() {
		delete[] Data;
	}

	/*
	 * Push a new element into the circular queue. May only be called by one single thread (producer)!
	 */
	bool push(T &element) throw () {
		const uint32_t nextElement = (writePos_ + 1) % Size_;
		if (nextElement != readPos_) {
			Data[writePos_] = element;
			writePos_ = nextElement;

			if (consumerIsSleeping && notifyCounter++ % notifiyModolo_ == 0) {
				conditionVar_->notify_one();
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
	void pop(T &element, int maxWaitMicros = 1000) throw () {
		while (readPos_ == writePos_) {
			sleepTillNextPush(maxWaitMicros);
		}

		const uint32_t nextElement = (readPos_ + 1) % Size_;

		element = Data[readPos_];
		readPos_ = nextElement;
	}

	/*
	 * remove the oledest element from the circular queue. May only be called by one single thread (consumer)!
	 *
	 * @return bool <true> if an element has been found and written to <&element>, <false> if the queue was empty
	 */
	bool tryPop(T &element, int maxWaitMicros = 1000) throw () {
		if (readPos_ == writePos_) {
			if (maxWaitMicros == 0) {
				return false;
			}
			sleepTillNextPush(maxWaitMicros);
			if (readPos_ == writePos_) {
				return false;
			}
		}

		const uint32_t nextElement = (readPos_ + 1) % Size_;

		element = Data[readPos_];
		readPos_ = nextElement;
		return true;
	}

	void sleepTillNextPush(unsigned int maxWaitMicros) {
		if (readPos_ == writePos_) {
			boost::mutex::scoped_lock lock(conditionVarMutex);
			consumerIsSleeping = true;

			if (maxWaitMicros > 0) {
				const boost::system_time waitEnd = boost::get_system_time() + boost::posix_time::microsec(maxWaitMicros);
				conditionVar_->timed_wait(lock, waitEnd);
			} else {
				conditionVar_->wait(lock);
			}
			consumerIsSleeping = false;
		}
	}
};

} /* namespace na62 */
#endif /* SLEEPINGQUEUE_H_ */
