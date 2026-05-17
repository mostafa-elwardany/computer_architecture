#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>    
#include <ctype.h> 
#define ADD 0
#define SUB 1
#define MUL 2
#define MOVI 3
#define BEQZ 4
#define ANDI 5
#define EOR 6
#define BR 7
#define SLC 8
#define SRC 9
#define LDR 10
#define STR 11 //these are just to help with comparasion instead of writing the bit values


typedef struct {
    uint16_t instruction;
    int valid;
    short int pc;
} IF_ID;

typedef struct {
    int8_t opcode;
    int8_t r1, r2;
    int8_t imm;
    int8_t val1, val2;
    int valid;
    short int pc;
} ID_EX;

int loadProgram(const char *filename, short int instructionMemory[1024]);
void execute(ID_EX *id_ex, int8_t *R, int8_t *SREG, int8_t *dataMemory, short int *PC, IF_ID *if_id);
void decode(int8_t *R, IF_ID *if_id, ID_EX *id_ex);
void fetch(short int *PC, short int program_size, short int instructionMemory[], IF_ID *if_id);
void printreg(int8_t *R);
void printSREG(int8_t sreg);
void printmemory(int8_t *dataMemory);
void printAllInstructions(short int *instructionMemory, int program_size);
int8_t sign_extend_6bit(int8_t val);
static void trim(char *s);
static int parseRegister(const char *tok);
int parseImmediate(const char *tok);
static int getOpcode(const char *mn);
static int isRFormat(int opcode);
void addValues(int8_t *R);
void datavalues(int8_t *dataMemory);

int main() {
    int clock_cycle = 1;
    short int instructionMemory[1024] = {0};
     int8_t dataMemory[2048] = {0};
    int8_t R[64]= {0}; //REGISTERS
    int8_t SREG=0;  // flags
    short int PC=0;  // program counter
    IF_ID if_id = {0};
    ID_EX id_ex = {0};
    int program_size = loadProgram("assembly.txt",instructionMemory);
    printf("Program loaded with %d instructions.\n", program_size);
    while (1) {
        printf("Clock Cycle: %d\n", clock_cycle);
        execute(&id_ex,R,&SREG,dataMemory,&PC,&if_id);
        decode(R, &if_id, &id_ex);
        fetch(&PC, program_size, instructionMemory, &if_id);
        clock_cycle++;
        if(PC >= program_size && if_id.valid == 0 && id_ex.valid == 0) {
            printf("Program execution completed.\n");
            break; // Exit the loop when there are no more instructions to execute
        }
    }
    printSREG(SREG);
    /*printreg(R);
    printmemory(dataMemory);
    printAllInstructions(instructionMemory, program_size);*/
    return 0;
}

 void execute(ID_EX *id_ex, int8_t *R, int8_t *SREG, int8_t *dataMemory, short int *PC, IF_ID *if_id) {
        // Implementation of the execute stage
        if(id_ex->valid == 0) {
            printf("No instruction to execute at clock cycle\n");
            return; // No valid instruction to execute
        }
        else{
        *SREG = 0; // Clear status register flags before execution
        int16_t result;
        printf("Executing instruction with opcode %d at PC %d\n", id_ex->opcode, id_ex->pc);
        switch(id_ex->opcode) {
            case ADD://checks for carry flag,zero,negative,sign,overflow flags
                printf("Executing ADD instruction\n");
                printf("val1(r1): %d, val2(r2): %d\n", id_ex->val1, id_ex->val2);
                result = id_ex->val1 + id_ex->val2;
                printf("Result of addition: %d\n", result);
                if((result & 0x100) != 0) { // Check for carry out of the 8th bit
                    *SREG |= 0x10; // Set carry flag
                    printf("Carry flag set\n");
                } 
                if((id_ex->val1 > 0) && (id_ex ->val2 > 0)) {
                    if((result >> 8) == 1) { // Check for overflow in case of adding two positive numbers resulting in a negative number
                        *SREG |= 0x08; // Set overflow flag
                        printf("Overflow flag set\n");
                    }
                }
                if((id_ex->val1 < 0) && (id_ex ->val2 < 0)) {
                    if((result >> 8) == 0) { // Check for overflow in case of adding two negative numbers resulting in a positive number
                        *SREG |= 0x08; // Set overflow flag
                        printf("Overflow flag set\n");
                    }
                }
                if((result & 0xFF) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                    printf("Zero flag set\n");
                }
                if((result & 0x80) != 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                    printf("Negative flag set\n");
                }
               if(((*SREG>> 3)& 0x01) ^ ((*SREG  >> 2) & 0x01)) { // Check for sign change between the 3rd and 4th bits to set the sign flag
                    *SREG |= 0x02; // Set sign flag
                    printf("Sign flag set\n");
                }
                id_ex->val1 = id_ex->val1 + id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case SUB://check for overflow,sign,negative,zero flags will do later
                 printf("Executing SUB instruction\n");
                printf("val1(r1): %d, val2(r2): %d\n", id_ex->val1, id_ex->val2);
                result = id_ex->val1 - id_ex->val2;
                 if(id_ex->val1 > 0 && (id_ex ->val2 <0)) {
                    if((result >> 8) == 1) { // Check for overflow in case of adding two positive numbers resulting in a negative number
                        *SREG |= 0x08; // Set overflow flag
                        printf("Overflow flag set\n");
                    }
                }
                if((id_ex->val1 < 0) && (id_ex ->val2 > 0)) {
                    if((result >> 8) == 0) { // Check for overflow in case of adding two negative numbers resulting in a positive number
                        *SREG |= 0x08; // Set overflow flag
                        printf("Overflow flag set\n");
                    }
                }
                 if((result & 0x80) != 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                    printf("Negative flag set\n");
                }
                if((result & 0xFF) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                    printf("Zero flag set\n");
                }
                 if(((*SREG>> 3)& 0x01) ^ ((*SREG  >> 2) & 0x01)) { // Check for sign change between the 3rd and 4th bits to set the sign flag
                    *SREG |= 0x02; // Set sign flag
                    printf("Sign flag set\n");
                }
                id_ex->val1 = id_ex->val1 - id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case MUL:
                 printf("Executing MUL instruction\n");
                printf("val1(r1): %d, val2(r2): %d\n", id_ex->val1, id_ex->val2);
                result = id_ex->val1 * id_ex->val2;
                if((result & 0x80) != 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                    printf("Negative flag set\n");
                }
                if((result & 0xFF) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                    printf("Zero flag set\n");
                }
                id_ex->val1 = id_ex->val1 * id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case EOR:
                printf("Executing EOR instruction\n");
                printf("val1(r1): %d, val2(r2): %d\n", id_ex->val1, id_ex->val2);
                result = id_ex->val1 ^ id_ex->val2;
                if((result & 0x80) != 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                    printf("Negative flag set\n");
                }
                if((result & 0xFF) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                    printf("Zero flag set\n");
                }

                id_ex->val1 = id_ex->val1 ^ id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case MOVI:
                printf("Executing MOVI instruction\n");
                printf("Immediate value: %d, val1(r1): %d\n", id_ex->imm, id_ex->val1);
                id_ex->val1 = id_ex->imm; // Move immediate value to val1
                R[id_ex->r1] = id_ex->val1; // Write the immediate value to the register file
                printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case BEQZ:
                if(id_ex->val1 == 0) {
                    // Branch to the address specified by imm if val1 is zero
                    *PC = id_ex->pc + id_ex->imm+1;
                    if_id->valid = 0; // Invalidate the instruction in the IF/ID pipeline register since we are branching
                    id_ex->valid = 0; // Invalidate the instruction in the ID/EX pipeline register since we are branching
                     printf("Executing BEQZ instruction: Branch taken to address %d\n", *PC);
                }
                break;
            case ANDI:
                 printf("Executing ANDI instruction\n");
                 printf("Immediate value: %d, val1(r1): %d\n", id_ex->imm, id_ex->val1);
               if((id_ex->val1 & id_ex->imm) < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                    printf("Negative flag set\n");
                }
                if((id_ex->val1 & id_ex->imm) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                    printf("Zero flag set\n");
                }
                id_ex->val1 = id_ex->val1 & id_ex->imm; // AND immediate value with val1
               R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                 printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case BR:
                // Unconditional branch to the address specified by imm
                *PC = ((id_ex->val1 & 0xFF) << 8) | (id_ex->val2 & 0xFF);
                if_id->valid = 0; // Invalidate the instruction in the IF/ID pipeline register since we are branching
                id_ex->valid = 0; // Invalidate the instruction in the ID/EX pipeline register since we are branching
                 printf("Executing BR instruction: Branching to address %d\n", *PC);
                break;
            case SLC:
                printf("Executing SLC instruction\n");
                printf("Immediate value: %d, val1(r1): %d\n", id_ex->imm, id_ex->val1);
                unsigned char uval = (unsigned char)id_ex->val1;
                unsigned char uresult = ((uval << id_ex->imm) | (uval >> (8 - id_ex->imm)));
                id_ex->val1 = (int8_t)uresult;  // Store back as signed
                 R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                 printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case SRC:
                printf("Executing SRC instruction\n");
                printf("Immediate value: %d, val1(r1): %d\n", id_ex->imm, id_ex->val1);
                unsigned char uval2 = (unsigned char)id_ex->val1;
                unsigned char uresult2 = ((uval2 >> id_ex->imm) | (uval2 << (8 - id_ex->imm)));
                id_ex->val1 = (int8_t)uresult2;  // Store back as signed
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case LDR:
            printf("Executing LDR instruction\n");
                printf("Memory address: %d, val1(r1): %d\n", id_ex->imm, id_ex->val1);
                id_ex->val1 = id_ex->imm; 
                R[id_ex->r1] = dataMemory[id_ex->val1]; // Load the value from memory into the register file
                printf("updated reg(r1): %d\n", id_ex->val1);
                break;
            case STR:
                  printf("Executing STR instruction\n");
                printf("Memory address: %d, val1(r1): %d\n", id_ex->imm, id_ex->val1);
                dataMemory[id_ex->imm] = R[id_ex->r1]; // Store the value from the register file into memory
                printf("updated memory at address %d: %d\n", id_ex->imm, dataMemory[id_ex->imm]);
              
                break;
        }
    }
}
    void decode(int8_t *R, IF_ID *if_id, ID_EX *id_ex) {
        // Implementation of the decode stage
        if(if_id->valid == 0) {
            id_ex->valid = 0; // No valid instruction to decode
            printf("No instruction to decode at clock cycle\n");
            return;
        }
        else{
        printf("Decoding instruction 0x%04X at PC %d\n", if_id->instruction, if_id->pc);
        id_ex->pc = if_id->pc; 
        id_ex->opcode = (if_id->instruction >> 12) & 0xF; // Extract opcode
        if(id_ex->opcode == ADD || id_ex->opcode == SUB || id_ex->opcode == MUL ||id_ex->opcode == EOR || id_ex->opcode == BR) {
            id_ex->r1 = (if_id->instruction >> 6) & 0x3F; // Extract r1
            id_ex->r2 = if_id->instruction & 0x3F; // Extract r2
            id_ex->val1 = R[id_ex->r1]; // Read value of r1
            id_ex->val2 = R[id_ex->r2]; // Read value of r2

        } else if(id_ex->opcode == MOVI || id_ex->opcode == BEQZ || id_ex->opcode == SLC || id_ex->opcode == SRC || id_ex->opcode == ANDI) {
            id_ex->r1 = (if_id->instruction >> 6) & 0x3F; // Extract r1
            id_ex->val1 = R[id_ex->r1]; // Read value of r1

        } else if(id_ex->opcode == LDR || id_ex->opcode == STR) {
            id_ex->r1 = (if_id->instruction >> 6) & 0x3F; // Extract r1
        }
        if (id_ex->opcode == SLC || id_ex->opcode == SRC) {
            id_ex->imm = if_id->instruction & 0x3F;
        } else {
            id_ex->imm = sign_extend_6bit(if_id->instruction & 0x3F);
        }
        id_ex->valid = 1; // Mark the instruction as valid for the execute stage
        printf("Decoded instruction: opcode=%d, r1=%d, r2=%d, imm=%d\n", id_ex->opcode, id_ex->r1, id_ex->r2, id_ex->imm);
        printf("Read values: val1(r1)=%d, val2(r2)=%d\n", id_ex->val1, id_ex->val2);
    }
        
    }
    void fetch(short int *PC, short int program_size,short int instructionMemory[], IF_ID *if_id) {
        // Implementation of the fetch stage
        if(*PC < program_size) {//would need to change this when we actually implement the instruction loading into memory
            if_id->instruction = instructionMemory[*PC];
            if_id->valid = 1;
             if_id->pc = *PC;
            (*PC)++;
            printf("Fetched instruction 0x%04X from address %d\n", if_id->instruction, *PC - 1);
        } else {
            if_id->valid = 0; // No more instructions to fetch
        }
    }

    int loadProgram(const char *filename, short int instructionMemory[1024]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: cannot open '%s'\n", filename);
        return -1;
    }
 
    char line[256];
    int  pc = 0;
 
    while (fgets(line, sizeof(line), file) != NULL) {
 
        
        line[strcspn(line, "\r\n")] = '\0';   
        trim(line);
 
        
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
            continue;
        char *commentPos = strpbrk(line, "#;");
        if (commentPos) {
            *commentPos = '\0';
            trim(line);
        }
        if (line[0] == '\0') continue;
 
        char mnemonic[16] = {0};
        char op1[16]      = {0};
        char op2[16]      = {0};
 
        int fieldCount = sscanf(line, "%15s %15s %15s", mnemonic, op1, op2);
        if (fieldCount < 1) continue;
        for (int i = 0; mnemonic[i]; i++)
            mnemonic[i] = toupper((unsigned char)mnemonic[i]);
 
        int opcode = getOpcode(mnemonic);
        if (opcode == -1) {
            fprintf(stderr,
                    "Warning: unknown instruction '%s' at line %d — skipped\n",
                    mnemonic, pc + 1);
            continue;
        }
 
        if (pc >= 1024) {
            fprintf(stderr,
                    "Error: instruction memory full (1024 words max). "
                    "Remaining instructions ignored.\n");
            break;
        }
        uint16_t encoded = 0;
 
        if (isRFormat(opcode)) {
            int r1 = parseRegister(op1);
            int r2 = parseRegister(op2);
            if (r1 < 0 || r1 > 63 || r2 < 0 || r2 > 63) {
                fprintf(stderr,
                        "Warning: register out of range in '%s' — skipped\n",
                        line);
                continue;
            }
 
            encoded = (uint16_t)(((opcode & 0xF) << 12) |
                                 ((r1     & 0x3F) <<  6) |
                                  (r2     & 0x3F));
        } else {
            int r1  = parseRegister(op1);
            int imm = parseImmediate(op2);
 
            if (r1 < 0 || r1 > 63) {
                fprintf(stderr,
                        "Warning: register out of range in '%s' — skipped\n",
                        line);
                continue;
            }
            if (imm < -32 || imm > 63) {          
                fprintf(stderr,
                        "Warning: immediate %d in '%s' may not fit in 6 bits\n",
                        imm, line);
            }
 
            encoded = (uint16_t)(((opcode & 0xF) << 12) |
                                 ((r1     & 0x3F) <<  6) |
                                  (imm    & 0x3F));
        }
 
        instructionMemory[pc] = (short int)encoded;
        printf("MEM[%4d] = 0x%04X  (binary: %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d)  <- %s\n",
               pc,
               encoded,
               (encoded >> 15) & 1, (encoded >> 14) & 1,
               (encoded >> 13) & 1, (encoded >> 12) & 1,
               (encoded >> 11) & 1, (encoded >> 10) & 1,
               (encoded >>  9) & 1, (encoded >>  8) & 1,
               (encoded >>  7) & 1, (encoded >>  6) & 1,
               (encoded >>  5) & 1, (encoded >>  4) & 1,
               (encoded >>  3) & 1, (encoded >>  2) & 1,
               (encoded >>  1) & 1, (encoded >>  0) & 1,
               line);
 
        pc++;
    }
 
    fclose(file);
    return pc;
}

static void trim(char *s) {
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
    int i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i > 0) memmove(s, s + i, len - i + 1);
}
static int parseRegister(const char *tok) {
    if (tok[0] == 'R' || tok[0] == 'r')
        return atoi(tok + 1);
    return atoi(tok);
}
int parseImmediate(const char *tok) {
    return atoi(tok);
}
static int getOpcode(const char *mn) {
    if (strcmp(mn, "ADD")  == 0) return ADD;
    if (strcmp(mn, "SUB")  == 0) return SUB;
    if (strcmp(mn, "MUL")  == 0) return MUL;
    if (strcmp(mn, "MOVI") == 0) return MOVI;
    if (strcmp(mn, "BEQZ") == 0) return BEQZ;
    if (strcmp(mn, "ANDI") == 0) return ANDI;
    if (strcmp(mn, "EOR")  == 0) return EOR;
    if (strcmp(mn, "BR")   == 0) return BR;
    if (strcmp(mn, "SLC")  == 0) return SLC;
    if (strcmp(mn, "SRC")  == 0) return SRC;
    if (strcmp(mn, "LDR")  == 0) return LDR;
    if (strcmp(mn, "STR")  == 0) return STR;
    return -1;
}
 int isRFormat(int opcode) {
    return (opcode == ADD || opcode == SUB || opcode == MUL ||
            opcode == EOR || opcode == BR);
}

void printreg(int8_t *R) {
    printf("Registers: ");
    for(int i = 0; i < 64; i++) {
        printf("R%d=%d ", i, R[i]);
    }
    printf("\n");
}

void printSREG(int8_t sreg) {
    printf("SREG = 0x%02X\n", sreg);
    printf("Bits: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d ", (sreg >> i) & 1);
    }
    printf("\n");
    printf("Pos:  7 6 5 4 3 2 1 0\n");
    printf("      - - - C V N S Z\n");  // Flag names
}

void printmemory(int8_t *dataMemory) {
    printf("Data Memory:\n");
    for(int i = 0; i < 2048; i++) { 
        printf("address: %d value: %d\n", i, dataMemory[i]);
    }
}

int8_t sign_extend_6bit(int8_t val) {
    if (val & 0x20) {  // 0x20 = 0b100000 (bit 5)
        val |= 0xC0;   // 0xC0 = 0b11000000 (set bits 7 and 6)
    }
    return val;
}

void printAllInstructions(short int *instructionMemory, int program_size) {
    printf("\nInstruction Memory (%d instructions):\n", program_size);
    for (int i = 0; i < program_size; i++) {
        uint16_t instr = (uint16_t)instructionMemory[i];
        printf("  0x%04X [%04d]: 0x%04X = ", i, i, instr);
        // Print binary
        for (int b = 15; b >= 0; b--) {
            printf("%d", (instr >> b) & 1);
            if (b == 12 || b == 6) printf(" ");
        }
        printf("\n");
    }
}