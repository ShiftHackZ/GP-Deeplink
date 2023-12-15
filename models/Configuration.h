#pragma once

#include <string>
#include <vector>
#include "Deeplink.h"

struct Configuration {
    std::string outputFolder;
    std::vector<Deeplink> deeplinks;
};
