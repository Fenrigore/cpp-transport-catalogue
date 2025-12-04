#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include <iostream>
#include <fstream>

void LastPracTest() {
    catalogue::TransportCatalogue catalogue;
    render::Renderer renderer;
    handler::RequestHandler handler(catalogue, renderer);
    JsonReader reader(handler, std::cin);
    reader.ProcessRequests(std::cout);
}

void FilesTest() {
    catalogue::TransportCatalogue catalogue;
    render::Renderer renderer;
    handler::RequestHandler handler(catalogue, renderer);
    std::ifstream input_file("s10_final_opentest_1.json");
    JsonReader reader(handler, input_file);
    std::ofstream output_file("s10_final_opentest_1_answer_mine.json");
    reader.ProcessRequests(output_file);

}

int main() {

    //FilesTest();
    LastPracTest();
    return 0;
}
