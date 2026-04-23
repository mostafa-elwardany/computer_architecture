#include <stdint.h>
#include <stdio.h>
#define ADD 0
#define SUB 1
#define MUL 2
#define MOVI 3
#define BEQZ 4
#define ANDI 5
#define EOR 6
#define BR 7
#define SAL 8
#define SAR 9
#define LDR 10
#define STR 11 //these are just to help with comparasion instead of writing the bit values
int main() {
    int clock_cycle = 0;
    short int instructionMemory[1024];
    int8_t dataMemory[2048];
    int8_t R[64]; //REGISTERS
    int8_t SREG;  // flags
    short int PC=0;  // program counter
    int program_size = 0; //the number of instructions in the instruction memory,dont know how we will add the instructions yet
    typedef struct {// pipeline register between IF and ID stages
        uint16_t instruction;
        int valid;
    } IF_ID;

    typedef struct {// pipeline register between ID and EX stages
        int8_t opcode;
        int8_t r1, r2;
        int8_t imm;//immediate value for any immediate instructions
        int8_t val1, val2;  // actual register values tho feel like they should be signed bits instead of unsigned
        int valid;
    } ID_EX;
    while (1) {
        execute();
        decode();
        fetch();
        clock_cycle++;
    }

    void execute() {
        // Implementation of the execute stage
    }
    void decode() {
        // Implementation of the decode stage
        if(IF_ID.valid == 0) {
            ID_EX.valid = 0; // No valid instruction to decode
            return;
        }
        else{
        ID_EX.opcode = (IF_ID.instruction >> 12) & 0xF; // Extract opcode
        if(ID.EX.opcode == ADD || ID_EX.opcode == SUB || ID_EX.opcode == MUL ||ID_EX.opcode == EOR) {
            ID_EX.r1 = (IF_ID.instruction >> 6) & 0x3F; // Extract r1
            ID_EX.r2 = IF_ID.instruction & 0x3F; // Extract r2
            ID_EX.val1 = R[ID_EX.r1]; // Read value of r1
            ID_EX.val2 = R[ID_EX.r2]; // Read value of r2

        } else if(ID_EX.opcode == MOVI || ID_EX.opcode == BEQZ || ID_EX.opcode == BR || ID_EX.opcode == SAL || ID_EX.opcode == SAR || ID_EX.opcode == ANDI) {
            ID_EX.r1 = (IF_ID.instruction >> 6) & 0x3F; // Extract r1
            ID_EX.imm = IF_ID.instruction & 0x3F; // Extract immediate value

        } else if(ID_EX.opcode == LDR || ID_EX.opcode == STR) {
            ID_EX.r1 = (IF_ID.instruction >> 6) & 0x3F; // Extract r1
            ID_EX.imm = IF_ID.instruction & 0x3F; // Extract immediate value for memory address
        }
        ID_EX.valid = 1; // Mark the instruction as valid for the execute stage
    }
        
    }
    void fetch() {
        // Implementation of the fetch stage
        if(PC < program_size) {
            IF_ID.instruction = instructionMemory[PC];
            IF_ID.valid = 1;
            PC++;
        } else {
            IF_ID.valid = 0; // No more instructions to fetch
        }
    }

    return 0;
}