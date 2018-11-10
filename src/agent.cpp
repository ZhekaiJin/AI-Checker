//
// Created by scott on 11/1/18.
//

#include <agent.h>
#include <checker.h>

GameSetting::move GameSetting::agent::play(GameSetting::Board const &board) {
    GameSetting::move move;
    std::vector<GameSetting::move> move_list;
    std::string user_input;
    std::string savedGameFilePath;
    bool moved;
    uint8_t i;

    // ai
    if(this->is_agent) {
        move_list = this->game->get_legal_moves(board, this->game->get_current_player());

        if(!move_list.size()) {
            move.agent_id = this->game->get_current_player();
            move.x_route[0] = move.y_route[0] = 0xFFU;
            moved = true;
            this->game->end_game();
        }

        else if(move_list.size() == 1) {
            move = move_list[0];
            this->max_depth_reached = 0;
        }

        else {
            move = this->alpha_beta_search(board); // pick using alpha beta
        }
    }

        // you
    else {
        moved = false;

        move_list = this->game->get_legal_moves(board, this->game->get_current_player());

        if(!move_list.size()) {
            move.agent_id = this->game->get_current_player();
            move.x_route[0] = move.y_route[0] = 0xFFU;
            moved = true;
            this->game->end_game();
        }

        while(!moved) {
            for(i = 0; i < move_list.size(); i++) {
                std::cout << "  [" << (i + 1) << "]  " << this->game->pretty_print_moves(move_list[i]) << std::endl;
            }
            std::cout << "  [S]  Save" << std::endl;
            std::cout << "  [F]  Forfeit" << std::endl;

            while((std::cout << "Please select an action: ")
                  && (!(std::cin >> user_input)
                      || (   GameSetting::fixed_input_set.count(user_input.c_str()[0]) == 0
                             && ((uint8_t) std::strtoll(user_input.c_str(), nullptr, 10) < 1
                                 || (uint8_t) std::strtoll(user_input.c_str(), nullptr, 10) > move_list.size())))) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }

            if(user_input.c_str()[0] == 'S' || user_input.c_str()[0] == 's') {
                std::cout << "Please enter a saved game file path: ";
                std::cin >> savedGameFilePath;
                this->game->save_game(savedGameFilePath);
            } else if(user_input.c_str()[0] == 'f' || user_input.c_str()[0] == 'F') {
                move.agent_id = this->game->get_current_player();
                move.x_route[0] = move.y_route[0] = 0xFFU;
                moved = true;
                std::cout << "Quitting game ..." << std::endl;
                this->game->end_game();
            } else {
                move = move_list[std::strtoll(user_input.c_str(), nullptr, 10) - 1];
                moved = true;
            }
        }
    }

    return move;
}

GameSetting::move GameSetting::agent::alpha_beta_search(GameSetting::Board const &board) {
    // the tree variables
    uint8_t depth, maxDepthReached = 1;
    struct {
        bool isMaxNode;
        int alpha;
        int beta;
        int value;
        GameSetting::Board nodeBoard;
        std::vector<GameSetting::move> nodeMoves;
        int moveIterator;
        int numMoves;
    } tree_node[50];

    // agent variables
    uint8_t players[2];
    players[0] = this->game->get_current_player();
    players[1] = (~players[0]) & 1;

    GameSetting::move the_move_to_make;
    GameSetting::time timeInitial = this->game->get_move_start_time();
    GameSetting::duration timeDiff;
    double time_left = this->timeLimit;

    int score, best_move;
    tree_node[0].nodeBoard = board;
    tree_node[0].nodeMoves = GameSetting::checker::get_legal_moves(board, players[0]);
    tree_node[0].numMoves = tree_node[0].nodeMoves.size();
    tree_node[0].isMaxNode = true;

    do {
        depth = 0;
        tree_node[0].alpha = INT_MIN;
        tree_node[0].beta = INT_MAX;
        tree_node[0].value = INT_MIN;
        tree_node[0].moveIterator = 0;

        while(tree_node[0].moveIterator < tree_node[0].numMoves && time_left > GameSetting::search_stop_time) {
            if(    tree_node[depth].beta <= tree_node[depth].alpha
                || tree_node[depth].moveIterator >= tree_node[depth].numMoves) {

                if(!depth--) {
                    if(tree_node[1].value > tree_node[0].value) {
                        tree_node[0].value = tree_node[1].value;
                        best_move = tree_node[0].moveIterator - 1;
                    }
                    if(tree_node[0].value > tree_node[0].alpha) {
                        tree_node[0].alpha = tree_node[0].value;
                    }
                    break;
                }
                if(tree_node[depth].isMaxNode) {
                    if(tree_node[depth + 1].value > tree_node[depth].value) {
                        tree_node[depth].value = tree_node[depth + 1].value;
                        if(!depth) {
                            best_move = tree_node[depth].moveIterator - 1;
                        }
                    }
                    if(tree_node[depth].value > tree_node[depth].alpha) {
                        tree_node[depth].alpha = tree_node[depth].value;
                    }
                }
                else {
                    if(tree_node[depth + 1].value < tree_node[depth].value) {
                        tree_node[depth].value = tree_node[depth + 1].value;
                    }

                    if(tree_node[depth].value < tree_node[depth].beta) {
                        tree_node[depth].beta = tree_node[depth].value;
                    }
                }
            }

            else {
                tree_node[depth + 1].nodeBoard =
                        this->game->transition_board(tree_node[depth].nodeMoves[tree_node[depth].moveIterator],
                                                     tree_node[depth].nodeBoard);
                tree_node[depth].moveIterator++;

                if(depth + 1 < maxDepthReached) {
                    // go one level deeper
                    depth++;
                    tree_node[depth].isMaxNode = !tree_node[depth - 1].isMaxNode;
                    tree_node[depth].value = tree_node[depth].isMaxNode? INT_MIN :INT_MAX;
                    tree_node[depth].beta = tree_node[depth - 1].beta;
                    tree_node[depth].alpha = tree_node[depth - 1].alpha;

                    tree_node[depth].nodeMoves = GameSetting::checker::get_legal_moves(tree_node[depth].nodeBoard,
                                                                                       players[depth & 1]);
                    tree_node[depth].numMoves = tree_node[depth].nodeMoves.size();

                    tree_node[depth].moveIterator = 0;
                }
                else {
                    score = this->score_board(tree_node[depth + 1].nodeBoard);

                    if(tree_node[depth].isMaxNode) {
                        if(score > tree_node[depth].value) {
                            tree_node[depth].value = score;
                            if(!depth) {
                                best_move = tree_node[depth].moveIterator - 1;
                            }
                        }

                        if(tree_node[depth].value > tree_node[depth].alpha) {
                            tree_node[depth].alpha = tree_node[depth].value;
                        }
                    }

                    else {
                        if(score < tree_node[depth].value) {
                            tree_node[depth].value = score;
                        }

                        if(tree_node[depth].value < tree_node[depth].beta) {
                            tree_node[depth].beta = tree_node[depth].value;
                        }
                    }
                }
            }

            timeDiff = GameSetting::clock::now() - timeInitial;
            time_left = this->timeLimit - double(timeDiff.count()) * GameSetting::clock::period::num / GameSetting::clock::period::den;
        }

        // iterative deeping
        if(time_left > GameSetting::search_stop_time) {
            this->max_depth_reached = maxDepthReached++;
            the_move_to_make = tree_node[0].nodeMoves[best_move];
        }
        timeDiff = GameSetting::clock::now() - timeInitial;
        time_left = this->timeLimit - double(timeDiff.count()) * GameSetting::clock::period::num / GameSetting::clock::period::den;
    } while(time_left > GameSetting::search_stop_time && maxDepthReached < 50);

    return the_move_to_make;
}

//heuristic
int GameSetting::agent::score_board(Board const &board) {
    int i, j;
    int score[2] = {0};
    int pieceCount[2] = {0};
    uint8_t the_player = this->game->get_current_player();
    uint8_t the_other_player = ~the_player & 1;
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
    }else if(GameSetting::checker::get_legal_moves(board, the_player).size() == 0) {
        return INT_MIN;
    }else if(GameSetting::checker::get_legal_moves(board, the_other_player).size() == 0) {
        return INT_MAX;
    }
    //return zero-sum score
    return score[the_player] - score[the_other_player];
}
