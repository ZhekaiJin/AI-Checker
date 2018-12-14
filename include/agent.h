//
// Created by scott on 11/1/18.
//

#ifndef THECHECKER_AGENT_H
#define THECHECKER_AGENT_H

#include<utility.hpp>

namespace GameSetting {

    class checker;

    class agent {
    public:
        agent(): game(nullptr), is_agent(false), previousMoveTime(duration::zero()),
                 totalMoveTime(duration::zero()), timeLimit(0), max_depth_reached(0) {};

        agent(checker* game, bool is_agent, double timeLimit) : game(game), is_agent(is_agent),
                                                               previousMoveTime(duration::zero()),
                                                               totalMoveTime(duration::zero()),
                                                               timeLimit(timeLimit),
                                                               max_depth_reached(0) {};
        move play(Board const &);
        void append_move(move const &move) {
            this->path_list.push_back(move);
        };
        int get_max_depth() {
            return is_agent ? this->max_depth_reached : -1;
        };

    private:
        friend class checker;
        checker *game;
        std::vector <move> path_list;

        //play helper function
        move alpha_beta_search(Board const &);
        int score_board(Board const &);

        //timing
        double timeLimit;
        duration totalMoveTime;
        duration previousMoveTime;

        // time functions
        void add_total_move_time(duration const &moveTime) {
            this->totalMoveTime += moveTime;
        }

        void set_prev_move_time(duration const &moveTime) {
            this->previousMoveTime = moveTime;
        }

        duration get_total_move_time() {
            return this->totalMoveTime;
        };

        duration get_prev_move_time() {
            return this->previousMoveTime;
        };

        //agent_info
        bool is_agent;
        int max_depth_reached;
    };
}



#endif //THECHECKER_AGENT_H
