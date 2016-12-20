#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <map>
#include <set>
#include <string.h>
#include <stdio.h>
#include "../common/PeerData.hpp"
#include <boost/thread/mutex.hpp>
#include <stdint.h>
#include <iostream>
#include <set>
#include <algorithm>
#include <utility>
#include <vector>
#include <sstream>

#define PEER_ACTIVE_COOLDOWN 5



// Alterações: Eliseu César Miguel
// 13/01/2015
class PeerManager
{
	private:
	unsigned int maxActivePeersIn;
	unsigned int maxActivePeersOut;
	unsigned int maxActivePeersOutFREE;                 //tamanho da lista de pares com pouca banda
	unsigned int outLimitToSeparateFree;                //insere pares pobres em lista outFree se out do par for menor que o limite


    map<string, PeerData> peerList; //todos conhecidos (vizinhos)

	set<string> peerActiveIn;       //ativos que enviam dados a este par
	set<string> peerActiveOut;      // ativos que recebem dados deste par
	set<string> peerActiveOutFREE;  // ativos que recebem dados e que possuem pouca banda

	map<string, unsigned int> peerActiveCooldownIn; // pares que podem ser removidos por pouca atividade
	map<string, unsigned int> peerActiveCooldownOut;
	map<string, unsigned int> peerActiveCooldownOutFREE;  // ... ele tenha sido removido apenas para flexibilização da rede

	boost::mutex peerListMutex;
    boost::mutex peerActiveMutexIn;
    boost::mutex peerActiveMutexOut;
    boost::mutex peerActiveMutexOutFREE;

    bool removeWorsePartner;                             // used to remove the worse peer if new connection asked

    void CheckpeerActiveCooldown(map<string, unsigned int>* peerActiveCooldown);
    int showPeerActive(set<string>* peerActive);

    public:
	PeerManager(); //atua na lista de vizinhos
	unsigned int GetMaxActivePeers(set<string>* peerActive);
	void SetMaxActivePeersIn(unsigned int maxActivePeers);
	void SetMaxActivePeersOut(unsigned int maxActivePeers);
	void SetMaxActivePeersOutFREE(unsigned int maxActivePeersFREE);
	void SetNewMMaxActivePeersOut(int newMaxActivePeers,set<string>* peerActiveOut);



	void SetMaxOutFreeToBeSeparated(unsigned int outLimitToSeparateFree);

	bool AddPeer(Peer* newPeer, int sizePeerListOut = -1, int sizePeerListOut_FREE = -1); //add na lista de vizinhos

    set<string>* GetPeerActiveIn();
    //set<string>* GetPeerActiveOut();
    set<string>* GetPeerActiveOut(bool separatedFree = false, uint16_t peerOut = 0);
    map<string, unsigned int>* GetPeerActiveCooldown(set<string>* peerActive);

	bool ConnectPeer(string peer, set<string>* peerActive);
	bool ConnectSpecial(string peer, set<string>* peerActive);
	void DisconnectPeer(string peer, set<string>* peerActive);
	void RemovePeer(string peer);//remove na lista de vizinhos

	void SetRemoveWorsePartner (bool removeWorsePartner);
	bool GetRemoveWorsePartner ();


	unsigned int GetPeerActiveSize(set<string>* peerActive);
	unsigned int GetPeerActiveSize_OPENED(set<string>* peerActive);
	unsigned int GetPeerActiveSizeTotal(); //usada para fornecer o total de pares ativos em In + Out sem repetição. Será removida!!!

	bool IsPeerActive(string peer,set<string>* peerActive);
	PeerData* GetPeerData(string peer); // a lista de vizinhos é a única que tem os dados do peer

	map<string, PeerData>* GetPeerList();
	boost::mutex* GetPeerListMutex();

	boost::mutex* GetPeerActiveMutex(set<string>* peerActive);

	void CheckPeerList();
    void ShowPeerList();

};
#endif

