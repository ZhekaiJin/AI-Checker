//
// Created by scott on 11/1/18.
//

#include "checker.h"

void GameSetting::checker::init_game () {
    //initialzie all variable
    GameSetting::Board board{};
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    this->move_count = 0;
    this->move_since_capture = 0;
    this->prev_move_time = duration::zero();
    this->total_move_time = duration::zero();

    int m = 0, n= 0;
    for(u_int8_t i = 0; i < 8; i++) {
        for(uint8_t j = 0; j < 8; j++) {
            if(!((i + j) & 1) && i < 3) {
                board.boxes[i][j] = 0;
                board.pieces[0][m].king = false;
                board.pieces[0][m].cor_x = j;
                board.pieces[0][m++].cor_y = i;
            } else if(!((i + j) & 1) && i > 4) {
                board.boxes[i][j] = 1;
                board.pieces[1][n].king = false;
                board.pieces[1][n].cor_x = j;
                board.pieces[1][n++].cor_y = i;
            } else {
                board.boxes[i][j] = 4;
            }
        }
    }
    this->set_board_in_play(board);
}
void GameSetting::checker::start_game (unsigned int game_mode_chosen, game_info game_info) {
    /**
    [1]	Agent vs Human player, Human player in red
    [2]	Agent vs Human player, Human player in white
    [3]	Agent vs Agent
    [4]	Human vs Human
    */
    unsigned int first_to_play = game_info.first_to_play;
    unsigned int agent_time_limit = game_info.agent_time_limit;

    if (game_mode_chosen == 1) {
        start_game(false, true, first_to_play, agent_time_limit);
    } else if (game_mode_chosen == 2) {
        start_game(true, false, first_to_play, agent_time_limit);
    } else if (game_mode_chosen == 3) {
        start_game(true, true, first_to_play, agent_time_limit);
    } else if (game_mode_chosen == 4){
        start_game(false, false, first_to_play, agent_time_limit);
    } else {
        std::cerr << "Something is not right" << std::endl;
    }
}
void GameSetting::checker::start_game (bool is_one_agent, bool is_two_agent, unsigned short fist_to_play,
                                      double time_limit) {
    GameSetting::Board temp_board;
    GameSetting::move move;
    GameSetting::time ti, tf;
    GameSetting::duration moveDuration;

    // initialize the players using given parameters
    this->players[0] = GameSetting::agent(this, is_one_agent, time_limit);
    this->players[1] = GameSetting::agent(this, is_two_agent, time_limit);

    // set up game state variables
    this->in_play = true;
    this->current_player = fist_to_play;

    while(this->in_play) {
        // print the board
        this->print_board();
        std::cout << "player " << (this->current_player + 1) << " 's turn ..." << std::endl;

        // get the current board
        temp_board = this->get_board_in_play();

        // give the board to the player to make their move
        ti = this->move_start_time = GameSetting::clock::now();
        move = this->players[this->current_player].play(temp_board);
        tf = GameSetting::clock::now();

        // determine the time it took to make the move
        moveDuration = tf - ti;

        // record move time for game
        this->prev_move_time = moveDuration;
        this->total_move_time += moveDuration;

        this->players[this->current_player].add_total_move_time(moveDuration);
        this->players[this->current_player].set_prev_move_time(moveDuration);
        this->paths_list.push_back(move);
        this->players[this->current_player].append_move(move);

        // board_info
        this->move_count++;
        this->move_since_capture++;

        temp_board = GameSetting::checker::transition_board(move);
        this->set_board_in_play(temp_board);

        this->current_player = (~this->current_player) & 1;

        if(this->move_since_capture > GameSetting::upper_step_limit) {
            this->end_game();
        }
    }
    if((move.x_route[0] > 7 || move.y_route[0] > 7) && this->move_since_capture <= GameSetting::upper_step_limit) {
        std::cout << "Game over! player " << (this->current_player + 1) << " wins!" << std::endl;
    }
    else {
        std::cout << "Game over! It's a draw!" << std::endl;
    }

}
void GameSetting::checker::load_game (std::string const &filePath) {
    std::ifstream inputFile(filePath.c_str());
    int i, j;
    int k[2] = {0};
    int square;
    uint8_t player;

    if(!inputFile.is_open()) {
        std::cout << "Warning: Game could not be loaded!" << std::endl;
        std::cout << "         Failed to open '" << filePath << "' for reading." << std::endl;
        exit(-1);
    } else {
        GameSetting::Board board{};

        // initialize the board to "zero'd out" state
        for(i = 0; i < 8; i++) {
            for(j = 0; j < 8; j++) {
                board.boxes[i][j] = 4;
            }
        }
        for(i = 0; i < 2; i++) {
            for(j = 0; j < 12; j++) {
                board.pieces[i][j].king = false;
                board.pieces[i][j].cor_x = 0xFFU;
                board.pieces[i][j].cor_y = 0xFFU;
            }
        }

        // read in board and pieces
        for(i = 7; i >= 0; i--) {
            for(j = 0; j < 8; j++) {
                inputFile >> square;
                board.boxes[i][j] = static_cast<uint8_t>(square);

                if(square < 4) {
                    player = static_cast<uint8_t>(square & 1);
                    board.pieces[player][k[player]].king = static_cast<bool>(square & 2);
                    board.pieces[player][k[player]].cor_x = static_cast<uint8_t>(j);
                    board.pieces[player][k[player]].cor_y = static_cast<uint8_t>(i);
                    k[player]++;
                }
            }
        }

        inputFile.close();
        std::cout << "Game successfully loaded from '" << filePath << "'!" << std::endl;
        this->set_board_in_play(board);
    }
}
void GameSetting::checker::save_game () {
    std::string filePath;
    std::cout << "Please enter a saved game file path: ";
    std::cin >> filePath;
    std::ofstream outputFile(filePath.c_str());
    GameSetting::Board board = this->get_board_in_play();

    if(!outputFile.is_open()) {
        std::cout << "Warning: Game could not be saved!" << std::endl;
        std::cout << "         Failed to open '" << filePath << "' for writing." << std::endl;
    } else {
        // write out board
        for(int i = 7; i >= 0; i--) {
            for(int j = 0; j < 8; j++) {
                outputFile << int(board.boxes[i][j]) << (j < 7 ? " " : "");
            }
            outputFile << std::endl;
        }

        outputFile.close();
        std::cout << "Game successfully saved to '" << filePath << "'!" << std::endl;
    }
}
void GameSetting::checker::print_board () {
    GameSetting::Board board = this->get_board_in_play();
    int i, j;
    int numReg[2] = {0};
    int numKing[2] = {0};

    for(i = 0; i < 2; i++) {
        for(j = 0; j < 12; j++) {
            if(board.pieces[i][j].cor_x <= 7 && board.pieces[i][j].cor_y <= 7) {
                if(board.pieces[i][j].king) {
                    numKing[i]++;
                } else {
                    numReg[i]++;
                }
            }
        }
    }

    std::cout << std::endl;

    std::cout << std::setw(40);
    for(i = 0; i < 8; i++) {
        std::cout << "   " << rang::bgB::black << rang::style::bold << rang::fgB::gray <<
                  GameSetting::column_names[i] <<
                  rang::bg::reset << rang::style::reset << rang::fg::reset << "   ";
    }
    std::cout << std::endl << std::endl;

    for(i = 7; i >= 0; i--) {
        // first row
        if (i == 1) {
            printf ("%-*.*s", 37, 37, " Agent 1 Info");
        } else if (i == 0) {
            double move_time = ((double) this->players[0].get_total_move_time().count()) * GameSetting::period::num / GameSetting::period::den;
            printf ("%.*s", 30, "  * Total move time:");
            std::cout << std::setprecision(2) << std::setw(7) << std::fixed << move_time << "s";
            std::cout << std::setw(9) << "";
        } else if (i == 7) {
            printf ("%-*.*s", 37, 37, " Board Info");
        } else if (i == 6) {
            printf ("%-*.*s %-*.*d", 0, 36, "  * Total move count:",3, 3, this->move_count);
            std::cout << std::setw(12) << "";
        } else {
            printf ("%-*.*s", 37, 37, "");
        }
        for(int j = 0; j < 8; j++) {
            if((j + i) & 1) {
                std::cout << termcolor::on_white << "       ";
            } else {
                std::cout << termcolor::on_green << "       ";
            }
        }
        std::cout << termcolor::reset;
        std::cout << " ";
        if(i == 7) {
            std::cout << "Agent 2 Info";
        } else if(i == 6) {
            std::cout << " * Total move time: " << (((double) this->players[1].get_total_move_time().count()) * GameSetting::period::num / GameSetting::period::den) << "s";
        }
        std::cout << std::endl;

        // second row
        if(i == 1) {
            std::cout << "  * Regular piece [" << termcolor::red << termcolor::on_yellow
                      << GameSetting::symbols[0] << termcolor::reset << "]:" << std::setw(10) << std::left << numReg[0];
        } else if(i == 0) {
            double prev_time = ((double) this->players[0].get_prev_move_time().count()) * GameSetting::period::num / GameSetting::period::den;
            printf ("%-*.*s", 0, 25, "  * Previous move time:");
            std::cout << std::setprecision(2) << std::setw(7) << std::fixed << prev_time << "s";
            std::cout << std::setw(1) << "";
        } else if(i == 7) {
            double total_time = ((double) this->total_move_time.count()) * GameSetting::period::num / GameSetting::period::den;
            printf ("%-*.*s", 0, 24, "  * Total move time:");
            std::cout << std::setprecision(2) << std::setw(7) << std::fixed << total_time << "s";
            std::cout << std::setw(4) << "";
        } else {
            printf ("%-*.*s", 32, 32, "");
        }

        std::cout << " " << rang::bgB::black <<
                  termcolor::bold << (i + 1) <<
                  rang::bg::reset  << termcolor::reset << "   ";

        for(j = 0; j < 8; j++) {
            if((j + i) & 1) {
                std::cout << termcolor::on_white << "   ";
            } else {
                std::cout << termcolor::on_green << "   ";
            }

            if(board.boxes[i][j] & 1) {
                std::cout << termcolor::white << GameSetting::symbols[board.boxes[i][j]];
            } else {
                std::cout << termcolor::red << GameSetting::symbols[board.boxes[i][j]];
            }

            std::cout << "   ";
        }

        std::cout << termcolor::reset;
        if(i == 7) {
            std::cout << "  * Regular piece [" <<  termcolor::white << termcolor::on_blue
                      << GameSetting::symbols[1] << termcolor::reset << "]: " << numReg[1];
        } else if(i == 6) {
            std::cout << "  * Previous move time : " << (((double) this->players[1].get_prev_move_time().count()) * GameSetting::period::num / GameSetting::period::den) << "s";
        }
        std::cout << std::endl;

        // Third row
        if (i == 1) {
            std::cout << "  * King piece [" << termcolor::red << termcolor::on_yellow
                      << GameSetting::symbols[2] << termcolor::reset << "]: " << numKing[0] << std::setw(16) << "";
        } else if(i == 0 && this->players[0].get_max_depth() >= 0) {
            printf ("%-*.*s %-*.*d", 0, 34, "  * Alpha-beta max depth : ",2, 2, this->players[0].get_max_depth());
            std::cout << std::setw(7) << "";
        } else if(i == 7) {
            double prev_time = ((double) this->prev_move_time.count()) * GameSetting::period::num / GameSetting::period::den;
            printf ("%-*.*s", 0, 30, "  * Previous move time:");
            std::cout << std::setprecision(2) << std::setw(7) << std::fixed << prev_time << "s";
            std::cout << std::setw(6) << "";
        } else {
            printf ("%-*.*s", 37, 37, "");
        }

        for(j = 0; j < 8; ++j) {
            if((j + i) & 1) {
                std::cout << termcolor::on_white << "       ";
            } else {
                std::cout << termcolor::on_green << "       ";
            }
        }
        std::cout << termcolor::reset;

        if(i == 7) {
            std::cout << "  * King piece [" << termcolor::white << termcolor::on_blue
                      << GameSetting::symbols[3] << termcolor::reset << "] : " << numKing[1];
        } else if(i == 6 && this->players[1].get_max_depth() >= 0) {
            std::cout << "  * Alpha-beta max depth : " << this->players[1].get_max_depth();
        }
        std::cout << std::endl;
        std::cout << termcolor::reset;
    }

    std::cout << std::endl;
}

GameSetting::Board GameSetting::checker::transition_board (move const &move) {
    GameSetting::Board board = this->get_board_in_play();
    return GameSetting::checker::transition_board(move, board);
}
GameSetting::Board GameSetting::checker::transition_board (move const &move, Board const &board) {
    GameSetting::Board next_board; GameSetting::move next_move;
    signed short start_box ,jumped_box;
    int x_dist, y_dist, i;
    uint8_t opponent;
    uint8_t x_jump, y_jump;

    // sanity check
    if(move.x_route[0] > 7 || move.y_route[0] > 7 || move.x_route[1] > 7 || move.y_route[1] > 7
       || ((move.x_route[0] + move.y_route[0]) & 1) || ((move.x_route[1] + move.y_route[1]) & 1)
       || board.boxes[move.y_route[1]][move.x_route[1]] != 4) {
        return board;
    } else if(abs(move.x_route[1] - move.x_route[0]) == 1 && abs(move.y_route[1] - move.y_route[0]) == 1) {
        start_box = -1; // normal move
        for(i = 0; i < 12; i++) {
            if(    board.pieces[move.agent_id][i].cor_x == move.x_route[0]
                   && board.pieces[move.agent_id][i].cor_y == move.y_route[0]) {
                start_box = i;
            }
        }
        if(start_box < 0) {
            return board;
        }
        next_board = board;
        if(!board.pieces[move.agent_id][start_box].king && move.y_route[1] == 7 * ((~move.agent_id) & 1)) {
            next_board.pieces[move.agent_id][start_box].king = true;
        }
        next_board.boxes[move.y_route[0]][move.x_route[0]] = 4;
        next_board.boxes[move.y_route[1]][move.x_route[1]] = (int(next_board.pieces[move.agent_id][start_box].king) << 1) | move.agent_id;
        next_board.pieces[move.agent_id][start_box].cor_x = move.x_route[1];
        next_board.pieces[move.agent_id][start_box].cor_y = move.y_route[1];
        return next_board;
    } else if(abs(x_dist = (move.x_route[1] - move.x_route[0])) == 2 && abs(y_dist = (move.y_route[1] - move.y_route[0])) == 2) {
        opponent = (~move.agent_id) & 1;
        start_box = -1;
        for(i = 0; i < 12; i++) {
            if(    board.pieces[move.agent_id][i].cor_x == move.x_route[0]
                   && board.pieces[move.agent_id][i].cor_y == move.y_route[0]) {
                start_box = i;
            }
        }
        if(start_box < 0) {
            return board;
        }
        next_board = board;
        next_move = move;
        do {
            jumped_box = -1;
            x_jump = next_move.x_route[0] + (0 < x_dist) - (x_dist < 0);
            y_jump = next_move.y_route[0] + (0 < y_dist) - (y_dist < 0);
            for(i = 0; i < 12; i++) {
                if(    next_board.pieces[opponent][i].cor_x == x_jump
                       && next_board.pieces[opponent][i].cor_y == y_jump) {
                    jumped_box = i;
                }
            }
            if(jumped_box < 0) {
                return board;
            }
            if(!board.pieces[move.agent_id][start_box].king && next_move.y_route[1] == 7 * opponent) {
                next_board.pieces[move.agent_id][start_box].king = true;
            }
            next_board.boxes[next_move.y_route[0]][next_move.x_route[0]] = 4;
            next_board.boxes[y_jump][x_jump] = 4;
            next_board.boxes[next_move.y_route[1]][next_move.x_route[1]] = (int(next_board.pieces[move.agent_id][start_box].king) << 1) | move.agent_id;
            next_board.pieces[move.agent_id][start_box].cor_x = 0xFFU;
            next_board.pieces[move.agent_id][start_box].cor_y = 0xFFU;
            next_board.pieces[opponent][jumped_box].cor_x = 0xFFU;
            next_board.pieces[opponent][jumped_box].cor_y = 0xFFU;
            next_board.pieces[move.agent_id][start_box].cor_x = next_move.x_route[1];
            next_board.pieces[move.agent_id][start_box].cor_y = next_move.y_route[1];
            if(!board.pieces[move.agent_id][start_box].king && next_board.pieces[move.agent_id][start_box].king) {
                this->move_since_capture = 0;
                return next_board;
            }
            for(i = 0; next_move.x_route[i] >= 0 && next_move.x_route[i] <= 7 && next_move.y_route[i] >= 0 && next_move.y_route[i] <= 7; i++) {
                next_move.x_route[i] = next_move.x_route[i + 1];
                next_move.y_route[i] = next_move.y_route[i + 1];
            }
        } while(!(next_move.x_route[0] > 7 || next_move.y_route[0] > 7 || next_move.x_route[1] > 7 || next_move.y_route[1] > 7
                  || ((next_move.x_route[0] + next_move.y_route[0]) & 1) || ((next_move.x_route[1] + next_move.y_route[1]) & 1)
                  || next_board.boxes[next_move.y_route[1]][next_move.x_route[1]] != 4)
                && (abs(x_dist = (next_move.x_route[1] - next_move.x_route[0])) == 2 && abs(y_dist = (next_move.y_route[1] - next_move.y_route[0])) == 2));
        this->move_since_capture = 0;
        return next_board;
    } else {
        return board;
    }
}

//make routine check a function
std::vector<GameSetting::move> GameSetting::checker::get_legal_moves(GameSetting::Board const &board,
                                                                     unsigned short player) {
    int i, j, k;
    GameSetting::move potential_moves;
    GameSetting::Piece temp;
    std::vector<GameSetting::move> moveList;
    int x_destni[2], y_destni[2];
    struct {
        uint8_t x, y;
        uint8_t checked;
        bool descJump;
    } board_node[13], point;
    bool pieceJumped[8][8] = {0};
    uint8_t depth;
    uint8_t opponent = (~player) & 1;

    potential_moves.agent_id = player;
    for(i = 0; i < 12; i++) {
        temp = board.pieces[player][i];
        potential_moves.x_route[0] = temp.cor_x;
        potential_moves.y_route[0] = temp.cor_y;

        board_node[0].x = temp.cor_x;
        board_node[0].y = temp.cor_y;
        board_node[0].checked = 0;
        depth = 0;

        if(temp.king) {

            while(board_node[0].checked != 0xFU || depth != 0) {
                point = board_node[depth];

                // check top right
                if(!(board_node[depth].checked & 0x8U)) {
                    board_node[depth].checked |= 0x8U;

                    if(    (point.x + 2) >= 0 && (point.x + 2) <= 7 && (point.y + 2) >= 0 && (point.y + 2) <= 7
                           && board.boxes[point.y + 1][point.x + 1] != 4
                           && (board.boxes[point.y + 1][point.x + 1] & 1) == opponent // jump enemy
                           && !pieceJumped[point.y + 1][point.x + 1]
                           && (board.boxes[point.y + 2][point.x + 2] == 4
                               || (point.x + 2 == board_node[0].x && point.y + 2 == board_node[0].y))) {


                        pieceJumped[point.y + 1][point.x + 1] = true; //  jumped
                        depth++;
                        board_node[depth].x = point.x + 2;
                        board_node[depth].y = point.y + 2;
                        board_node[depth].checked = 0;
                        board_node[depth].descJump = false;
                    }
                }

                    // check top left
                else if(!(board_node[depth].checked & 0x4U)) {
                    board_node[depth].checked |= 0x4U;

                    if(    (point.x - 2) >= 0 && (point.x - 2) <= 7 && (point.y + 2) >= 0 && (point.y + 2) <= 7
                           && board.boxes[point.y + 1][point.x - 1] != 4
                           && (board.boxes[point.y + 1][point.x - 1] & 1) == opponent
                           && !pieceJumped[point.y + 1][point.x - 1]
                           && (board.boxes[point.y + 2][point.x - 2] == 4
                               || (point.x - 2 == board_node[0].x && point.y + 2 == board_node[0].y))) {
                        pieceJumped[point.y + 1][point.x - 1] = true; // we have jumped that piece
                        depth++;
                        board_node[depth].x = point.x - 2;
                        board_node[depth].y = point.y + 2;
                        board_node[depth].checked = 0;
                        board_node[depth].descJump = false;
                    }
                }

                    // check bottom left
                else if(!(board_node[depth].checked & 0x2U)) {
                    board_node[depth].checked |= 0x2U;

                    if(    (point.x - 2) >= 0 && (point.x - 2) <= 7 && (point.y - 2) >= 0 && (point.y - 2) <= 7
                           && board.boxes[point.y - 1][point.x - 1] != 4
                           && (board.boxes[point.y - 1][point.x - 1] & 1) == opponent
                           && !pieceJumped[point.y - 1][point.x - 1]
                           && (board.boxes[point.y - 2][point.x - 2] == 4
                               || (point.x - 2 == board_node[0].x && point.y - 2 == board_node[0].y))) {

                        pieceJumped[point.y - 1][point.x - 1] = true; // we have jumped that piece
                        depth++;
                        board_node[depth].x = point.x - 2;
                        board_node[depth].y = point.y - 2;
                        board_node[depth].checked = 0;
                        board_node[depth].descJump = false;
                    }
                }

                    // check bottom right
                else if(!(board_node[depth].checked & 0x1U)) {
                    board_node[depth].checked |= 0x1U;

                    if(    (point.x + 2) >= 0 && (point.x + 2) <= 7 && (point.y - 2) >= 0 && (point.y - 2) <= 7
                           && board.boxes[point.y - 1][point.x + 1] != 4              // jump square not empty
                           && (board.boxes[point.y - 1][point.x + 1] & 1) == opponent // jump square holds enemy piece
                           && !pieceJumped[point.y - 1][point.x + 1]                    // piece wasn't already jumped
                           && (board.boxes[point.y - 2][point.x + 2] == 4             // dest square empty OR
                               || (point.x + 2 == board_node[0].x && point.y - 2 == board_node[0].y))) { // dest square same as start square

                        pieceJumped[point.y - 1][point.x + 1] = true; // we have jumped that piece
                        depth++;
                        board_node[depth].x = point.x + 2;
                        board_node[depth].y = point.y - 2;
                        board_node[depth].checked = 0;
                        board_node[depth].descJump = false;
                    }
                } else {
                    if(!board_node[depth].descJump) {
                        for(j = 1; j <= depth; j++) {
                            potential_moves.x_route[j] = board_node[j].x;
                            potential_moves.y_route[j] = board_node[j].y;
                        }
                        if(j < 13) {
                            potential_moves.x_route[j] = potential_moves.y_route[j] = 0xFFU;
                        }
                        moveList.push_back(potential_moves);
                    }
                    board_node[depth - 1].descJump = true;
                    pieceJumped[(point.y + board_node[depth - 1].y) / 2][(point.x + board_node[depth - 1].x) / 2] = false;
                    depth--;
                }
            }
        } else { //regular
            while(board_node[0].checked != 0xCU || depth != 0) {
                point = board_node[depth];
                if(!(board_node[depth].checked & 0x8U)) {
                    board_node[depth].checked |= 0x8U;

                    if(    (point.x + 2) >= 0 && (point.x + 2) <= 7 && (point.y + (4 * opponent - 2)) >= 0 && (point.y + (4 * opponent - 2)) <= 7 // in range
                           && board.boxes[point.y + (2 * opponent - 1)][point.x + 1] != 4              // jump square not empty
                           && (board.boxes[point.y + (2 * opponent - 1)][point.x + 1] & 1) == opponent // jump square holds enemy piece
                           && !pieceJumped[point.y + (2 * opponent - 1)][point.x + 1]                    // piece wasn't already jumped
                           && board.boxes[point.y + (4 * opponent - 2)][point.x + 2] == 4) {           // dest square empty OR
                        pieceJumped[point.y + (2 * opponent - 1)][point.x + 1] = true;
                        depth++;
                        board_node[depth].x = point.x + 2;
                        board_node[depth].y = point.y + (4 * opponent - 2);
                        board_node[depth].checked = 0;
                        board_node[depth].descJump = false;
                    }
                } else if(!(board_node[depth].checked & 0x4U)) {
                    board_node[depth].checked |= 0x4U;

                    if(    (point.x - 2) >= 0 && (point.x - 2) <= 7 && (point.y + (4 * opponent - 2)) >= 0 && (point.y + (4 * opponent - 2)) <= 7 // in range
                           && board.boxes[point.y + (2 * opponent - 1)][point.x - 1] != 4              // jump square not empty
                           && (board.boxes[point.y + (2 * opponent - 1)][point.x - 1] & 1) == opponent // jump square holds enemy piece
                           && !pieceJumped[point.y + (2 * opponent - 1)][point.x - 1]                    // piece wasn't already jumped
                           && board.boxes[point.y + (4 * opponent - 2)][point.x - 2] == 4) {           // dest square empty OR
                        pieceJumped[point.y + (2 * opponent - 1)][point.x - 1] = true; // we have jumped that piece
                        depth++;
                        board_node[depth].x = point.x - 2;
                        board_node[depth].y = point.y + (4 * opponent - 2);
                        board_node[depth].checked = 0;
                        board_node[depth].descJump = false;
                    }
                } else {
                    if(!board_node[depth].descJump) {
                        for(j = 1; j <= depth; j++) {
                            potential_moves.x_route[j] = board_node[j].x;
                            potential_moves.y_route[j] = board_node[j].y;
                        } if(j < 13) {
                            potential_moves.x_route[j] = potential_moves.y_route[j] = 0xFFU;
                        }
                        moveList.push_back(potential_moves);
                    } board_node[depth - 1].descJump = true;
                    pieceJumped[(point.y + board_node[depth - 1].y) / 2][(point.x + board_node[depth - 1].x) / 2] = false;
                    depth--;
                }
            }
        }
    }
    if(!moveList.size()) {
        for(i = 0; i < 12; i++) {
            temp = board.pieces[player][i];
            potential_moves.x_route[0] = temp.cor_x;
            potential_moves.y_route[0] = temp.cor_y;
            potential_moves.x_route[2] = potential_moves.y_route[2] = 0xFFU;

            x_destni[0] = temp.cor_x - 1;
            x_destni[1] = temp.cor_x + 1;
            y_destni[0] = temp.cor_y - 1;
            y_destni[1] = temp.cor_y + 1;

            if(temp.king) {
                for(j = 0; j < 2; j++) {
                    for(k = 0; k < 2; k++) {
                        if(    x_destni[j] >= 0 && x_destni[j] <= 7
                               && y_destni[k] >= 0 && y_destni[k] <= 7
                               && board.boxes[y_destni[k]][x_destni[j]] & 4) {
                            potential_moves.x_route[1] = x_destni[j];
                            potential_moves.y_route[1] = y_destni[k];
                            moveList.push_back(potential_moves);
                        }
                    }
                }
            } else {
                for(j = 0; j < 2; j++) {
                    if(    x_destni[j] >= 0 && x_destni[j] <= 7
                           && y_destni[opponent] >= 0 && y_destni[opponent] <= 7
                           && board.boxes[y_destni[opponent]][x_destni[j]] & 4) {
                        potential_moves.x_route[1] = x_destni[j];
                        potential_moves.y_route[1] = y_destni[opponent];
                        moveList.push_back(potential_moves);
                    }
                }
            }
        }
    }

    return moveList;
}
