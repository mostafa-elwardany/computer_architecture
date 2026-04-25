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

int main() {
    int clock_cycle = 0;
    short int instructionMemory[1024];
    int8_t dataMemory[2048];
    int8_t R[64]; //REGISTERS
    int8_t SREG;  // flags
    short int PC=0;  // program counter
    int program_size = 0; //the number of instructions in the instruction memory,dont know how we will add the instructions yet
    IF_ID if_id = {0};
    ID_EX id_ex = {0};
    read_instructions(instructionMemory, &program_size); // Function to read instructions into instruction memory
    while (1) {
        execute(&id_ex,R,&SREG,dataMemory,&PC,&if_id);
        decode(R, &if_id, &id_ex);
        fetch(&PC, program_size, instructionMemory, &if_id);
        clock_cycle++;
    }
    return 0;
}

 void execute(ID_EX *id_ex, int8_t *R, int8_t *SREG, int8_t *dataMemory, short int *PC, IF_ID *if_id) {
        // Implementation of the execute stage
        if(id_ex->valid == 0) {
            return; // No valid instruction to execute
        }
        else{
        *SREG = 0; // Clear status register flags before execution
        int16_t result;
        switch(id_ex->opcode) {
            case ADD://checks for carry flag,zero,negative,sign,overflow flags
                id_ex->val1 = id_ex->val1 + id_ex->val2;
                result = id_ex->val1 + id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                if(result >> 8 == 1) { // Check for carry out of the 8th bit
                    *SREG |= 0x10; // Set carry flag
                } 
                if(id_ex->val1 > 0 && id_ex ->val2 >0) {
                    if(result >> 8 == 1) { // Check for overflow in case of adding two positive numbers resulting in a negative number
                        *SREG |= 0x08; // Set overflow flag
                    }
                }
                if(id_ex->val1 < 0 && id_ex ->val2 < 0) {
                    if(result >> 8 == 0) { // Check for overflow in case of adding two negative numbers resulting in a positive number
                        *SREG |= 0x08; // Set overflow flag
                    }
                }
                if(result == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                }
                if(id_ex->val1 + id_ex->val2 < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                }
                if((*SREG&& 0x00) >> 3 ^ (*SREG&& 0x00) >> 2) { // Check for sign change between the 3rd and 4th bits to set the sign flag
                    *SREG |= 0x02; // Set sign flag
                }
                break;
            case SUB://check for overflow,sign,negative,zero flags will do later
                id_ex->val1 = id_ex->val1 - id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                result = id_ex->val1 - id_ex->val2;
                 if(id_ex->val1 > 0 && id_ex ->val2 <0) {
                    if(result >> 8 == 1) { // Check for overflow in case of adding two positive numbers resulting in a negative number
                        *SREG |= 0x08; // Set overflow flag
                    }
                }
                if(id_ex->val1 < 0 && id_ex ->val2 > 0) {
                    if(result >> 8 == 0) { // Check for overflow in case of adding two negative numbers resulting in a positive number
                        *SREG |= 0x08; // Set overflow flag
                    }
                }
                 if(id_ex->val1 - id_ex->val2 < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                }
                if(result == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                }
                 if((*SREG&& 0x00) >> 3 ^ (*SREG&& 0x00) >> 2) { // Check for sign change between the 3rd and 4th bits to set the sign flag
                    *SREG |= 0x02; // Set sign flag
                }
                break;
            case MUL:
                id_ex->val1 = id_ex->val1 * id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                result = id_ex->val1 * id_ex->val2;
                if(id_ex->val1 * id_ex->val2 < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                }
                if(result == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                }
                break;
            case EOR:
                id_ex->val1 = id_ex->val1 ^ id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                if(id_ex->val1 ^ id_ex->val2 < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                }
                if((id_ex->val1 ^ id_ex->val2) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                }
                break;
            case MOVI:
                id_ex->val1 = id_ex->imm; // Move immediate value to val1
                R[id_ex->r1] = id_ex->val1; // Write the immediate value to the register file
                break;
            case BEQZ:
                if(id_ex->val1 == 0) {
                    // Branch to the address specified by imm if val1 is zero
                    *PC += id_ex->imm;
                    if_id->valid = 0; // Invalidate the instruction in the IF/ID pipeline register since we are branching
                }
                break;
            case ANDI:
                id_ex->val1 = id_ex->val1 & id_ex->imm; // AND immediate value with val1
               R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
               if(id_ex->val1 & id_ex->imm < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                }
                break;
            case BR:
                // Unconditional branch to the address specified by imm
                *PC = strcat(id_ex->val1, id_ex->val2);
                if_id->valid = 0; // Invalidate the instruction in the IF/ID pipeline register since we are branching
                break;
            case SAL:
                id_ex->val1 = id_ex->val1 << id_ex->imm; // Shift left logical by imm bits
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file   
                break;
            case SAR:
                id_ex->val1 = id_ex->val1 >> id_ex->imm; // Shift right arithmetic by imm bits
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file  
                break;
            case LDR:
                id_ex->val1 = id_ex->imm; 
                R[id_ex->r1] = dataMemory[id_ex->val1]; // Load the value from memory into the register file
                break;
            case STR:
                id_ex->val1 = id_ex->imm; 
                dataMemory[id_ex->val1] = R[id_ex->r1]; // Store the value from the register file into memory
                break;
        }
    }
}
    void decode(int8_t *R, IF_ID *if_id, ID_EX *id_ex) {
        // Implementation of the decode stage
        if(if_id->valid == 0) {
            id_ex->valid = 0; // No valid instruction to decode
            return;
        }
        else{
        id_ex->opcode = (if_id->instruction >> 12) & 0xF; // Extract opcode
        if(id_ex->opcode == ADD || id_ex->opcode == SUB || id_ex->opcode == MUL ||id_ex->opcode == EOR || id_ex->opcode == BR) {
            id_ex->r1 = (if_id->instruction >> 6) & 0x3F; // Extract r1
            id_ex->r2 = if_id->instruction & 0x3F; // Extract r2
            id_ex->val1 = R[id_ex->r1]; // Read value of r1
            id_ex->val2 = R[id_ex->r2]; // Read value of r2

        } else if(id_ex->opcode == MOVI || id_ex->opcode == BEQZ || id_ex->opcode == SAL || id_ex->opcode == SAR || id_ex->opcode == ANDI) {
            id_ex->r1 = (if_id->instruction >> 6) & 0x3F; // Extract r1
            id_ex->imm = sign_extend_6bit(if_id->instruction & 0x3F); // Extract immediate value

        } else if(id_ex->opcode == LDR || id_ex->opcode == STR) {
            id_ex->r1 = (if_id->instruction >> 6) & 0x3F; // Extract r1
            id_ex->imm = sign_extend_6bit(if_id->instruction & 0x3F); // Extract immediate value for memory address
        }
        id_ex->valid = 1; // Mark the instruction as valid for the execute stage
    }
        
    }
    void fetch(short int *PC, short int program_size,short int instructionMemory[], IF_ID *if_id) {
        // Implementation of the fetch stage
        if(*PC < program_size) {//would need to change this when we actually implement the instruction loading into memory
            if_id->instruction = instructionMemory[*PC];
            if_id->valid = 1;
            (*PC)++;
        } else {
            if_id->valid = 0; // No more instructions to fetch
        }
    }

    void read_instructions(short int instructionMemory[], int *program_size) {
        
    }