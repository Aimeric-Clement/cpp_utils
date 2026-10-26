#ifndef STRINGMANIP_HPP
#define STRINGMANIP_HPP

#include <string>
#include <functional>
#include <array>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <algorithm>

namespace utl
{
    inline std::string substitute_braced_tokens(const std::string& text
                                       , const std::function<std::string(const std::string&)> pick_replacement
                                       , const std::string& brace_open
                                       , const std::string& brace_close)
    {
        size_t i = 0;
        size_t open_brace_pos = text.size();
        std::string final_text = text;

        while(i <= final_text.size())
        {
            if(final_text.substr(i, brace_open.size()) == brace_open)
            {
                open_brace_pos = i;
                i += brace_open.size();
            }
            else if (final_text.substr(i, brace_close.size()) == brace_close && open_brace_pos != final_text.size())
            {
                // Found closing bracket

                // Split the different components of the string
                const std::string token =   std::string(final_text.cbegin() + open_brace_pos + brace_open.size()
                                                      , final_text.cbegin() + i);
                const std::string before =  std::string(final_text.cbegin()
                                                      , final_text.cbegin() + open_brace_pos);
                const std::string after =   std::string(final_text.cbegin() + i + brace_close.size()
                                                      , final_text.cend());

                // Get the value for the given variable and insert it back in
                const std::string substitute = pick_replacement(token);

                final_text = before + substitute + after;
                i = open_brace_pos + substitute.size();
            }
            else
                i++;
        }
        return final_text;
    }

    inline std::vector<std::string> list_braced_tokens(const std::string& text
                                              , const std::string& brace_open
                                              , const std::string& brace_close) noexcept
    {
        std::vector<std::string> tokens;
        size_t i = 0;
        size_t open_brace_pos = text.size();

        while(i <= text.size())
        {
            if(text.substr(i, brace_open.size()) == brace_open)
            {
                open_brace_pos = i;
            }
            else if (text.substr(i, brace_close.size()) == brace_close && open_brace_pos != text.size())
            {
                // Found closing bracket:
                // register the braced token
                tokens.emplace_back(text.cbegin() + open_brace_pos + brace_open.size()
                                  , text.cbegin() + i);
            }
            i++;
        }
        return tokens;
    }

    /**
     * @brief Remove leading and trailing spaces from a string
     */
    inline std::string trim_spaces(const std::string& str) noexcept
    {
        std::string::const_iterator start = str.cbegin();
        while (start != str.end() && std::isspace(*start))
        {
            start++;
        }
        std::string::const_iterator end = str.cend();
        do
        {
            end--;
        } while (end > start && std::isspace(*end));

        return std::string(start, end + 1);
    }

    inline bool has_quotes(const std::string& str) noexcept
    {
        return !str.empty() && ((str.front() == '\'' && str.back() == '\'')
                             || (str.front() == '"' && str.back() == '"'));
    }

    inline std::string trim_quotes(const std::string& str) noexcept
    {
        std::string out = str;
        if (has_quotes(str))
            out.erase(0, 1).pop_back();
        return out;
    }

    /**
     * @brief Provide an explicit function to make a string lowercase regardless of encoding
     */
    inline std::string to_lowercase(const std::string& str) noexcept
    {
        std::string out (str.size(), ' ');
        std::transform(str.begin(), str.end(), out.begin(), [](std::string::value_type c){ return std::tolower(c); });
        return out;
    }

    /**
     * @brief Compares if two strings are sensibly similar
     * @details Compares two strings after putting these as lowercase (unless any of the two is quoted), removing all trailing and
     * leading spaces, and removing the surrounding quotes:
     * 
     * @code
     * - utl::loosely_match("true",  "TRUE")           // is true
     * - utl::loosely_match("None",  " none  ")        // is true
     * - utl::loosely_match("foo", "bar")              // is false
     * - utl::loosely_match("' chapter'", "chapter")   // is false
     * - utl::loosely_match("'true'",  "'TRUE'")       // is false
     * - utl::loosely_match("\"true\"",  "TRUE")       // is false
     * @endcode
     */
    inline bool loosely_match(const std::string& a, const std::string& b) noexcept
    {
        std::string simplified_a = utl::trim_spaces(a);
        std::string simplified_b = utl::trim_spaces(b);
        const bool is_protected = has_quotes(simplified_a) || has_quotes(simplified_b);
        simplified_a = trim_quotes(simplified_a);
        simplified_b = trim_quotes(simplified_b);
        if (!is_protected)
        {
            simplified_a = to_lowercase(simplified_a);
            simplified_b = to_lowercase(simplified_b);
        }

        return simplified_a == simplified_b;
    }

    template <char brace_open, char brace_close>
    inline std::pair<size_t, size_t> find_matching_brace(const std::string& text) noexcept
    {
        const size_t open_bracket_pos = text.find("[");
        size_t close_bracket_pos {open_bracket_pos};
        int depth {1};
        while (close_bracket_pos < text.size() && depth > 0)
        {
            close_bracket_pos++;
            switch (text[close_bracket_pos])
            {
                case brace_open: depth++; break;
                case brace_close: depth--; break;
            }
        }
        // the bracket closing bracket is misplaced
        if (close_bracket_pos == text.size())
            close_bracket_pos = std::string::npos;
        return std::make_pair(open_bracket_pos, close_bracket_pos);
    }

    /**
     * @brief A minimal parser for a csv file that extract the selected columns into an ordered table
     * @warning Does not account for escaped sequences
     * @throw std::runtime_error if the parsing fails for any reason
     * @tparam number of columns
     */
    template <size_t N>
    inline std::vector<std::array<std::string, N>> extract_from_csv(const std::filesystem::path& file, const std::array<std::string, N>& column_names, const char separator = ';')
    {
        std::vector<std::array<std::string, N>> parsed_table;
        std::string line;
        std::ifstream csv_data(file);

        // parse header
        if (!std::getline(csv_data, line))
            throw std::runtime_error("the csv file '" + file.generic_string() + "' is empty");
        
        std::vector<int> associated_offset_for_column {};
        std::string column_name;
        int column_input = 0;
        int found_cnt = 0;
        auto line_stream = std::stringstream(line);
        while (std::getline(line_stream, column_name, separator))
        {
            int& new_association = associated_offset_for_column.emplace_back(-1);
            for (int i{0}; i < column_names.size(); i++)
            {
                if (column_name == column_names[i])
                {
                    new_association = i;
                    found_cnt++;
                }
            }
        }
        if (found_cnt != N)
            throw std::runtime_error("failed to find all the inquired columns in the header '" + line_stream.str()
                                   + "' of the csv file '" + file.generic_string() + "'");

        // parse body
        while (std::getline(csv_data, line))
        {
            std::array<std::string, N>& parsed_line = parsed_table.emplace_back();
            std::string cell;
            column_input = 0;
            line_stream = std::stringstream(line);
            while(std::getline(line_stream, cell, separator))
            {
                const int target_column = associated_offset_for_column[column_input];
                if(target_column != -1)
                    parsed_line[target_column] = cell;

                column_input++;
            }
        }
        return parsed_table;
    }

    /// @brief checks whether a string starts by a given prefix
    inline bool begins_with(const std::string& text, const std::string& prefix) noexcept
    {
        return text.length() >= prefix.length()
            && text.compare(0, prefix.size(), prefix) == 0;
    }

    /// @brief checks whether a string ends by a given postfix 
    inline bool ends_with(const std::string& text, const std::string& postfix) noexcept
    {
        const size_t compare_begin = text.length() - postfix.length();
        return text.length() >= postfix.length()
            && text.compare(compare_begin, text.length(), postfix) == 0;
    }
}

#endif