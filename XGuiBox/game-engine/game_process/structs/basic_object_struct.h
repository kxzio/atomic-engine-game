#pragma once
#include <string>
#include "../../resources/imgui_resource/imgui.h"

enum selecting_type
{
    NOT_SELECTED,
    MULTIPLE_SELECTED,
    SOLO_SELECTED
};

struct map_objects
{
    std::string name;

    ImVec2 pos;

    selecting_type selected = NOT_SELECTED;

    bool hovered = false;

    bool highlighted = false;
};