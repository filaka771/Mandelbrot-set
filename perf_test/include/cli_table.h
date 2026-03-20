#pragma once
#include <boost/math/constants/constants.hpp>
#include <vector>
#include <string>
#include <numeric>
#include <iostream>

namespace cli_table {
    namespace color {
        const std::string RESET   = "\033[0m";
        const std::string BLACK   = "\033[30m";
        const std::string RED     = "\033[31m";
        const std::string GREEN   = "\033[32m";
        const std::string YELLOW  = "\033[33m";
        const std::string BLUE    = "\033[34m";
        const std::string MAGENTA = "\033[35m";
        const std::string CYAN    = "\033[36m";
        const std::string WHITE   = "\033[37m";
        const std::string BOLD    = "\033[1m";
    }

    namespace symbol {
        const std::string HORZ = "─";
        const std::string VERT = "│";
        const std::string TL   = "┌"; // top-left
        const std::string TR   = "┐"; // top-right
        const std::string BL   = "└"; // bottom-left
        const std::string BR   = "┘"; // bottom-right
        const std::string ML   = "├"; // middle-left
        const std::string MR   = "┤"; // middle-right
        const std::string TM   = "┬"; // top-middle (T down)
        const std::string BM   = "┴"; // bottom-middle (T up)
        const std::string MM   = "┼"; // middle-middle
    }

    class table {
    public:
        void print() {
            // Print title
            if (table_title_ != "") {
                print_horizontal_line();
                print_title_text();
                print_horizontal_line();
            }
            // Print table
            print_top_horizontal_border();

            for(int row = 0; row < (table_.row_num - 1); row ++) {
                print_table_row(row);
                print_inter_horizontal_border();
            }

            print_table_row(table_.row_num - 1);
            print_botom_horizontal_border();
        };

        explicit table(const std::vector<std::vector<std::string>>& cells_content, const std::string& title = "")
        : cells_content_(cells_content), table_title_(title) {
            determine_table_layout();
        }
    private:
        struct table_layout {
            int col_num;
            int row_num;
            std::vector<int> cols_width;
        };

        table_layout table_;
        std::string table_title_;
        std::vector<std::vector<std::string>> cells_content_;

        // Helper to repeat a string n times
        static std::string repeat(const std::string& s, int n) {
            std::string result;
            result.reserve(s.size() * n);
            for (int i = 0; i < n; ++i)
                result += s;
            return result;
        }

        void determine_table_layout() {
            // Set num of lines and columns
            table_.row_num = cells_content_.size();
            table_.col_num = cells_content_[0].size();

            table_.cols_width.resize(table_.col_num);

            // Calculate appropriate cells sizes
            for(int column = 0; column < table_.col_num; column ++) {
                table_.cols_width[column] = 0;
                for(int row = 0 ; row < table_.row_num; row ++) {
                    if(table_.cols_width[column] < (int)cells_content_[row][column].length()) 
                        table_.cols_width[column] =
                            cells_content_[row][column].length();
                }
            }
        };

        // ----------------------- Auxiliary table print methods -----------------------
        void print_top_horizontal_border() {
            std::cout << symbol::TL;
            for (int col = 0; col < table_.col_num; ++col) {
                // +2 for the spaces around the content
                std::cout << repeat(symbol::HORZ, table_.cols_width[col] + 2);
                if (col < table_.col_num - 1)
                    std::cout << symbol::TM;
            }
            std::cout << symbol::TR << std::endl;
        };

        void print_inter_horizontal_border() {
            std::cout << symbol::ML;
            for (int col = 0; col < table_.col_num; ++col) {
                std::cout << repeat(symbol::HORZ, table_.cols_width[col] + 2);
                if (col < table_.col_num - 1)
                    std::cout << symbol::MM;
            }
            std::cout << symbol::MR << std::endl;
        };

        void print_botom_horizontal_border() {
            std::cout << symbol::BL;
            for (int col = 0; col < table_.col_num; ++col) {
                std::cout << repeat(symbol::HORZ, table_.cols_width[col] + 2);
                if (col < table_.col_num - 1)
                    std::cout << symbol::BM;
            }
            std::cout << symbol::BR << std::endl;
        };

        void print_table_row(int row) {
            std::cout << symbol::VERT << " " << cells_content_[row][0];
            int padding = table_.cols_width[0] - cells_content_[row][0].length() + 1;
            std::cout << std::string(padding, ' ') << symbol::VERT;


            for (int column = 1; column < table_.col_num; column ++) {
                std::cout << " " <<cells_content_[row][column];
                // Pad to column width
                int padding = table_.cols_width[column] - cells_content_[row][column].length() + 1;
                std::cout << std::string(padding, ' ') << symbol::VERT;
            }

            std::cout << std::endl;
        };

        // ----------------------- Auxiliary title print methods -----------------------

        void print_horizontal_line() {
            int table_width = std::accumulate(table_.cols_width.begin(), table_.cols_width.end()
                                              , 0, std::plus<int>());
            table_width += (table_.col_num + 1) * 2 + 1;

            for(int dash = 0; dash < table_width; ++dash) {
                std::cout << symbol::HORZ;
            }

            std::cout << std::endl;
        };

        void print_dash_line() {
            int table_width = std::accumulate(table_.cols_width.begin(), table_.cols_width.end()
                                              , 0, std::plus<int>());
            table_width += (table_.col_num + 1) * 2;

            std::cout << " ";

            for(int dash = 0; dash < table_width / 2; ++dash) {
                std::cout << symbol::HORZ << " ";
            }

            std::cout << std::endl;
        };

        void print_title_text() {
            int table_width = std::accumulate(table_.cols_width.begin(), table_.cols_width.end()
                                              , 0, std::plus<int>());
            table_width += (table_.col_num + 1) * 2;

            int prev_space = 0;
            int prev_newline = 0;
            int current_space = 0;
            for(int symb = 0; symb < table_title_.length(); symb ++) {
                if(table_title_[symb] == ' ') {
                    current_space = symb; 

                    if(current_space - prev_newline < table_width)
                        prev_space = current_space;

                    else if(current_space - prev_newline >= table_width && prev_space != prev_newline) {
                        table_title_.insert(table_title_.begin() + prev_space, '\n');
                        prev_space   = current_space;
                        prev_newline = current_space;
                    }
                    else {
                        table_title_.insert(table_title_.begin() + current_space, '\n');
                        prev_space   = current_space;
                        prev_newline = current_space;
                    }
                }
            }
            std::cout << " " << table_title_ << std::endl;
        };
    };
}
