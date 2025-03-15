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
*   Write 4 8bit words using a 32bit binary string in to a given file
* 
*   std::ofstream& file - file to write to
*   std::string& command - 32bit binary command string
*   bool& last_command - bool that decides wether \n is added after the last 8bit word
*/
void file_command_write(std::ofstream& file, std::string& command, bool& last_command) {

    file << command.substr(24, 8) << "\n";
    file << command.substr(16, 8) << "\n";
    file << command.substr(8, 8) << "\n";
    if (last_command) file << command.substr(0, 8);
    else file << command.substr(0, 8) << "\n";

}

/*
*   Write 4 8bit words using a 32bit binary string in to a given file
*
*   std::ofstream& file - File to write to
*   std::string& command - 32bit binary command string
*   bool& last_command - Bool that decides wether \n is added after the last 8bit word
*/
void file_word_write(std::ofstream& file, std::string& command, bool& last_command) {

    file << command.substr(24, 8) << "\n";
    file << command.substr(16, 8) << "\n";
    file << command.substr(8, 8) << "\n";
    if (last_command) file << command.substr(0, 8);
    else file << command.substr(0, 8) << "\n";

}

/*
*   Write 2 8bit words using a 16bit binary string in to a given file
*
*   std::ofstream& file - File to write to
*   std::string& command - 16bit binary command string
*   bool& last_command - Bool that decides wether \n is added after the last 8bit word
*/
void file_half_word_write(std::ofstream& file, std::string& command, bool& last_command) {

    file << command.substr(8, 8) << "\n";
    if (last_command) file << command.substr(0, 8);
    else file << command.substr(0, 8) << "\n";

}

/*
*   Write an 8bit word using an 8bit binary string in to a given file
*
*   std::ofstream& file - File to write to
*   std::string& command - 8bit binary command string
*   bool& last_command - Bool that decides wether \n is added after the 8bit word
*/
void file_byte_write(std::ofstream& file, std::string& command, bool& last_command) {

    if (last_command) file << command.substr(0, 8);
    else file << command.substr(0, 8) << "\n";

}