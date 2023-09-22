#pragma once

#include <vector>
#include <list>

#include "cannetwork.hpp"
#include "eposnode.hpp"
#include "candefines.h"

#ifndef EPOS_NET_SIZE
#define EPOS_NET_SIZE 10
#endif

class EposNetwork
{
private:
    CanNetwork *_can;

public:
    std::vector<EposNode *> _node;
    std::list<int> _node_idx;

    EposNetwork();

    ~EposNetwork();

    EposNetwork(CanNetwork &can)
    {
        _node.reserve(EPOS_NET_SIZE);
        printf("net._node.size(): %ld\n", EPOS_NET_SIZE);
        _can = &can;
    }

    void readPDO_TX_all(){
        for(auto idx : _node_idx){
            _node[idx]->readPDO_TX_all();
        }
    }

    void init()
    {
        _can->sendFrame(FRAME_NMT_PREOPERATIONAL);
        can_tic_toc;
        _can->sendFrame(FRAME_NMT_START_1);
        can_tic_toc;
        _can->sendFrame(FRAME_NMT_START_2);
        can_tic_toc;
    }

    void sync(bool async=true)
    {
        _can->sendFrame(FRAME_SYNC, async);
    }

    void setNode(int id, EposNode &node);

    EposNode *getNode(int id)
    {
        return _node[id];
    }
};
