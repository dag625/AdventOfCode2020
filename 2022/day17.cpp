//
// Created by Dan on 12/17/2022.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>

#include <array>
#include <iostream>
#include <numeric>

#include "utilities.h"
#include "point.h"
#include "ranges.h"

namespace {

    using namespace aoc;

    std::string get_input(const std::vector<std::string>& lines) {
        return lines | std::views::join | std::ranges::to<std::string>();
    }

    const std::array<std::vector<position>, 5>& shapes() {
        //Sort each vector<position> to put highest y values first.
        static std::array<std::vector<position>, 5> retval {
                std::vector<position>{{2, 3}, {3, 3}, {4, 3}, {5, 3}},
                std::vector<position>{{3, 5}, {2, 4}, {3, 4}, {4, 4}, {3, 3}},
                std::vector<position>{{4, 5}, {4, 4}, {2, 3}, {3, 3}, {4, 3}},
                std::vector<position>{{2, 6}, {2, 5}, {2, 4}, {2, 3}},
                std::vector<position>{{2, 4}, {3, 4}, {2, 3}, {3, 3}}
        };
        return retval;
    }

    constexpr int64_t P2_ROUNDS = 1000000000000;

    void print_floor(const std::vector<position>& floor, const std::optional<std::vector<position>>& shape = std::nullopt) {
        std::cout << '\n';
        if (floor.empty() && (!shape || shape->empty())) {
            std::cout << "+-------+\n";
            return;
        }
        int first_y = 1;
        if (!floor.empty()) {
            first_y = floor.front().y;
        }
        if (shape && shape->front().y > first_y) {
            first_y = shape->front().y;
        }
        auto current = floor.begin();
        for (int y = first_y + 4; y >= 0; --y) {
            std::cout << '|';
            for (int x = 0; x < 7; ++x) {
                const position dp {x, y};
                if (current != floor.end() && *current == dp) {
                    std::cout << '#';
                    ++current;
                }
                else if (shape && std::find_if(shape->begin(), shape->end(), [dp](position sp){ return dp == sp; }) != shape->end()) {
                    std::cout << '@';
                }
                else {
                    std::cout << '.';
                }
            }
            std::cout << "|\n";
        }
        std::cout << "+-------+\n";
    }

    bool is_in_floor(const std::vector<position>& floor, const position& p) {
        const auto found = std::lower_bound(floor.begin(), floor.end(), p,
            [](position a, position b) { if (a.y > b.y) { return true; } else if (a.y == b.y) { return a.x < b.x; } else { return false; } });
        return found != floor.end() && *found == p;
        //Original implementation:
        //return std::any_of(floor.begin(), floor.end(), [&p](position fp) { return p == fp; });
    }

    void shift(std::vector<position>& shape, const std::vector<position>& floor, char s) {
        velocity v {1, 0};
        if (s =='<') {
            v.dx = -1;
            if (std::any_of(shape.begin(), shape.end(), [](position p){ return p.x == 0; })) {
                return;
            }
        }
        else if (std::any_of(shape.begin(), shape.end(), [](position p){ return p.x == 6; })) {
            return;
        }
        std::vector<position> new_shape = shape;
        for (auto& p : new_shape) {
            p += v;
            if (is_in_floor(floor, p)) {
                return;
            }
        }
        shape.swap(new_shape);
    }

    bool fall(std::vector<position>& shape, std::vector<position>& floor) {
        const velocity v {0, -1};
        std::vector<position> new_s;
        new_s.reserve(shape.size());
        bool no_overlap = true;
        for (auto& p : shape) {
            new_s.push_back(p + v);
            no_overlap = new_s.back().y >= 0 && !is_in_floor(floor, new_s.back());
            if (!no_overlap) {
                const auto mid = floor.size();
                floor.insert(floor.end(), shape.begin(), shape.end());
                std::inplace_merge(floor.begin(), floor.begin() + static_cast<ptrdiff_t >(mid), floor.end(),
                                   [](position a, position b){ if (a.y > b.y) { return true; } else if (a.y == b.y) { return a.x < b.x; } else { return false; } });
                return false;
            }
        }
        shape.swap(new_s);
        return true;
    }

    int cleanup_floor(std::vector<position>& floor, const bool trunc_floor) {
        std::array<int, 7> max_ys{};
        for (const auto p : floor) {
            if (max_ys[p.x] < p.y) {
                max_ys[p.x] = p.y;
            }
        }
        const auto [min_y_it, max_y_it] = std::minmax_element(max_ys.begin(), max_ys.end());
        if (trunc_floor && floor.size() > 128) {
            floor.resize(128);
        }
        return *max_y_it + 1;
    }

    int drop_shape(std::vector<position> shape, std::string_view& dirs, const std::string& raw_dirs, std::vector<position>& floor, const int starting_floor_y, const bool trunc_floor) {
        for (auto& p : shape) {
            p.y += starting_floor_y;
        }
        bool keep_falling = true;
        while (keep_falling) {
            if (dirs.empty()) {
                dirs = raw_dirs;
            }
            //std::cout << "Next Shift:  " << dirs.front() << '\n';
            //print_floor(floor, shape);
            shift(shape, floor, dirs.front());
            //print_floor(floor, shape);
            dirs.remove_prefix(1);
            keep_falling = fall(shape, floor);
            //print_floor(floor, shape);
            //std::cout << "==================================\n";
        }
        return cleanup_floor(floor, trunc_floor);
    }

    struct repitition_info {
        int start_pos = -1;
        int length = -1;
        int total_delta = -1;

        bool operator<(const repitition_info& rhs) const noexcept {
            if (length > rhs.length) {
                return true;
            }
            else if (length == rhs.length) {
                return start_pos < rhs.start_pos;
            }
            else {
                return false;
            }
        }
    };

    repitition_info find_repeated(const std::vector<int>& height_deltas) {
        std::vector<repitition_info> found;
        for (int start = 0; start < height_deltas.size() - 1; ++start) {
            int max_len = static_cast<int>(height_deltas.size() - start - 1) / 2;
            for (int len = max_len; len > 1; --len) {
                if (std::equal(height_deltas.begin() + start, height_deltas.begin() + start + len, height_deltas.begin() + start + len)) {
                    const auto diff = std::accumulate(height_deltas.begin() + start,
                                                       height_deltas.begin() + start + len, 0);
                    found.push_back({start, len, diff});
                }
            }
        }
        return *std::min_element(found.begin(), found.end());
    }

    /************************* Part 1 *************************/
    std::string part_1(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);
        std::vector<position> floor;
        floor.reserve(64);
        int starting_floor_y = 0;
        std::string_view dirs{input};
        for (int i = 0; i < 2022; ++i) {
            starting_floor_y = drop_shape(shapes()[i % 5], dirs, input, floor, starting_floor_y, true);
        }
        //print_floor(floor);
        //std::cout << "\n\n";
        return std::to_string(floor.front().y + 1);
    }

    /************************* Part 1 *************************/
    std::string part_2(const std::vector<std::string>& lines) {
        const auto input = get_input(lines);        std::vector<position> floor;
        floor.reserve(24 * 1024);
        int starting_floor_y = 0;
        std::string_view dirs{input};
        std::vector<int> height_deltas;
        const int NUM_ROUNDS = 4000;
        height_deltas.reserve(NUM_ROUNDS);
        int last_height = 0;
        for (int i = 0; i < NUM_ROUNDS; ++i) {
            starting_floor_y = drop_shape(shapes()[i % 5], dirs, input, floor, starting_floor_y, false);
            height_deltas.push_back(floor.front().y + 1 - last_height);
            last_height = floor.front().y + 1;
        }
        //print_floor(floor);
        //std::cout << "\n\n";
        const auto [r_start, r_len, r_height] = find_repeated(height_deltas);
        //std::cout << "Repeats at " << r_start << " for " << r_len << " with a height increase of " << r_height << ".\n";
        const auto num_repeated = (P2_ROUNDS - r_start) / r_len;
        const auto num_left = (P2_ROUNDS - r_start) % r_len;
        const auto total_height = num_repeated * r_height +
                std::accumulate(height_deltas.begin(), height_deltas.begin() + r_start, 0) +
                std::accumulate(height_deltas.begin() + r_start, height_deltas.begin() + r_start + num_left, 0);
        return std::to_string(total_height);
    }

    aoc::registration r{2022, 17, part_1, part_2};

    TEST_SUITE("2022_day17") {
        TEST_CASE("2022_day17:example") {
            const std::string input = ">>><<><>><<<>><>>><<<>>><<<><<<>><>><<>>";
            std::vector<position> floor;
            floor.reserve(64);
            int starting_floor_y = 0;
            std::string_view dirs{input};
            for (int i = 0; i < 2022; ++i) {
                starting_floor_y = drop_shape(shapes()[i % 5], dirs, input, floor, starting_floor_y, true);
//                if (i < 20) {
//                    print_floor(floor);
//                    std::cout << "\n\n";
//                }
            }
            CHECK_EQ(floor.front().y + 1, 3068);
        }
    }

}