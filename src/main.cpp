#include <cstdlib>
#include <fstream>
#include <iostream>
#include <checker.h>
#include <rang.hpp>
#include "utility.hpp"

int main(int argc, char ** argv) {
    //Handling Prompts//
    std::cout << "  Welcome to the AI checker game. Please answer a few questions before the game." << std::endl;
    unsigned int chosen = 0;
    while (!chosen) {
        std::cout << "  Please choose one option below:" << std::endl;
        std::cout << "\t[1]\tAgent vs Human player, Human player in " <<
                  rang::fg::black << rang::bgB::red << "red";
        std::cout << rang::fg::reset << rang::bg::reset  << std::endl;
        std::cout << "\t[2]\tAgent vs Human player, Human player in " <<
                  rang::fg::black << rang::bgB::gray << "white";
        std::cout << rang::fg::reset << rang::bg::reset  << std::endl;;
        std::cout << "\t[3]\tAgent vs Agent" << std::endl;
        std::cout << "\t[4]\tHuman vs Human" << std::endl;
        std::cout << ">:";
        if (!(std::cin >> chosen) || chosen >= 5 || chosen <= 0) {
            chosen = 0;
            // get rid of failure state
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Invalid Response." << std::endl;
            continue;
        }
        std::cout << "  Option [" << chosen << "] chosen" << std::endl;
    }
    GameSetting::game_info info;
    if(chosen != 4) {
        double agent_time_limit = - 1;
        while((std::cout << "  Please enter a time limit for the agent/agents (in seconds [" << GameSetting::lower_time_limit << ", " << GameSetting::upper_time_limit << "]): ")
              && (!(std::cin >> agent_time_limit) ||agent_time_limit < GameSetting::lower_time_limit ||agent_time_limit > GameSetting::upper_time_limit)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Invalid Response." << std::endl;
        }
        std::cout << "  Time limit set to " <<agent_time_limit << " s."<< std::endl;
        info.agent_time_limit = agent_time_limit;
    }
    unsigned short first_to_play = 0xffff;
    while((std::cout << "  Which Player will make the first move? (1 [red]/ 2[white]): ")
          && (!(std::cin >> first_to_play) || ( first_to_play != 1 &&  first_to_play != 2))) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    info.first_to_play = static_cast<unsigned short>(first_to_play - 1);
    std::cout << "  Player ";
    auto flag = info.first_to_play == 0 ?  rang::bgB::red : rang::bgB::gray;
    std::cout << rang::fgB::black << flag << first_to_play;
    std::cout << rang::fg::reset << rang::bg::reset ;
    std::cout << " set to player first."<< std::endl;
    std::string to_load;
    while((std::cout << "  Would you like to load from a saved game? (y / n): ")
          && (!(std::cin >> to_load) || (to_load != "y" && to_load != "n"))) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    if (to_load == "y") GameSetting::loadsaved = true;
    if(GameSetting::loadsaved) {
        std::cout << "  Please enter the file path: ";
        std::cin >> GameSetting::path_to_load ;
    }
    std::cout << "  Your game will start in a bit "<< std::endl;
    std::cout << std::endl;

    GameSetting::checker the_checker = GameSetting::checker();
    if (GameSetting::loadsaved) {
        the_checker.load_game(GameSetting::path_to_load);
    }
    the_checker.start_game(chosen, info);
    return 0;
}
