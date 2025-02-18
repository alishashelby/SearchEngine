#include "../trie/trie.hpp"

#include <string>

enum class TokenType {
    WORD,
    AND,
    OR,
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
    END,
};

struct Token {
    TokenType type;
    std::string value;
};

struct Node {
    int64_t id;
    int64_t file_pos_in_posting_list;
    double bm;

    bool operator<(const Node& other) const {
        return other.bm < bm;
    }

};

struct ASTNode {
    double bm;
    TokenType type;
    std::string value;
    std::shared_ptr<ASTNode> left;
    std::shared_ptr<ASTNode> right;
    std::shared_ptr<ASTNode> parent;

    ASTNode(TokenType type, std::string value)
        : type(type), value(std::move(value)), left(nullptr), right(nullptr), parent(nullptr) {}
};

class Lexer {
public:
    explicit Lexer(std::string input) : input(std::move(input)) {
        this->pos = 0;
    }

    Token getNextToken() {
        while (pos < input.length()) {
            while (pos < input.length() && std::isspace(input[pos])) {
                ++pos;
            }

            if (pos == input.length()) {
                return {TokenType::END, ""};
            }

            if (std::isalpha(input[pos])) {
                std::string value;
                while (pos < input.length() && std::isalpha(input[pos])) {
                    value += input[pos++];
                }
                if (value == "AND") {
                    return {TokenType::AND, value};
                } else if (value == "OR") {
                    return {TokenType::OR, value};
                } else {
                    for (size_t i = 0; i < value.length(); ++i) {
                        value[i] = std::tolower(value[i]);
                    }
                    return {TokenType::WORD, value};
                }
            }

            char current = input[pos++];
            switch (current) {
                case '(':
                    return {TokenType::OPEN_PARENTHESIS, "("};
                case ')':
                    return {TokenType::CLOSE_PARENTHESIS, ")"};
                default:
                    std::cerr << "--invalid character encountered";
                    std::exit(EXIT_FAILURE);
            }
        }

        return {TokenType::END, ""};
    }

private:
    std::string input;
    size_t pos = 0;
};

class Parser {
public:
    explicit Parser(Lexer lexer) : lexer(std::move(lexer)) {
        currentToken = this->lexer.getNextToken();
    }

    int64_t getInd() {
        return ind;
    }

    void setInd(int64_t x) {
        ind = x;
    }

    std::shared_ptr<ASTNode> parse() {
        auto ast = expr();
        if (currentToken.type != TokenType::END) {
            std::cerr << "--unexpected tokens after expression";
            std::exit(EXIT_FAILURE);
        }
        return ast;
    }

    void getTermsFromAST(std::shared_ptr<ASTNode> node, std::vector<std::string>& all_terms) {
        if (node == nullptr) {
            return;
        }

        getTermsFromAST(node->left, all_terms);
        getTermsFromAST(node->right, all_terms);

        if (node->type == TokenType::WORD) {
            all_terms.push_back(node->value);
        }

    }

    void setZero(std::shared_ptr<ASTNode> node) {
        if (node == nullptr) {
            return;
        }

        setZero(node->left);
        setZero(node->right);

        if (node->type == TokenType::WORD) {
            node->bm = 0;
        }
    }

    void setScores(std::shared_ptr<ASTNode> node, std::unordered_map<std::string, double>& map) {
        if (node == nullptr) {
            return;
        }

        setScores(node->left, map);
        setScores(node->right, map);

        if (node->type == TokenType::WORD) {
            node->bm = map[node->value];
        }
    }

    void setORanD(std::shared_ptr<ASTNode> node) {
        if (node == nullptr) {
            return;
        }

        setORanD(node->left);
        setORanD(node->right);

        if (node->type == TokenType::AND) {
            node->bm = node->left->bm * node->right->bm;
        } else if (node->type == TokenType::OR) {
            node->bm = node->left->bm + node->right->bm;
        }
    }
private:

    Lexer lexer;
    Token currentToken;
    int64_t ind;

    void eat(TokenType type) {
        if (currentToken.type == type) {
            currentToken = lexer.getNextToken();
        } else {
            std::cerr << "--unexpected token type";
            std::exit(EXIT_FAILURE);
        }
    }

    std::shared_ptr<ASTNode> factor() {
        if (currentToken.type == TokenType::WORD) {
            auto node = std::make_shared<ASTNode>(currentToken.type, currentToken.value);
            eat(TokenType::WORD);
            return node;
        } else if (currentToken.type == TokenType::OPEN_PARENTHESIS) {
            eat(TokenType::OPEN_PARENTHESIS);
            auto node = expr();
            eat(TokenType::CLOSE_PARENTHESIS);
            return node;
        }
        std::cerr << "--unexpected token in factor";
        std::exit(EXIT_FAILURE);
    }

    std::shared_ptr<ASTNode> term() {
        auto node = factor();
        while (currentToken.type == TokenType::AND) {
            auto token = currentToken;
            eat(TokenType::AND);
            auto newNode = std::make_shared<ASTNode>(token.type, token.value);
            newNode->left = node;
            newNode->left->parent = newNode;
            newNode->right = factor();
            newNode->right->parent = newNode;
            node = newNode;
        }
        return node;
    }

    std::shared_ptr<ASTNode> expr() {
        auto node = term();
        while (currentToken.type == TokenType::OR) {
            auto token = currentToken;
            eat(TokenType::OR);
            auto newNode = std::make_shared<ASTNode>(token.type, token.value);
            newNode->left = node;
            newNode->left->parent = newNode;
            newNode->right = term();
            newNode->right->parent = newNode;
            node = newNode;
        }
        return node;
    }
};
