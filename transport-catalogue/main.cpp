#include "transport_catalogue.h"
#include "request_handler.h"
#include "transport_router.h"
#include "json_reader.h"
#include <iostream>
#include <fstream>

void LastPracTest() {
    catalogue::TransportCatalogue catalogue;
    render::Renderer renderer;
    router::TransportRouter router(catalogue);
    handler::RequestHandler handler(catalogue, renderer, router);
    JsonReader reader(handler, std::cin);
    reader.ProcessRequests(std::cout);
}

void FilesTest() {
    catalogue::TransportCatalogue catalogue;
    render::Renderer renderer;
    router::TransportRouter router(catalogue);
    handler::RequestHandler handler(catalogue, renderer, router);
    std::ifstream input_file("e4_input.json");
    JsonReader reader(handler, input_file);
    std::ofstream output_file("e4_output_mine.json");
    reader.ProcessRequests(output_file);

}

int main() {

    //FilesTest();
    LastPracTest();
    return 0;
}
