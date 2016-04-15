/*
 * CommonExceptions.h
 *
 *  Created on: Apr 11, 2016
 *      Author: NA62 collaboration, G. Lehmann Miotto/CERN
 */

#pragma once

#ifdef USE_ERS
#include <ers/ers.h>

ERS_DECLARE_ISSUE(na62,                                 // namespace name
                Message,		                       	// issue name
                msg,									// message
                ((std::string) msg)					    // first attribute
                   )


ERS_DECLARE_ISSUE_BASE( na62,                                                                        // namespace name
				        BadConfiguration,                          									 // issue name
				        na62::Message, 		                       								     //base issue name
						"Bad option '" << option << "': " << value << ". Try --help" ,               // message
				        ERS_EMPTY,     																 // base class attributes
				        ((std::string)option )  ((std::string) value )                               // this class attributes
				                   )


ERS_DECLARE_ISSUE_BASE( na62,
						CorruptedPacket,
						na62::Message,
						"Corrupted packet received from IP " << IP address << ", port " << port << ". Reason: " << reason,
						ERS_EMPTY,
						((std::string)ipaddr )  ((uint) port ) ((std::string) reason)
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						CorruptedMEP,
						na62::Message,
						"Corrupted MEP received from detector. Reason: " << reason,
						ERS_EMPTY,
						((std::string) reason)
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						UnexpectedFragment,
						na62::Message,
						"Unexpected fragment with event number " << eventNumber << " received from detector " << detector << ", subsourceID 0x" << std::hex << subSourceID
						<< std::dec,
						ERS_EMPTY,
						((uint) eventNumber ) ((std::string) detector )  ((uint) subSourceID )
								   )
ERS_DECLARE_ISSUE_BASE( na62,
						UnrequestedFragment,
						na62::Message,
						"L1 Fragment with event number " << eventNumber << " received from detector " << detector << ", subsourceID 0x" << std::hex << subSourceID
						<< std::dec << ". It was not requested yet.",
						ERS_EMPTY,
						((uint) eventNumber ) ((std::string) detector )  ((uint) subSourceID )
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						DuplicateFragment,
						na62::Message,
						"Already received all fragments from detector "
						<< std::hex << detector << ", sourceSubID 0x" << sourceSubID
						<< " for event " << std::dec << eventNumber,
						ERS_EMPTY,
						((std::string) detector ) ((uint) subSourceID ) ((uint) eventNumber )
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						UnexpectedEventNumber,
						na62::Message,
						"Unexpected event number " << eventNumber << " requested. It should be in node: " << otherNodeID << ", but this is node: " << nodeID,
						ERS_EMPTY,
						((uint) eventNumber ) ((uint) otherNodeID ) ((uint) nodeID ) // this class attributes
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						TooLargeEventNumber,
						na62::Message,
						"Event number requested is " << eventNumber << " but maximum supported is " << maxEventNumber,
						ERS_EMPTY,
						((uint) eventNumber ) ((uint) maxEventNumber )
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						MissingEvent,
						na62::Message,
						"There is no trace of event " << eventNumber << " in burst " << burstID,
						ERS_EMPTY,
						((uint) eventNumber ) ((uint) maxEventNumber )
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						MissingFragments,
						na62::Message,
						"Event " << eventNumber << " is missing " << missingFrags << " fragments for detector "<< detector,
						ERS_EMPTY,
						((uint) eventNumber ) ((uint) missingFrags ) ((std::string) detector)
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						OutOfBurstIO,
						na62::Message,
						"Dropping " << type << " because burst is long finished.",
						ERS_EMPTY,
						((std::string) type)
								   )

ERS_DECLARE_ISSUE_BASE( na62,
						UnknownSourceID,
						na62::Message,
						"Unknown source ID 0x" << std::hex << sourceID << std::dec << ":" << subSourceID << " received.",
						ERS_EMPTY,
						((uint) sourceID ) ((uint) subSourceID )
								   )

#endif
