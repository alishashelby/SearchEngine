#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <set>

extern const char* files_paths_p;
extern const char* posting_lists_p;
extern const char* trie_p;
extern const char* line_nums_p;

const int postingListSizeof = 4096;

struct TrieNode {
    char symbol;
    int64_t posting_list_pos;
    int64_t line_nums_pos;
    std::vector<TrieNode*> children;
    std::set<int64_t> set_nums_of_lines;

    TrieNode() : posting_list_pos(-1), line_nums_pos(-1) {}
    TrieNode(char s) : symbol(s), posting_list_pos(-1), line_nums_pos(-1) {}

    void eraseset() {
        set_nums_of_lines.clear();
    }
};

class Trie {
public:
    Trie() : root_(new TrieNode()) {}

    ~Trie() {
        delete root_;
    }

    int64_t insert(std::string& term, std::fstream& posting_lists, int64_t& num_of_line) {
        TrieNode* node = root_;
        bool flag = false;

        for (char c : term) {
            flag = false;
            for (TrieNode* i : node->children) {
                if (i->symbol == c) {
                    flag = true;
                    node = i;
                    break;
                }
            }
            if (!flag) {
                TrieNode* child = new TrieNode(std::tolower(c));
                node->children.push_back(child);
                node = child;
            }
        }

        node->set_nums_of_lines.insert(num_of_line);
        node->line_nums_pos = 0;
        if (node->posting_list_pos == -1) {

            posting_lists.seekp(0, std::ios::end);
            node->posting_list_pos = posting_lists.tellp();

            int64_t posting_list_size = 0;
            posting_lists.write(reinterpret_cast<char*>(&posting_list_size), sizeof(int64_t));

            char* posting_list = new char[postingListSizeof];
            for (int64_t i = 0; i < postingListSizeof; ++i) {
                posting_list[i] = 0;
            }
            posting_lists.write(posting_list, postingListSizeof);
            delete[] posting_list;

        }
        return node->posting_list_pos;
    }

    int64_t find(std::string term) {
        TrieNode* node = root_;
        bool flag;
        for (char c : term) {
            flag = false;
            for (TrieNode* i : node->children) {
                if (i->symbol == c) {
                    flag = true;
                    node = i;
                    break;
                }
            }
            if (!flag) {
                return -1;
            }
        }
        return node->posting_list_pos;
    }

    void saveTrieInFile(std::fstream& trie_file) {
        save(root_, trie_file);
    }

    Trie* saveBackToRAM(std::fstream& trie_file) {
        root_ = toRAM(trie_file);
        return this;
    }

    void writelinesinfile(int64_t& file_id) {
        std::fstream line_nums;
        line_nums.open(line_nums_p, std::ios::binary | std::ios::in | std::ios::out);

        std::fstream posting_lists;
        posting_lists.open(posting_lists_p, std::ios::binary | std::ios::in | std::ios::out);

        std::string currentWord;
        posting_lists.seekg(0);
        writelinesinfilehelp(root_, currentWord, line_nums, file_id, posting_lists);
        posting_lists.seekg(0);

        line_nums.close();
        posting_lists.close();
    }

private:
    TrieNode* root_;

    void save(const TrieNode* node, std::fstream& trie_file) {
        if (!node) return;

        trie_file.write(&node->symbol, sizeof(char));
        trie_file.write(reinterpret_cast<const char*>(&node->posting_list_pos), sizeof(int64_t));
        trie_file.write(reinterpret_cast<const char*>(&node->line_nums_pos), sizeof(int64_t));

        size_t children_count = node->children.size();
        trie_file.write(reinterpret_cast<const char*>(&children_count), sizeof(size_t));

        size_t set_size = node->set_nums_of_lines.size();
        trie_file.write(reinterpret_cast<const char*>(&set_size), sizeof(size_t));
        for (const int64_t& line_num : node->set_nums_of_lines) {
            trie_file.write(reinterpret_cast<const char*>(&line_num), sizeof(int64_t));
        }

        for (TrieNode* child : node->children) {
            save(child, trie_file);
        }
    }

    TrieNode* toRAM(std::fstream& trie_file) {
        char symbol;
        int64_t posting_list_pos;
        int64_t line_nums_pos;
        size_t children_count;

        trie_file.read(&symbol, sizeof(char));
        trie_file.read(reinterpret_cast<char*>(&posting_list_pos), sizeof(int64_t));
        trie_file.read(reinterpret_cast<char*>(&line_nums_pos), sizeof(int64_t));

        trie_file.read(reinterpret_cast<char*>(&children_count), sizeof(size_t));

        TrieNode* node = new TrieNode(symbol);
        node->posting_list_pos = posting_list_pos;
        node->line_nums_pos = line_nums_pos;
        node->children.resize(children_count);


        size_t set_size;
        trie_file.read(reinterpret_cast<char*>(&set_size), sizeof(size_t));
        for (size_t i = 0; i < set_size; ++i) {
            int64_t line_num;
            trie_file.read(reinterpret_cast<char*>(&line_num), sizeof(int64_t));
            node->set_nums_of_lines.insert(line_num);
        }

        for (size_t i = 0; i < children_count; ++i) {
            node->children[i] = toRAM(trie_file);
        }

        return node;
    }

    void writelinesinfilehelp(TrieNode* node, std::string& currentWord, std::fstream& line_nums, int64_t& file_id, std::fstream& posting_lists) {
        if (!node) return;

        if (node->line_nums_pos != -1) {

            line_nums.seekp(0, std::ios::end);

            int64_t pos_of_lines = line_nums.tellg();


            posting_lists.seekg(node->posting_list_pos);
            int64_t df;
            posting_lists.read(reinterpret_cast<char*>(&df), sizeof(int64_t));

            int64_t dId_ind;
            int64_t cur_pos;
            for (int64_t i = 0; i < df; ++i) {
                posting_lists.seekp(node->posting_list_pos + sizeof(int64_t) + i * (40));
                posting_lists.read(reinterpret_cast<char*>(&dId_ind), sizeof(int64_t));
                if (dId_ind == file_id) {
                    cur_pos = posting_lists.tellg();
                    posting_lists.seekg(cur_pos + 3 * sizeof(int64_t));
                    posting_lists.write(reinterpret_cast<char*>(&pos_of_lines), sizeof(int64_t));

                    break;
                }
            }


            line_nums.seekp(pos_of_lines);

            int64_t line_nums_count = node->set_nums_of_lines.size();
            line_nums.write(reinterpret_cast<char*>(&line_nums_count), sizeof(int64_t));

            for (int64_t i : node->set_nums_of_lines) {
                line_nums.write(reinterpret_cast<char*>(&i), sizeof(int64_t));
            }

            node->eraseset();
            node->line_nums_pos = -1;
        }

        for (TrieNode* child : node->children) {
            if (child) {
                currentWord.push_back(child->symbol);
                writelinesinfilehelp(child, currentWord, line_nums, file_id, posting_lists);
                currentWord.pop_back();
            }
        }
    }
};
