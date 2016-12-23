#include "PeerData.hpp"

using namespace std;

/** 
 *Metodo construtor
 * ID que o o endereço IP do peer
 * ECM - Inclusao do ttlIn e ttlOut
 */
PeerData::PeerData(Peer* peer, int ttlIn, int ttlOut, int ttlChannel, int size) : chunkMap(size)
{
	//ECM
	this->ttlIn = ttlIn;
	this->ttlOut = ttlOut;
	this->ttlout_free = this->ttlOut;
	this->ttlChannel = ttlChannel;
    this->peer = peer;
    uploadScore = 0;
    mode = MODE_CLIENT;
    pendingRequests = 0;
    delay = 0;
    peerSentChunks=0;
    this->hit_count=0;
}

void PeerData::Inc_peerSentChunks (int value){
	this->peerSentChunks = this->peerSentChunks + value;
}
uint32_t PeerData::Get_peerSentChunks(){
	return this->peerSentChunks;
}
void PeerData::Set_peerSentChunks(uint32_t value){
	this->peerSentChunks = value;
}


/** Retorna o TTL*****************/
int PeerData::GetTTLIn()
{
    return ttlIn;
}
int PeerData::GetTTLOut(bool peerListOut_free)
{
	if (peerListOut_free)
		return ttlout_free;
    return ttlOut;
}

int PeerData::GetTTLChannel()
{
    return ttlChannel;
}


/** Altera o TTL*****************/
void PeerData::SetTTLIn(int ttlIn)
{
	this->ttlIn = ttlIn;
}
void PeerData::SetTTLOut(int ttlOutMax, bool peerListOut_free)
{
	if (peerListOut_free)
		this->ttlout_free = ttlOutMax;
	else
		this->ttlOut = ttlOutMax;
}

void PeerData::SetTTLChannel(int ttlChannel)
{
	this->ttlChannel = ttlChannel;
}

/** Decrementa o TTL em 1*********/
void PeerData::DecTTLIn()
{
    ttlIn--;
}
void PeerData::DecTTLOut(bool peerListOut_free)
{
	if (peerListOut_free)
		ttlout_free--;
	else
		ttlOut--;
}

void PeerData::DecTTLChannel()
{
    ttlChannel--;
}
/** Retorna o ID*/
Peer* PeerData::GetPeer()
{
    return peer;
}

/** Altera o Modo do Peer */
void PeerData::SetMode(PeerModes mode)
{
    this->mode = mode;
}

/** Retorna o Modo do Peer */
PeerModes PeerData::GetMode()
{
    return this->mode;
}

void PeerData::SetChunkMap(ChunkUniqueID chunkMapHead, boost::dynamic_bitset<> map)
{
    chunkMap = HeadedBitset(chunkMapHead, map);
    //SetChunkMapHead(chunkMapHead);
}

bool PeerData::GetChunkMapPos(int i) const
{
    return (bool)chunkMap[i];
}

void PeerData::SetChunkMapHead(ChunkUniqueID chunkMapHead)
{
    chunkMap.SetHead(chunkMapHead);
}

ChunkUniqueID PeerData::GetChunkMapHead()
{
    return chunkMap.GetHead();
}

uint32_t PeerData::GetChunkMapSize() const
{
    return chunkMap.size();
}

void PeerData::IncPendingRequests()
{
    pendingRequests++;
}

void PeerData::DecPendingRequests()
{
    pendingRequests--;
}

int PeerData::GetPendingRequests()
{
    return pendingRequests;
}

void PeerData::SetDelay(float value)
{
    delay = value;
}

float PeerData::GetDelay()
{
    return delay;
}

void PeerData::SetUploadScore(int value)
{
    uploadScore = value;
}

int PeerData::GetUploadScore()
{
    return uploadScore;
}

std::ostream& operator<<(std::ostream& os, const PeerData& pd)
{
    os << "PeerID: " << pd.peer << " Mode: " << (int)pd.mode << endl;
    os << pd.chunkMap; 
    return os;
}

int PeerData::GetSizePeerListOutInformed ()
{
   return this->GetPeer()->GetSizePeerListOutInformed();
}

int PeerData::GetSizePeerListOutInformed_FREE ()
{
   return this->GetPeer()->GetSizePeerListOutInformed_FREE();
}


void PeerData::SetSizePeerListOutInformed(int sizePeerListOutInformed)
{
	this->GetPeer()->SetSizePeerListOutInformed(sizePeerListOutInformed);
}

void PeerData::SetSizePeerListOutInformed_FREE(int sizePeerListOutInformed_FREE)
{
	this->GetPeer()->SetSizePeerListOutInformed_FREE(sizePeerListOutInformed_FREE);
}

int PeerData::GetLimitUpload(){
	return this->GetPeer()->GetLimitUpload();
}
void PeerData::SetLimitUpload(int limitUpload){
	this->GetPeer()->SetLimitUpload(limitUpload);
}
PairStrInt PeerData::GetPairStrInt()
{
    return PairStrInt(this->GetPeer()->GetID(), this->peerSentChunks);
}

void PeerData::SetHit_count(uint16_t hit_count){
	this->hit_count = hit_count;
}
void PeerData::DecHit_count(){
	if (this->hit_count >0)
	    this->hit_count--;
}

uint16_t PeerData::GetHit_count(){
	return this->hit_count;
}


int PeerData::GetSizePeerListOutOld ()
{
   return this->GetPeer()->GetSizePeerListOutOld();
}

int PeerData::GetSizePeerListOutOld_FREE ()
{
   return this->GetPeer()->GetSizePeerListOutOld_FREE();
}


void PeerData::SetSizePeerListOutOld(int sizePeerListOutOld)
{
	this->GetPeer()->SetSizePeerListOutOld(sizePeerListOutOld);
}

void PeerData::SetSizePeerListOutOld_FREE(int sizePeerListOutOld_FREE)
{
	this->GetPeer()->SetSizePeerListOutOld_FREE(sizePeerListOutOld_FREE);
}


