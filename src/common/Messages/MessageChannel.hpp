#ifndef MESSAGE_CHANNEL_H
#define MESSAGE_CHANNEL_H

#include "Message.hpp" 

class MessageChannel : public Message
{
	public:
		MessageChannel(Message* message) : Message(message->GetFirstByte()) {};
		MessageChannel(uint8_t channelFlag, bool performingPunch, uint16_t extPort, uint32_t idChannel, uint32_t nowTime,
				uint16_t maxPeerListOutInformed = -1, uint16_t maxPeerListOutInformed_FREE = -1, uint16_t limitUpload = -1);
		vector<int> GetHeaderValuesDataVector(uint8_t channelFlag, bool performingPunch, uint16_t extPort, uint32_t idChannel, uint32_t nowTime,
				uint16_t maxPeerListOutInformed = -1, uint16_t maxPeerListOutInformed_FREE = -1, uint16_t limitUpload = -1);
		vector<uint8_t> GetHeaderValuesSizeVector();
};

#endif // MESSAGE_CHANNEL_H	
