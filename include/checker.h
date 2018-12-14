//
// Created by scott on 11/1/18.
//

#ifndef THECHECKER_CHECKER_H
#define THECHECKER_CHECKER_H
#include<utility.hpp>
#include<agent.h>
namespace GameSetting {
    class checker {
    public:
        checker(): in_play(false),current_player(0),prev_move_time(duration::zero()),
        total_move_time(duration::zero()), move_count(0), move_since_capture(0){
            this->players[0] = GameSetting::agent(nullptr, false, 0);
            this->players[1] = GameSetting::agent(nullptr, false, 0);
            this->init_game();
        };

        // game control
        void init_game();
        void start_game(bool, bool, unsigned short, double);
        void start_game(unsigned int, game_info);
        void end_game(){
            this->in_play = false;
        }

        //utility
        void load_game(std::string const &);
        void save_game();
        void print_board();


        Board transition_board(move const &);
        Board transition_board(move const &, Board const &);

        Board get_board_in_play() { return this->board_in_play; };
        void set_board_in_play(Board const &board) { this->board_in_play = board; };

        static std::vector <move> get_legal_moves(Board const &, unsigned short);
        static std::string pretty_print_moves (move const &move) {
            std::stringstream movePath;

            for(int i = 0; i < 13; i++) {
                if(move.x_route[i] > 7 || move.y_route[i] > 7 || 1 & (move.x_route[0] + move.y_route[0])) {
                    return movePath.str();
                }
                movePath << (i ? " -> " : "");
                movePath << "( "<< GameSetting::column_names[move.x_route[i]] << " , "<< move.y_route[i] + 1 << " )";
            }

            return movePath.str();
        };

        uint8_t get_current_player () { return this->current_player;}
        time get_move_start_time() { return this->move_start_time; };

    private:
        // game stats
        bool in_play;
        Board board_in_play;

        agent players[2];
        uint8_t current_player;

        unsigned int move_count;
        unsigned int move_since_capture; // 50 step rule
        std::vector <move> paths_list;

        time move_start_time;
        duration prev_move_time;
        duration total_move_time;
    };
}

#endif //THECHECKER_CHECKER_H
