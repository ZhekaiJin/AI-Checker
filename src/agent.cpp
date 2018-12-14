//
// Created by scott on 11/1/18.
//

#include <agent.h>
#include <checker.h>

GameSetting::move GameSetting::agent::play(GameSetting::Board const &board) {
    GameSetting::move move;
    std::vector<GameSetting::move> moves;
    std::string user_input;
    bool moved;
    uint8_t i;
    if(this->is_agent) {     // ai
        moves = this->game->get_legal_moves(board, this->game->get_current_player());
        if(moves.empty()) {
            move.agent_id = this->game->get_current_player();
            move.x_route[0] = move.y_route[0] = 0xFFU;
            moved = true;
            this->game->end_game();
        } else if(moves.size() == 1) {
            move = moves[0];
            this->max_depth_reached = 0;
        } else {
            move = this->alpha_beta_search(board); // pick using alpha beta
        }
    } else { // y ou
        moved = false;
        moves = this->game->get_legal_moves(board, this->game->get_current_player());
        if(moves.empty()) {
            move.agent_id = this->game->get_current_player();
            move.x_route[0] = move.y_route[0] = 0xFFU;
            moved = true;
            this->game->end_game();
        }
        while(!moved) {
            for(i = 0; i < moves.size(); i++) {
                std::cout << "  [" << (i + 1) << "]  " << this->game->pretty_print_moves(moves[i]) << std::endl;
            }
            std::cout << "  [S]  Save" << std::endl;
            std::cout << "  [F]  Forfeit" << std::endl;
            while((std::cout << "Please select a step: ")
                  && (!(std::cin >> user_input)
                      || (   GameSetting::fixed_input_set.count(user_input.c_str()[0]) == 0
                             && ((uint8_t) std::strtoll(user_input.c_str(), nullptr, 10) < 1
                                 || (uint8_t) std::strtoll(user_input.c_str(), nullptr, 10) > moves.size())))) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            if(user_input.c_str()[0] == 'S' || user_input.c_str()[0] == 's') {
                this->game->save_game();
            } else if(user_input.c_str()[0] == 'f' || user_input.c_str()[0] == 'F') {
                move.agent_id = this->game->get_current_player();
                move.x_route[0] = move.y_route[0] = 0xFFU;
                moved = true;
                std::cout << "Forfeiting game ..." << std::endl;
                this->game->end_game();
            } else {
                move = moves[std::strtoll(user_input.c_str(), nullptr, 10) - 1];
                moved = true;
            }
        }
    }
    return move;
}

GameSetting::move GameSetting::agent::alpha_beta_search(GameSetting::Board const &board) {
    //initialize search tree
    uint8_t depth, max_depth = 1;
    GameSetting::tree_node tree_nodes[20];

    // agent variables
    uint8_t players[2];
    players[0] = this->game->get_current_player();
    players[1] = static_cast<uint8_t>((~players[0]) & 1);

    GameSetting::move the_move_to_make = move();
    GameSetting::time time_start = this->game->get_move_start_time();
    GameSetting::duration timeDiff;
    double time_left = this->timeLimit;

    int score, best_move;
    tree_nodes[0].node_board = board;
    tree_nodes[0].node_moves = GameSetting::checker::get_legal_moves(board, players[0]);
    tree_nodes[0].numMoves = static_cast<int>(tree_nodes[0].node_moves.size());
    tree_nodes[0].is_max = true;

    do {
        // clear root
        depth = 0;
        tree_nodes[0].alpha = INT_MIN;
        tree_nodes[0].beta = INT_MAX;
        tree_nodes[0].value = INT_MIN;
        tree_nodes[0].move_index = 0;

        while(tree_nodes[0].move_index < tree_nodes[0].numMoves && time_left > GameSetting::search_stop_time) {
            if(    tree_nodes[depth].beta <= tree_nodes[depth].alpha
                || tree_nodes[depth].move_index >= tree_nodes[depth].numMoves) {
                if(!depth--) {
                    if(tree_nodes[1].value > tree_nodes[0].value) {
                        tree_nodes[0].value = tree_nodes[1].value;
                        best_move = tree_nodes[0].move_index - 1;
                    }
                    if(tree_nodes[0].value > tree_nodes[0].alpha) {
                        tree_nodes[0].alpha = tree_nodes[0].value;
                    }
                    break;
                }
                if(tree_nodes[depth].is_max) {
                    if(tree_nodes[depth + 1].value > tree_nodes[depth].value) {
                        tree_nodes[depth].value = tree_nodes[depth + 1].value;
                        if(!depth) {
                            best_move = tree_nodes[depth].move_index - 1;
                        }
                    }
                    if(tree_nodes[depth].value > tree_nodes[depth].alpha) {
                        tree_nodes[depth].alpha = tree_nodes[depth].value;
                    }
                } else {
                    if(tree_nodes[depth + 1].value < tree_nodes[depth].value) {
                        tree_nodes[depth].value = tree_nodes[depth + 1].value;
                    }
                    if(tree_nodes[depth].value < tree_nodes[depth].beta) {
                        tree_nodes[depth].beta = tree_nodes[depth].value;
                    }
                }
            } else {
                tree_nodes[depth + 1].node_board =
                        this->game->transition_board(tree_nodes[depth].node_moves[tree_nodes[depth].move_index],
                                                     tree_nodes[depth].node_board);
                tree_nodes[depth].move_index++;

                if(depth + 1 < max_depth) { // still doable
                    depth++;
                    tree_nodes[depth].is_max = !tree_nodes[depth - 1].is_max;
                    tree_nodes[depth].value = tree_nodes[depth].is_max? INT_MIN :INT_MAX;
                    tree_nodes[depth].beta = tree_nodes[depth - 1].beta;
                    tree_nodes[depth].alpha = tree_nodes[depth - 1].alpha;

                    tree_nodes[depth].node_moves = GameSetting::checker::get_legal_moves(tree_nodes[depth].node_board,
                                                                                       players[depth & 1]);
                    tree_nodes[depth].numMoves = static_cast<int>(tree_nodes[depth].node_moves.size());
                    tree_nodes[depth].move_index = 0;
                }
                else {
                    score = this->score_board(tree_nodes[depth + 1].node_board);
                    if(tree_nodes[depth].is_max) {
                        if(score > tree_nodes[depth].value) {
                            tree_nodes[depth].value = score;
                            if(!depth) {
                                best_move = tree_nodes[depth].move_index - 1;
                            }
                        }
                        if(tree_nodes[depth].value > tree_nodes[depth].alpha) {
                            tree_nodes[depth].alpha = tree_nodes[depth].value;
                        }
                    } else {
                        if(score < tree_nodes[depth].value) {
                            tree_nodes[depth].value = score;
                        }
                        if(tree_nodes[depth].value < tree_nodes[depth].beta) {
                            tree_nodes[depth].beta = tree_nodes[depth].value;
                        }
                    }
                }
            }

            timeDiff = GameSetting::clock::now() - time_start;
            time_left = this->timeLimit - double(timeDiff.count()) * GameSetting::period::num / GameSetting::period::den;
        }

        // iterative deepening
        // won't start if total time > 2 * current step time + previous time
        if(time_left > GameSetting::search_stop_time) {
            this->max_depth_reached = max_depth++;
            the_move_to_make = tree_nodes[0].node_moves[best_move];
        }
        timeDiff = GameSetting::clock::now() - time_start;
        time_left = this->timeLimit - double(timeDiff.count()) * GameSetting::period::num / GameSetting::period::den;
    } while(time_left > GameSetting::search_stop_time && max_depth < 20);
    // 12 step is cosiderate to be expert
    return the_move_to_make;
}

//heuristic
int GameSetting::agent::score_board(Board const &board) {
    int i, j;
    int score[2] = {0};
    int pieceCount[2] = {0};
    uint8_t the_player = this->game->get_current_player();
    uint8_t the_other_player = static_cast<uint8_t>(~the_player & 1);
    uint8_t square;

    GameSetting::Piece temp{};
    //iterate through all the pieces
    for(i = 0; i < 2; i++) {
        for(j = 0; j < 12; j++) {
            temp = board.pieces[i][j];
            if(temp.cor_x <= 7 && temp.cor_y <= 7) {
                //for_clear_result
                pieceCount[i]++;

                score[i] += 300; //base score

                // King worth 9 where Reg worth 5
                if (temp.king) {
                    score[i] += 1600;
                } else {
                    score[i] += 1000;
                }

                // protect myself: have my piece around
                if(temp.cor_y + (2 * i - 1) >= 0 && temp.cor_y + (2 * i - 1) <= 7 && !temp.king) {
                    if(temp.cor_x - 1 >= 0) {
                        // check squares down right/left
                        square = board.boxes[temp.cor_y + (2 * i - 1)][temp.cor_x - 1];
                        if(square != 4 && (square & 1) == i) {
                            score[i] += 150;
                        }
                    }
                    if(temp.cor_x + 1 <= 7) {
                        // check squares down left/right
                        square = board.boxes[temp.cor_y + (2 * i - 1)][temp.cor_x + 1];
                        if(square != 4 && (square & 1) == i) {
                            score[i] += 150;
                        }
                    }

                    // get closer to my enemy
                    if(temp.cor_y + (2 - 4 * i) >= 0 && temp.cor_y + (2 - 4 * i) <= 7) {
                        square = board.boxes[temp.cor_y + (2 - 4 * i)][temp.cor_x];
                        if(square != 4 && (square & 1) == (~i & 1)) {
                            score[i] += 99;
                        }
                    }
                }

                //try to keep myself at the center
                score[i] -= 30 * (abs(4 - temp.cor_x) + abs(4 - temp.cor_y));

                if(!temp.king) {
                    score[i] += 50 * abs(7 * i - temp.cor_y); //advance to enemy row
                    if(temp.cor_y == 7 * i) {//if the piece is in the back row
                        score[i] += 300;
                    }
                }

            }
        }
    }

    if(!pieceCount[the_player]) {
        return INT_MIN;
    }else if(!pieceCount[the_other_player]) {
        return INT_MAX;
    }else if(GameSetting::checker::get_legal_moves(board, the_player).empty()) {
        return INT_MIN;
    }else if(GameSetting::checker::get_legal_moves(board, the_other_player).empty()) {
        return INT_MAX;
    }
    //return zero-sum score
    return score[the_player] - score[the_other_player];
}
