#pragma once
#include "basic_object_struct.h"
#include <vector>

struct city : public map_objects
{
    city() = default;

    int population;
    city(std::string iname, ImVec2 ipos, int ipopulation = 0)
    {
        name = iname;
        pos = ipos;
        population = ipopulation;
    }
};
