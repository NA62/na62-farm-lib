/*
 * EventSerializer.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "EventSerializer.h"

#include <cstring>
#include "../options/Logging.h"
#include "../eventBuilding/Event.h"
#include "../eventBuilding/SourceIDManager.h"
#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"
#include "../l1/MEPFragment.h"
#include "../l1/Subevent.h"
#include "../structs/Event.h"
#include "../structs/Versions.h"

namespace na62 {

uint EventSerializer::InitialEventBufferSize_;
int EventSerializer::TotalNumberOfDetectors_;
bool isUnfinishedEOB;

void EventSerializer::initialize() {
	/*
	 * L0 + L1 sources
	 */
	TotalNumberOfDetectors_ = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES + SourceIDManager::NUMBER_OF_L1_DATA_SOURCES ;
	InitialEventBufferSize_ = 4096; // allocate 4 kB initially for the serialized event
	isUnfinishedEOB = false;
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

	isUnfinishedEOB = false;
	EVENT_HDR* header = reinterpret_cast<EVENT_HDR*>(eventBuffer);

	header->eventNum = event->getEventNumber();
	header->formatVersion = EVENT_HDR_FORMAT_VERSION; // TODO: update current format
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

	writeL0Data(event, eventBuffer, eventOffset, eventBufferSize,
			pointerTableOffset);

	writeL1Data(event, eventBuffer, eventOffset, eventBufferSize,
			pointerTableOffset);

	/*
	 * Trailer
	 */
       if (eventOffset + sizeof(EVENT_TRAILER) > eventBufferSize) {
                        eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
                                        eventBufferSize + sizeof(EVENT_TRAILER));
                        eventBufferSize += sizeof(EVENT_TRAILER);
                }


	EVENT_TRAILER* trailer = (EVENT_TRAILER*) (eventBuffer + eventOffset);
	trailer->eventNum = event->getEventNumber();
	trailer->reserved = 0;

	const int eventLength = eventOffset + sizeof(EVENT_TRAILER);

	if (eventBufferSize > InitialEventBufferSize_) {
		InitialEventBufferSize_ = eventBufferSize;
	}

	/*
	 * header may have been overwritten -> redefine it
	 */
	header = reinterpret_cast<EVENT_HDR*>(eventBuffer);
	if (isUnfinishedEOB) header->triggerWord = 0xfefe23;

	header->length = eventLength / 4;

	return header;
}

char* EventSerializer::writeL0Data(const Event* event, char*& eventBuffer, uint& eventOffset,
uint& eventBufferSize, uint& pointerTableOffset) {
	/*
	 * Write all L0 data sources
	 */
	for (int sourceNum = 0; sourceNum != SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; sourceNum++) {
		const l0::Subevent* const subevent = event->getL0SubeventBySourceIDNum(sourceNum);

		if (eventOffset + 4 > eventBufferSize) {
			eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
					eventBufferSize + 4096); // add 4kB to the buffer
			eventBufferSize += 4096;
		}

		/*
		 * Put the sub-detector into the pointer table
		 */
		uint eventOffset32 = eventOffset / 4;
		std::memcpy(eventBuffer + pointerTableOffset, &eventOffset32, 3);
		std::memset(eventBuffer + pointerTableOffset + 3,
				SourceIDManager::sourceNumToID(sourceNum), 1);
		pointerTableOffset += 4;

		/*
		 * Write all fragments
		 */
		int payloadLength;
		for (uint i = 0; i != subevent->getNumberOfFragments(); i++) {
			const l0::MEPFragment* const fragment = subevent->getFragment(i);
			payloadLength = fragment->getPayloadLength() + sizeof(L0_BLOCK_HDR);
			if (eventOffset + payloadLength > eventBufferSize) {
				eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
						eventBufferSize + 4096);
				eventBufferSize += 4096; // add another 4kB, no point in being too precise...
			}

			L0_BLOCK_HDR* blockHdr = reinterpret_cast<L0_BLOCK_HDR*>(eventBuffer
					+ eventOffset);
			blockHdr->dataBlockSize = payloadLength;
			blockHdr->sourceSubID = fragment->getSourceSubID();
			blockHdr->reserved = 0x01;
			blockHdr->timestamp = fragment->getTimestamp();			

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
		// Add here missing fragments: could actually be handled dynamically by decoders
		if (subevent->getNumberOfFragments() < subevent->getNumberOfExpectedFragments() ) {
			uint missingFrags = subevent->getNumberOfExpectedFragments() - subevent->getNumberOfFragments();
			for (uint i = 0; i != missingFrags; i++) {
				payloadLength = sizeof(L0_BLOCK_HDR);
                if (eventOffset + payloadLength > eventBufferSize) {
                         eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
                                         eventBufferSize + 4096);
                         eventBufferSize += 4096;
                 }
	 	 isUnfinishedEOB = true;
                 L0_BLOCK_HDR* blockHdr = reinterpret_cast<L0_BLOCK_HDR*>(eventBuffer
                                 + eventOffset);
                 blockHdr->dataBlockSize = payloadLength;
                 blockHdr->reserved = 0x01;
                 blockHdr->sourceSubID = 0x00;
                 blockHdr->timestamp = 0xffffffff;
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
	}

	return eventBuffer;
}

char* EventSerializer::writeL1Data(const Event* event, char*& eventBuffer, uint& eventOffset,
		uint& eventBufferSize, uint& pointerTableOffset) {

	for (int sourceNum = 0; sourceNum != SourceIDManager::NUMBER_OF_L1_DATA_SOURCES; sourceNum++) {
		const l1::Subevent* const subevent = event->getL1SubeventBySourceIDNum(sourceNum);

		if (eventOffset + 4 > eventBufferSize) {
			eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
					eventBufferSize + 4096);
			eventBufferSize += 4096;
		}

		uint eventOffset32 = eventOffset / 4;
		/*
		 * Put the LKr into the pointer table
		 */
		std::memcpy(eventBuffer + pointerTableOffset, &eventOffset32, 3);
		std::memset(eventBuffer + pointerTableOffset + 3, SourceIDManager::l1SourceNumToID(sourceNum), 1);
		pointerTableOffset += 4;

		for (uint fragmentNum = 0; fragmentNum != subevent->getNumberOfFragments(); fragmentNum++) {
			l1::MEPFragment* e = subevent->getFragment(fragmentNum);

			if (eventOffset + e->getEventLength() > eventBufferSize) {
				eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
						eventBufferSize + 4096);
				eventBufferSize += 4096;
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

		// Add here missing fragments: could actually be handled dynamically by decoders
		if (subevent->getNumberOfFragments() < subevent->getNumberOfExpectedFragments() ) {
			uint missingFrags = subevent->getNumberOfExpectedFragments() - subevent->getNumberOfFragments();
			for (uint i = 0; i != missingFrags; i++) {
				int payloadLength = sizeof(l1::L1_EVENT_RAW_HDR);
                if (eventOffset + payloadLength > eventBufferSize) {
                         eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
                                         eventBufferSize + 4096);
                         eventBufferSize += 4096;
                 }

		isUnfinishedEOB=true;
                 l1::L1_EVENT_RAW_HDR* blockHdr = reinterpret_cast<l1::L1_EVENT_RAW_HDR*>(eventBuffer + eventOffset);

                 blockHdr->eventNumber = event->getEventNumber();
                 blockHdr->sourceID = SourceIDManager::l1SourceNumToID(sourceNum);
                 blockHdr->numberOf4BWords = payloadLength/4;
                 blockHdr->timestamp = 0xffffffff;
                 blockHdr->sourceSubID = 0;
                 blockHdr->reserved = 0;
                 blockHdr->reserved2 = 0;
                 blockHdr->l0TriggerWord = 0x23;
                 memcpy(eventBuffer + eventOffset, blockHdr, sizeof(l1::L1_EVENT_RAW_HDR));
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
	}
	return eventBuffer;
}

} /* namespace na62 */
