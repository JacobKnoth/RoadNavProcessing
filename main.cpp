#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <iostream>

int main() {
    mongocxx::instance inst{};
    mongocxx::client  conn{mongocxx::uri{}};
    std::cout << "Connected: " << conn["admin"].name() << '\n';
}
