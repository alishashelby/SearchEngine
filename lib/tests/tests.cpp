#include <gtest/gtest.h>

#include "../index/index.hpp"
#include "../search/search.hpp"
#include "../trie/trie.hpp"

#include <sstream>

class SimpleSearchEngineTest : public testing::Test {
protected:
    InvertedIndex ii;
    Search s;
};

class ParserTest : public testing::Test {
protected:
    std::unique_ptr<Parser> parser;

    bool parse(const std::string& input) {
        Lexer lexer(input);
        parser = std::make_unique<Parser>(lexer);
        try {
            auto ast = parser->parse();
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
};

using ParserDeathTest = ParserTest;

TEST_F(ParserTest, parse) {
    EXPECT_TRUE(parse("for"));
    EXPECT_TRUE(parse("vector OR list"));
    EXPECT_TRUE(parse("vector AND list"));
    EXPECT_TRUE(parse("(for)"));
    EXPECT_TRUE(parse("(vector OR list)"));
    EXPECT_TRUE(parse("(vector AND list)"));
    EXPECT_TRUE(parse("(while OR for) AND vector"));
    EXPECT_TRUE(parse("for AND and"));
}

TEST_F(ParserDeathTest, parseDeath) {
    EXPECT_EXIT({
            parse("for AND");
        }, 
        testing::ExitedWithCode(EXIT_FAILURE),
        "--unexpected token in factor"
    );

    EXPECT_EXIT({
            parse("vector list");
        },
        testing::ExitedWithCode(EXIT_FAILURE),
        "--unexpected tokens after expression"
    );

    EXPECT_EXIT({
            parse("for AND OR list");
        },
        testing::ExitedWithCode(EXIT_FAILURE),
        "--unexpected token in factor"
    );

    EXPECT_EXIT({
            parse("vector Or list");
        },
        testing::ExitedWithCode(EXIT_FAILURE),
        "--unexpected tokens after expression"
    );
}

TEST_F(SimpleSearchEngineTest, K1) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(1);

    std::string input = "pupa";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \n");
}

TEST_F(SimpleSearchEngineTest, K2) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(2);
    std::string input = "pupa";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \nTERM: 'pupa'\n     name of file ../../test/3.txt   nums of lines: 1 4 5 6 \n");
}

TEST_F(SimpleSearchEngineTest, K3_Cout2) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(3);
    std::string input = "pupa";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \nTERM: 'pupa'\n     name of file ../../test/3.txt   nums of lines: 1 4 5 6 \n");
}

TEST_F(SimpleSearchEngineTest, OR_K1) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(1);
    std::string input = "pupa OR papulya";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \nTERM: 'papulya'\n     name of file ../../test/1.txt   nums of lines: 7 \n");
}

TEST_F(SimpleSearchEngineTest, OR_K2) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(2);
    std::string input = "pupa OR papulya";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \nTERM: 'papulya'\n     name of file ../../test/1.txt   nums of lines: 7 \nTERM: 'papulya'\n     name of file ../../test/2.txt   nums of lines: 9 \n");
}

TEST_F(SimpleSearchEngineTest, OR_K3) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(3);
    std::string input = "pupa OR papulya";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \nTERM: 'papulya'\n     name of file ../../test/1.txt   nums of lines: 7 \nTERM: 'papulya'\n     name of file ../../test/2.txt   nums of lines: 9 \nTERM: 'pupa'\n     name of file ../../test/3.txt   nums of lines: 1 4 5 6 \nTERM: 'papulya'\n     name of file ../../test/3.txt   nums of lines: 3 7 \n");
}

TEST_F(SimpleSearchEngineTest, AND_K1) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(1);
    std::string input = "pupa AND papulya";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \nTERM: 'papulya'\n     name of file ../../test/1.txt   nums of lines: 7 \n");
}

TEST_F(SimpleSearchEngineTest, AND_K2) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(2);
    std::string input = "pupa AND papulya";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \nTERM: 'papulya'\n     name of file ../../test/1.txt   nums of lines: 7 \nTERM: 'pupa'\n     name of file ../../test/3.txt   nums of lines: 1 4 5 6 \nTERM: 'papulya'\n     name of file ../../test/3.txt   nums of lines: 3 7 \n");
}

TEST_F(SimpleSearchEngineTest, AND_K3_Cout2) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(3);
    std::string input = "pupa AND papulya";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'pupa'\n     name of file ../../test/1.txt   nums of lines: 1 5 10 14 \nTERM: 'papulya'\n     name of file ../../test/1.txt   nums of lines: 7 \nTERM: 'pupa'\n     name of file ../../test/3.txt   nums of lines: 1 4 5 6 \nTERM: 'papulya'\n     name of file ../../test/3.txt   nums of lines: 3 7 \n");
}

TEST_F(SimpleSearchEngineTest, OR_K1_InDifferentFiles) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(1);
    std::string input = "heheheheh OR hello";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "TERM: 'hello'\n     name of file ../../test/3.txt   nums of lines: 8 \n");
}

TEST_F(SimpleSearchEngineTest, AND_K1_InDifferentFiles) {
    ii.erase();
    ii.traverse("../../test");

    s.chooseK(1);
    std::string input = "heheheheh AND hello";

    std::stringstream buffer;
    std::streambuf* coutbuf = std::cout.rdbuf(buffer.rdbuf());
    s.createParser(input);
    std::cout.rdbuf(coutbuf);
    std::string output = buffer.str();

    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output, "--sorry, nothing was found");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    return RUN_ALL_TESTS();
}
