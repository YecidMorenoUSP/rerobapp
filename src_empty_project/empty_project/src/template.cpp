
#include <iostream>

#include <cannetwork.hpp>
#include <eposnetwork.hpp>
#include <eposnode.hpp>

CanNetwork can(r "can0");
EposNode node4(4, can);
EposNetwork net(can);

int main(){

    can.connect();
    can.disconnect();

    printf("Empty project . . . \n");
    return 0;
}