#include "index.hpp"

int main(int argc, char* argv[]) {

    InvertedIndex ii;
    ii.erase();
    ii.traverse(argv[1]);
}
