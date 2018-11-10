//
// Created by scott on 11/1/18.
//

#ifndef CHECKERS_UTILITY_H
#define CHECKERS_UTILITY_H

#include <climits>
#include <chrono>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <termcolor.hpp>
#include <iomanip>
#include <rang.hpp>

namespace GameSetting {

    static bool loadsaved = false;
    static std::string path_to_load;

    static unsigned int lower_time_limit = 5;
    static unsigned int upper_time_limit = 360; //"6 min rule"
    static unsigned int upper_step_limit = 50; // refer to 50-steps rule
    static double search_stop_time = 0.1;

    static char symbols[5] = {
            '&',  // Player 0 Regular piece
            'O',  // Player 1 Regular piece
            'K',  // Player 0 Regular piece
            'Q',  // Player 1 King piece
            ' '   // Unoccupied
    };
    static std::set<char> fixed_input_set{
            'f', 'F', 'S','s'
    };

    static char column_names[8] = {
            'A',
            'B',
            'C',
            'D',
            'E',
            'F',
            'G',
            'H'
    };




    //basic data structure will be used
    class Piece {
    public:
        bool king;
        uint8_t cor_x;
        uint8_t cor_y;
    };

    class Board {
    public:
        uint8_t boxes[8][8];
        Piece pieces[2][12];
    };

    // Move type definition
    typedef struct {
        uint8_t agent_id;
        uint8_t x_route[13];
        uint8_t y_route[13];
    } move;

    typedef struct {
        unsigned short first_to_play;
        double agent_time_limit;
    } gameinfo;

    typedef std::chrono::steady_clock::time_point time;
    typedef std::chrono::steady_clock clock;
    typedef std::chrono::steady_clock::duration duration;
};


#endif //CHECKERS_UTILITY_H
