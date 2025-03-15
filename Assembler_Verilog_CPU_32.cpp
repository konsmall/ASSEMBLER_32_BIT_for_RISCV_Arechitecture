// Assembler_Verilog_CPU_32.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <bitset>
#include <stdlib.h>

typedef unsigned int uint;

#include "Command_List.h"
#include "Instruction_Construction.h"
#include "File_Write.h"


int main(int argc, char** argv)
{
    int JUMP_REF = 0;
    int DATA_JUMP_REF = 100;
    std::string ASSEMBLY_PATH = "test.txt";
    if (argc == 2) ASSEMBLY_PATH = argv[1];
    if (argc == 3) JUMP_REF = strtol(argv[2], NULL, 10);
    if (argc == 4) DATA_JUMP_REF = strtol(argv[3], NULL, 10);

    std::ifstream ASSEMBLY(ASSEMBLY_PATH);

    if (!ASSEMBLY.is_open()) {
        std::cout << "Could not open assenbly file:    " << ASSEMBLY_PATH << "\n";
        return -1;
    }

    std::string line = "";
    std::string word = "";
    std::vector< std::vector< std::string > >  COMMANDS;
    std::vector< std::vector< std::string > > TAGS;
    std::vector< std::string > LINE_BREAKDOWN;
    bool empty_command_flag = false;

    std::vector< std::vector< std::string > >  DATA_COMMANDS;
    std::vector< std::string > DATA_TAGS;
    std::vector< uint > DATA_LENGTH;
    std::vector< uint > DATA_POSITION;
    bool data_section_flag = false;
    bool data_tag_flag = true;

    while (getline(ASSEMBLY, line)) {
        std::stringstream sstream(line);

        std::vector< std::string > temp_line;
        TAGS.push_back(std::vector< std::string >{});
        if (empty_command_flag) {
            TAGS.pop_back();
            empty_command_flag = false;
        }

        while (getline(sstream, word, ' ')) {
            if (word.find("//") != std::string::npos) break;

            if (line == "DATA_SECTION:") {
                data_section_flag = true;
                empty_command_flag = true;
                break;
            }
            else if (word[word.size() - 1] == ':') {
                word.pop_back();
                TAGS[TAGS.size() - 1].push_back(word);
                empty_command_flag = true;
            }
            else if (data_section_flag == false) {
                LINE_BREAKDOWN.push_back(word);
            }

            if (data_section_flag) {
                if (data_tag_flag) {
                    DATA_TAGS.push_back(word);
                    DATA_POSITION.push_back(0); // init position
                    data_tag_flag = false;
                }
                else {
                    LINE_BREAKDOWN.push_back(word);
                }
            }
        }

        if (!LINE_BREAKDOWN.empty()) {
            if (data_section_flag == false) COMMANDS.push_back(LINE_BREAKDOWN);
            else DATA_COMMANDS.push_back(LINE_BREAKDOWN);
            empty_command_flag = false;
        }


        data_tag_flag = true; // Used only when storing data. Used to store the data name/tag in different vector
        LINE_BREAKDOWN.clear();
    }

    for (uint i = 0; i < DATA_TAGS.size(); i++) { // CONNECT LOOSE STRINGS IN TO ONE 
        if (DATA_COMMANDS[i][0] != "STRING") continue; // Instea of having many one words string we end up with only one

        for (uint j = 2; j < DATA_COMMANDS[i].size(); j++) {
            DATA_COMMANDS[i][1] += " " + DATA_COMMANDS[i][j];
        }

        DATA_COMMANDS[i].resize(2);
    }

    for (uint i = 0; i < DATA_COMMANDS.size(); i++) { // Replace/Initialize data STRINGS by requesting the number of BYTES allocated in memory eg. str1 STRING (7)
        for (uint j = 0; j < DATA_COMMANDS[i].size(); j++) {
            if ( DATA_COMMANDS[i][j][0] != '('  &&  DATA_COMMANDS[i][j][DATA_COMMANDS[i][j].size() - 1] != ')') continue; // Locate STRING to allocate memory
            
            std::string str_size_in_str = DATA_COMMANDS[i][j].substr(1, DATA_COMMANDS[i][j].size() - 2); // Extract string NUMBER to int
            uint str_size_in_int = stoi( str_size_in_str );

            std::string temp_str ( str_size_in_int , '\0' ); // Create and replace string with NUMBER chars
            DATA_COMMANDS[i][j] = temp_str;
        }
    }

    for (uint i = 0; i < DATA_TAGS.size(); i++) { // CALCULATE DATA POSITIONS IN MEMORY
        if (i == 0) {
            //DATA_POSITION[0] = DATA_JUMP_REF; // <-- Place first data entry at a given position (static)
            for ( uint j = 0; j < COMMANDS.size(); j++ ) {
                if (COMMANDS[j][0] == "LAF" || COMMANDS[j][0] == "LIF" || COMMANDS[j][0] == "CALLF") { // Place the first data entry at the end of the commands
                    DATA_POSITION[0] += 8; // Each command takes up 4 BYTES in memmory, except for some "macro" commands (NOT YET IMPLEMENTED) thatare made up from 2 commands, thus 8 BYTES
                }
                else {
                    DATA_POSITION[0] += 4;
                }
            }
        }
        else { // Calculate the positions for each next data entry using the length and posotion of the previous data entry
            int data_type = 0;
            uint data_prev_length = 0;
            if (DATA_COMMANDS[i - 1][0] == "WORD") data_prev_length = 4; // Previous data length based on data type
            else if (DATA_COMMANDS[i - 1][0] == "HALF_WORD") data_prev_length = 2;
            else if (DATA_COMMANDS[i - 1][0] == "BYTE") data_prev_length = 1;
            else if (DATA_COMMANDS[i - 1][0] == "STRING") data_prev_length = DATA_COMMANDS[i - 1][1].size();

            DATA_POSITION[i] = data_prev_length + DATA_POSITION[i - 1];
        }
    }

    for (uint i = 0; i < COMMANDS.size(); i++) {
        for (uint j = 0; j < COMMANDS[i].size(); j++) {
            if (COMMANDS[i][j].find("(") == std::string::npos) continue;

            std::string temp_var_tag = COMMANDS[i][j].substr(1, COMMANDS[i][j].size() - 2);
            uint pos_index = 0;
            uint target_position_to_replace = 0;

            auto it_data_tag = find(DATA_TAGS.begin(), DATA_TAGS.end(), temp_var_tag);
            if (it_data_tag != DATA_TAGS.end()) {
                pos_index = it_data_tag - DATA_TAGS.begin();
                COMMANDS[i][j] = "O" + std::to_string(DATA_POSITION[pos_index]);
            }
        }
    }


    ASSEMBLY.close();


    std::ofstream BINARY_COMMANDS("BINARY_COMMANDS.txt");

    if (!BINARY_COMMANDS.is_open()) {
        return -2;
    }

    for (uint i = 0; i < COMMANDS.size(); i++) { // For each command create and write the corelating bits
        std::vector< std::string > line = COMMANDS[i];

        bool last_command = (i == (COMMANDS.size() - 1));
        last_command = false; // Since we are writing commands and data in to the same file, the last command is never teh end of the file

        std::string command = "";


        if (line[0] == "ADD") { // ADD
            command = R_Instruction_Constructor(line, ADD);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SUB") { // SUBTRACT
            command = R_Instruction_Constructor(line, SUB);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MUL") { // MULTIPLY
            command = R_Instruction_Constructor(line, MUL);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "DIV") { // DIVIDE
            command = R_Instruction_Constructor(line, DIV);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SLT") { // SET LESS THAN
            command = R_Instruction_Constructor(line, SLT);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "ADDI") { // ADD IMMEDIATE
            command = I_Instruction_Constructor(line, ADDI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SUBI") { // SUBTRACT IMMEDIATE
            command = I_Instruction_Constructor(line, SUBI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MULI") { // MULTIPLY IMMEDIATE
            command = I_Instruction_Constructor(line, MULI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "DIVI") { // DIVIDE IMMEDIATE
            command = I_Instruction_Constructor(line, DIVI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SLTI") { // SET LESS THAN IMMEDIATE
            command = I_Instruction_Constructor(line, SLTI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "LUI") { // LOAD UPPER IMMEDIATE
            command = U_Instruction_Constructor(line, LUI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "AUIP") { // ADD UPPER IMMEDIATE TO PC
            command = I_Instruction_Constructor(line, AUIP);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }


        else if (line[0] == "SLL") { // SET LEFT LOGICAL
            command = R_Instruction_Constructor(line, SLL);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SRL") { // SET RIGHT LOGICAL
            command = R_Instruction_Constructor(line, SRL);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SRA") { // SET RIGHT ARITHMETIC
            command = R_Instruction_Constructor(line, SRA);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "OR") { // OR
            command = R_Instruction_Constructor(line, OR);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "AND") { // AND
            command = R_Instruction_Constructor(line, AND);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "XOR") { // XOR
            command = R_Instruction_Constructor(line, XOR);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "NOT") { // NOT
            command = R_Instruction_Constructor(line, NOT);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SLLI") { // SET LEFT LOGICAL IMMEDIATE
            command = R_Instruction_Constructor(line, SLLI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SRLI") { // SET RIGHT LOGICAL IMMEDIATE
            command = R_Instruction_Constructor(line, SRLI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SRAI") { // SET RIGHT ARITHMETIC IMMEDIATE
            command = R_Instruction_Constructor(line, SRAI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "ORI") { // OR IMMEDIATE
            command = R_Instruction_Constructor(line, ORI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "ANDI") { // AND IMMEDIATE
            command = R_Instruction_Constructor(line, ANDI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "XORI") { // XOR IMMEDIATE
            command = R_Instruction_Constructor(line, XORI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "NOTI") { // NOT IMMEDIATE
            command = R_Instruction_Constructor(line, NOTI);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }


        else if (line[0] == "LW") { // LOAD WORD
            command = I_Instruction_Constructor(line, LW);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "LH") { // LOAD HALF WORD
            command = I_Instruction_Constructor(line, LH);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "LB") { // LOAD BYTE
            command = I_Instruction_Constructor(line, LB);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SW") { // STORE WORD
            command = S_Instruction_Constructor(line, SW);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SH") { // STORE HALF WORD
            command = S_Instruction_Constructor(line, SH);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "SB") { // STORE BYTE
            command = S_Instruction_Constructor(line, SB);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }


        else if (line[0] == "BEQ") { // BRANCH IF EQUAL
            command = SB_Instruction_Constructor(line, BEQ, i, TAGS, JUMP_REF);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "BNE") { // BRANCH IF NOT EQUAL
            command = SB_Instruction_Constructor(line, BNE, i, TAGS, JUMP_REF);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "BGE") { // BRANCH IF GREATER OR EQUAL
            command = SB_Instruction_Constructor(line, BGE, i, TAGS, JUMP_REF);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "BGEU") { // BRANCH IF GREATER OR EQUAL UNSIGNED
            command = SB_Instruction_Constructor(line, BGEU, i, TAGS, JUMP_REF);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "BLT") { // BRANCH IF LESS
            command = SB_Instruction_Constructor(line, BLT, i, TAGS, JUMP_REF);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "BLTU") { // BRANCH IF LESS UNSIGNED
            command = SB_Instruction_Constructor(line, BLTU, i, TAGS, JUMP_REF);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "JAL") { // JUMP AND LINK
            command = UJ_Instruction_Constructor(line, JAL, i, TAGS, JUMP_REF);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "JALR") { // JUMP AND LINK REGISTER
            command = I_Instruction_Constructor(line, JALR);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }


        else if (line[0] == "MLW") { // MEMRISTOR LOAD WORD
            command = I_Instruction_Constructor(line, MLW);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MSW") { // MEMRISTOR STORE WORD
            command = S_Instruction_Constructor(line, MSW);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MOR") { // MEMRISTOR OR TO REGISTER
            command = R_Instruction_Constructor(line, MOR);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MAND") { // MEMRISTOR AND TO REGISTER
            command = R_Instruction_Constructor(line, MAND);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MXOR") { // MEMRISTOR XOR TO REGISTER
            command = R_Instruction_Constructor(line, MXOR);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MNOR") { // MEMRISTOR NOR TO REGISTER
            command = R_Instruction_Constructor(line, MNOR);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MNAND") { // MEMRISTOR NAND TO REGISTER
            command = R_Instruction_Constructor(line, MNAND);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MXNOR") { // MEMRISTOR XNOR TO REGISTER
            command = R_Instruction_Constructor(line, MXNOR);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MORM") { // MEMRISTOR OR TO MEMRISTOR
            command = R_Instruction_Constructor(line, MORM);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MANDM") { // MEMRISTOR AND TO MEMRISTOR
            command = R_Instruction_Constructor(line, MANDM);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MXORM") { // MEMRISTOR XOR TO MEMRISTOR
            command = R_Instruction_Constructor(line, MXORM);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MNORM") { // MEMRISTOR NOR TO MEMRISTOR
            command = R_Instruction_Constructor(line, MNORM);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MNANDM") { // MEMRISTOR NAND TO MEMRISTOR
            command = R_Instruction_Constructor(line, MNANDM);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "MXNORM") { // MEMRISTOR XNOR TO MEMRISTOR
            command = R_Instruction_Constructor(line, MXNORM);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }


        else if (line[0] == "END") { // END JUMP TO CURRENT POSITION
            std::vector< std::string > temp_line;
            temp_line.push_back("END");
            temp_line.push_back("R0");
            temp_line.push_back("R0");
            temp_line.push_back("END");


            command = SB_Instruction_Constructor(temp_line, BEQ, i, TAGS, JUMP_REF);
            file_command_write(BINARY_COMMANDS, command, last_command);
        }
    }


    for (uint i = 0; i < DATA_COMMANDS.size(); i++) {
        std::vector< std::string > line = DATA_COMMANDS[i];

        bool last_command = (i == (DATA_COMMANDS.size() - 1));

        std::string command = "";


        if (line[0] == "WORD") {
            command = std::bitset< 32 >(stoi(line[1])).to_string();
            file_word_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "HALF_WORD") {
            command = std::bitset< 16 >(stoi(line[1])).to_string();
            file_half_word_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "BYTE") {
            if (std::string::npos != line[1].find_first_of("0123456789")) {
                command = std::bitset< 8 >(stoi(line[1])).to_string();
            }
            else {
                command = std::bitset< 8 >(line[1][0]).to_string();
            }

            file_byte_write(BINARY_COMMANDS, command, last_command);
        }
        else if (line[0] == "STRING") {
            for (uint j = 0; j < line[1].size(); j++) {
                command = std::bitset< 8 >(line[1][j]).to_string();
                bool last_command_str = ( j == (line[1].size() - 1) ) && (last_command);
                file_byte_write(BINARY_COMMANDS, command, last_command_str);
            }
        }
    }

    BINARY_COMMANDS.close();


    /*BINARY_COMMANDS.close();


    std::ofstream BINARY_DATA("BINARY_DATA.txt");

    if (!BINARY_DATA.is_open()) {
        return -3;
    }

    for (uint i = 0; i < DATA_COMMANDS.size(); i++) {
        std::vector< std::string > line = DATA_COMMANDS[i];

        bool last_command = (i == (DATA_COMMANDS.size() - 1));

        std::string command = "";


        if (line[0] == "WORD") {
            command = std::bitset< 32 >(stoi(line[1])).to_string();
            file_word_write(BINARY_DATA, command, last_command);
        }
        else if (line[0] == "HALF_WORD") {
            command = std::bitset< 16 >(stoi(line[1])).to_string();
            file_half_word_write(BINARY_DATA, command, last_command);
        }
        else if (line[0] == "BYTE") {
            if (std::string::npos != line[1].find_first_of("0123456789")) {
                command = std::bitset< 8 >(stoi(line[1])).to_string();
            }
            else {
                command = std::bitset< 8 >(line[1][0]).to_string();
            }

            file_byte_write(BINARY_DATA, command, last_command);
        }
        else if (line[0] == "STRING") {
            for (uint j = 0; j < line[1].size(); j++) {
                command = std::bitset< 8 >(line[1][j]).to_string();
                bool last_command_str = (j == (line[1].size() - 1));
                file_byte_write(BINARY_DATA, command, last_command_str);
            }
        }
    }

    BINARY_DATA.close();*/
    return 0;
}
