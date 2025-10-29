#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include <fstream>

using namespace std;

int main() {
    //создали каталог
    catalogue::TransportCatalogue catalogue;
    //создали ридер, считали данные
    ireader::InputReader reader;
    reader.Read(cin, catalogue);
    //считали команды
    sreader::Read(cin, cout, catalogue);
}