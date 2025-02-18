#include "search.hpp"

int main(int argc, char* argv[]) {
    
    Search s;

    if (argc == 2) {
        s.chooseK(std::stoi(argv[1]));
    }
        
    std::string input;
    std::getline(std::cin, input);

    s.createParser(input);
}