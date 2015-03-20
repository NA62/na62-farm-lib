/*
 * EventSerializer.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "EventSerializer.h"

#include <sys/types.h>
#include <atomic>
#include <cstring>

#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"
#include "../LKr/LkrFragment.h"
#include "../structs/Event.h"
#include "Event.h"
#include "SourceIDManager.h"

namespace na62 {

uint EventSerializer::InitialEventBufferSize_;
int EventSerializer::TotalNumberOfDetectors_;

void EventSerializer::initialize() {
	/*
	 * L0 sources + LKr
	 */
	if (SourceIDManager::NUMBER_OF_EXPECTED_CREAM_PACKETS_PER_EVENT == 0) {
		TotalNumberOfDetectors_ = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES;
	} else {
		TotalNumberOfDetectors_ = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES
				+ 1;
	}

	if (SourceIDManager::MUV1_NUMBER_OF_FRAGMENTS != 0) {
		TotalNumberOfDetectors_++;
	}

	if (SourceIDManager::MUV2_NUMBER_OF_FRAGMENTS != 0) {
		TotalNumberOfDetectors_++;
	}

	InitialEventBufferSize_ = 1000;
}

char* EventSerializer::ResizeBuffer(char* buffer, const int oldLength,
		const int newLength) {
	char* newBuffer = new char[newLength];
	memcpy(newBuffer, buffer, oldLength);
	delete[] buffer;
	return newBuffer;
}

EVENT_HDR* EventSerializer::SerializeEvent(const Event* event) {
	uint eventBufferSize = InitialEventBufferSize_;
	char* eventBuffer = new char[InitialEventBufferSize_];

	EVENT_HDR* header = reinterpret_cast<EVENT_HDR*>(eventBuffer);

	header->eventNum = event->getEventNumber();
	header->format = 0x62; // TODO: update current format
	// header->length will be written later on
	header->burstID = event->getBurstID();
	header->timestamp = event->getTimestamp();
	header->triggerWord = event->getTriggerTypeWord();
	header->reserved1 = 0;
	header->fineTime = event->getFinetime();
	header->numberOfDetectors = TotalNumberOfDetectors_;
	header->reserved2 = 0;
	header->processingID = event->getProcessingID();
	header->SOBtimestamp = 0; // Will be set by the merger

	uint sizeOfPointerTable = 4 * TotalNumberOfDetectors_;
	uint pointerTableOffset = sizeof(EVENT_HDR);
	uint eventOffset = sizeof(EVENT_HDR) + sizeOfPointerTable;

	for (int sourceNum = 0;
			sourceNum != SourceIDManager::NUMBER_OF_L0_DATA_SOURCES;
			sourceNum++) {
		l0::Subevent* subevent = event->getL0SubeventBySourceIDNum(sourceNum);

		if (eventOffset + 4 > eventBufferSize) {
			eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
					eventBufferSize + 1000);
			eventBufferSize += 1000;
		}

		/*
		 * Put the sub-detector  into the pointer table
		 */
		uint eventOffset32 = eventOffset / 4;
		std::memcpy(eventBuffer + pointerTableOffset, &eventOffset32, 3);
		std::memset(eventBuffer + pointerTableOffset + 3,
				SourceIDManager::sourceNumToID(sourceNum), 1);
		pointerTableOffset += 4;

		/*
		 * Write the L0 data
		 */
		int payloadLength;
		for (uint i = 0; i != subevent->getNumberOfFragments(); i++) {
			l0::MEPFragment* fragment = subevent->getFragment(i);
			payloadLength = fragment->getPayloadLength() + sizeof(L0_BLOCK_HDR);
			if (eventOffset + payloadLength > eventBufferSize) {
				eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
						eventBufferSize + payloadLength);
				eventBufferSize += payloadLength;
			}

			L0_BLOCK_HDR* blockHdr = reinterpret_cast<L0_BLOCK_HDR*>(eventBuffer
					+ eventOffset);
			blockHdr->dataBlockSize = payloadLength;
			blockHdr->sourceSubID = fragment->getSourceSubID();
			blockHdr->reserved = 0;

			memcpy(eventBuffer + eventOffset + sizeof(L0_BLOCK_HDR),
					fragment->getPayload(),
					payloadLength - sizeof(L0_BLOCK_HDR));
			eventOffset += payloadLength;

			/*
			 * 32-bit alignment
			 */
			if (eventOffset % 4 != 0) {
				memset(eventBuffer + eventOffset, 0, eventOffset % 4);
				eventOffset += eventOffset % 4;
			}
		}
	}

	/*
	 * Write the LKr data
	 */
	if (SourceIDManager::NUMBER_OF_EXPECTED_LKR_CREAM_FRAGMENTS != 0) {
		writeCreamData(eventBuffer, eventOffset, eventBufferSize,
				pointerTableOffset, event->getZSuppressedLkrFragments(),
				event->getNumberOfZSuppressedLkrFragments(), SOURCE_ID_LKr);
	}

	if (SourceIDManager::MUV1_NUMBER_OF_FRAGMENTS != 0) {
		writeCreamData(eventBuffer, eventOffset, eventBufferSize,
				pointerTableOffset, event->getMuv1Fragments(),
				event->getNumberOfMuv1Fragments(), SOURCE_ID_MUV1);
	}

	if (SourceIDManager::MUV2_NUMBER_OF_FRAGMENTS != 0) {
		writeCreamData(eventBuffer, eventOffset, eventBufferSize,
				pointerTableOffset, event->getMuv2Fragments(),
				event->getNumberOfMuv2Fragments(), SOURCE_ID_MUV2);
	}

	/*
	 * Trailer
	 */
	EVENT_TRAILER* trailer = (EVENT_TRAILER*) (eventBuffer + eventOffset);
	trailer->eventNum = event->getEventNumber();
	trailer->reserved = 0;

	const int eventLength = eventOffset + 4/*trailer*/;

	if (eventBufferSize > InitialEventBufferSize_) {
		InitialEventBufferSize_ = eventBufferSize;
	}

	/*
	 * header may have been overwritten -> redefine it
	 */
	header = reinterpret_cast<EVENT_HDR*>(eventBuffer);

	header->length = eventLength / 4;

	return header;
}

char* EventSerializer::writeCreamData(char*& eventBuffer, uint& eventOffset,
		uint& eventBufferSize, uint& pointerTableOffset,
		cream::LkrFragment** fragments, uint numberOfFragments, uint sourceID) {
	/*
	 * Write the LKr data
	 */
	if (eventOffset + 4 > eventBufferSize) {
		eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
				eventBufferSize + 1000);
		eventBufferSize += 1000;
	}

	uint eventOffset32 = eventOffset / 4;
	/*
	 * Put the LKr into the pointer table
	 */
	std::memcpy(eventBuffer + pointerTableOffset, &eventOffset32, 3);
	std::memset(eventBuffer + pointerTableOffset + 3, sourceID, 1);
	pointerTableOffset += 4;

	for (uint fragmentNum = 0; fragmentNum != numberOfFragments;
			fragmentNum++) {
		cream::LkrFragment* e = fragments[fragmentNum];

		if (eventOffset + e->getEventLength() > eventBufferSize) {
			eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
					eventBufferSize + e->getEventLength());
			eventBufferSize += e->getEventLength();
		}

		memcpy(eventBuffer + eventOffset, e->getDataWithHeader(),
				e->getEventLength());
		eventOffset += e->getEventLength();

		/*
		 * 32-bit alignment
		 */
		if (eventOffset % 4 != 0) {
			memset(eventBuffer + eventOffset, 0, eventOffset % 4);
			eventOffset += eventOffset % 4;
		}
	}

	return eventBuffer;
}

} /* namespace na62 */
