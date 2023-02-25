#include <iostream>
#include <set>
#include <vector>
#include "field_cell.hpp"

namespace runebound {
namespace field {
int StandartHeight = 5, StandartWeight = 5;

unsigned int make_river_index(int x1, int y1, int x2, int y2) {
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    } else {
        if ((x1 == x2) && (y1 > y2)) {
            std::swap(y1, y2);
        }
    }

    int step = 64;
    return (((x1 * step) + y1) * step + x2) * step + y2;
}

struct Map {
private:
    std::vector<std::vector<FieldCell>> m_map;
    std::set<unsigned int> m_rivers;
    const int m_height, m_weight;
    const std::vector<std::pair<int, int>> directions_odd_row{
        {0, -1}, {1, 0}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}};
    const std::vector<std::pair<int, int>> directions_even_row{
        {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 0}, {0, -1}};

public:
    void make_map(std::vector<TypeCell> cells) {
        m_map.resize(m_height);
        for (int i = 0; i < m_height; i++) {
            for (int j = 0; j < m_weight; j++) {
                m_map[i].push_back(FieldCell(cells[i * m_height + j]));
            }
        }
    }

    Map() : m_weight(StandartWeight), m_height(StandartHeight) {
        std::vector<TypeCell> cells =

            {TypeCell::FOREST,    TypeCell::FOREST,    TypeCell::PLAIN,
             TypeCell::WATER,     TypeCell::WATER,     TypeCell::FOREST,
             TypeCell::FOREST,    TypeCell::PLAIN,     TypeCell::WATER,
             TypeCell::WATER,     TypeCell::PLAIN,     TypeCell::PLAIN,
             TypeCell::PLAIN,     TypeCell::PLAIN,     TypeCell::PLAIN,
             TypeCell::HILLS,     TypeCell::HILLS,     TypeCell::PLAIN,
             TypeCell::MOUNTAINS, TypeCell::MOUNTAINS, TypeCell::HILLS,
             TypeCell::HILLS,     TypeCell::PLAIN,     TypeCell::MOUNTAINS,
             TypeCell::MOUNTAINS};
        make_map(cells);

        m_rivers.insert(make_river_index(0, 2, 1, 1));
        m_rivers.insert(make_river_index(0, 2, 1, 2));
        m_rivers.insert(make_river_index(0, 2, 1, 3));
    }

    Map(int weight, int height) : m_height(height), m_weight(weight) {
    }

    int get_field_cell_int(int height, int weight) {
        return (m_map[height][weight]).type();
    }

    std::vector<std::pair<int, int>> get_neighbours_coor(int x, int y) {
        std::vector<std::pair<int, int>> result;
        const std::vector<std::pair<int, int>> *directions;
        if (x % 2 == 0) {
            directions = &directions_even_row;
        } else {
            directions = &directions_odd_row;
        }
        for (auto [dx, dy] : *directions) {
            result.push_back({x + dx, y + dy});
        }
        return result;
    }

    bool check_river(unsigned int river_index) {
        if (m_rivers.find(river_index) != m_rivers.end()) {
            return true;
        } else {
            return false;
        }
    }
};
}  // namespace field
}  // namespace runebound

int main() {
    runebound::field::Map test_map;
    for (int i = 0; i < 5; i++) { //координата y
        for (int j = 0; j < 5; j++) { //координата x
            char c = ' ';
            for (auto [x, y] : test_map.get_neighbours_coor(j, i)) {
                if (test_map.runebound::field::Map::check_river(
                        runebound::field::make_river_index(j, i, x, y)
                    )) {
                    c = '*';
                }
            }
            if (i % 2 == 0) {
                std::cout << "  " << test_map.get_field_cell_int(i, j) << c <<" ";
            } else {
                std::cout << test_map.get_field_cell_int(i, j) << c << "   ";
            }
        }
        std::cout << '\n';
    }
    return 0;
}