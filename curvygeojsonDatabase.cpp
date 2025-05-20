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
    std::unordered_map<std::string, double> curvature_map;
    for (const auto& feature : geojson_data["features"]) {
        if (feature.contains("properties") && feature["properties"].contains("curvature")) {
            for (const auto& ways : feature["properties"]["ways"]) {
                if (ways.contains("id")) {
                    int  id= ways["id"];
                    std::string name = "w" + std::to_string(id);
                    float curvature = feature["properties"]["curvature"];
                    curvature_map[name] = curvature;
                }
            }
        }
    }
    mongocxx::instance  inst{};
    mongocxx::uri       myUri("mongodb://localhost:27017");
    try {
        mongocxx::client conn{myUri};
        mongocxx::database db = conn["osm"];
        mongocxx::collection collection = db["roadid"];


        mongocxx::cursor cursor = collection.find({});
        int count = 0;
        for (auto doc : cursor) {
            bsoncxx::builder::stream::document filter;
            bsoncxx::builder::stream::document update;
            auto id_elem = doc["id"];
            if (!id_elem || id_elem.type() != bsoncxx::type::k_string) continue;
            std::string idString = std::string(id_elem.get_string().value);
            auto it = curvature_map.find(idString);
            if (it == curvature_map.end())
                continue;
            double curvature_value = it->second;
            auto properties_elem = doc["properties"];
            if (!properties_elem || properties_elem.type() != bsoncxx::type::k_document) continue;
            if (properties_elem)
            {
                filter << "_id" << doc["_id"].get_oid().value;
                bsoncxx::document::view properties_view = properties_elem.get_document().view();
                update << "$set" << bsoncxx::builder::stream::open_document
                    << "properties.curvature" << curvature_value
                    << bsoncxx::builder::stream::close_document;
                collection.update_one(filter.view(), update.view());
                count++;
                if (count % BATCH == 0) {
                    std::cout << "Updated " << count << " documents." << std::endl;
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    
    //geojson_file.close();
}
