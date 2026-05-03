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
#define SLC 8
#define SRC 9
#define LDR 10
#define STR 11 //these are just to help with comparasion instead of writing the bit values

typedef struct {// pipeline register between IF and ID stages
        uint16_t instruction;
        int valid;
        short int pc;
    } IF_ID;

    typedef struct {// pipeline register between ID and EX stages
        int8_t opcode;
        int8_t r1, r2;
        int8_t imm;//immediate value for any immediate instructions
        int8_t val1, val2;  // actual register values tho feel like they should be signed bits instead of unsigned
        int valid;
        short int pc;
    } ID_EX;

int main() {
    int clock_cycle = 0;
    short int instructionMemory[1024];
    int8_t dataMemory[2048];
    int8_t R[64]; //REGISTERS
    int8_t SREG;  // flags
    short int PC=0;  // program counter
    IF_ID if_id = {0};
    ID_EX id_ex = {0};
    int program_size = loadProgram("assembly_code.txt", &instructionMemory);
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
                result = id_ex->val1 + id_ex->val2;
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
                if((result & 0xFF) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                }
                if(id_ex->val1 + id_ex->val2 < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                }
                if((*SREG && 0x00) >> 3 ^ (*SREG && 0x00) >> 2) { // Check for sign change between the 3rd and 4th bits to set the sign flag
                    *SREG |= 0x02; // Set sign flag
                }
                id_ex->val1 = id_ex->val1 + id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                break;
            case SUB://check for overflow,sign,negative,zero flags will do later
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
                if((result & 0xFF) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                }
                 if((*SREG&& 0x00) >> 3 ^ (*SREG&& 0x00) >> 2) { // Check for sign change between the 3rd and 4th bits to set the sign flag
                    *SREG |= 0x02; // Set sign flag
                }
                id_ex->val1 = id_ex->val1 - id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                break;
            case MUL:
                result = id_ex->val1 * id_ex->val2;
                if(id_ex->val1 * id_ex->val2 < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                }
                if((result & 0xFF) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                }
                id_ex->val1 = id_ex->val1 * id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                break;
            case EOR:
                if(id_ex->val1 ^ id_ex->val2 < 0) { // Check for negative result
                    *SREG |= 0x04; // Set negative flag
                }
                if((id_ex->val1 ^ id_ex->val2) == 0) { // Check for zero result
                    *SREG |= 0x01; // Set zero flag
                }
                id_ex->val1 = id_ex->val1 ^ id_ex->val2;
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file
                break;
            case MOVI:
                id_ex->val1 = id_ex->imm; // Move immediate value to val1
                R[id_ex->r1] = id_ex->val1; // Write the immediate value to the register file
                break;
            case BEQZ:
                if(id_ex->val1 == 0) {
                    // Branch to the address specified by imm if val1 is zero
                    *PC = id_ex->pc + id_ex->imm;
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
                *PC = ((id_ex->val1 & 0xFF) << 8) | (id_ex->val2 & 0xFF);
                if_id->valid = 0; // Invalidate the instruction in the IF/ID pipeline register since we are branching
                break;
            case SLC:
                id_ex->val1 = id_ex->val1 << id_ex->imm; // Shift left logical by imm bits
                R[id_ex->r1] = id_ex->val1; // Write the result back to the register file   
                break;
            case SRC:
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
        id_ex->pc = if_id->pc; 
        id_ex->opcode = (if_id->instruction >> 12) & 0xF; // Extract opcode
        if(id_ex->opcode == ADD || id_ex->opcode == SUB || id_ex->opcode == MUL ||id_ex->opcode == EOR || id_ex->opcode == BR) {
            id_ex->r1 = (if_id->instruction >> 6) & 0x3F; // Extract r1
            id_ex->r2 = if_id->instruction & 0x3F; // Extract r2
            id_ex->val1 = R[id_ex->r1]; // Read value of r1
            id_ex->val2 = R[id_ex->r2]; // Read value of r2

        } else if(id_ex->opcode == MOVI || id_ex->opcode == BEQZ || id_ex->opcode == SLC || id_ex->opcode == SRC || id_ex->opcode == ANDI) {
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
            if_id->pc = *PC;
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
 
        /* ── Normalise the line ─────────────────────────────────────────── */
        line[strcspn(line, "\r\n")] = '\0';   /* strip CR / LF */
        trim(line);
 
        /* Skip blank lines and comment lines */
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';')
            continue;
 
        /* Remove inline comments (everything from '#' or ';' onward) */
        char *commentPos = strpbrk(line, "#;");
        if (commentPos) {
            *commentPos = '\0';
            trim(line);
        }
        if (line[0] == '\0') continue;
 
        /* ── Tokenise: MNEMONIC  OP1  OP2 ──────────────────────────────── */
        char mnemonic[16] = {0};
        char op1[16]      = {0};
        char op2[16]      = {0};
 
        int fieldCount = sscanf(line, "%15s %15s %15s", mnemonic, op1, op2);
        if (fieldCount < 1) continue;
 
        /* Convert mnemonic to upper-case for case-insensitive matching */
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
 
        /* ── Encode into 16-bit word ────────────────────────────────────── */
        uint16_t encoded = 0;
 
        if (isRFormat(opcode)) {
            /*
             * R-Format: [opcode 4][R1 6][R2 6]
             * Both operands are registers.
             */
            int r1 = parseRegister(op1);
            int r2 = parseRegister(op2);
 
            /* Validate register range 0-63 */
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
            /*
             * I-Format: [opcode 4][R1 6][IMM 6]
             * Immediate is a signed 6-bit value (2's complement).
             * We mask to 6 bits so negative values are stored correctly.
             *
             * Exception: SLC / SRC immediates are always positive (per spec).
             */
            int r1  = parseRegister(op1);
            int imm = parseImmediate(op2);
 
            if (r1 < 0 || r1 > 63) {
                fprintf(stderr,
                        "Warning: register out of range in '%s' — skipped\n",
                        line);
                continue;
            }
 
            /* Clamp/warn if immediate can't fit in 6 signed bits (-32..31) */
            if (imm < -32 || imm > 63) {          /* 63 covers unsigned ADDRESS */
                fprintf(stderr,
                        "Warning: immediate %d in '%s' may not fit in 6 bits\n",
                        imm, line);
            }
 
            encoded = (uint16_t)(((opcode & 0xF) << 12) |
                                 ((r1     & 0x3F) <<  6) |
                                  (imm    & 0x3F));       /* mask keeps 6 LSBs */
        }
 
        instructionMemory[pc] = (short int)encoded;
 
        /* Diagnostic print so you can verify the encoding */
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
    return pc;   /* caller uses this as program_size */
}

static void trim(char *s) {
    /* trailing */
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1]))
        s[--len] = '\0';
    /* leading */
    int i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i > 0) memmove(s, s + i, len - i + 1);
}
 
/* "R5" -> 5,  "R63" -> 63 */
static int parseRegister(const char *tok) {
    if (tok[0] == 'R' || tok[0] == 'r')
        return atoi(tok + 1);
    return atoi(tok);          /* fallback: bare number */
}
 
/* Signed decimal -> int  (handles negative immediates) */
static int parseImmediate(const char *tok) {
    return atoi(tok);
}
 
/* Return opcode 0-11, or -1 if unknown */
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
 
/* R-format instructions use two register operands */
static int isRFormat(int opcode) {
    return (opcode == ADD || opcode == SUB || opcode == MUL ||
            opcode == EOR || opcode == BR);
}