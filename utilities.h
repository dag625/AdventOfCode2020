//
// Created by Daniel Garcia on 12/1/20.
//

#ifndef ADVENTOFCODE2020_UTILITIES_H
#define ADVENTOFCODE2020_UTILITIES_H

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <optional>
#include <sstream>

namespace aoc {

    std::string ltrim(const std::string& s);
    std::string rtrim(const std::string& s);
    std::string trim(const std::string& s);

    std::string_view ltrim(std::string_view s);
    std::string_view rtrim(std::string_view s);
    std::string_view trim(std::string_view s);

    std::vector<std::string_view> split(std::string_view s, char c);
    std::vector<std::string_view> split(std::string_view s, std::string_view spl);
    std::vector<std::string_view> split_no_empty(std::string_view s, char c);
    std::vector<std::string_view> split_no_empty(std::string_view s, std::string_view spl);

    std::vector<std::string_view> split_by_all(std::string_view s, std::string_view spl);
    std::vector<std::string_view> split_by_all_no_empty(std::string_view s, std::string_view spl);

    //Include the splitter at the end of each element
    std::vector<std::string_view> split_with(std::string_view s, char c, bool keep_empty_end = false);
    std::vector<std::string_view> split_with(std::string_view s, std::string_view spl, bool keep_empty_end = false);

    std::optional<std::string_view> starts_with(std::string_view str, std::string_view to_find);

    std::vector<std::string> read_file_lines(const std::filesystem::path &file, bool need_trim = true);
    std::string read_file(const std::filesystem::path& file);

    void trim_lines_inplace(std::vector<std::string>& lines);
    inline std::vector<std::string> trim_lines(std::vector<std::string> lines) {
        trim_lines_inplace(lines);
        return lines;
    }

    template <typename T>
    T default_to_string(T&& t) {
        return std::forward<T>(t);
    }

    template <typename Iter, typename F>
    std::string join(Iter begin, const Iter end, std::string_view join_str, F func = default_to_string<decltype(*std::declval<Iter>())>) {
        std::stringstream js;
        bool first = true;
        for (; begin != end; ++begin) {
            if (!first) {
                js << join_str;
            }
            first = false;
            js << func(*begin);
        }
        return js.str();
    }

    template <typename Iter, typename F>
    std::string join(Iter begin, const Iter end, char join_ch, F func = default_to_string<decltype(*std::declval<Iter>())>) {
        std::stringstream js;
        bool first = true;
        for (; begin != end; ++begin) {
            if (!first) {
                js << join_ch;
            }
            first = false;
            js << func(*begin);
        }
        return js.str();
    }

    template <typename Container, typename F = decltype(default_to_string<decltype(*std::declval<Container>().begin())>)>
    std::string join(Container c, std::string_view join_str, F func = default_to_string<decltype(*std::declval<Container>().begin())>) {
        return join(std::begin(c), std::end(c), join_str, func);
    }

    template <typename Container, typename F = decltype(default_to_string<decltype(*std::declval<Container>().begin())>)>
    std::string join(Container c, char join_ch, F func = default_to_string<decltype(*std::declval<Container>().begin())>) {
        return join(std::begin(c), std::end(c), join_ch, func);
    }

    template <typename T>
    T opt_or(std::optional<T> opt_val, T&& default_val) {
        if (opt_val) {
            return std::move(*opt_val);
        }
        else {
            return std::forward(default_val);
        }
    }

    template <typename T>
    std::string opt_or_str(std::optional<T> opt_val, std::string_view default_val) {
        std::stringstream str;
        if (opt_val) {
            str << *opt_val;
        }
        else {
            str << default_val;
        }
        return str.str();
    }

} /* namespace aoc */

#endif //ADVENTOFCODE2020_UTILITIES_H
