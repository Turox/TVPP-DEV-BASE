#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include <time.h>
#include <vector>
#include <map>
#include "PeerData.hpp"
#include "TopologyData.hpp"
#include "Strategy/Strategy.hpp"


using namespace std;

/**
* This class implements the Channel abstraction
*/

class Channel
{
    public:
        /**
        * Constructor
        * @param idServer server ip and port (ip:port)
        */
        Channel(unsigned int channelId = 0, Peer* serverPeer = NULL, bool dynamicTopologyArrangement = false);

        ChunkUniqueID GetServerNewestChunkID();
		void SetServerNewestChunkID(ChunkUniqueID serverNewestChunkID);
        int GetServerEstimatedStreamRate();
		void SetServerEstimatedStreamRate(int serverEstimatedStreamRate);
		
        Peer* GetServer();
        Peer* GetPeer(Peer* peer);
		bool HasPeer(Peer* peer);
        void AddPeer(Peer* peer, uint16_t hit_count = 0);
        void RemovePeer(Peer* peer);
		void RemovePeer(string peerId);
		PeerData& GetPeerData(Peer* peer);
        time_t GetCreationTime();

        //topology experiments
        void RenewOUTALL();
        int NormalizeClasses(TopologyClasses classRef, vector<PairStrInt>* ordinaryNodeVector, int start, int stop);
        void SetIndicateClassPosition(bool indicateClassPosition);
        TopologyClasses SugestedClass(int bandwidth);


        vector<PeerData*> SelectPeerList(Strategy* strategy, Peer* srcPeer, unsigned int peerQuantity, bool virtualPeer,
        		                         uint8_t minimumBandwidth, uint8_t minimumBandwidth_FREE, bool separatedFreeOutList);
        unsigned int GetPeerListSize();
        void CheckActivePeers();
        void PrintPeerList();
        void printChannelProfile();

        FILE* GetPerformanceFile();
        FILE* GetOverlayFile();
        bool firstTimeOverlay;
		
		friend bool operator<(const Channel &a, const Channel &b) {return a.channelId<b.channelId;};
		
    private:
        unsigned int channelId;
        Peer* serverPeer; 
        map<string, PeerData> peerList;

        map <TopologyClasses,TopologyData> classTopologySettings;
        bool dynamicTopologyArrangement;
        bool indicateClassPosition;

		ChunkUniqueID serverNewestChunkID;
        int serverEstimatedStreamRate;

        time_t creationTime;

        FILE* performanceFile;
        FILE* overlayFile;
};

#endif // CHANNEL_H

