//
// Created by Dan on 12/22/2021.
//

#include "registration.h"

#include <doctest/doctest.h>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <vector>
#include <regex>
#include <concepts>

#include "utilities.h"
#include "point_nd.h"
#include "ranges.h"
#include "parse.h"
#include "sum.h"

namespace fs = std::filesystem;

namespace {

    using namespace aoc;

    using point_3d = point<3>;

    enum class toggle : bool {
        off = false,
        on = true
    };

    class box {
        point_3d m_min;
        point_3d m_max;
        toggle m_state = toggle::on;
    public:
        box(const std::array<int, 3>& min, const std::array<int, 3> max) : m_min{min}, m_max{max} {}
        box(toggle s, const std::array<int, 3>& min, const std::array<int, 3> max) : m_min{min}, m_max{max}, m_state{s} {}

        [[nodiscard]] int64_t size() const {
            return static_cast<int64_t>(m_max[0] - m_min[0] + 1) *
                static_cast<int64_t>(m_max[1] - m_min[1] + 1) *
                static_cast<int64_t>(m_max[2] - m_min[2] + 1) *
                (m_state == toggle::on ? 1 : -1);
        }

        [[nodiscard]] bool is_in(const box& bigger) const {
            for (int i = 0; i < 3; ++i) {
                if (m_min[i] < bigger.m_min[i] || m_max[i] > bigger.m_max[i]) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] bool is_add() const noexcept { return m_state == toggle::on; }

        [[nodiscard]] std::optional<box> overlap(const box& b) const {
            bool non_intercepting = b.m_max[0] < m_min[0] || b.m_min[0] > m_max[0] ||
                       b.m_max[1] < m_min[1] || b.m_min[1] > m_max[1] ||
                       b.m_max[2] < m_min[2] || b.m_min[2] > m_max[2];
            if (non_intercepting) {
                return std::nullopt;
            }
            return box{static_cast<toggle>(!static_cast<bool>(m_state)),
                {std::max(m_min[0], b.m_min[0]), std::max(m_min[1], b.m_min[1]), std::max(m_min[2], b.m_min[2])},
                {std::min(m_max[0], b.m_max[0]), std::min(m_max[1], b.m_max[1]), std::min(m_max[2], b.m_max[2])}};
        }
    };

    box parse_step(std::string_view s) {
        using namespace std::string_view_literals;
        static std::regex re {R"((on|off) x=(-?\d+)..(-?\d+),y=(-?\d+)..(-?\d+),z=(-?\d+)..(-?\d+))"};
        std::match_results<std::string_view::const_iterator> res;
        if (!std::regex_match(s.begin(), s.end(), res, re)) {
            throw std::runtime_error{fmt::format("Line did not match expected format:  {}", s)};
        }
        toggle state = static_cast<toggle>(res[1].str() == "on"sv);
        int x1 = parse<int>(res[2].str());
        int x2 = parse<int>(res[3].str());
        int y1 = parse<int>(res[4].str());
        int y2 = parse<int>(res[5].str());
        int z1 = parse<int>(res[6].str());
        int z2 = parse<int>(res[7].str());
        return box{state,
                {std::min(x1, x2), std::min(y1, y2), std::min(z1, z2)},
                 {std::max(x1, x2), std::max(y1, y2), std::max(z1, z2)}};
    }

    std::vector<box> get_input(const fs::path& input_dir) {
        const auto lines = read_file_lines(input_dir / "2021" / "day_22_input.txt");
        return lines | std::views::transform(&parse_step) | to<std::vector<box>>();
    }

    bool step_covers_part1_region(const box& s) {
        constexpr int min_coord = -50, max_coord = 50;
        return s.is_in(box{{min_coord, min_coord, min_coord}, {max_coord, max_coord, max_coord}});
    }

    int64_t count_cells(const std::vector<box>& states) noexcept {
        return sum(states, [](const box& b){ return b.size(); });
    }

    void null_step_logger(const std::vector<box>& states, int) {}

    template <std::invocable<const std::vector<box>&, int> F = decltype(&null_step_logger)>
    std::vector<box> follow_steps(const std::vector<box>& input, bool all_space = false, F step_logger = &null_step_logger) {
        std::vector<box> states;
        states.reserve(128);
        int step = 0;
        for (const auto& s : input) {
            if (all_space || step_covers_part1_region(s)) {
                std::vector<box> to_add;
                if (s.is_add()) {
                    to_add.push_back(s);
                }
                for (const auto& ps : states) {
                    auto overlap = ps.overlap(s);
                    if (overlap) {
                        to_add.push_back(*overlap);
                    }
                }
                states.insert(states.end(), to_add.begin(), to_add.end());
                step_logger(states, step);
                ++step;
            }
        }
        return states;
    }

    /*
    --- Day 22: Reactor Reboot ---
    Operating at these extreme ocean depths has overloaded the submarine's reactor; it needs to be rebooted.

    The reactor core is made up of a large 3-dimensional grid made up entirely of cubes, one cube per integer 3-dimensional coordinate (x,y,z). Each cube can be either on or off; at the start of the reboot process, they are all off. (Could it be an old model of a reactor you've seen before?)

    To reboot the reactor, you just need to set all of the cubes to either on or off by following a list of reboot steps (your puzzle input). Each step specifies a cuboid (the set of all cubes that have coordinates which fall within ranges for x, y, and z) and whether to turn all of the cubes in that cuboid on or off.

    For example, given these reboot steps:

    on x=10..12,y=10..12,z=10..12
    on x=11..13,y=11..13,z=11..13
    off x=9..11,y=9..11,z=9..11
    on x=10..10,y=10..10,z=10..10
    The first step (on x=10..12,y=10..12,z=10..12) turns on a 3x3x3 cuboid consisting of 27 cubes:

    10,10,10
    10,10,11
    10,10,12
    10,11,10
    10,11,11
    10,11,12
    10,12,10
    10,12,11
    10,12,12
    11,10,10
    11,10,11
    11,10,12
    11,11,10
    11,11,11
    11,11,12
    11,12,10
    11,12,11
    11,12,12
    12,10,10
    12,10,11
    12,10,12
    12,11,10
    12,11,11
    12,11,12
    12,12,10
    12,12,11
    12,12,12
    The second step (on x=11..13,y=11..13,z=11..13) turns on a 3x3x3 cuboid that overlaps with the first. As a result, only 19 additional cubes turn on; the rest are already on from the previous step:

    11,11,13
    11,12,13
    11,13,11
    11,13,12
    11,13,13
    12,11,13
    12,12,13
    12,13,11
    12,13,12
    12,13,13
    13,11,11
    13,11,12
    13,11,13
    13,12,11
    13,12,12
    13,12,13
    13,13,11
    13,13,12
    13,13,13
    The third step (off x=9..11,y=9..11,z=9..11) turns off a 3x3x3 cuboid that overlaps partially with some cubes that are on, ultimately turning off 8 cubes:

    10,10,10
    10,10,11
    10,11,10
    10,11,11
    11,10,10
    11,10,11
    11,11,10
    11,11,11
    The final step (on x=10..10,y=10..10,z=10..10) turns on a single cube, 10,10,10. After this last step, 39 cubes are on.

    The initialization procedure only uses cubes that have x, y, and z positions of at least -50 and at most 50. For now, ignore cubes outside this region.

    Here is a larger example:

    on x=-20..26,y=-36..17,z=-47..7
    on x=-20..33,y=-21..23,z=-26..28
    on x=-22..28,y=-29..23,z=-38..16
    on x=-46..7,y=-6..46,z=-50..-1
    on x=-49..1,y=-3..46,z=-24..28
    on x=2..47,y=-22..22,z=-23..27
    on x=-27..23,y=-28..26,z=-21..29
    on x=-39..5,y=-6..47,z=-3..44
    on x=-30..21,y=-8..43,z=-13..34
    on x=-22..26,y=-27..20,z=-29..19
    off x=-48..-32,y=26..41,z=-47..-37
    on x=-12..35,y=6..50,z=-50..-2
    off x=-48..-32,y=-32..-16,z=-15..-5
    on x=-18..26,y=-33..15,z=-7..46
    off x=-40..-22,y=-38..-28,z=23..41
    on x=-16..35,y=-41..10,z=-47..6
    off x=-32..-23,y=11..30,z=-14..3
    on x=-49..-5,y=-3..45,z=-29..18
    off x=18..30,y=-20..-8,z=-3..13
    on x=-41..9,y=-7..43,z=-33..15
    on x=-54112..-39298,y=-85059..-49293,z=-27449..7877
    on x=967..23432,y=45373..81175,z=27513..53682
    The last two steps are fully outside the initialization procedure area; all other steps are fully within it. After executing these steps in the initialization procedure region, 590784 cubes are on.

    Execute the reboot steps. Afterward, considering only cubes in the region x=-50..50,y=-50..50,z=-50..50, how many cubes are on?
    */
    std::string part_1(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);
        const auto states = follow_steps(input);
        return std::to_string(count_cells(states));
    }

    /*
    --- Part Two ---
    Now that the initialization procedure is complete, you can reboot the reactor.

    Starting with all cubes off, run all of the reboot steps for all cubes in the reactor.

    Consider the following reboot steps:

    on x=-5..47,y=-31..22,z=-19..33
    on x=-44..5,y=-27..21,z=-14..35
    on x=-49..-1,y=-11..42,z=-10..38
    on x=-20..34,y=-40..6,z=-44..1
    off x=26..39,y=40..50,z=-2..11
    on x=-41..5,y=-41..6,z=-36..8
    off x=-43..-33,y=-45..-28,z=7..25
    on x=-33..15,y=-32..19,z=-34..11
    off x=35..47,y=-46..-34,z=-11..5
    on x=-14..36,y=-6..44,z=-16..29
    on x=-57795..-6158,y=29564..72030,z=20435..90618
    on x=36731..105352,y=-21140..28532,z=16094..90401
    on x=30999..107136,y=-53464..15513,z=8553..71215
    on x=13528..83982,y=-99403..-27377,z=-24141..23996
    on x=-72682..-12347,y=18159..111354,z=7391..80950
    on x=-1060..80757,y=-65301..-20884,z=-103788..-16709
    on x=-83015..-9461,y=-72160..-8347,z=-81239..-26856
    on x=-52752..22273,y=-49450..9096,z=54442..119054
    on x=-29982..40483,y=-108474..-28371,z=-24328..38471
    on x=-4958..62750,y=40422..118853,z=-7672..65583
    on x=55694..108686,y=-43367..46958,z=-26781..48729
    on x=-98497..-18186,y=-63569..3412,z=1232..88485
    on x=-726..56291,y=-62629..13224,z=18033..85226
    on x=-110886..-34664,y=-81338..-8658,z=8914..63723
    on x=-55829..24974,y=-16897..54165,z=-121762..-28058
    on x=-65152..-11147,y=22489..91432,z=-58782..1780
    on x=-120100..-32970,y=-46592..27473,z=-11695..61039
    on x=-18631..37533,y=-124565..-50804,z=-35667..28308
    on x=-57817..18248,y=49321..117703,z=5745..55881
    on x=14781..98692,y=-1341..70827,z=15753..70151
    on x=-34419..55919,y=-19626..40991,z=39015..114138
    on x=-60785..11593,y=-56135..2999,z=-95368..-26915
    on x=-32178..58085,y=17647..101866,z=-91405..-8878
    on x=-53655..12091,y=50097..105568,z=-75335..-4862
    on x=-111166..-40997,y=-71714..2688,z=5609..50954
    on x=-16602..70118,y=-98693..-44401,z=5197..76897
    on x=16383..101554,y=4615..83635,z=-44907..18747
    off x=-95822..-15171,y=-19987..48940,z=10804..104439
    on x=-89813..-14614,y=16069..88491,z=-3297..45228
    on x=41075..99376,y=-20427..49978,z=-52012..13762
    on x=-21330..50085,y=-17944..62733,z=-112280..-30197
    on x=-16478..35915,y=36008..118594,z=-7885..47086
    off x=-98156..-27851,y=-49952..43171,z=-99005..-8456
    off x=2032..69770,y=-71013..4824,z=7471..94418
    on x=43670..120875,y=-42068..12382,z=-24787..38892
    off x=37514..111226,y=-45862..25743,z=-16714..54663
    off x=25699..97951,y=-30668..59918,z=-15349..69697
    off x=-44271..17935,y=-9516..60759,z=49131..112598
    on x=-61695..-5813,y=40978..94975,z=8655..80240
    off x=-101086..-9439,y=-7088..67543,z=33935..83858
    off x=18020..114017,y=-48931..32606,z=21474..89843
    off x=-77139..10506,y=-89994..-18797,z=-80..59318
    off x=8476..79288,y=-75520..11602,z=-96624..-24783
    on x=-47488..-1262,y=24338..100707,z=16292..72967
    off x=-84341..13987,y=2429..92914,z=-90671..-1318
    off x=-37810..49457,y=-71013..-7894,z=-105357..-13188
    off x=-27365..46395,y=31009..98017,z=15428..76570
    off x=-70369..-16548,y=22648..78696,z=-1892..86821
    on x=-53470..21291,y=-120233..-33476,z=-44150..38147
    off x=-93533..-4276,y=-16170..68771,z=-104985..-24507
    After running the above reboot steps, 2758514936282235 cubes are on. (Just for fun, 474140 of those are also in the initialization procedure region.)

    Starting again with all cubes off, execute all reboot steps. Afterward, considering all cubes, how many cubes are on?
    */
    std::string part_2(const std::filesystem::path& input_dir) {
        const auto input = get_input(input_dir);
        const auto states = follow_steps(input, true);
        return std::to_string(count_cells(states));
    }

    aoc::registration r {2021, 22, part_1, part_2};

    TEST_SUITE("2021_day22") {
        TEST_CASE("2021_day22:example") {
            std::vector<std::string> lines = {
                    "on x=10..12,y=10..12,z=10..12",
                    "on x=11..13,y=11..13,z=11..13",
                    "off x=9..11,y=9..11,z=9..11",
                    "on x=10..10,y=10..10,z=10..10"
            };
            const auto input = lines | std::views::transform(&parse_step) | to<std::vector<box>>();
            const std::vector<std::size_t> step_sizes = {
                    27,
                    46,
                    38,
                    39
            };
            const auto states = follow_steps(input, false, [&step_sizes](const std::vector<box>& s, int step){
                //log_on(s);
                REQUIRE_EQ(count_cells(s), step_sizes[step]);
            });
        }
    }

}