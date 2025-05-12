// node_way_index_highway.cpp
// Compile: 
/*
g++ -std=c++17 -O2 node_way_index_highway.cpp \
    $(pkg-config --cflags libmongocxx) \
    -pthread \
    $(pkg-config --libs libmongocxx) \
    -lssl -lcrypto -lz -lbz2 -lexpat \
    -o node_way_highway
*/
#include <osmium/io/reader.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/bulk_write.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <unordered_set>
#include <vector>
#include <iostream>
#include <cstring>
#include <functional>

// ───────────────────────────── settings
constexpr std::size_t BATCH = 50'000;           // bulk‑write chunk

// Allow‑list (leave empty to accept *any* highway value)
const std::unordered_set<std::string> ALLOWED = {
    /* "motorway", "trunk", "primary", "secondary", "tertiary", */
};

// ───────────────────────────── helpers
std::uint64_t hashed_way_id(const osmium::Way& w) {
    std::uint64_t h = 14695981039346656037ULL;
    for (const auto& nr : w.nodes()) {
        h ^= static_cast<std::uint64_t>(nr.ref());
        h *= 1099511628211ULL;
    }
    return h;
}

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

// ───────────────────────────── bulk buffer
struct Bulk {
    explicit Bulk(mongocxx::collection c) : col{std::move(c)} {}

    void upsert(std::int64_t node, std::int64_t way) {

        // 1. Build owning BSON values
        bsoncxx::document::value filter_doc =
            make_document(kvp("_id", node));
    
        bsoncxx::document::value update_doc =
            make_document(
                kvp("$addToSet",
                    make_document(kvp("ways", way)))
            );
    
        // 2. Construct the model with *values* (will move‑construct)
        mongocxx::model::update_one u{std::move(filter_doc), std::move(update_doc)};
        u.upsert(true);
    
        // 3. Push into the ops vector
        ops.emplace_back(std::move(u));
    
        if (ops.size() >= BATCH) flush();
    }
    

    void flush() {
        if (!ops.empty()) {
            col.bulk_write(ops);
            ops.clear();
        }
    }
private:
    mongocxx::collection col;
    std::vector<mongocxx::model::update_one> ops;
};

// ───────────────────────────── Osmium handler
class Handler : public osmium::handler::Handler {
public:
    Handler(Bulk& b) : buf{b} {}

    void way(const osmium::Way& w) {
        const char* highway = w.tags()["highway"];
        if (!highway) return;                          // skip non‑highways
        if (!ALLOWED.empty() && !ALLOWED.count(highway)) return;

        std::int64_t wid = w.id() ? w.id() : (std::int64_t)hashed_way_id(w);
        for (const auto& n : w.nodes()) buf.upsert(n.ref(), wid);
    }
private:
    Bulk& buf;
};

// ───────────────────────────── main
int main(int argc, char* argv[]) try {
    if (argc < 4 || !std::strcmp(argv[1], "-h")) {
        std::cerr << "Use: " << argv[0]
                  << " file.osm.pbf mongodb://host db_name [coll=node_to_way]\n";
        return 1;
    }
    const char* pbf   = argv[1];
    const char* uri   = argv[2];
    const char* db    = argv[3];
    const char* coll  = (argc >= 5) ? argv[4] : "node_to_way";

    mongocxx::instance inst{};
    mongocxx::client   cli{mongocxx::uri{uri}};
    auto c = cli[db][coll];
    c.drop();                                    // fresh build each run

    Bulk bulk{c};
    Handler h{bulk};

    osmium::io::Reader r{pbf, osmium::osm_entity_bits::way};
    osmium::apply(r, h);
    r.close();
    bulk.flush();

    c.create_index(document{} << "ways" << 1 << finalize);
    std::cout << "Done. Docs: " << c.count_documents({}) << '\n';
}
catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << '\n';
    return 2;
}
