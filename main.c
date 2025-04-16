#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>

#define STANDARD_DECK_SIZE 52
#define NUM_HOLE_CARDS_PER_PLAYER 2
#define MAX_NUM_COMMUNITY_CARDS 5
#define NUM_SUITS 4
#define NUM_CARD_RANKS 13
#define MAX_NUM_BURNED_CARDS 3
#define MAX_NUM_SEQUENCES_OF_CARDS_OF_EQUAL_RANK_IN_HAND 5

#define MIN_SUIT 0
#define MAX_SUIT 3
#define MIN_CARD_RANK 0
#define MAX_CARD_RANK 12

#define HAND_LENGTH 5

#define MIN_NUM_PLAYERS 2
#define MAX_NUM_PLAYERS 22

// card ranks:
#define TWO 0
#define THREE 1
#define FOUR 2
#define FIVE 3
#define SIX 4
#define SEVEN 5
#define EIGHT 6
#define NINE 7
#define TEN 8
#define JACK 9
#define QUEEN 10
#define KING 11
#define ACE 12

// suits:
#define CLUBS 0
#define DIAMONDS 1
#define HEARTS 2
#define SPADES 3

// hand ranks:
#define NOTHING 0
#define PAIR 1
#define TWO_PAIRS 2
#define THREE_OF_A_KIND 3
#define STRAIGHT 4
#define FLUSH 5
#define FULL_HOUSE 6
#define FOUR_OF_A_KIND 7
#define STRAIGHT_FLUSH 8

char card_rank_to_human_readable[NUM_CARD_RANKS][3];
wchar_t suit_to_human_readable[NUM_SUITS];
Deck original_unshuffled_standard_deck;

typedef struct {
    uint8_t rank;
    uint8_t suit;
} Card;

typedef struct {
    Card cards[NUM_HOLE_CARDS_PER_PLAYER];
    uint8_t count;
} HoleCards;

HoleCards init_hole_cards() {
    HoleCards res;
    res.count = 0;
    return res;
}

typedef struct {
    HoleCards hole_cards[MAX_NUM_PLAYERS];
    uint8_t count;
} Players;

Players init_players(uint8_t* count) {
    Players res;
    for (res.count = 0; res.count < *count; ++(res.count)) {
        res.hole_cards[res.count] = init_hole_cards();
    }
    return res;
}

typedef struct {
    Card cards[MAX_NUM_COMMUNITY_CARDS];
    uint8_t count;
} CommunityCards;

CommunityCards init_community_cards() {
    CommunityCards res;
    res.count = 0;
    return res;
}

typedef struct {
    Card cards[HAND_LENGTH];
    uint8_t count;
} Hand;

Hand init_hand() {
    Hand res;
    res.count = 0;
    return res;
}

typedef struct {
    Card cards[STANDARD_DECK_SIZE];
    uint8_t count;
} Deck;

Deck init_deck() {
    Deck res;
    res.count = 0;
    return res;
}

typedef struct {
    Card cards[MAX_NUM_BURNED_CARDS];
    uint8_t count;
} BurnedCards;

BurnedCards init_burned_cards() {
    BurnedCards res;
    res.count = 0;
    return res;
}

typedef struct {
    Deck deck;
    Players players;
    CommunityCards community_cards;
    BurnedCards burned_cards;
} Game;

Game init_game(uint8_t* num_players) {
    Game res;
    res.deck = init_deck();
    res.players = init_players(num_players);
    res.community_cards = init_community_cards();
    res.burned_cards = init_burned_cards();
    return res;
}

typedef struct {
    uint8_t rank;
    uint8_t length;
} SequenceOfCardsOfEqualRank;

typedef struct {
    SequenceOfCardsOfEqualRank sequences[MAX_NUM_SEQUENCES_OF_CARDS_OF_EQUAL_RANK_IN_HAND];
    uint8_t count;
} SequencesOfCardsOfEqualRank;

SequencesOfCardsOfEqualRank init_sequences_of_cards_of_equal_rank() {
    SequencesOfCardsOfEqualRank res;
    res.count = 0;
    return res;
}

Deck get_unseen_cards_from_perspective_of_tv_watcher(Deck* deck, BurnedCards* burned_cards) {
    Deck res = init_deck();
    uint8_t i;
    for (i = 0; i < deck->count; ++i) {
        res.cards[(res.count)++] = deck->cards[i];
    }
    for (i = 0; i < burned_cards->count; ++i) {
        res.cards[(res.count)++] = burned_cards->cards[i];
    }
    return res;
}

void print_card(Card* card) {
    printf("%s%lc", card_rank_to_human_readable[card->rank], suit_to_human_readable[card->suit]);
}

void print_cards(Card* cards, uint8_t* count) {
    uint8_t i;
    for (i = 0; i < *count - 1; ++i) {
        print_card(&cards[i]);
        printf(" ");
    }
    print_card(&cards[i]);
}

void display_table_for_tv_watcher(Players* players, CommunityCards* community_cards, double* players_equities) {
    uint8_t i;
    for (i = 0; i < players->count; ++i) {
        printf("Player %d: ", i);
        print_cards(players->hole_cards[i].cards, &players->hole_cards[i].count);
        printf(" (%.0Lf%%)\n", players_equities[i] * 100);
    }
    for (i = 0; i < 7; ++i) {
        printf("\t");
    }
    print_cards(community_cards->cards, &community_cards->count);
    printf("\n");
}

void init_card_rank_to_human_readable() {
    card_rank_to_human_readable[TWO][0] = '2';
    card_rank_to_human_readable[THREE][0] = '3';
    card_rank_to_human_readable[FOUR][0] = '4';
    card_rank_to_human_readable[FIVE][0] = '5';
    card_rank_to_human_readable[SIX][0] = '6';
    card_rank_to_human_readable[SEVEN][0] = '7';
    card_rank_to_human_readable[EIGHT][0] = '8';
    card_rank_to_human_readable[NINE][0] = '9';
    card_rank_to_human_readable[TEN][0] = '1';
    card_rank_to_human_readable[TEN][1] = '0';
    card_rank_to_human_readable[JACK][0] = 'J';
    card_rank_to_human_readable[QUEEN][0] = 'Q';
    card_rank_to_human_readable[KING][0] = 'K';
    card_rank_to_human_readable[ACE][0] = 'A';
    for (uint8_t i = 0; i < 13; ++i) {
        if (i == 8) {
            card_rank_to_human_readable[i][2] = '\0';
        }
        else {
            card_rank_to_human_readable[i][1] = '\0';
        }
    }
}

void init_suit_to_human_readable() {
    setlocale(LC_ALL, "en_US.UTF-8");
    suit_to_human_readable[CLUBS] = L'♣';
    suit_to_human_readable[DIAMONDS] = L'♦';
    suit_to_human_readable[HEARTS] = L'♥';
    suit_to_human_readable[SPADES] = L'♠';
}

void init_rand() {
    srand(time(NULL));
}

void init_original_unshuffled_standard_deck() {
    original_unshuffled_standard_deck = get_unshuffled_standard_deck();
}

void init_globals() {
    init_card_rank_to_human_readable();
    init_suit_to_human_readable();
    init_original_unshuffled_standard_deck();
}

void init() {
    init_rand();
    init_globals();
}

uint8_t get_rand_index(uint8_t* length) {
    return rand() % *length;
}

void pop_and_append_card(Card* source_cards, uint8_t* source_cards_count, uint8_t pop_index, Card* destination_cards, uint8_t* destination_cards_count) {
    destination_cards[(*destination_cards_count)++] = source_cards[pop_index];
    --(*source_cards_count);
    for (uint8_t i = pop_index; i < *source_cards_count; ++i) {
        source_cards[i] = source_cards[i + 1];
    }
}

Deck get_unshuffled_standard_deck() {
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

Deck get_shuffled_deck(Deck* original_unshuffled_deck) {
    Deck res = init_deck();
    Deck original_unshuffled_deck_copy = *original_unshuffled_deck;
    while (original_unshuffled_deck_copy.count > 0) {
        pop_and_append_card(original_unshuffled_deck_copy.cards, &original_unshuffled_deck_copy.count, get_rand_index(&original_unshuffled_deck_copy.count), res.cards, &res.count);
    }
    return res;
}

void deal_card(Deck* deck, Card* destination_cards, uint8_t* destination_cards_count) {
    pop_and_append_card(deck->cards, &deck->count, deck->count - 1, destination_cards, destination_cards_count);
}

int compare_cards(const void* card_0, const void* card_1) {
    return ((Card*)card_0)->rank - ((Card*)card_1)->rank;
}
void sort_hand(Hand* hand) {
    qsort(hand->cards, hand->count, sizeof(Card), compare_cards);
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

void append_sequence_of_cards_of_equal_rank(SequencesOfCardsOfEqualRank* existing_sequences, uint8_t* new_sequence_rank, uint8_t* new_sequence_length) {
    existing_sequences->sequences[existing_sequences->count].rank = *new_sequence_rank;
    existing_sequences->sequences[existing_sequences->count].length = *new_sequence_length;
    ++(existing_sequences->count);
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

void deal_hole_cards(Players* players, Deck* deck) {
    for (uint8_t i = 0; i < NUM_HOLE_CARDS_PER_PLAYER; ++i) {
        for (uint8_t j = 0; j < players->count; ++j) {
            deal_card(deck, players->hole_cards[j].cards, &players->hole_cards[j].count);
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

void set_players_equities(double* players_equities, Players* players, Card* community_cards) {
    uint8_t num_winning_players = 1;
    double equity_for_each_winner = (double) 1 / num_winning_players;
    players_equities[0] = equity_for_each_winner;
    uint32_t strongest_hand = get_player_strongest_hand(players->hole_cards[0].cards, community_cards);
    for (uint8_t i = 1; i < players->count; ++i) {
        players_equities[i] = 0;
    }
    uint32_t cur_player_strongest_hand_strength;
    for (uint8_t i = 1; i < players->count; ++i) {
        cur_player_strongest_hand_strength = get_player_strongest_hand(players->hole_cards[i].cards, community_cards);
        if (cur_player_strongest_hand_strength > strongest_hand) {
            strongest_hand = cur_player_strongest_hand_strength;
            num_winning_players = 1;
            equity_for_each_winner = (double) 1 / num_winning_players;
            players_equities[i] = equity_for_each_winner;
            for (uint8_t j = 0; j < i; ++j) {
                players_equities[j] = 0;
            }
        }
        else if (cur_player_strongest_hand_strength == strongest_hand) {
            ++num_winning_players;
            equity_for_each_winner = (double) 1 / num_winning_players;
            players_equities[i] = equity_for_each_winner;
            for (uint8_t j = 0; j < i; ++j) {
                if (players_equities[j] > 0) {
                    players_equities[j] = equity_for_each_winner;
                }
            }
        }
    }
}







// #############################################
// Simulations
// #############################################

#define DEPTH 1e5
#define NUM_GAMES 1e0


void set_winning_probability_distribution(Game* game, double winning_probability_distribution[MAX_NUM_PLAYERS]) {
    double wins_distribution[MAX_NUM_PLAYERS];
    for (uint8_t i = 0; i < game->players.count; ++i) {
        wins_distribution[i] = 0;
    }
    uint64_t iters;
    for (iters = 0; iters < DEPTH; ++iters) {

        
        Deck unseen_cards = get_unseen_cards_from_perspective_of_tv_watcher(&game->deck, &game->burned_cards);

        Deck possible_deck = get_shuffled_deck(&unseen_cards);
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

        double players_equities[MAX_NUM_PLAYERS];

        set_players_equities(players_equities, &game->players, community_cards_copy.cards);

        for (uint8_t i = 0; i < MIN_NUM_PLAYERS; ++i) {
            wins_distribution[i] += players_equities[i];
        }
    }
    for (uint8_t i = 0; i < MIN_NUM_PLAYERS; ++i) {
        winning_probability_distribution[i] = wins_distribution[i] / iters;
    }
}

void simulate_game() {

    Game game = { .deck = get_shuffled_deck(&original_unshuffled_standard_deck), .players = init_players(), .burned_cards = init_burned_cards(), .community_cards = init_community_cards() };
    
    deal_hole_cards(game.players.players, &game.deck);

    calculate_winning_probability_distribution(&game);
    display_table(&game.players, &game.community_cards);

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



HoleCards get_hole_cards_from_input() {

    char hole_cards[8];

    printf("\nEnter your hole cards: ");

    fgets(hole_cards, sizeof(hole_cards), stdin);

    printf("%s.", hole_cards);
    
    HoleCards res = init_hole_cards();

    return res;
}

CommunityCards get_community_cards_from_input() {
    CommunityCards res = init_community_cards();
    return res;
}


void tool() {
    HoleCards users_hole_cards = get_hole_cards_from_input();
    printf("\nNumber of user's hole cards: %d\n", users_hole_cards.count);
    printf("User's hole card 0 rank: %d\n", users_hole_cards.cards[0].rank);
    printf("User's hole card 0 suit: %d\n", users_hole_cards.cards[0].suit);
    printf("User's hole card 1 rank: %d\n", users_hole_cards.cards[1].rank);
    printf("User's hole card 1 suit: %d\n", users_hole_cards.cards[1].suit);
}


int main() {

    init();

    tool();

    return 0;
}
