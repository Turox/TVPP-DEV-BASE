#include "MessageChannel.hpp"

MessageChannel::MessageChannel(uint8_t channelFlag, bool performingPunch, uint16_t externalPort, uint32_t idChannel, uint32_t nowTime,
		                       uint16_t maxPeerListOutInformed,  uint16_t maxPeerListOutInformed_FREE, uint16_t limitUpload)
{
    vector<int> data = GetHeaderValuesDataVector(channelFlag, performingPunch, externalPort, idChannel, nowTime,
    		                                      maxPeerListOutInformed, maxPeerListOutInformed_FREE, limitUpload);
	
    firstByte = new uint8_t[MESSAGE_CHANNEL_HEADER_SIZE];
    Message::AssembleHeader(OPCODE_CHANNEL, MESSAGE_CHANNEL_HEADER_SIZE, 0, data);
}

vector<int> MessageChannel::GetHeaderValuesDataVector(uint8_t channelFlag, bool performingPunch, uint16_t externalPort, uint32_t idChannel, uint32_t nowTime,
		                                              uint16_t maxPeerListOutInformed,  uint16_t maxPeerListOutInformed_FREE, uint16_t limitUpload)
{
    vector<int> data(9);
    data[0] = channelFlag;
    data[1] = performingPunch;
    data[2] = VERSION;
    data[3] = externalPort;
    data[4] = idChannel;
    data[5] = nowTime;
    data[6] = maxPeerListOutInformed;        //ECM
    data[7] = maxPeerListOutInformed_FREE;   //ECM
    data[8] = limitUpload;                   //ECM

    return data;
}

vector<uint8_t> MessageChannel::GetHeaderValuesSizeVector()
{
    vector<uint8_t> sizes(9);
    sizes[0] = 8;                                                    //CHANNEL FLAG
    sizes[1] = 8;                                                    //CHANNEL FLAG
    sizes[2] = 16;                                                    //VERSION
    sizes[3] = 16;                                                    //EXTERNAL PORT
    sizes[4] = 32;                                                    //CHANNEL ID
    sizes[5] = 32;                                                    //NOWTIME
    sizes[6] = 16;                                                    //INFORME LISTOUT SIZE
    sizes[7] = 16;                                                    //INFORME LISTOUT-FREE SIZE
    sizes[8] = 16;                                                    //INFORME limitUpload

    return sizes;
}
