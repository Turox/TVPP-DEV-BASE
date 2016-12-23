/*
 * TopologyData.cpp
 *
 *  Created on: 21 de dez de 2016
 *      Author: eliseu

*/

#include "TopologyData.hpp"

	TopologyData::TopologyData (int joinMinimunBandwidth, int in, int out, int out_free, int percentOfPeer){
	  this->joinMinimunBandwidth = joinMinimunBandwidth;
	  this->in = in;
	  this->out = out;
	  this->out_free = out_free;
	  this->percentOfPeer = percentOfPeer;
	}

