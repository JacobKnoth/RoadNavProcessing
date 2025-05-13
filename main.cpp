#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>
/*
g++ -std=c++17 main.cpp \
    $(pkg-config --cflags --libs libmongocxx) \
    -o main

*/
int main() {
    mongocxx::instance  inst{};
    mongocxx::uri       myUri("mongodb://localhost:27017");

    try {
        mongocxx::client conn{myUri};
        mongocxx::database db = conn["osm"];
        mongocxx::collection collection = db["roadid"];

        mongocxx::cursor cursor = collection.find({});
        for (auto doc : cursor) {
            std::cout << bsoncxx::to_json(doc) << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
