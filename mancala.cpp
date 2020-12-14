#include <stdio.h>
#include <string.h>
#include <array>

#define NUM_HOUSES   6
#define TOTAL_HOUSES (2*(NUM_HOUSES+1))
#define INIT_COUNT   4
#define INF          999999999

class Game {
public:
    typedef int Count;
    typedef int Pindex;
    enum class Player { A, B, None };
    enum class PlayOutcome { INVALID, OK, AGAIN };
    enum { NoMove = -1 };

    Game() {
        for (Player player = Player::A; player != Player::None; player = next_player(player)) {
            house_[home_store(player)] = 0;
            for (Pindex px = 0; px < NUM_HOUSES; ++px)
                house_[house_index(player, px)] = INIT_COUNT;
        }
    }

    Game(Game const& game) : house_(game.house_) {
    }

    PlayOutcome play(Player player, Pindex px) {
        if (px >= NUM_HOUSES)
            return PlayOutcome::INVALID;
        size_t index = house_index(player, px);
        Count stones = house_[index];
        if (stones == 0)
            return PlayOutcome::INVALID;
        house_[index] = 0;
        while (stones-- > 0) {
            index = next_house(player, index);
            house_[index] += 1; // sow
        }
        if (index == home_store(player))
            return PlayOutcome::AGAIN;
        if (house_[index] == 1 && side(index) == player) {
            size_t oindex = opposite(index);
            house_[home_store(player)] += house_[oindex] + 1;
            house_[oindex] = house_[index] = 0;
        }
        return PlayOutcome::OK;
    }

    bool game_over() {
        for (Player player = Player::A; player != Player::None; player = next_player(player)) {
            if (side_empty(player)) {
                Player oplayer = other_player(player);
                for (Pindex px = 0; px < NUM_HOUSES; ++px) {
                    size_t index = house_index(oplayer, px);
                    house_[home_store(oplayer)] += house_[index];
                    house_[index] = 0;
                }
                return true;
            }
        }
        return false;
    }

    Count score(Player player) const {
        return house_[home_store(player)];
    }

    void print() const {
        printf("(%2d) ", house_[home_store(Player::B)]);
        for (unsigned px = NUM_HOUSES; px > 0; )
            printf("%2d ", house_[house_index(Player::B, --px)]);
        printf("\n     ");
        for (unsigned px = 0; px < NUM_HOUSES; )
            printf("%2d ", house_[house_index(Player::A, px++)]);
        printf("(%2d)\n", house_[home_store(Player::A)]);
    }

    static Player other_player(Player player) {
        return (player == Player::A) ? Player::B : Player::A;
    }

protected:

    static Player next_player(Player player) {
        switch (player) {
        case Player::A: return Player::B;
        default: return Player::None;
        }
    }

    static size_t next_house(Player player, size_t index) {
        index = next_index(index);
        if (index == home_store(Player::A) && player != Player::A)
            index = next_index(index);
        else if (index == home_store(Player::B) && player != Player::B)
            index = next_index(index);
        return index;
    }

    static size_t house_index(Player player, Pindex px) {
        if (player == Player::A)
            return px;
        return px + NUM_HOUSES+1;
    }

    static size_t next_index(size_t index) {
        if (++index >= TOTAL_HOUSES) index = 0;
        return index;
    }

    static size_t home_store(Player player) {
        return (player == Player::A) ? NUM_HOUSES : TOTAL_HOUSES-1;
    }

    static Player side(size_t index) {
        return (index <= NUM_HOUSES) ? Player::A : Player::B;
    }

    bool side_empty(Player player) const {
        for (Pindex px = 0; px < NUM_HOUSES; ++px)
            if (house_[house_index(player, px)] > 0)
                return false;
        return true;
    }

    static size_t opposite(size_t index) {
        return 2*NUM_HOUSES - index;
    }

private:
    std::array<Count, TOTAL_HOUSES> house_;
};

// --------------------------------------------------------------------

struct Move {
    Game::Pindex move;
    Game::Count score;
};

Move best_move(Game const& game, unsigned depth, int alpha, int beta, Game::Player player) {
    bool maxim = (player == Game::Player::A);
    Game::Player oplayer = Game::other_player(player);
    Move best;
    best.move = Game::NoMove;
printf("depth %d, player %d\n", depth, (int)player);
    if (depth == 0) {
        best.score = game.score(player) - game.score(oplayer);
printf("depth 0, score %d-%d\n", game.score(player), game.score(oplayer));
        return best;
    }
    best.score = maxim ? -INF : +INF;
    for (Game::Pindex px = 0; px < NUM_HOUSES; ++px) {
        Game game2 (game);
        Game::PlayOutcome outcome = game2.play(player, px);
printf("play %d: outcome %d\n", px, (int)outcome);
if (outcome == Game::PlayOutcome::AGAIN) outcome = Game::PlayOutcome::OK;
        if (outcome == Game::PlayOutcome::OK) {
            Move move = best_move(game2, depth-1, alpha, beta, oplayer);
printf("depth %d, best %d = score %d\n", depth, move.move, move.score);
            if (maxim) {
                if (move.score > best.score) {
printf("A:new best\n");
                    best = move;
                }
                if (best.score > alpha) {
printf("A:alpha %d->%d\n", alpha, best.score);
                    alpha = best.score;
                }
                if (alpha >= beta)
                    break;
            } else { // minim
                if (move.score < best.score) {
printf("B:new best\n");
                    best = move;
                }
                if (best.score < beta) {
printf("B:alpha %d->%d\n", alpha, best.score);
                    beta = best.score;
                }
                if (beta <= alpha)
                    break;
            }
        }
    }
printf("depth %d, return %d = score %d\n", depth, best.move, best.score);
    return best;
}

// --------------------------------------------------------------------

int main() {
    Game game;
    Game::Player turn = Game::Player::A;
    for (;;) {
        game.print();
        printf("%s>", (turn == Game::Player::A) ? "A" : "B");
        char line[80];
        fgets(line, sizeof(line), stdin);
        if (line[0] == 'q') break;
        if (strncmp(line, "best", 4) == 0) {
            Move move = best_move(game, 2, -INF, +INF, turn);
            printf("%d\n", move.move);
            continue;
        }
        Game::Pindex px = (Game::Pindex) atoi(line);
        if (px == 0) {
            printf("Choose a house between 1 and %d\n", NUM_HOUSES);
            continue;
        }
        switch (game.play(turn, px-1)) {
        case Game::PlayOutcome::OK:
            if (game.game_over()) {
                printf("final score %d - %d\n", game.score(Game::Player::A), game.score(Game::Player::B));
                return 0;
            }
            turn = game.other_player(turn);
            break;
        case Game::PlayOutcome::INVALID:
            printf("Invalid move\n");
            break;
        case Game::PlayOutcome::AGAIN:
            printf("Play again\n");
            break;
        }
    }
    return 0;
}
