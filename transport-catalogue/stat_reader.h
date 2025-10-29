#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"
namespace sreader {

    void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
        std::ostream& output);

    void Read(std::istream& input, std::ostream& output, catalogue::TransportCatalogue& catalogue);
}