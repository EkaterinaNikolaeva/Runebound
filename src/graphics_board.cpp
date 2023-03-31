#include <cmath>
#include <graphics_board.hpp>
#include <map>

namespace {
int sign(int x) {
    if (x < 0) {
        return -1;
    }
    if (x > 0) {
        return 1;
    }
    return 0;
}
}  // namespace

namespace runebound::graphics {
// constants for hexagons
const SDL_Color SELECTED_COLOR = {0xFF, 0xF7, 0x00, 0xFF};
const int HEXAGON_RADIUS = 36;

// =============================================================================
// TODO
Point centerize(int i, int j) {
    const int dy = (HEXAGON_RADIUS * 56756) >> 16;
    Point center;
    if (j % 2 == 0) {
        center = Point(HEXAGON_RADIUS * (2 + j * 3) / 2, dy * (1 + 2 * i));
    } else {
        center = Point(HEXAGON_RADIUS * (2 + j * 3) / 2, dy * 2 * (1 + i));
    }
    return center;
}

// =============================================================================

// constants for rivers
const ::std::map<::std::pair<int, int>, ::std::pair<int, int>>
    RIVER_DIRECTIONS = {{{-1, 0}, {3, 4}}, {{-1, -1}, {2, 3}},
                        {{-1, 1}, {4, 5}}, {{1, 0}, {0, 1}},
                        {{1, -1}, {1, 2}}, {{1, 1}, {5, 0}}};

Board::Board(const ::runebound::map::MapClient &map) {
    // adding cells
    for (int row = 0; row < ::runebound::map::STANDARD_SIZE; ++row) {
        for (int col = 0; col < ::runebound::map::STANDARD_SIZE; ++col) {
            // get necessary color
            SDL_Color fill_color;
            switch (map.m_map[row][col].get_type_cell()) {
                case ::runebound::map::TypeCell::WATER:
                    fill_color = {0x37, 0x1A, 0xCA, 0xFF};
                    break;
                case ::runebound::map::TypeCell::FOREST:
                    fill_color = {0x15, 0x66, 0x1D, 0xFF};
                    break;
                case ::runebound::map::TypeCell::MOUNTAINS:
                    fill_color = {0x68, 0x7C, 0x7C, 0xFF};
                    break;
                case ::runebound::map::TypeCell::HILLS:
                    fill_color = {0x72, 0xB0, 0x34, 0xFF};
                    break;
                case ::runebound::map::TypeCell::PLAIN:
                    fill_color = {0x11, 0xF0, 0x4D, 0xFF};
                    break;
                case ::runebound::map::TypeCell::TOWN:
                    fill_color = {0x03, 0x07, 0x06, 0xFF};
                    break;
            }

            auto cntr = centerize(row, col);
            ::std::vector<Point> vec;
            vec.push_back({cntr + Point(HEXAGON_RADIUS/ 4, -HEXAGON_RADIUS / 4)});
            vec.push_back({cntr + Point(3 * HEXAGON_RADIUS/ 4, -HEXAGON_RADIUS / 4)});
            vec.push_back({cntr + Point(3 * HEXAGON_RADIUS/ 4, HEXAGON_RADIUS / 4)});
            vec.push_back({cntr + Point(HEXAGON_RADIUS/ 4, HEXAGON_RADIUS / 4)});
            PolygonShape poly(vec);
            switch(map.m_map[row][col].get_special_type_cell()) {
                case ::runebound::map::SpecialTypeCell::SETTLEMENT:
                    add_special(
                        poly, {0xE2, 0xE2, 0x18, 0xFF}, {0x00, 0x00, 0x00, 0xFF}
                    );
                    break;
                case map::SpecialTypeCell::SANCTUARY:
                    add_special(
                        poly, {0xFF, 0xFF, 0xFF, 0xFF}, {0x00, 0x00, 0x00, 0xFF}
                    );
                    break;
                case map::SpecialTypeCell::FORTRESS:
                    add_special(
                        poly, {0xA0, 0xA0, 0xA0, 0xFF}, {0x00, 0x00, 0x00, 0xFF}
                    );
                    break;
                case map::SpecialTypeCell::NOTHING:
                    break;
            }

            HexagonShape hex = {centerize(row, col), HEXAGON_RADIUS};
            add_cell(hex, fill_color, SDL_Color{0x00, 0x00, 0x00, 0xFF});

            // adding tokens
            bool tr = true;
            switch (map.m_map[row][col].get_token()) {
                case ::runebound::AdventureType::MEETING:
                    fill_color = {0x9D, 0x00, 0xC4, 0xFF};
                    break;
                case AdventureType::RESEARCH:
                    fill_color = {0x00, 0x9F, 0x00, 0xFF};
                    break;
                case AdventureType::FIGHT:
                    fill_color = {0xC4, 0x90, 0x00, 0xFF};
                    break;
                case AdventureType::NOTHING:
                    tr = false;
                    break;
            }
            if (tr) {
                CircleShape cir = {centerize(row, col), HEXAGON_RADIUS / 2};
                add_token(cir, fill_color, {0x00, 0x00, 0x00, 0xFF});
            }

            // adding roads
            // TODO
            if (map.m_map[row][col].check_road()) {
                for (auto [i, j] :
                     map.get_all_neighbours(::runebound::Point(row, col))) {
                    if (map.m_map[i][j].check_road()) {
                        Segment seg(centerize(row, col), centerize(i, j));
                        add_road(seg, {0x80, 0x80, 0x80, 0xFF});
                        is_connected_to_city.push_back(false);
                    }
                    if (map.m_map[i][j].get_type_cell() ==
                        ::runebound::map::TypeCell::TOWN) {
                        Segment seg(centerize(row, col), centerize(i, j));
                        add_road(seg, {0x80, 0x80, 0x80, 0xFF});
                        is_connected_to_city.push_back(true);
                    }
                }
            }
        }
    }

    // adding rivers
    for (const auto &pair : map.m_rivers) {
        SDL_Color river_color = {0x37, 0x1A, 0xFA, 0xFF};
        auto [x1, y1] = pair.first;
        auto [x2, y2] = pair.second;
        auto [i, v] = *RIVER_DIRECTIONS.find(
            {sign(x1 - x2) +
                 (1 - ::std::abs(sign(x1 - x2))) * (2 * (y1 % 2) - 1),
             sign(y1 - y2)}
        );

        HexagonShape hex = m_cells[x1 * ::runebound::map::STANDARD_SIZE + y1];
        Segment seg = {hex.get_vertex(v.first), hex.get_vertex(v.second)};
        add_river(seg, river_color);
    }
}

void Board::add_cell(
    HexagonShape &hex,
    SDL_Color fill_col,
    SDL_Color border_col
) {
    m_cells.push_back(::std::move(hex));
    m_cell_fill_color.push_back(fill_col);
    m_cell_border_color.push_back(border_col);
    ++m_cell_amount;
}

void Board::add_river(Segment &seg, SDL_Color col) {
    m_rivers.push_back(::std::move(seg));
    m_river_color.push_back(col);
    ++m_river_amount;
}

void Board::add_road(Segment &seg, SDL_Color col) {
    m_roads.push_back(seg);
    m_road_color.push_back(col);
    ++m_road_amount;
}

void Board::add_token(
    CircleShape &cir,
    SDL_Color fill_col,
    SDL_Color border_col
) {
    m_tokens.push_back(::std::move(cir));
    m_token_fill_color.push_back(fill_col);
    m_token_border_color.push_back(border_col);
    ++m_token_amount;
}

void Board::render(SDL_Renderer *renderer) const {
    for (::std::size_t i = 0; i < m_cell_amount; ++i) {
        m_cells[i].render(
            renderer, m_cell_fill_color[i], m_cell_border_color[i]
        );
    }
    if (m_selected_cell != 0xFFFF && m_selected_token == 0xFFFF) {
        m_cells[m_selected_cell].render(
            renderer, SELECTED_COLOR, m_cell_border_color[m_selected_cell]
        );
    }
    for (::std::size_t i = 0; i < m_river_amount; ++i) {
        m_rivers[i].render(renderer, m_river_color[i], 5);
    }
    // TODO
    for (::std::size_t i = 0; i < m_road_amount; ++i) {
        if (is_connected_to_city[i]) {
            m_roads[i].half_render(renderer, m_road_color[i], 7);
        } else {
            m_roads[i].render(renderer, m_road_color[i], 7);
        }
    }

    for(::std::size_t i = 0; i < m_special_amount; ++i) {
        m_specials[i].render(renderer, m_special_fill_color[i], m_special_border_color[i]);
    }

    for (::std::size_t i = 0; i < m_token_amount; ++i) {
        m_tokens[i].render(
            renderer, m_token_fill_color[i], m_token_border_color[i]
        );
    }
    if (m_selected_token != 0xFFFF) {
        m_tokens[m_selected_token].render(
            renderer, SELECTED_COLOR, m_token_border_color[m_selected_token]
        );
    }
}

void Board::update_selection(const Point &dot) {
    m_selected_cell = 0xFFFF;
    m_selected_token = 0xFFFF;
    for (::std::size_t i = 0; i < m_cell_amount; ++i) {
        if (m_cells[i].in_bounds(dot)) {
            m_selected_cell = i;
            break;
        }
    }
    for (::std::size_t i = 0; i < m_token_amount; ++i) {
        if (m_tokens[i].in_bounds(dot)) {
            m_selected_token = i;
            break;
        }
    }
}

void Board::add_special(
    PolygonShape &poly,
    SDL_Color fill_col,
    SDL_Color border_col
) {
    m_specials.push_back(poly);
    m_special_fill_color.push_back(fill_col);
    m_special_border_color.push_back(border_col);
    ++m_special_amount;
}
}  // namespace runebound::graphics