#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>

#define INITIAL_DECK_SIZE 52
#define NUM_HOLE_CARDS_PER_PLAYER 2
#define NUM_SUITS 4
#define NUM_CARD_RANKS 13
#define MAX_NUM_COMMUNITY_CARDS 5
#define MAX_NUM_BURNED_CARDS 3
#define MAX_NUM_SEQUENCES_OF_CARDS_OF_EQUAL_RANK 5
#define MIN_SUIT 0
#define MAX_SUIT 3
#define MIN_CARD_RANK 0
#define MAX_CARD_RANK 12
#define HAND_LENGTH 5
#define DEPTH 1e4
#define NUM_GAMES 1e0

#define NUM_PLAYERS 4 // must be 2-22

//* card rank (uint8_t):
// 0 -> 2
#define TWO 0
// 1 -> 3
// 2 -> 4
// 3 -> 5
// 4 -> 6
// 5 -> 7
// 6 -> 8
// 7 -> 9
// 8 -> 10
// 9 -> J
// 10 -> Q
// 11 -> K
// 12 -> A
#define ACE 12

char* rank_to_human_readable[13];

//* suit (uint8_t):
// 0 -> clubs
// 1 -> diamonds
// 2 -> hearts
// 3 -> spades

wchar_t suit_to_human_readable[4];

//* hand rank (uint8_t):
// 0 -> nothing
#define NOTHING 0
// 1 -> pair
#define PAIR 1
// 2 -> two pairs
#define TWO_PAIRS 2
// 3 -> three of a kind
#define THREE_OF_A_KIND 3
// 4 -> straight
#define STRAIGHT 4
// 5 -> flush
#define FLUSH 5
// 6 -> full house
#define FULL_HOUSE 6
// 7 -> four of a kind
#define FOUR_OF_A_KIND 7
// 8 -> straight flush
#define STRAIGHT_FLUSH 8

typedef struct {
    uint8_t rank;
    uint8_t suit;
} Card;

typedef struct {
    Card cards[NUM_HOLE_CARDS_PER_PLAYER];
    uint8_t count;
} HoleCards;

typedef struct {
    uint8_t id;
    HoleCards hole_cards;
    long double probability_of_winning;
} Player;

typedef struct {
    Player players[NUM_PLAYERS];
    uint8_t count;
} Players;

typedef struct {
    Card cards[MAX_NUM_COMMUNITY_CARDS];
    uint8_t count;
} CommunityCards;

typedef struct {
    Card cards[INITIAL_DECK_SIZE];
    uint8_t count;
} Deck;

typedef struct {
    Card cards[MAX_NUM_BURNED_CARDS];
    uint8_t count;
} BurnedCards;

typedef struct {
    Deck deck;
    Players players;
    CommunityCards community_cards;
    BurnedCards burned_cards;
} Game;

typedef struct {
    uint8_t rank;
    uint8_t length;
} SequenceOfCardsOfEqualRank;

typedef struct {
    SequenceOfCardsOfEqualRank sequences[MAX_NUM_SEQUENCES_OF_CARDS_OF_EQUAL_RANK];
    uint8_t count;
} SequencesOfCardsOfEqualRank;

void print_card(Card* card) {
    printf("%s%lc", rank_to_human_readable[card->rank], suit_to_human_readable[card->suit]);
}

void print_cards(Card* cards, uint8_t* count) {
    uint8_t i = 0;
    for (; i < *count - 1; ++i) {
        print_card(&cards[i]);
        printf(" ");
    }
    print_card(&cards[i]);
}

void display_table(Players* players, CommunityCards* community_cards) {
    uint8_t i;
    for (i = 0; i < players->count; ++i) {
        printf("Player %d: ", players->players[i].id);
        print_cards(players->players[i].hole_cards.cards, &players->players[i].hole_cards.count);
        printf(" (%.2Lf%%)\n", players->players[i].probability_of_winning * 100);
    }
    for (i = 0; i < 7; ++i) {
        printf("\t");
    }
    print_cards(community_cards->cards, &community_cards->count);
    printf("\n");
}

void init_rand() {
    srand(time(NULL));
}

uint8_t get_rand_index(uint8_t* length) {
    return rand() % *length;
}

void pop_and_transfer_card(Card* source_cards_cards, uint8_t* source_cards_count, uint8_t pop_index, Card* destination_cards_cards, uint8_t* destination_cards_count) {
    destination_cards_cards[(*destination_cards_count)++] = source_cards_cards[pop_index];
    --(*source_cards_count);
    for (uint8_t i = pop_index; i < *source_cards_count; ++i) {
        source_cards_cards[i] = source_cards_cards[i + 1];
    }
}

Deck init_deck() {
    Deck res;
    res.count = 0;
    return res;
}

Deck get_original_unshuffled_52_card_deck() {
    Deck res = init_deck();
    for (uint8_t suit = MIN_SUIT; suit <= MAX_SUIT; ++suit) {
        for (uint8_t rank = MIN_CARD_RANK; rank <= MAX_CARD_RANK; ++rank) {
            res.cards[res.count].rank = rank;
            res.cards[res.count].suit = suit;
            ++(res.count);
        }
    }
    return res;
}

Deck original_unshuffled_52_card_deck;

Deck get_shuffled_deck(Deck* original_unshuffled_deck) {
    Deck res = init_deck();
    Deck original_unshuffled_deck_copy = *original_unshuffled_deck;
    while (original_unshuffled_deck_copy.count > 0) {
        pop_and_transfer_card(original_unshuffled_deck_copy.cards, &original_unshuffled_deck_copy.count, get_rand_index(&original_unshuffled_deck_copy.count), res.cards, &res.count);
    }
    return res;
}

// Card pop_card(Card* source_cards_cards, uint8_t* source_cards_count, uint8_t index) {
//     Card res = source_cards_cards[index];
//     for (; index < *source_cards_count - 1; ++index) {
//         source_cards_cards[index] = source_cards_cards[index + 1];
//     }
//     --*source_cards_count;
//     return res;
// }

void deal_card(Deck* deck, Card* destination_cards_cards, uint8_t* destination_cards_count) {
    pop_and_transfer_card(deck->cards, &deck->count, deck->count - 1, destination_cards_cards, destination_cards_count);
}

int compare_cards(const void* card_0, const void* card_1) {
    return ((Card*)card_0)->rank - ((Card*)card_1)->rank;
}
void sort_hand(Card* hand) {
    qsort(hand, HAND_LENGTH, sizeof(Card), compare_cards);
}

int compare_sequences_of_cards_of_equal_rank(const void* sequence_0, const void* sequence_1) {
    int length_difference = ((SequenceOfCardsOfEqualRank*)sequence_0)->length - ((SequenceOfCardsOfEqualRank*)sequence_1)->length;
    if (length_difference != 0) {
        return length_difference;
    }
    return ((SequenceOfCardsOfEqualRank*)sequence_0)->rank - ((SequenceOfCardsOfEqualRank*)sequence_1)->rank;
}
void sort_sequences_of_cards_of_equal_rank(SequencesOfCardsOfEqualRank* sequences) {
    qsort(sequences->sequences, sequences->count, sizeof(SequenceOfCardsOfEqualRank), compare_sequences_of_cards_of_equal_rank);
}

SequencesOfCardsOfEqualRank init_sequences_of_cards_of_equal_rank() {
    SequencesOfCardsOfEqualRank res;
    res.count = 0;
    return res;
}

void append_sequence_of_cards_of_equal_rank(SequencesOfCardsOfEqualRank* sequences, uint8_t* new_sequence_rank, uint8_t* new_sequence_length) {
    sequences->sequences[sequences->count].rank = *new_sequence_rank;
    sequences->sequences[sequences->count].length = *new_sequence_length;
    ++(sequences->count);
}

SequencesOfCardsOfEqualRank get_sorted_sequences_of_cards_of_equal_rank(Card* sorted_hand) {
    SequencesOfCardsOfEqualRank res = init_sequences_of_cards_of_equal_rank();
    uint8_t cur_sequence_rank = sorted_hand[0].rank;
    uint8_t cur_sequence_length = 1;
    for (uint8_t i = 1; i < HAND_LENGTH; ++i) {
        if (sorted_hand[i].rank == cur_sequence_rank) {
            ++cur_sequence_length;
        }
        else {
            append_sequence_of_cards_of_equal_rank(&res, &cur_sequence_rank, &cur_sequence_length);
            cur_sequence_rank = sorted_hand[i].rank;
            cur_sequence_length = 1;
        }
    }
    append_sequence_of_cards_of_equal_rank(&res, &cur_sequence_rank, &cur_sequence_length);
    sort_sequences_of_cards_of_equal_rank(&res);
    return res;
}

bool is_card_rank_one_higher_than_previous(Card* hand, uint8_t* card_index) {
    return hand[*card_index].rank - 1 == hand[*card_index - 1].rank;
}

bool is_hand_straight(Card* sorted_hand) {
    uint8_t i = 1;
    for (; i < HAND_LENGTH - 1; ++i) {
        if (is_card_rank_one_higher_than_previous(sorted_hand, &i) == false) {
            return false;
        }
    }
    return is_card_rank_one_higher_than_previous(sorted_hand, &i) == true || (sorted_hand[i].rank == ACE && sorted_hand[0].rank == TWO);
}

bool is_hand_flush(Card* hand) {
    for (uint8_t i = 1; i < HAND_LENGTH; ++i) {
        if (hand[i].suit != hand[i - 1].suit) {
            return false;
        }
    }
    return true;
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

uint32_t hand_strength(Card* sorted_hand) {

    // Every hand will have a rank and at least 1 kicker
    uint8_t hand_rank;
    uint8_t kicker_1_rank;
    uint8_t kicker_2_rank = 0;
    uint8_t kicker_3_rank = 0;
    uint8_t kicker_4_rank = 0;
    uint8_t kicker_5_rank = 0;

    SequencesOfCardsOfEqualRank sequences = get_sorted_sequences_of_cards_of_equal_rank(sorted_hand);
    
    if (sequences.count == 2) {
        // four of a kind OR full house
        if (sequences.sequences[0].length == 1) {
            // four of a kind
            hand_rank = FOUR_OF_A_KIND;
        }
        else {
            // full house
            hand_rank = FULL_HOUSE;
        }
        kicker_1_rank = sequences.sequences[1].rank;
        kicker_2_rank = sequences.sequences[0].rank;
    }
    else if (sequences.count == 3) {
        // three of a kind OR two pairs
        if (sequences.sequences[1].length == 1) {
            // three of a kind
            hand_rank = THREE_OF_A_KIND;
        }
        else {
            // two pairs
            hand_rank = TWO_PAIRS;
        }
        kicker_1_rank = sequences.sequences[2].rank;
        kicker_2_rank = sequences.sequences[1].rank;
        kicker_3_rank = sequences.sequences[0].rank;
    }
    else if (sequences.count == 4) {
        // pair
        hand_rank = PAIR;
        kicker_1_rank = sequences.sequences[3].rank;
        kicker_2_rank = sequences.sequences[2].rank;
        kicker_3_rank = sequences.sequences[1].rank;
        kicker_4_rank = sequences.sequences[0].rank;
    }
    else {
        // straight flush OR flush OR straight OR nothing
        bool is_straight = is_hand_straight(sorted_hand);
        bool is_flush = is_hand_flush(sorted_hand);

        if (is_straight == true) {
            // straight flush OR straight

            if (is_flush == true) {
                // straight flush
                hand_rank = STRAIGHT_FLUSH;
            }
            else {
                // straight
                hand_rank = STRAIGHT;
            }

            if (sorted_hand[HAND_LENGTH - 1].rank == ACE && sorted_hand[0].rank == TWO) {
                kicker_1_rank = sorted_hand[HAND_LENGTH - 1 - 1].rank;
            }
            else {
                kicker_1_rank = sorted_hand[HAND_LENGTH - 1].rank;
            }

        }
        else {
            // flush OR nothing
            if (is_flush == true) {
                // flush
                hand_rank = FLUSH;
            }
            else {
                // nothing
                hand_rank = NOTHING;
            }
            kicker_1_rank = sequences.sequences[4].rank;
            kicker_2_rank = sequences.sequences[3].rank;
            kicker_3_rank = sequences.sequences[2].rank;
            kicker_4_rank = sequences.sequences[1].rank;
            kicker_5_rank = sequences.sequences[0].rank;
        }
    }

    uint32_t res = 0;
    res += hand_rank * pow(13, 5);
    res += kicker_1_rank * pow(13, 4);
    res += kicker_2_rank * pow(13, 3);
    res += kicker_3_rank * pow(13, 2);
    res += kicker_4_rank * pow(13, 1);
    res += kicker_5_rank * pow(13, 0);
    return res;
}

void deal_hole_cards(Player* players, Deck* deck) {
    for (uint8_t i = 0; i < NUM_HOLE_CARDS_PER_PLAYER; ++i) {
        for (uint8_t j = 0; j < NUM_PLAYERS; ++j) {
            deal_card(deck, players[j].hole_cards.cards, &players[j].hole_cards.count);
        }
    }
}

void burn_card(Deck* deck, BurnedCards* burned_cards) {
    deal_card(deck, burned_cards->cards, &burned_cards->count);
}

void deal_community_card(CommunityCards* community_cards, Deck* deck) {
    deal_card(deck, community_cards->cards, &community_cards->count);
}

void deal_the_flop(CommunityCards* community_cards, Deck* deck, BurnedCards* burned_cards) {
    burn_card(deck, burned_cards);
    for (uint8_t i = 0; i < 3; ++i) {
        deal_community_card(community_cards, deck);
    }
}

void deal_the_turn(CommunityCards* community_cards, Deck* deck, BurnedCards* burned_cards) {
    burn_card(deck, burned_cards);
    deal_community_card(community_cards, deck);
}

void deal_the_river(CommunityCards* community_cards, Deck* deck, BurnedCards* burned_cards) {
    burn_card(deck, burned_cards);
    deal_community_card(community_cards, deck);
}

Players init_players() {
    Players res;
    res.count = 0;
    while (res.count < NUM_PLAYERS) {
        res.players[res.count].hole_cards.count = 0;
        res.players[res.count].id = res.count;
        ++(res.count);
    }
    return res;
}

CommunityCards init_community_cards() {
    CommunityCards res;
    res.count = 0;
    return res;
}

uint32_t get_player_strongest_hand(Card* hole_cards, Card* community_cards) {
    
    Card available_cards[MAX_NUM_COMMUNITY_CARDS + NUM_HOLE_CARDS_PER_PLAYER];

    uint8_t i;
    for (i = 0; i < MAX_NUM_COMMUNITY_CARDS; ++i) {
        available_cards[i] = community_cards[i];
    }
    for (i = 0; i < NUM_HOLE_CARDS_PER_PLAYER; ++i) {
        available_cards[MAX_NUM_COMMUNITY_CARDS + i] = hole_cards[i];
    }

    uint32_t res = 0;

    Card possible_hand[5];
    uint32_t possible_strength;

    for (uint8_t i = 0; i < 3; ++i) {
        for (uint8_t j = i + 1; j < 4; ++j) {
            for (uint8_t k = j + 1; k < 5; ++k) {
                for (uint8_t l = k + 1; l < 6; ++l) {
                    for (uint8_t m = l + 1; m < 7; ++m) {

                        possible_hand[0] = available_cards[i];
                        possible_hand[1] = available_cards[j];
                        possible_hand[2] = available_cards[k];
                        possible_hand[3] = available_cards[l];
                        possible_hand[4] = available_cards[m];

                        sort_hand(possible_hand);

                        possible_strength = hand_strength(possible_hand);

                        if (possible_strength > res) {
                            res = possible_strength;
                        }
                    }
                }
            }
        }
    }

    return res;
}

void set_winning_players(long double* winning_players, Player* players, Card* community_cards) {
    uint8_t num_winning_players = 1;
    long double fraction_for_each_winner = (long double) 1 / num_winning_players;
    winning_players[0] = fraction_for_each_winner;
    uint32_t strongest_hand = get_player_strongest_hand(players[0].hole_cards.cards, community_cards);
    for (uint8_t i = 1; i < NUM_PLAYERS; ++i) {
        winning_players[i] = 0;
    }
    uint32_t cur_player_strongest_hand_strength;
    for (uint8_t i = 1; i < NUM_PLAYERS; ++i) {
        cur_player_strongest_hand_strength = get_player_strongest_hand(players[i].hole_cards.cards, community_cards);
        if (cur_player_strongest_hand_strength > strongest_hand) {
            strongest_hand = cur_player_strongest_hand_strength;
            num_winning_players = 1;
            fraction_for_each_winner = (long double) 1 / num_winning_players;
            winning_players[i] = fraction_for_each_winner;
            for (uint8_t j = 0; j < i; ++j) {
                winning_players[j] = 0;
            }
        }
        else if (cur_player_strongest_hand_strength == strongest_hand) {
            ++num_winning_players;
            fraction_for_each_winner = (long double) 1 / num_winning_players;
            winning_players[i] = fraction_for_each_winner;
            for (uint8_t j = 0; j < i; ++j) {
                if (winning_players[j] > 0) {
                    winning_players[j] = fraction_for_each_winner;
                }
            }
        }
    }
}

BurnedCards init_burned_cards() {
    BurnedCards res;
    res.count = 0;
    return res;
}

Deck get_shuffled_unseen_cards(Deck* deck, BurnedCards* burned_cards) {
    Deck res = init_deck();
    uint8_t i;
    for (i = 0; i < deck->count; ++i) {
        res.cards[(res.count)++] = deck->cards[i];
    }
    for (i = 0; i < burned_cards->count; ++i) {
        res.cards[(res.count)++] = burned_cards->cards[i];
    }
    return get_shuffled_deck(&res);
}

void calculate_winning_probability_distribution(Game* game) {
    long double wins_distribution[NUM_PLAYERS];
    for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
        wins_distribution[i] = 0;
    }
    uint64_t iters;
    for (iters = 0; iters < DEPTH; ++iters) {

        Deck possible_deck = get_shuffled_unseen_cards(&game->deck, &game->burned_cards);
        BurnedCards possible_burned_cards = init_burned_cards();
        for (uint8_t i = 0; i < game->burned_cards.count; ++i) {
            burn_card(&possible_deck, &possible_burned_cards);
        }
        CommunityCards community_cards_copy = game->community_cards;

        if (community_cards_copy.count < 3) {
            deal_the_flop(&community_cards_copy, &possible_deck, &possible_burned_cards);
        }
        if (community_cards_copy.count < 4) {
            deal_the_turn(&community_cards_copy, &possible_deck, &possible_burned_cards);
        }
        if (community_cards_copy.count < 5) {
            deal_the_river(&community_cards_copy, &possible_deck, &possible_burned_cards);
        }

        long double winning_players[NUM_PLAYERS];

        set_winning_players(winning_players, game->players.players, community_cards_copy.cards);

        for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
            wins_distribution[i] += winning_players[i];
        }
    }
    for (uint8_t i = 0; i < NUM_PLAYERS; ++i) {
        game->players.players[i].probability_of_winning = wins_distribution[i] / iters;
    }
}

void simulate_game() {

    Game game = { .deck = get_shuffled_deck(&original_unshuffled_52_card_deck), .players = init_players(), .burned_cards = init_burned_cards(), .community_cards = init_community_cards() };
    
    deal_hole_cards(game.players.players, &game.deck);

    deal_the_flop(&game.community_cards, &game.deck, &game.burned_cards);

    calculate_winning_probability_distribution(&game);

    display_table(&game.players, &game.community_cards);

    deal_the_turn(&game.community_cards, &game.deck, &game.burned_cards);

    calculate_winning_probability_distribution(&game);

    display_table(&game.players, &game.community_cards);

    deal_the_river(&game.community_cards, &game.deck, &game.burned_cards);

    calculate_winning_probability_distribution(&game);

    display_table(&game.players, &game.community_cards);
}

int main() {

    rank_to_human_readable[0] = "2";
    rank_to_human_readable[1] = "3";
    rank_to_human_readable[2] = "4";
    rank_to_human_readable[3] = "5";
    rank_to_human_readable[4] = "6";
    rank_to_human_readable[5] = "7";
    rank_to_human_readable[6] = "8";
    rank_to_human_readable[7] = "9";
    rank_to_human_readable[8] = "10";
    rank_to_human_readable[9] = "J";
    rank_to_human_readable[10] = "Q";
    rank_to_human_readable[11] = "K";
    rank_to_human_readable[12] = "A";

    setlocale(LC_ALL, "en_US.UTF-8");

    suit_to_human_readable[0] = L'♣';
    suit_to_human_readable[1] = L'♦';
    suit_to_human_readable[2] = L'♥';
    suit_to_human_readable[3] = L'♠';

    original_unshuffled_52_card_deck = get_original_unshuffled_52_card_deck();

    init_rand();

    for (uint64_t i = 0; i < NUM_GAMES; ++i) {
        simulate_game();
    }

    return 0;
}
