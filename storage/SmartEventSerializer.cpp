/*
 * SmartEventSerializer.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: Jonas Kunze (kunze.jonas@gmail.com)
 */

#include "SmartEventSerializer.h"

#include <cstring>
#include <algorithm>
#include "../options/Logging.h"
#include "../eventBuilding/Event.h"
#include "../eventBuilding/SourceIDManager.h"
#include "../l0/MEPFragment.h"
#include "../l0/Subevent.h"
#include "../l1/MEPFragment.h"
#include "../l1/Subevent.h"
#include "../structs/Event.h"
#include "../structs/Versions.h"
#include "exceptions/SerializeError.h"

namespace na62 {

uint SmartEventSerializer::InitialEventBufferSize_;

int SmartEventSerializer::TotalNumberOfDetectors_;
bool SmartEventSerializer::DumpFlag_;

bool isUnfinishedEOB;

void SmartEventSerializer::initialize() {
	/*
	 * L0 + L1 sources
	 */
	TotalNumberOfDetectors_ = SourceIDManager::NUMBER_OF_L0_DATA_SOURCES + SourceIDManager::NUMBER_OF_L1_DATA_SOURCES ;
	//InitialEventBufferSize_ = 1000;
	//InitialEventBufferSize_ = 4096; // allocate 4 kB initially for the serialized event
	//Should not be less than the header size!!!
	InitialEventBufferSize_ = 4096; //Used for event that don't have to be serialized on the shared memory, will change during the the lifetime of the process
	//SharedMemoryBufferSize_ = 32768; //Used to serialize on the shared memory will not change during the lifetime of the process TODO bind this value to SharedMemorymaster
	LOG_INFO("Buffer size initialized at: "<<InitialEventBufferSize_);

	isUnfinishedEOB = false;
	DumpFlag_ = true;
}

char* SmartEventSerializer::ResizeBuffer(char* buffer, const int oldLength,	const int newLength, bool& isInitialEventBufferSizeFixed) {
	if (isInitialEventBufferSizeFixed) {
		throw SerializeError("Serialized Event too big for the shared memory");
	}
	char* newBuffer = new char[newLength];
	memcpy(newBuffer, buffer, oldLength);
	delete[] buffer;
	return newBuffer;
}

EVENT_HDR* SmartEventSerializer::SerializeEvent(const Event* event) {
	uint eventBufferSize = InitialEventBufferSize_;
	char* eventBuffer = new char[eventBufferSize];
	bool isInitialEventBufferSizeFixed = false; //Lenght can change
	return SmartEventSerializer::doSerialization(event, eventBuffer, eventBufferSize, isInitialEventBufferSizeFixed);
}

EVENT_HDR* SmartEventSerializer::SerializeEvent(const Event* event, l1_SerializedEvent* seriale) {
	uint eventBufferSize = sizeof(l1_SerializedEvent); //Set the length of the buffersize equal to the size of the fragment of the shared memory
	char* eventBuffer = (char*) seriale;
	bool isInitialEventBufferSizeFixed = true; //Length can't change
	return SmartEventSerializer::doSerialization(event, eventBuffer, eventBufferSize, isInitialEventBufferSizeFixed);
}

EVENT_HDR* SmartEventSerializer::doSerialization(const Event* event, char* eventBuffer, uint& eventBufferSize, bool& isInitialEventBufferSizeFixed) {

	uint sizeOfPointerTable = 4 * TotalNumberOfDetectors_;
	uint pointerTableOffset = sizeof(EVENT_HDR);
	uint eventOffset = sizeof(EVENT_HDR) + sizeOfPointerTable;
	bool isUnfinishedEOB = false;

	writeL0Data(event, eventBuffer, eventOffset, eventBufferSize, pointerTableOffset, isUnfinishedEOB, isInitialEventBufferSizeFixed);
	writeL1Data(event, eventBuffer, eventOffset, eventBufferSize, pointerTableOffset, isUnfinishedEOB, isInitialEventBufferSizeFixed);
	writeTrailer(event, eventBuffer, eventOffset, eventBufferSize, isInitialEventBufferSizeFixed);

	if (eventBufferSize > InitialEventBufferSize_) {
		InitialEventBufferSize_ = eventBufferSize;
	}

	return writeHeader(event, eventBuffer, eventOffset, isUnfinishedEOB);
}

EVENT_HDR* SmartEventSerializer::writeHeader(const Event* event, char*& eventBuffer, uint& eventOffset, bool& isUnfinishedEOB) {
	EVENT_HDR* header = reinterpret_cast<EVENT_HDR*>(eventBuffer);
	header->eventNum = event->getEventNumber();
	header->formatVersion = EVENT_HDR_FORMAT_VERSION; // TODO: update current format
	header->length = (eventOffset + sizeof(EVENT_TRAILER)) / 4; //Number of 32 words
	header->burstID = event->getBurstID();
	header->timestamp = event->getTimestamp();
	header->triggerWord = event->getTriggerTypeWord();
	header->reserved1 = 0;
	header->fineTime = event->getFinetime();
	header->numberOfDetectors = TotalNumberOfDetectors_;
	//std::cout << "Total number of detectors: " << TotalNumberOfDetectors_ << std::endl;
	header->reserved2 = 0;
	header->processingID = event->getProcessingID();
	header->SOBtimestamp = 0; // Will be set by the merger

	if (isUnfinishedEOB) {
		header->triggerWord = 0xfefe23;
	}

	return header;}


char* SmartEventSerializer::writeL0Data(const Event* event, char*& eventBuffer, uint& eventOffset,
uint& eventBufferSize, uint& pointerTableOffset, bool& isUnfinishedEOB, bool& isInitialEventBufferSizeFixed) {
	/*
	 * Write all L0 data sources
	 */
	for (int sourceNum = 0; sourceNum != SourceIDManager::NUMBER_OF_L0_DATA_SOURCES; sourceNum++) {
		const l0::Subevent* const subevent = event->getL0SubeventBySourceIDNum(sourceNum);

		if (eventOffset + 4 > eventBufferSize) {
			eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize, eventBufferSize + 4096, isInitialEventBufferSizeFixed); // add 4kB to the buffer
			eventBufferSize += 4096;
		}

		/*
		 * Put the sub-detector into the pointer table
		 */
		uint eventOffset32 = eventOffset / 4;
		std::memcpy(eventBuffer + pointerTableOffset, &eventOffset32, 3);
		std::memset(eventBuffer + pointerTableOffset + 3, SourceIDManager::sourceNumToID(sourceNum), 1);
		pointerTableOffset += 4;
//		std::cout<<"Detector Table: "<< std::hex <<(int) SourceIDManager::sourceNumToID(sourceNum) << " Offset: "<< std::dec << eventOffset32 << std::endl;

		/*
		 * Write all fragments
		 */
		int payloadLength;
		for (uint i = 0; i != subevent->getNumberOfFragments(); i++) {
			const l0::MEPFragment* const fragment = subevent->getFragment(i);
			payloadLength = fragment->getPayloadLength() + sizeof(L0_BLOCK_HDR);
			if (eventOffset + payloadLength > eventBufferSize) {
				eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize, eventBufferSize + std::max(4096, payloadLength), isInitialEventBufferSizeFixed);
				eventBufferSize += std::max(4096, payloadLength); // add another 4kB, no point in being too precise...
			}

			L0_BLOCK_HDR* blockHdr = reinterpret_cast<L0_BLOCK_HDR*>(eventBuffer + eventOffset);
			blockHdr->dataBlockSize = payloadLength;
			blockHdr->sourceSubID = fragment->getSourceSubID();
			blockHdr->reserved = 0x01;
			blockHdr->timestamp = fragment->getTimestamp();			

			memcpy(eventBuffer + eventOffset + sizeof(L0_BLOCK_HDR),
					fragment->getPayload(),
					payloadLength - sizeof(L0_BLOCK_HDR));
			eventOffset += payloadLength;

//			std::cout << "Serialization of frag of det 0x" << std::hex << (int) fragment->getSourceID()
//					<< " subid 0x" << (int) blockHdr->sourceSubID << std::dec
//					<<" size "<< (int) blockHdr->dataBlockSize<< std::endl;

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
                         eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,  eventBufferSize + std::max(4096, payloadLength), isInitialEventBufferSizeFixed);
                         eventBufferSize += std::max(4096, payloadLength);
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

char* SmartEventSerializer::writeL1Data(const Event* event, char*& eventBuffer, uint& eventOffset,
		uint& eventBufferSize, uint& pointerTableOffset, bool& isUnfinishedEOB, bool& isInitialEventBufferSizeFixed) {

	for (int sourceNum = 0; sourceNum != SourceIDManager::NUMBER_OF_L1_DATA_SOURCES; sourceNum++) {
		const l1::Subevent* const subevent = event->getL1SubeventBySourceIDNum(sourceNum);

		if (eventOffset + 4 > eventBufferSize) {
			eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
					eventBufferSize + 4096, isInitialEventBufferSizeFixed);
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

			if (SourceIDManager::l1SourceNumToID(sourceNum)!=SOURCE_ID_LKr||e->getEventLength()!=28) {	// RF 22.09.2016

				if (eventOffset + e->getEventLength() > eventBufferSize) {
					eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
							eventBufferSize + std::max(4096, (int) e->getEventLength()), isInitialEventBufferSizeFixed);
					eventBufferSize += std::max(4096, (int) e->getEventLength());
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
			} // RF 22.09.2016
		}

		// Add here missing fragments: could actually be handled dynamically by decoders
		if (subevent->getNumberOfFragments() < subevent->getNumberOfExpectedFragments() ) {
			uint missingFrags = subevent->getNumberOfExpectedFragments() - subevent->getNumberOfFragments();
			for (uint i = 0; i != missingFrags; i++) {
				int payloadLength = sizeof(l1::L1_EVENT_RAW_HDR);
                if (eventOffset + payloadLength > eventBufferSize) {
                         eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize,
                                         eventBufferSize + std::max(4096, payloadLength), isInitialEventBufferSizeFixed);
                         eventBufferSize += std::max(4096, payloadLength);
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

bool SmartEventSerializer::compareSerializedEvent(EVENT_HDR* first_event, EVENT_HDR* second_event) {
	//std::cout<<"Checking Serialization"<<std::endl;
	std::cout<<"Length event 1: "<<first_event->length<<" Length event 2: "<<second_event->length<<std::endl;
	if (first_event->length != second_event->length) {
		std::cout<<"Length event 1: "<<first_event->length<<" Length event 2: "<<second_event->length<<std::endl;
		return false;
	}

	bool is_ok = true;
	//std::cout<<"Length is the same: "<<first_event->length<<" "<<second_event->length<<std::endl;
	int * first_pointer = (int*) first_event;
	int * second_pointer = (int*) second_event;
	//Length is expressed in 32 bit words
	for (uint a = 0 ; a < first_event->length; ++a) {
		//std::cout<<"Checking "<<std::endl;
		if (*(first_pointer) != *(second_pointer)){
			is_ok = false;
			std::cout <<"Serialized Event mismatch"<< std::endl;
		} else {
			//std::cout << *(first_pointer) <<" "<< *(second_pointer)<< std::endl;
		}
		first_pointer++;
		second_pointer++;
	}
	return is_ok;
}

EVENT_TRAILER* SmartEventSerializer::writeTrailer(const Event* event, char*& eventBuffer, uint& eventOffset, uint& eventBufferSize, bool& isInitialEventBufferSizeFixed) {
	if (eventOffset + sizeof(EVENT_TRAILER) > eventBufferSize) {
		eventBuffer = ResizeBuffer(eventBuffer, eventBufferSize, eventBufferSize + sizeof(EVENT_TRAILER), isInitialEventBufferSizeFixed);
		eventBufferSize += sizeof(EVENT_TRAILER);
	}
	EVENT_TRAILER* trailer = (EVENT_TRAILER*) (eventBuffer + eventOffset);
	trailer->eventNum = event->getEventNumber();
	trailer->reserved = 0;
	return trailer;
}

} /* namespace na62 */
