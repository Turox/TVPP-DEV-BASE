#include "Peer.hpp"

using namespace std;

Peer::Peer(string IP, string port, int sizePeerListOutInformed, int sizePeerListOutInformed_FREE, int limitUpload)
{
    ConstructorAux(IP,port, sizePeerListOutInformed, sizePeerListOutInformed_FREE, limitUpload);
}

Peer::Peer(string IP_port, int sizePeerListOutInformed, int sizePeerListOutInformed_FREE,  int limitUpload)
{
    string IP, port;
    size_t pos;
    
    pos = IP_port.find(IP_PORT_SEPARATOR);
    IP = IP_port.substr(0, pos);
    port = IP_port.substr(pos+1);

    ConstructorAux(IP,port, sizePeerListOutInformed, sizePeerListOutInformed_FREE, limitUpload);
}

void Peer::ConstructorAux(string IP, string port, int sizePeerListOutInformed, int sizePeerListOutInformed_FREE, int limitUpload)
{
    this->IP = IP;
    this->port = port;
    ResetID();
    this->sizePeerListOutInformed = sizePeerListOutInformed;
    this->sizePeerListOutInformed_FREE = sizePeerListOutInformed_FREE;
    this->limitUpload = limitUpload;
    this->sizePeerListOutNew = sizePeerListOutInformed;
    this->sizePeerListOutNew_FREE = sizePeerListOutInformed_FREE;

}
void Peer::ResetID()
{
    this->ID = this->IP+IP_PORT_SEPARATOR+this->port;
}

void Peer::SetID(string ID)
{
    this->ID = ID;
}
void Peer::SetIP(string IP)
{
    this->IP = IP;
    ResetID();
}
void Peer::SetPort(string port)
{
    this->port = port;
    ResetID();
}

string Peer::GetID()
{
    return ID;
}
string Peer::GetIP()
{
    return IP;
}
string Peer::GetPort()
{
    return port;
}
int Peer::GetSizePeerListOutInformed()
{
	return sizePeerListOutInformed;
}

void Peer::SetSizePeerListOutInformed(int sizePeerListOutInformed)
{
	this->sizePeerListOutInformed = sizePeerListOutInformed;
}

int Peer::GetSizePeerListOutInformed_FREE()
{
	return sizePeerListOutInformed_FREE;
}

void Peer::SetSizePeerListOutInformed_FREE(int sizePeerListOutInformed_FREE)
{
	this->sizePeerListOutInformed_FREE = sizePeerListOutInformed_FREE;
}

int Peer::GetLimitUpload(){
	return this->limitUpload;
}
void Peer::SetLimitUpload(int limitUpload){
	this->limitUpload = limitUpload;
}

int Peer::GetSizePeerListOutNew()
{
	return sizePeerListOutNew;
}

void Peer::SetSizePeerListOutNew(int sizePeerListOutNew)
{
	this->sizePeerListOutNew = sizePeerListOutNew;
}

int Peer::GetSizePeerListOutNew_FREE()
{
	return sizePeerListOutNew_FREE;
}

void Peer::SetSizePeerListOutNew_FREE(int sizePeerListOutNew_FREE)
{
	this->sizePeerListOutNew_FREE = sizePeerListOutNew_FREE;
}
