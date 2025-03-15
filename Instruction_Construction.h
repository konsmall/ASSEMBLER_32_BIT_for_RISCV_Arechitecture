// Assembler_Verilog_CPU_32.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <bitset>
#include <stdlib.h>


/*
*   Create a 32bit R type instruction using the ASSEMBLY COMMANDS and the charecteristic OPCODE
*
*   std::vector< std::string >& line - String vector containing the ASSEMBLY strings
*   std::string opcode - Characteristic OPCODE for differentiating between all R instructions
*/
std::string R_Instruction_Constructor(std::vector< std::string >& line, std::string opcode) {
    std::string command = "";

    command += "0000000";
    command += std::bitset< 5 >(stoi(line[3].substr(1))).to_string();
    command += std::bitset< 5 >(stoi(line[2].substr(1))).to_string();
    command += "000";
    command += std::bitset< 5 >(stoi(line[1].substr(1))).to_string();
    command += opcode;

    return command;
}

/*
*   Create a 32bit I type instruction using the ASSEMBLY COMMANDS and the charecteristic OPCODE
*
*   std::vector< std::string >& line - String vector containing the ASSEMBLY strings
*   std::string opcode - Characteristic OPCODE for differentiating between all I instructions
*/
std::string I_Instruction_Constructor(std::vector< std::string >& line, std::string opcode) {
    std::string command = "";

    command += std::bitset< 12 >(stoi(line[3].substr(1))).to_string();
    command += std::bitset< 5 >(stoi(line[2].substr(1))).to_string();
    command += "000";
    command += std::bitset< 5 >(stoi(line[1].substr(1))).to_string();
    command += opcode;

    return command;
}

/*
*   Create a 32bit S type instruction using the ASSEMBLY COMMANDS and the charecteristic OPCODE
*
*   std::vector< std::string >& line - String vector containing the ASSEMBLY strings
*   std::string opcode - Characteristic OPCODE for differentiating between all S instructions
*/
std::string S_Instruction_Constructor(std::vector< std::string >& line, std::string opcode) {
    std::string command = "";

    std::string imm12 = std::bitset< 12 >(stoi(line[3].substr(1))).to_string();

    command += imm12.substr(0, 7);
    command += std::bitset< 5 >(stoi(line[2].substr(1))).to_string();
    command += std::bitset< 5 >(stoi(line[1].substr(1))).to_string();
    command += "000";
    command += imm12.substr(7, 5);
    command += opcode;

    return command;
}

/*
*   Create a 32bit U type instruction using the ASSEMBLY COMMANDS and the charecteristic OPCODE
*
*   std::vector< std::string >& line - String vector containing the ASSEMBLY strings
*   std::string opcode - Characteristic OPCODE for differentiating between all U instructions
*/
std::string U_Instruction_Constructor(std::vector< std::string >& line, std::string opcode) {
    std::string command = "";

    std::string imm20 = std::bitset< 20 >(stoi(line[2].substr(1))).to_string();

    command += imm20;
    command += std::bitset< 5 >(stoi(line[1].substr(1))).to_string();
    command += opcode;

    return command;
}

/*
*   Create a 32bit SB type instruction using the ASSEMBLY COMMANDS and the charecteristic OPCODE
*   used mainly by branching commands, thus it requires more data to calculate the distance
*   between the current and target command
*
*   std::vector< std::string >& line - String vector containing the ASSEMBLY strings
*   std::string opcode - Characteristic OPCODE for differentiating between all SB instructions
*   uint& command_pos - Stores the position of the command the program will jump to
*   std::vector< std::vector< std::string > >& TAGS - Used to calculate the position of the current command
*   int& JUMP_REF - Offset variable in case the commands donst start from memory cell 0
*/
std::string SB_Instruction_Constructor(std::vector< std::string >& line, std::string opcode, uint& command_pos, std::vector< std::vector< std::string > >& TAGS, int& JUMP_REF) {
    std::string command = "";

    uint tag_index = -1;
    for (uint i = 0; i < TAGS.size(); i++) {
        auto& tag_line = TAGS[i];
        auto it = find(tag_line.begin(), tag_line.end(), line[3]);
        if (it != tag_line.end()) {
            tag_index = i;
            break;
        }
    }
    int distance = (tag_index - command_pos) * 4 + JUMP_REF;
    if (line[0] == "END")  distance = 0;
    std::string imm12 = std::bitset< 12 >(distance).to_string();

    command += imm12.substr(0, 7);
    command += std::bitset< 5 >(stoi(line[2].substr(1))).to_string();
    command += std::bitset< 5 >(stoi(line[1].substr(1))).to_string();
    command += "000";
    command += imm12.substr(7, 5);
    command += opcode;

    return command;
}

/*
*   Create a 32bit UJ type instruction using the ASSEMBLY COMMANDS and the charecteristic OPCODE
*   used mainly by branching commands, thus it requires more data to calculate the distance
*   between the current and target command
*
*   std::vector< std::string >& line - String vector containing the ASSEMBLY strings
*   std::string opcode - Characteristic OPCODE for differentiating between all UJ instructions
*   uint& command_pos - Stores the position of the command the program will jump to
*   std::vector< std::vector< std::string > >& TAGS - Used to calculate the position of the current command
*   int& JUMP_REF - Offset variable in case the commands donst start from memory cell 0
*/
std::string UJ_Instruction_Constructor(std::vector< std::string >& line, std::string opcode, uint& command_pos, std::vector< std::vector< std::string > >& TAGS, int& JUMP_REF) {
    std::string command = "";

    uint tag_index = -1;
    for (uint i = 0; i < TAGS.size(); i++) {
        auto& tag_line = TAGS[i];
        auto it = find(tag_line.begin(), tag_line.end(), line[2]);
        if (it != tag_line.end()) {
            tag_index = i;
            break;
        }
    }
    int distance = (tag_index - command_pos) * 4 + JUMP_REF;
    std::string imm20 = std::bitset< 20 >(distance).to_string();

    command += imm20;
    command += std::bitset< 5 >(stoi(line[1].substr(1))).to_string();
    command += opcode;

    return command;
}