#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <wchar.h>

#define INITIAL_DECK_SIZE 52

typedef struct {
    uint8_t rank;
    uint8_t suit;
} Card;

typedef struct {
    Card* cards;
    uint8_t count;
    uint8_t capacity;
} Cards;

typedef struct {
    uint8_t rank;
    uint8_t length;
} SequenceOfEqualRank;

typedef struct {
    SequenceOfEqualRank* sequences;
    uint8_t count;
    uint8_t capacity;
} SequencesOfEqualRank;

typedef struct {
    Card card_0;
    Card card_1;
} PlayerHoleCards;

typedef struct {
    PlayerHoleCards* players_hole_cards;
    uint8_t num_players;
} PlayersHoleCards;

typedef struct {
    uint8_t hand_value;
    // 0 = nothing
    // 1 = pair
    // 2 = two pairs
    // 3 = three of a kind
    // 4 = straight
    // 5 = flush
    // 6 = full house
    // 7 = four of a kind
    // 8 = straight flush
    uint8_t first_kicker;
    uint8_t second_kicker;
    uint8_t third_kicker;
    uint8_t fourth_kicker;
    uint8_t fifth_kicker;
    // 0 = 2
    // 1 = 3
    // 2 = 4
    // 3 = 5
    // 4 = 6
    // 5 = 7
    // 6 = 8
    // 7 = 9
    // 8 = 10
    // 9 = J
    // 10 = Q
    // 11 = K
    // 12 = A
} HandStrength;

wchar_t* rank_to_human_readable[] = { L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"10", L"J", L"Q", L"K", L"A" };
wchar_t* suit_to_human_readable[] = { L"♣", L"♦", L"♥", L"♠" };

void init_rand() {
    srand(time(NULL));
}

uint8_t get_rand_index(uint8_t* length) {
    return rand() % *length;
}

void append_card(Cards* cards, uint8_t* rank, uint8_t* suit) {
    (cards->cards + cards->count)->rank = *rank;
    (cards->cards + cards->count)->suit = *suit;
    ++cards->count;
}

Cards* get_empty_deck() {
    Cards* res = malloc(sizeof(Cards));
    res->count = 0;
    res->cards = malloc(sizeof(Card) * INITIAL_DECK_SIZE);
    res->capacity = INITIAL_DECK_SIZE;
    return res;
}

Cards* get_unshuffled_deck() {
    Cards* res = get_empty_deck();
    for (uint8_t suit = 0; suit < 4; ++suit) {
        for (uint8_t rank = 0; rank < 13; ++rank) {
            append_card(res, &rank, &suit);
        }
    }
    return res;
}

Card pop_card(Cards* cards, uint8_t index) {
    Card res = cards->cards[index];
    for (; index != cards->count - 1; ++index) {
        cards->cards[index] = cards->cards[index + 1];
    }
    --cards->count;
    return res;
}

Card draw_card(Cards* cards) {
    return cards->cards[--cards->count];
}

Cards* get_shuffled_deck() {
    Cards* unshuffled_deck = get_unshuffled_deck();
    Cards* res = get_empty_deck();
    while (res->count != INITIAL_DECK_SIZE) {
        Card popped_card = pop_card(unshuffled_deck, get_rand_index(&unshuffled_deck->count));
        append_card(res, &popped_card.rank, &popped_card.suit);
    }
    free(unshuffled_deck->cards);
    free(unshuffled_deck);
    return res;
}

void print_card(Card* card) {
    wprintf(L"%ls%ls", rank_to_human_readable[card->rank], suit_to_human_readable[card->suit]);
}

void print_cards(Cards* cards) {
    uint8_t i = 0;
    for (; i != cards->count - 1; ++i) {
        print_card(cards->cards + i);
        printf(" ");
    }
    print_card(cards->cards + i);
    printf("\n\n");
}

int compare_cards(const void* card_0, const void* card_1) {
    return ((Card*)card_0)->rank - ((Card*)card_1)->rank;
}
void sort_cards_by_rank(Cards* cards) {
    qsort(cards->cards, cards->count, sizeof(Card), compare_cards);
}

bool is_card_rank_higher_than_previous_by_x(Card* cards, uint8_t* card_index, uint8_t x) {
    return (cards + *card_index)->rank == (cards + *card_index - 1)->rank + x;
}

bool is_card_rank_one_higher_than_previous(Card* cards, uint8_t* card_index) {
    return is_card_rank_higher_than_previous_by_x(cards, card_index, 1);
}

bool is_card_rank_same_as_previous(Card* cards, uint8_t* card_index) {
    return is_card_rank_higher_than_previous_by_x(cards, card_index, 0);
}

SequencesOfEqualRank* get_sequences_of_equal_rank(Card* sorted_hand) {
    SequencesOfEqualRank* res = malloc(sizeof(SequencesOfEqualRank));
    res->count = 0;
    res->sequences = malloc(sizeof(SequenceOfEqualRank) * 5);
    res->capacity = 5;
    uint8_t cur_sequence_rank = sorted_hand->rank;
    uint8_t cur_sequence_length = 1;
    for (uint8_t i = 1; i < 5; ++i) {
        if ((sorted_hand + i)->rank == cur_sequence_rank) {
            ++cur_sequence_length;
        }
        else {
            (res->sequences + res->count)->rank = cur_sequence_rank;
            (res->sequences + res->count)->length = cur_sequence_length;
            ++res->count;
            cur_sequence_rank = (sorted_hand + i)->rank;
            cur_sequence_length = 1;
        }
    }
    (res->sequences + res->count)->rank = cur_sequence_rank;
    (res->sequences + res->count)->length = cur_sequence_length;
    ++res->count;
    return res;
}

bool is_hand_straight(Card* sorted_hand) {
    uint8_t i = 1;
    for (; i < 4; ++i) {
        if (!is_card_rank_one_higher_than_previous(sorted_hand, &i)) {
            return false;
        }
    }
    return is_card_rank_one_higher_than_previous(sorted_hand, &i) || ((sorted_hand + i)->rank == 12 && sorted_hand->rank == 0);
}

bool is_hand_flush(Card* hand) {
    for (uint8_t i = 1; i < 5; ++i) {
        if ((hand + i)->suit != (hand + i - 1)->suit) {
            return false;
        }
    }
    return true;
}

bool is_hand_straight_flush(Card* sorted_hand) {
    return is_hand_flush(sorted_hand) && is_hand_straight(sorted_hand);
}

uint32_t get_hand_strength(HandStrength* hand_strength) {
    uint32_t res = 0;
    res += hand_strength->hand_value * pow(13, 5);
    res += hand_strength->first_kicker * pow(13, 4);
    res += hand_strength->second_kicker * pow(13, 3);
    res += hand_strength->third_kicker * pow(13, 2);
    res += hand_strength->fourth_kicker * pow(13, 1);
    res += hand_strength->fifth_kicker * pow(13, 0);
    return res;
}

// 2 sequences -> full house, four of a kind (2)
// 3 sequences -> two pairs, three of a kind (2)
// 4 sequences -> pair (1)
// 5 sequences -> nothing, straight, flush, straight flush (4)

// nothing (0) -> 5 kickers
// pair (1) -> 4 kickers
// two pairs (2) -> 3 kickers
// three of a kind (3) -> 3 kickers
// straight (4) -> 1 kicker
// flush (5) -> 5 kickers
// full house (6) -> 2 kickers
// four of a kind (7) -> 2 kickers
// straight flush (8) -> 1 kicker

HandStrength* evaluate_hand(Card* sorted_hand) {

    HandStrength* res = malloc(sizeof(HandStrength));

    SequencesOfEqualRank* sequences_of_equal_rank = get_sequences_of_equal_rank(sorted_hand);

    if (sequences_of_equal_rank->count == 2) {
        // four of a kind OR full house
        if (sequences_of_equal_rank->sequences->length == 4 || sequences_of_equal_rank->sequences->length == 1) {
            // four of a kind
            res->hand_value = 7;
            if ((sequences_of_equal_rank->sequences + 1)->length == 4) {
                res->first_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
                res->second_kicker = sequences_of_equal_rank->sequences->rank;
            }
            else {
                res->first_kicker = sequences_of_equal_rank->sequences->rank;
                res->second_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
            }
        }
        else {
            // full house
            res->hand_value = 6;
            if ((sequences_of_equal_rank->sequences + 1)->length == 3) {
                res->first_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
                res->second_kicker = sequences_of_equal_rank->sequences->rank;
            }
            else {
                res->first_kicker = sequences_of_equal_rank->sequences->rank;
                res->second_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
            }
        }
        res->third_kicker = 0;
        res->fourth_kicker = 0;
        res->fifth_kicker = 0;
    }

    else if (sequences_of_equal_rank->count == 3) {
        // three of a kind OR two pairs
        if (sequences_of_equal_rank->sequences->length == 3 || (sequences_of_equal_rank->sequences + 1)->length == 3 || (sequences_of_equal_rank->sequences + 2)->length == 3) {
            // three of a kind
            res->hand_value = 3;
            if ((sequences_of_equal_rank->sequences + 2)->length == 3) {
                res->first_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
                res->second_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
                res->third_kicker = sequences_of_equal_rank->sequences->rank;
            }
            else if ((sequences_of_equal_rank->sequences + 1)->length == 3) {
                res->first_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
                res->second_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
                res->third_kicker = sequences_of_equal_rank->sequences->rank;
            }
            else {
                res->first_kicker = sequences_of_equal_rank->sequences->rank;
                res->second_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
                res->third_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
            }
        }
        else {
            // two pairs
            res->hand_value = 2;
            if (sequences_of_equal_rank->sequences->length == 1) {
                res->first_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
                res->second_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
                res->third_kicker = sequences_of_equal_rank->sequences->rank;
            }
            else if ((sequences_of_equal_rank->sequences + 1)->length == 1) {
                res->first_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
                res->second_kicker = sequences_of_equal_rank->sequences->rank;
                res->third_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
            }
            else {
                res->first_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
                res->second_kicker = sequences_of_equal_rank->sequences->rank;
                res->third_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
            }
        }
        res->fourth_kicker = 0;
        res->fifth_kicker = 0;
    }

    else if (sequences_of_equal_rank->count == 4) {
        // pair
        res->hand_value = 1;
        if ((sequences_of_equal_rank->sequences + 3)->length == 2) {
            res->first_kicker = (sequences_of_equal_rank->sequences + 3)->rank;
            res->second_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
            res->third_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
            res->fourth_kicker = sequences_of_equal_rank->sequences->rank;
        }
        else if ((sequences_of_equal_rank->sequences + 2)->length == 2) {
            res->first_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
            res->second_kicker = (sequences_of_equal_rank->sequences + 3)->rank;
            res->third_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
            res->fourth_kicker = sequences_of_equal_rank->sequences->rank;
        }
        else if ((sequences_of_equal_rank->sequences + 1)->length == 2) {
            res->first_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
            res->second_kicker = (sequences_of_equal_rank->sequences + 3)->rank;
            res->third_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
            res->fourth_kicker = sequences_of_equal_rank->sequences->rank;
        }
        else {
            res->first_kicker = sequences_of_equal_rank->sequences->rank;
            res->second_kicker = (sequences_of_equal_rank->sequences + 3)->rank;
            res->third_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
            res->fourth_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
        }
        res->fifth_kicker = 0;
    }

    else {
        // straight flush OR flush OR straight OR nothing
        bool is_flush = is_hand_flush(sorted_hand);
        bool is_straight = is_hand_straight(sorted_hand);

        if (is_flush == true) {
            if (is_straight == true) {
                // straight flush
                res->hand_value = 8;
                if ((sequences_of_equal_rank->sequences + 4)->rank == 12 && sequences_of_equal_rank->sequences->rank == 0) {
                    res->first_kicker = (sequences_of_equal_rank->sequences + 3)->rank;
                }
                else {
                    res->first_kicker = (sequences_of_equal_rank->sequences + 4)->rank;
                }
                res->second_kicker = 0;
                res->third_kicker = 0;
                res->fourth_kicker = 0;
                res->fifth_kicker = 0;
            }
            else {
                // flush
                res->hand_value = 5;
                res->first_kicker = (sequences_of_equal_rank->sequences + 4)->rank;
                res->second_kicker = (sequences_of_equal_rank->sequences + 3)->rank;
                res->third_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
                res->fourth_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
                res->fifth_kicker = sequences_of_equal_rank->sequences->rank;
            }
        }
        else if (is_straight == true) {
            // straight
            res->hand_value = 4;
            if ((sequences_of_equal_rank->sequences + 4)->rank == 12 && sequences_of_equal_rank->sequences->rank == 0) {
                res->first_kicker = (sequences_of_equal_rank->sequences + 3)->rank;
            }
            else {
                res->first_kicker = (sequences_of_equal_rank->sequences + 4)->rank;
            }
            res->second_kicker = 0;
            res->third_kicker = 0;
            res->fourth_kicker = 0;
            res->fifth_kicker = 0;
        }
        else {
            // nothing
            res->hand_value = 0;
            res->first_kicker = (sequences_of_equal_rank->sequences + 4)->rank;
            res->second_kicker = (sequences_of_equal_rank->sequences + 3)->rank;
            res->third_kicker = (sequences_of_equal_rank->sequences + 2)->rank;
            res->fourth_kicker = (sequences_of_equal_rank->sequences + 1)->rank;
            res->fifth_kicker = sequences_of_equal_rank->sequences->rank;
        }
    }

    return res;
}

void deal_hole_cards(PlayersHoleCards* players_hole_cards, Cards* deck) {
    uint8_t i = 0;
    for (; i != players_hole_cards->num_players; ++i) {
        (players_hole_cards->players_hole_cards + i)->card_0 = draw_card(deck);
    }
    for (i = 0; i != players_hole_cards->num_players; ++i) {
        (players_hole_cards->players_hole_cards + i)->card_1 = draw_card(deck);
    }
}

void burn_card(Cards* deck) {
    --deck->count;
}

bool valid_number_of_players(uint8_t* num_players) {
    return *num_players >= 2 && *num_players <= 22;
}

void deal_community_card(Cards* community_cards, Cards* deck) {
    Card card = draw_card(deck);
    append_card(community_cards, &card.rank, &card.suit);
}

void deal_the_flop(Cards* community_cards, Cards* deck) {
    for (uint8_t i = 0; i < 3; ++i) {
        deal_community_card(community_cards, deck);
    }
}

void deal_the_turn(Cards* community_cards, Cards* deck) {
    deal_community_card(community_cards, deck);
}

void deal_the_river(Cards* community_cards, Cards* deck) {
    deal_community_card(community_cards, deck);
}

Cards* init_community_cards() {
    Cards* res = malloc(sizeof(Cards));
    res->count = 0;
    res->cards = malloc(sizeof(Card) * 5);
    res->capacity = 5;
    return res;
}

PlayersHoleCards* init_players_hole_cards(uint8_t* num_players) {
    PlayersHoleCards* res = malloc(sizeof(PlayersHoleCards));
    res->players_hole_cards = malloc(sizeof(PlayerHoleCards) * *num_players);
    res->num_players = *num_players;
    return res;
}

void deal(PlayersHoleCards* players_hole_cards, Cards* deck, Cards* community_cards) {
    deal_hole_cards(players_hole_cards, deck);
    burn_card(deck);
    deal_the_flop(community_cards, deck);
    burn_card(deck);
    deal_the_turn(community_cards, deck);
    burn_card(deck);
    deal_the_river(community_cards, deck);
}

void simulate_game(uint8_t* num_players) {
    if (valid_number_of_players(num_players) == false) {
        printf("Error: Number of players must be 2-22.");
        return;
    }
    Cards* deck = get_shuffled_deck();
    PlayersHoleCards* players_hole_cards = init_players_hole_cards(num_players);
    Cards* community_cards = init_community_cards();
    deal(players_hole_cards, deck, community_cards);
}

int main() {

    init_rand();

    uint8_t num_players = 12;

    for (uint64_t i = 0; i < 1e6; ++i) {
        simulate_game(&num_players);
    }

    return 0;
}
