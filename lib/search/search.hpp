#pragma once
#include "parsing.hpp"

#include <queue>
#include <functional>
#include <cmath>
#include "algorithm"

class Search {
public:
    Search() : trie(new Trie()), k_(1) {
        std::fstream trie_tree;
        trie_tree.open(trie_p, std::ios::binary | std::ios::in | std::ios::out);

        trie_tree.seekg(0);

        trie_tree.read(reinterpret_cast<char*>(&doc_count), sizeof(int64_t));
        scores_of_files.resize(doc_count, 0.0);

        trie_tree.read(reinterpret_cast<char*>(&dlavg), sizeof(int64_t));
        trie = trie->saveBackToRAM(trie_tree);
        trie_tree.close();
    }
    
    ~Search() {
        delete trie;
        delete lexer;
    }

    void chooseK(int kaka) {
        k_ = kaka;
    }

    void createParser(std::string& input) {
        lexer = new Lexer(input);
        parser = new Parser(*lexer);

        std::shared_ptr<ASTNode> ast;
        try {
            ast = (*parser).parse();
        } catch (const std::exception& e) {
            std::cerr << "--error: " << e.what() << '\n';
        }

        std::vector<std::string> all_terms;
        parser->getTermsFromAST(ast, all_terms);

        std::vector<std::pair<int64_t, std::string> > iterators;
        std::unordered_map<std::string, int64_t> iterators_indexes;
        for (auto it : all_terms) {
            iterators.push_back({doc_count + 1, it});
        }

        std::fstream posting_lists;
        posting_lists.open(posting_lists_p, std::ios::binary | std::ios::in | std::ios::out);

        for (int64_t i = 0; i < all_terms.size(); ++i) {
            int64_t posting_list_pos = trie->find(all_terms[i]);
            if (posting_list_pos == -1) {
                std::cerr << "--term was not found in trie: " << all_terms[i] << '\n';
                std::exit(EXIT_FAILURE);
            }

            posting_lists.seekg(posting_list_pos);
            int64_t df;
            posting_lists.read(reinterpret_cast<char*>(&df), sizeof(int64_t));

            if (df > 0) {
                posting_lists.read(reinterpret_cast<char*>(&iterators[i].first), sizeof(int64_t));
            }
        }
        posting_lists.close();

        posting_lists.open(posting_lists_p, std::ios::binary | std::ios::in | std::ios::out);

        bool flag = false;
        while (true) {
            std::sort(iterators.begin(), iterators.end());
            std::unordered_map<std::string, double> map;

            int64_t index = 0;
            for (; index < iterators.size(); ++index) {
                if (index > 0 && iterators[index - 1].first != iterators[index].first) {
                    break;
                }
                double score = findScore(iterators[index].first, iterators[index].second);
                map.insert({iterators[index].second, score});

                ++iterators_indexes[iterators[index].second];
                parser->setInd(iterators[index].first);
            }
            parser->setZero(ast);
            parser->setScores(ast, map);
            parser->setORanD(ast);
            double rez = ast->bm;
            if (rez > 0) {
                pr.push({parser->getInd(), rez});
            }

            int64_t cur_posting_list_pos;

            for (auto& it : iterators) {

                cur_posting_list_pos = trie->find(it.second);
                posting_lists.seekg(cur_posting_list_pos);

                int64_t df;
                posting_lists.read(reinterpret_cast<char*>(&df), sizeof(int64_t));
                cur_posting_list_pos = posting_lists.tellg();

                if (it.first != doc_count + 1) {
                    if (iterators_indexes[it.second] >= df) {
                        flag = true;

                        break;
                    }
                    posting_lists.seekg(cur_posting_list_pos + (iterators_indexes[it.second]) * 5 * sizeof(int64_t));
                    posting_lists.read(reinterpret_cast<char*>(&it.first), sizeof(int64_t));
                }
            }
            if (flag) { 
                break; 
            }
        }
        posting_lists.close();
        double rez = ast->bm;
        
        DisplayAnswer(all_terms);
    }

private:

    Trie* trie;
    int64_t dlavg;
    int64_t doc_count;
    Lexer* lexer;
    Parser* parser;
    int64_t k_;

    std::vector<double> scores_of_files;
    std::priority_queue<std::pair<int64_t, double> > pr;

    void DisplayAnswer(std::vector<std::string>& all_terms) {
        if (pr.size() == 0) {
            std::cout << "--sorry, nothing was found";

            return;
        }

        std::fstream posting_lists;
        posting_lists.open(posting_lists_p, std::ios::binary | std::ios::in | std::ios::out);

        std::fstream files_paths;
        files_paths.open(files_paths_p, std::ios::binary | std::ios::in | std::ios::out);

        std::fstream line_nums;
        line_nums.open(line_nums_p, std::ios::binary | std::ios::in | std::ios::out);

        while (pr.size() != 0 && k_ > 0) {
            int64_t cur_posting_list_pos;

            for (std::string& term : all_terms) {
                cur_posting_list_pos = trie->find(term);

                posting_lists.seekg(cur_posting_list_pos);
                int64_t df;
                posting_lists.read(reinterpret_cast<char*>(&df), sizeof(int64_t));

                for (int64_t i = 0; i < df; ++i) {
                    posting_lists.seekp(cur_posting_list_pos + sizeof(int64_t) + i * 5 * sizeof(int64_t));
                    int64_t file_ind;
                    posting_lists.read(reinterpret_cast<char*>(&file_ind), sizeof(int64_t));

                    if (file_ind == pr.top().first) {
                        std::cout << "TERM: '" << term << "'\n     ";
                        int64_t name_pos;
                        posting_lists.read(reinterpret_cast<char*>(&name_pos), sizeof(int64_t));
                        
                        files_paths.seekg(name_pos);

                        int64_t file_path_len;
                        files_paths.read(reinterpret_cast<char*>(&file_path_len), sizeof(int64_t));

                        char* buffer = new char[file_path_len + 1];
                        buffer[file_path_len] = '\0';
                        files_paths.read(buffer, file_path_len);

                        std::cout << "name of file " << buffer << "   nums of lines: ";

                        cur_posting_list_pos = posting_lists.tellg();
                        posting_lists.seekg(cur_posting_list_pos + 2 * sizeof(int64_t));

                        int64_t line_nums_pos;
                        posting_lists.read(reinterpret_cast<char*>(&line_nums_pos), sizeof(int64_t));

                        line_nums.seekg(line_nums_pos);

                        int64_t line_nums_count;
                        line_nums.read(reinterpret_cast<char*>(&line_nums_count), sizeof(int64_t));

                        int64_t line;
                        for (int64_t j = 0; j < line_nums_count; ++j) {
                            line_nums.read(reinterpret_cast<char*>(&line), sizeof(int64_t));
                            std::cout << line << " ";
                        }

                        std::cout << '\n';

                        break;
                    }
                }
            }
            pr.pop();
            --k_;
        }

        posting_lists.close();
        files_paths.close();
        line_nums.close();
    }

    double findScore(int64_t id, std::string& term) {
        std::fstream posting_lists;
        posting_lists.open(posting_lists_p, std::ios::binary | std::ios::in | std::ios::out);

        int64_t posting_list_pos = trie->find(term);
        posting_lists.seekg(posting_list_pos);

        int64_t df;
        int64_t cur_posting_list_pos;
        posting_lists.read(reinterpret_cast<char*>(&df), sizeof(int64_t));
        for (int64_t i = 0; i < df; ++i) {
            int64_t ind;
            posting_lists.read(reinterpret_cast<char*>(&ind), sizeof(int64_t));

            if (ind != id) {
                cur_posting_list_pos = posting_lists.tellg();
                posting_lists.seekg(cur_posting_list_pos + 4 * sizeof(int64_t));
                continue;
            }

            cur_posting_list_pos = posting_lists.tellg();
            posting_lists.seekg(cur_posting_list_pos + sizeof(int64_t));

            int64_t dl;
            posting_lists.read(reinterpret_cast<char*>(&dl), sizeof(int64_t));

            int64_t tf;
            posting_lists.read(reinterpret_cast<char*>(&tf), sizeof(int64_t));

            cur_posting_list_pos = posting_lists.tellg();
            posting_lists.seekg(cur_posting_list_pos + sizeof(int64_t));

            posting_lists.close();

            return BM25(tf, df, dlavg, dl);
        }
    }

    double BM25(int64_t& tf, int64_t& df, int64_t& dlavg, int64_t& dl) {
        ++tf;
        double rez = 0.0;
        double k = 1.2;
        double b = 0.75;

        double qtw = log((df - df + 0.5) / (df + 0.5));

        for (int64_t t = 0; t < tf; ++t) {
            rez += (tf * (k + 1)) / (tf + k * (1 - b + b * static_cast<double>(dl) / dlavg));
        }

        return rez;
    }

};
