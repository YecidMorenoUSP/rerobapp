#include "eposnode.hpp"





EposNode::EposNode(int id, CanNetwork &can)
{
    _id = id;
    _can = &can;
    _init(_id);
}

EposNode::~EposNode()
{
    this->stopMotors();
    printf("EposNode::~EposNode()\n");
}
