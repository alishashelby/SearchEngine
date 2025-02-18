#pragma once
#include "../trie/trie.hpp"

#include <filesystem>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

struct DID {
    int64_t ind;
    int64_t name_pos;
    int64_t dl;
    int64_t tf;
    int64_t pos_of_nums_lines;
    DID() = default;
    DID(int64_t x, int64_t p) : ind(x), name_pos(p), dl(0), tf(0) {}
};

class InvertedIndex {
public:
    InvertedIndex() : doc_count_(0), terms_count(0), trie(new Trie()) {}

    ~InvertedIndex() {
        delete trie;
    }

    void erase() {
        std::fstream files_paths;
        files_paths.open(files_paths_p, std::ios::out | std::ios::trunc);
        files_paths.clear();
        files_paths.close();

        std::fstream posting_lists;
        posting_lists.open(posting_lists_p, std::ios::out | std::ios::trunc);
        posting_lists.clear();
        posting_lists.close();

        std::fstream line_nums;
        line_nums.open(line_nums_p, std::ios::out | std::ios::trunc);
        line_nums.clear();
        line_nums.close();

        std::fstream trie_tree;
        trie_tree.open(trie_p, std::ios::out | std::ios::trunc);
        trie_tree.clear();
        trie_tree.close();
    }

    void traverse(const fs::path& path) {

        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::recursive_directory_iterator(path)) {
                if (fs::is_regular_file(entry.status()) && entry.path().filename().string() != ".DS_Store") {

                    addDoc(entry.path().string().c_str());
                    ++doc_count_;
                }
            }
        } else {
            std::cerr << "--path is not a directory || does not exist." << '\n';
            std::exit(EXIT_FAILURE);
        }
    
        int64_t dlavg = terms_count / doc_count_;

        std::fstream trie_tree;
        trie_tree.open(trie_p, std::ios::out | std::ios::trunc);
        trie_tree.clear();
        trie_tree.close();

        trie_tree.open(trie_p, std::ios::binary | std::ios::in | std::ios::out);
        trie_tree.seekp(0);

        trie_tree.write(reinterpret_cast<char*>(&doc_count_), sizeof(int64_t));
        trie_tree.write(reinterpret_cast<char*>(&dlavg), sizeof(int64_t));
        trie->saveTrieInFile(trie_tree);

        trie_tree.close();
    }

private:
    int64_t doc_count_;
    int64_t terms_count;

    Trie* trie;

    void addDoc(const char* p) {
        std::fstream files_paths;
        files_paths.open(files_paths_p, std::ios::binary | std::ios::in | std::ios::out);
        files_paths.seekg(0, std::ios::end);
        DID dId = DID(doc_count_, files_paths.tellg());

        int64_t file_path_len = strlen(p);
        files_paths.write(reinterpret_cast<char*>(&file_path_len), sizeof(int64_t));
        files_paths.write(p, file_path_len);

        files_paths.close();

        GetTerms(p, dId);
        trie->writelinesinfile(dId.ind);
    }

    void GetTerms(const char* p, DID& dId) {
        std::fstream file;
        file.open(p, std::ios::binary | std::ios::in | std::ios::out);
        if (!file.is_open()) {
            std::cout << "--expected an input file";
            std::exit(EXIT_FAILURE);
        }

        char c;
        std::string term;
        while (file.get(c)) {
            if (isalpha(c)) {
                term.push_back(std::tolower(c));
            } else {
                if (!term.empty()) {
                    ++dId.dl;
                }
                term.clear();
            }
        }
        if (!term.empty()) {
            ++dId.dl;
        }
        terms_count += dId.dl;
        term.clear();

        file.close();
        file.open(p, std::ios::binary | std::ios::in | std::ios::out);

        int64_t line_num_in_file = 1;
        while (file.get(c)) {
            if (c != ' ' && c != '\n') {
                term.push_back(std::tolower(c));
            } else {

                if (!term.empty()) {
                    addDocToPostingList(term, p, dId, line_num_in_file);
                }
                if (c == '\n') {
                    ++line_num_in_file;
                }
                term.clear();
            }
        }
        if (!term.empty()) {
            addDocToPostingList(term, p, dId, line_num_in_file);
        }

        file.close();
    }

    void addDocToPostingList(std::string& term, const char* p, DID& dId, int64_t& num_of_line) {
        std::fstream posting_lists;
        posting_lists.open(posting_lists_p, std::ios::binary | std::ios::in | std::ios::out);

        int64_t posting_list_pos = trie->insert(term, posting_lists, num_of_line);
        posting_lists.seekg(posting_list_pos);
        int64_t df;
        posting_lists.read(reinterpret_cast<char*>(&df), sizeof(int64_t));

        DID temp;
        bool is_here = false;

        for (int64_t i = 0; i < df; ++i) {
            posting_lists.seekp(posting_list_pos + sizeof(int64_t) + i * (sizeof(DID)));
            posting_lists.read(reinterpret_cast<char*>(&temp), sizeof(DID));

            if (temp.ind == dId.ind) {
                ++temp.tf;
                int64_t pt = posting_lists.tellg();
                posting_lists.seekp(pt - 2 * sizeof(int64_t));
                posting_lists.write(reinterpret_cast<char*>(&temp.tf), sizeof(int64_t));
                is_here = true;
                break;
            }
        }

        if (is_here) {
            posting_lists.seekg(0, std::ios::end);

            return;
        }

        ++df;
        temp.tf = 1;
        posting_lists.seekp(posting_list_pos);
        posting_lists.write(reinterpret_cast<char*>(&df), sizeof(int64_t));
        posting_lists.seekp(posting_list_pos + sizeof(int64_t) + df * sizeof(DID) -  sizeof(DID));
        posting_lists.write(reinterpret_cast<char*>(&dId), sizeof(DID));
        posting_lists.seekg(0, std::ios::end);

        posting_lists.close();
    }
};
