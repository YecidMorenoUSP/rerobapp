#include "eposnetwork.hpp"

void EposNetwork::setNode(int id, EposNode &node)
{
    _node[id] = &node;
    _node_idx.push_back(id);
    
}

EposNetwork::EposNetwork()
{
}

EposNetwork::~EposNetwork()
{
    for(auto idx : _node_idx){
        // delete(_node[idx]);
    }
}
