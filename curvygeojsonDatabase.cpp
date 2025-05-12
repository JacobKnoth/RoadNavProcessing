#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/bulk_write.hpp>
#include <bsoncxx/json.hpp>

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


/* build using 
g++ -std=c++17 curvygeojsonDatabase.cpp \
    $(pkg-config --cflags --libs libmongocxx) \
    -o curvygeojsonDatabase

    */
using json = nlohmann::json;

constexpr std::size_t BATCH = 5'000;

int main(){
    std::ifstream geojson_file("washington_curvy.geojson");
    if (!geojson_file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    json geojson_data;
    geojson_file >> geojson_data;

    if (!geojson_data.contains("features") || geojson_data["features"].empty()) {
        std::cerr << "No features found\n";
        return 1;
    }
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

    
    geojson_file.close();
}


/*
const auto& feature = geojson_data["features"][0];
    
    // Print type of the feature
    std::cout << "Feature type: " << feature["type"] << "\n";

    // Print geometry type
    const auto& geometry = feature["geometry"];
    std::cout << "Geometry type: " << geometry["type"] << "\n";

    // Print coordinates
    const auto& coordinates = geometry["coordinates"];
    std::cout << "Coordinates:\n";
    for (const auto& coord : coordinates) {
        std::cout << "  [" << coord[0] << ", " << coord[1] << "]\n";
    }

    // Print properties (like name and curvature)
    const auto& properties = feature["properties"];
    std::cout << "Properties:\n";
    for (auto& [key, value] : properties.items()) {
        std::cout << "  " << key << ": " << value << "\n";
    }
*/