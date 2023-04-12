#include "game.hpp"
#include <nlohmann/json.hpp>
#include "point.hpp"
#include "runebound_fwd.hpp"

namespace runebound {
namespace game {


void to_json(nlohmann::json &json, const Game &game) {
    json["m_map"] = game.m_map;
    json["m_characters"] = game.m_characters;
    json["m_card_deck_research"] = game.m_card_deck_research;
    json["m_tokens"] = game.m_tokens;
    json["m_turn"] = game.m_turn;
    json["m_count_players"] = game.m_count_players;
    json["ALL_CARDS_RESEARCH"] = game.ALL_CARDS_RESEARCH;
}

void from_json(const nlohmann::json &json, Game &game) {
    int count_players = json["M_COUNT_PLAYERS"];
    std::vector<unsigned int> cards;
    for (const nlohmann::json &card : json["m_card_deck_research"]) {
        cards.push_back(card);
    }
    game.m_card_deck_research = cards;
    game.m_count_players = json["m_count_players"];
    game.m_map = json["m_map"];
    game.m_characters = json["m_characters"];
    game.m_tokens = json["m_tokens"];
    game.m_turn = json["m_turn"];
}

Point Game::get_position_character(::runebound::character::Character *chr
) const {
    return chr->m_current_position;
}

std::vector<cards::CardResearch> Game::generate_all_cards_research() {
    std::vector<cards::CardResearch> cards;
    m_indexes_card_research.resize(DECK_SIZE);
    for (int i = 0; i < DECK_SIZE; ++i) {
        cards.emplace_back(cards::CardResearch());
        m_indexes_card_research[i] = i;
    }
    return cards;
}

void Game::relax(const ::runebound::character::Character *chr) {
    if (chr != &m_characters[m_turn]) {
        throw WrongCharacterTurnException();
    }
    if (m_characters[m_turn].get_action_points() < 1) {
        throw NotEnoughActionPointsException();
    }
    m_characters[m_turn].relax();
    m_characters[m_turn].update_action_points(-1);
}

void Game::check_and_get_card_adventure_because_of_token(
    ::runebound::character::Character *chr
) {
    if (m_map.get_cell_map(chr->m_current_position).get_token() !=
            ::runebound::AdventureType::NOTHING &&
        m_map.get_cell_map(chr->m_current_position).get_side_token() ==
            ::runebound::Side::FRONT) {
        if (m_map.get_cell_map(chr->m_current_position).get_token() ==
            ::runebound::AdventureType::RESEARCH) {
            unsigned int card =
                m_indexes_card_research[rng() % m_indexes_card_research.size()];
            chr->add_card(card);
            pop_element_from_vector(card, m_card_deck_research);
            pop_element_from_vector(card, m_indexes_card_research);
        }
        m_map.get_cell_map(chr->m_current_position).reverse_token();
    }
}

std::vector<Point> Game::make_move(
    const ::runebound::character::Character *chr,
    const Point &end,
    std::vector<::runebound::dice::HandDice> &dice_roll_results
) {
    if (chr != &m_characters[m_turn]) {
        throw WrongCharacterTurnException();
    }
    if (m_characters[m_turn].get_action_points() < 2) {
        throw NotEnoughActionPointsException();
    }
    std::vector<Point> result = m_map.check_move(
        m_characters[m_turn].m_current_position, end, dice_roll_results
    );
    if (result.empty()) {
        throw InaccessibleMoveException();
    }
    m_characters[m_turn].m_current_position = end;
    m_characters[m_turn].update_action_points(-2);
    return result;
}

}  // namespace game
}  // namespace runebound
