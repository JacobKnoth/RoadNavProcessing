#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/bulk_write.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/document/view.hpp>

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>


/* build using 
g++ -std=c++17 curvygeojsonDatabase.cpp \
    $(pkg-config --cflags --libs libmongocxx) \
    -o curvygeojsonDatabase

    */
using json = nlohmann::json;

constexpr std::size_t BATCH = 5'000;

int main(){
    // std::ifstream geojson_file("10_curvy.geojson");
    // if (!geojson_file.is_open()) {
    //     std::cerr << "Error opening file" << std::endl;
    //     return 1;
    // }

    // json geojson_data;
    // geojson_file >> geojson_data;

    // if (!geojson_data.contains("features") || geojson_data["features"].empty()) {
    //     std::cerr << "No features found\n";
    //     return 1;
    // }
    // std::unordered_map<std::string, float> curvature_map;
    // for (const auto& feature : geojson_data["features"]) {
    //     if (feature.contains("properties") && feature["properties"].contains("curvature")) {
    //         for (const auto& ways : feature["properties"]["ways"]) {
    //             if (ways.contains("id")) {
    //                 int  id= ways["id"];
    //                 std::string name = "w" + std::to_string(id);
    //                 float curvature = feature["properties"]["curvature"];
    //                 curvature_map[name] = curvature;
    //             }
    //         }
    //     }
    // }
    mongocxx::instance  inst{};
    mongocxx::uri       myUri("mongodb://localhost:27017");
    try {
        mongocxx::client conn{myUri};
        mongocxx::database db = conn["osm"];
        mongocxx::collection collection = db["roadid"];

        bsoncxx::builder::stream::document filter;
        bsoncxx::builder::stream::document update;
        mongocxx::cursor cursor = collection.find({});
        for (auto doc : cursor) {
            auto properties_elem = doc["properties"];
            if (properties_elem && properties_elem.type() == bsoncxx::type::k_document) {
                bsoncxx::document::view properties_view = properties_elem.get_document().view();
            
                auto id_elem = properties_view["id"];
                if (id_elem && id_elem.type() == bsoncxx::type::k_string) {
                    std::string id_str = std::string(id_elem.get_string().value);
                    std::cout << "Found properties.id = " << id_str << std::endl;
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    
    //geojson_file.close();
}
