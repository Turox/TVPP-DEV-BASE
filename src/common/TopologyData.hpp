/*
 * TopologyData.hpp
 *
 *  Created on: 21 de dez de 2016
 *      Author: eliseu
 */

#ifndef SRC_COMMON_TOPOLOGYDATA_HPP_
#define SRC_COMMON_TOPOLOGYDATA_HPP_

using namespace std;

class TopologyData
{

public:
	TopologyData (int joinMinimunBandwidth = 0, int in = 0, int out = 0, int out_free = 0, int percentOfPeer = 0);
	int GetJoinMinimunBandwidth () {return this->joinMinimunBandwidth;}
	int GetIn(){return this->in;};
	int GetOut(){return this->out;};
	int GetOut_free(){return this->out_free;};
	int GetPercentOfPeer() {return this->percentOfPeer;};

    void SetJoinMinimunBandwidth (int joinMinimunBandwidth) {this->joinMinimunBandwidth = joinMinimunBandwidth;}
	void SetIn(int in){this->in = in;};
	void SetOut(int out){this->out = out;};
	void SetOut_free(int out_free){this->out_free = out_free;};
	void SetPercentOfPeer (int percentOfPeer){this->percentOfPeer = percentOfPeer;};

private:
	int joinMinimunBandwidth;
	int in;
	int out;
	int out_free;
	int percentOfPeer;
};

#endif /* SRC_COMMON_TOPOLOGYDATA_HPP_ */
