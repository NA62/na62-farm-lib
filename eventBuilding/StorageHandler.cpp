/*
 * StorageHandler.cpp
 *
 *  Created on: Mar 4, 2014
 *      Author: root
 */

#include "StorageHandler.h"

#include <sys/types.h>
#include <zmq.h>
#include <string>

#include "../options/Options.h"
#include "../socket/ZMQHandler.h"

namespace na62 {

std::vector<zmq::socket_t*> StorageHandler::MergerSockets_;

void StorageHandler::Initialize() {
	for (int i = 0; i < Options::GetInt(OPTION_NUMBER_OF_EBS); i++) {
		zmq::socket_t* sock = ZMQHandler::GenerateSocket(ZMQ_PUSH);
		MergerSockets_.push_back(sock);
		sock->connect(ZMQHandler::GetMergerAddress().c_str());
	}
}

int StorageHandler::SendEvent(const uint16_t& thredNum, Event* event) {
	char* eventBuffer_ = new char[Options::Instance()->MAX_EVENT_LENGTH];

	struct EVENT_HDR* header = (struct EVENT_HDR*) eventBuffer_;

	header->eventNum = event->getEventNumber();
	header->format = 0x62; // TODO: update current format
	// header->length will be written later on;
	header->burstID = event->getBurstID();
	header->timestamp = event->getTimestamp();
	header->triggerWord = event->getTriggerTypeWord();
	header->reserved1 = 0;
	header->fineTime = event->getFinetime();
	header->numberOfDetectors = Options::GetTotalNumberOfDetectors();
	header->reserved2 = 0;
	header->processingID = event->getProcessingID();
	header->SOBtimestamp = 0; // Will be set by the merger

	uint32_t sizeOfPointerTable = 4 * Options::GetTotalNumberOfDetectors();
	uint32_t pointerTableOffset = sizeof(struct EVENT_HDR);
	uint32_t eventOffset = sizeof(struct EVENT_HDR) + sizeOfPointerTable;

	for (int sourceNum = Options::Instance()->NUMBER_OF_L0_DATA_SOURCES - 1;
			sourceNum >= 0; sourceNum--) {
		l0::Subevent* subevent = event->getL0SubeventBySourceIDNum(sourceNum);

		if (eventOffset + 4 > Options::Instance()->MAX_EVENT_LENGTH) {
			throw na62::NA62Error(
					"StorageHandler: Trying create too large event. Try raising --"
							+ std::string(OPTION_MAX_EVENT_LENGTH));
		}

		/*
		 * Put the sub-detector  into the pointer table
		 */
		uint32_t eventOffset32 = eventOffset / 4;
		std::memcpy(eventBuffer_ + pointerTableOffset, &eventOffset32, 3);
		std::memset(eventBuffer_ + pointerTableOffset + 3,
				Options::sourceNumToID(sourceNum), 1);
		pointerTableOffset += 4;

		/*
		 * Write the L0 data
		 */
		int payloadLength;
		for (int i = subevent->getNumberOfParts() - 1; i >= 0; i--) {
			l0::MEPEvent* e = subevent->getPart(i);
			payloadLength = e->getEventLength()
					- sizeof(struct l0::MEPEVENT_RAW_HDR)
					+ sizeof(struct L0_BLOCK_HDR);
			if (eventOffset + payloadLength
					> Options::Instance()->MAX_EVENT_LENGTH) {
				throw na62::NA62Error(
						"StorageHandler: Trying to create a too large event. Try raising --"
								+ std::string(OPTION_MAX_EVENT_LENGTH));
			}

			struct L0_BLOCK_HDR* blockHdr = (struct L0_BLOCK_HDR*) (eventBuffer_
					+ eventOffset);
			blockHdr->dataBlockSize = payloadLength;
			blockHdr->sourceSubID = e->getSourceSubID();
			blockHdr->reserved = 0;

			memcpy(eventBuffer_ + eventOffset + sizeof(struct L0_BLOCK_HDR),
					e->getData(), payloadLength - sizeof(struct L0_BLOCK_HDR));
			eventOffset += payloadLength;

			/*
			 * 32-bit alignment
			 */
			if (eventOffset % 4 != 0) {
				memset(eventBuffer_ + eventOffset, 0, eventOffset % 4);
				eventOffset += eventOffset % 4;
			}
		}
	}

	/*
	 * Write the LKr data
	 */
	if (eventOffset + 4 > Options::Instance()->MAX_EVENT_LENGTH) {
		throw na62::NA62Error(
				"StorageHandler: Trying create too large event. Try raising --"
						+ std::string(OPTION_MAX_EVENT_LENGTH));
	}

	if (Options::Instance()->NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT > 0) {
		uint32_t eventOffset32 = eventOffset / 4;
		/*
		 * Put the LKr into the pointer table
		 */
		std::memcpy(eventBuffer_ + pointerTableOffset, &eventOffset32, 3);
		std::memset(eventBuffer_ + pointerTableOffset + 3, LKR_SOURCE_ID, 1); // 0x24 is the LKr sourceID

		for (int localCreamID = event->getNumberOfZSuppressedLKrEvents() - 1;
				localCreamID >= 0; localCreamID--) {
			cream::LKREvent* e = event->getZSuppressedLKrEvent(localCreamID);

			if (eventOffset + e->getEventLength()
					> Options::Instance()->MAX_EVENT_LENGTH) {
				throw na62::NA62Error(
						"StorageHandler: Trying create too large event. Try raising --"
								+ std::string(OPTION_MAX_EVENT_LENGTH));
			}

			memcpy(eventBuffer_ + eventOffset, e->getDataWithHeader(),
					e->getEventLength());
			eventOffset += e->getEventLength();

			/*
			 * 32-bit alignment
			 */
			if (eventOffset % 4 != 0) {
				memset(eventBuffer_ + eventOffset, 0, eventOffset % 4);
				eventOffset += eventOffset % 4;
			}
		}
	}

	/*
	 * Trailer
	 */
	EVENT_TRAILER* trailer = (EVENT_TRAILER*) (eventBuffer_ + eventOffset);
	trailer->eventNum = event->getEventNumber();
	trailer->reserved = 0;

	header->length = eventOffset / 4 + 1/*trailer*/;
	while (!events_[thredNum].push(header)) {
		mycout("StorageHandler queue overrun!");
		usleep(100);
	}

	return header->length * 4;
}
} /* namespace na62 */
