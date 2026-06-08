#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

typedef int(*opcode_function_t)(unsigned char, unsigned char);

#define OPCODE_ADD 0
#define OPCODE_SUB 1
#define OPCODE_MUL 2
#define OPCODE_DIV 3
#define OPCODE_MOD 4
#define OPCODE_STP 5
#define OPCODE_LDI 6
#define OPCODE_ADR 7
#define OPCODE_SUR 8
#define OPCODE_INC 9
#define OPCODE_DEC 10
#define OPCODE_JMP 11
#define OPCODE_CMP 12

#define OPCODE_JE  13
#define OPCODE_JNE 14

#define OPCODE_JL  15
#define OPCODE_JG  16
#define OPCODE_JLE 17
#define OPCODE_JGE 18

#define OPCODE_LDM 19
#define OPCODE_STI 20
#define OPCODE_STR 21

#define OPCODE_ADM 22 
#define OPCODE_SUBM 23
#define OPCODE_MUM 24
#define OPCODE_DUM 25


#define PROGRAM_SIZE          sizeof(mem_program)
#define INSTRUCTIONS_COUNT    26
#define INSTRUCTION_SIZE      3
#define DATA_SIZE 9
#define PROGRAM_BASE DATA_SIZE

#define LEFT_OPERAND  IP + 1
#define RIGHT_OPERAND IP + 2

#define RX_COUNT 4


static unsigned char mem_program [] = {
    29, 15, 32, 1, 22, 6, 12, 9, 10,
  /* 09 */  0, 127, 127,    /* ADD 9, 7   */
  /* 12 */  1, 8, 4,    /* SUB 8, 4   */
  /* 15 */  6, 3, 10,   /* LDI R3, 10 */
  /* 18 */  22,4, 3,    /* ADM M(4),R3 */
  /* 21 */  0, 5, 8,    /* ADD 5, 8   */
  /* 24 */  6, 2, 77,   /* LDI R2, 77 */
  /* 27 */  7, 2, 23,   /* ADR R2, 23 */
  /* 30 */  8, 2, 5,    /* SUR R2, 5  */
  /* 33 */  2, 3, 6,    /* MUL 3, 6   */
  /* 36 */  3, 9, 3,    /* DIV 9, 3   */
  /* 39 */  4, 7, 2,    /* MOD 7, 2   */
  /* 42 */  6, 0, 10,   /* LDI R0, 10 */
  /* 45 */  6, 1, 20,   /* LDI R1, 20 */
  /* 48 */  11, 63, 0,  /* JMP 63     */
  /* 51 */  6, 0, 70,   /* LDI R0, 70 */
  /* 54 */  9, 0, 0,    /* INC R0     */
  /* 57 */  10, 0, 0,   /* DEC R0     */
  /* 60 */  12, 0, 1,   /* CMP R0, R1 */
  /* 63 */  15, 51, 0,  /* JL  51     */
  /* 66 */  5, 0, 0     /* STP 0      */
};



/* Registers */
static int IP = PROGRAM_BASE;
static unsigned char IR[INSTRUCTION_SIZE] = {0, 0, 0};
static int OUTPUT = 0;

static unsigned char FLAGS = 0;

#define FLAG_NOT_EQUAL 0
#define FLAG_EQUAL 1 // ==
#define FLAG_GREATER 2 // >
#define FLAG_LESS     4 // <
#define FLAG_OVERFLOW 8
// #define FLAG_GREATER_EQUAL 8 // >=
// #define FLAG_LESS_EQUAL 16 // <=

/* 0000 0000 */
/* xxxO LGEN */

static unsigned char R0 = 0;
static unsigned char R1 = 0;
static unsigned char R2 = 0;
static unsigned char R3 = 0;


static opcode_function_t current_instruction = NULL;

bool overflow_occured(int a, int b) {
    if(a > (CHAR_MAX - b)) {
        FLAGS += FLAG_OVERFLOW;
        printf("Overflow Occured\n");
        return true;
    }
    return false;
}

int opcode_add(unsigned char left_operand, unsigned char right_operand){
    int operand_op_result = left_operand + right_operand;
    overflow_occured(left_operand, right_operand);
    return operand_op_result;
}

int opcode_sub(unsigned char left_operand, unsigned char right_operand){
    int operand_op_result = left_operand - right_operand;
    overflow_occured(left_operand, right_operand);
    return operand_op_result;
}

int opcode_mul(unsigned char left_operand, unsigned char right_operand){
    int operand_op_result = left_operand * right_operand;
    overflow_occured(left_operand, right_operand);
    return operand_op_result;
}

int opcode_div(unsigned char left_operand, unsigned char right_operand){
    if(right_operand == 0){
        printf("Exception: divide by zero\n");
        return false;
    }
    int operand_op_result = left_operand / right_operand;
    overflow_occured(left_operand, right_operand);
    return operand_op_result;
}

int opcode_mod(unsigned char left_operand, unsigned char right_operand){
    int operand_op_result = left_operand % right_operand;
    overflow_occured(left_operand, right_operand);
    return operand_op_result;
}

static unsigned char get_rx_value(unsigned char operand){
     switch(operand){
        case 0: return R0;
        case 1: return R1;
        case 2: return R2;
        case 3: return R3;
        // skip default case
    }

    return 255;
}

static void set_rx_value(unsigned char operand, unsigned char value){
     switch(operand){
        case 0: R0 = value; return;
        case 1: R1 = value; return;
        case 2: R2 = value; return;
        case 3: R3 = value; return;
        // skip default case
    }
}

int opcode_ldi(unsigned char left_operand, unsigned char right_operand){
    switch(left_operand){
        case 0: R0 = right_operand; break;
        case 1: R1 = right_operand; break;
        case 2: R2 = right_operand; break;
        case 3: R3 = right_operand; break;
        default:
            printf("Invalid register address\n");
            exit(0);
    }

    return right_operand;
}

int get_value_memory_address(unsigned char idx) {
    if(idx > DATA_SIZE-1) {
        printf("Invalid memory address\n");
        exit(0);
    }
    return mem_program[idx];
}

int opcode_sti(unsigned char left_operand, unsigned char right_operand){
    switch(DATA_SIZE-1){
        case 0: mem_program[0] = right_operand; break;
        case 1: mem_program[1] = right_operand; break;
        case 2: mem_program[2] = right_operand; break;
        case 3: mem_program[3] = right_operand; break;
        case 4: mem_program[4] = right_operand; break;
        case 5: mem_program[5] = right_operand; break;
        case 6: mem_program[6] = right_operand; break;
        case 7: mem_program[7] = right_operand; break;
        case 8: mem_program[8] = right_operand; break;
        default:
            printf("Invalid memory address\n");
            exit(0);
    }

    return right_operand;
}

int opcode_str(unsigned char left_operand, unsigned char right_operand){
    switch(DATA_SIZE-1){
        case 0: mem_program[0] = get_rx_value(right_operand); break;
        case 1: mem_program[1] = get_rx_value(right_operand); break;
        case 2: mem_program[2] = get_rx_value(right_operand); break;
        case 3: mem_program[3] = get_rx_value(right_operand); break;
        case 4: mem_program[4] = get_rx_value(right_operand); break;
        case 5: mem_program[5] = get_rx_value(right_operand); break;
        case 6: mem_program[6] = get_rx_value(right_operand); break;
        case 7: mem_program[7] = get_rx_value(right_operand); break;
        case 8: mem_program[8] = get_rx_value(right_operand); break;
        default:
            printf("Invalid memory address\n");
            exit(0);
    }

    return right_operand;
}

int opcode_ldm(unsigned char left_operand, unsigned char right_operand){
    switch(left_operand){
        case 0: R0 = get_value_memory_address(right_operand); break;
        case 1: R1 = get_value_memory_address(right_operand); break;
        case 2: R2 = get_value_memory_address(right_operand); break;
        case 3: R3 = get_value_memory_address(right_operand); break;
        default:
            printf("Invalid register address\n");
            exit(0);
    }

    return right_operand;
}

int opcode_stp(unsigned char left_operand, unsigned char right_operand){
    OUTPUT = left_operand;
    exit(left_operand);
}

int opcode_adr(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand) + right_operand;
    overflow_occured(left_operand, right_operand);
    set_rx_value(left_operand, Rx);
    return Rx;
}

int opcode_sur(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand) - right_operand;
    overflow_occured(left_operand, right_operand);
    set_rx_value(left_operand, Rx);
    return Rx;
}


int opcode_inc(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand);
    Rx++;
    set_rx_value(left_operand, Rx);
    return Rx;
}

int opcode_dec(unsigned char left_operand, unsigned char right_operand){
    if(left_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = get_rx_value(left_operand);
    Rx--;
    set_rx_value(left_operand, Rx);
    return Rx;
}

int opcode_jmp(unsigned char left_operand, unsigned char right_operand){
    IP = left_operand;
    return left_operand;
}

int opcode_cmp(unsigned char left_operand, unsigned char right_operand){
    unsigned char Rx = get_rx_value(left_operand);
    unsigned char Ry = get_rx_value(right_operand);

    OUTPUT = Rx - Ry;
    FLAGS = 0; // Rx != Ry
    
    if(OUTPUT == 0) {
        FLAGS = FLAG_EQUAL;
    } 
    if(OUTPUT > 0) {
        FLAGS = FLAG_GREATER;
    } 
    if(OUTPUT < 0) {
        FLAGS = FLAG_LESS;
    } 

    return FLAGS;
}

int opcode_je(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_EQUAL){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jne(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_NOT_EQUAL){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jg(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_GREATER){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jl(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_LESS){
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jle(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_LESS){
        IP = left_operand;
        return left_operand;
    } else if(FLAGS == FLAG_EQUAL) {
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_jge(unsigned char left_operand, unsigned char right_operand){

    if(FLAGS == FLAG_GREATER){
        IP = left_operand;
        return left_operand;
    } else if(FLAGS == FLAG_EQUAL) {
        IP = left_operand;
        return left_operand;
    }
    
    return 255;
}

int opcode_adm(unsigned char left_operand, unsigned char right_operand){
    if(right_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = mem_program[left_operand] + get_rx_value(right_operand);
    overflow_occured(left_operand, right_operand);
    set_rx_value(right_operand, Rx);
    return Rx;
}

int opcode_subm(unsigned char left_operand, unsigned char right_operand){
    if(right_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = mem_program[left_operand] - get_rx_value(right_operand);
    overflow_occured(left_operand, right_operand);
    set_rx_value(right_operand, Rx);
    return Rx;
}

int opcode_mum(unsigned char left_operand, unsigned char right_operand){
    if(right_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    unsigned char Rx = mem_program[left_operand] * get_rx_value(right_operand);
    overflow_occured(left_operand, right_operand);
    set_rx_value(right_operand, Rx);
    return Rx;
}

int opcode_dum(unsigned char left_operand, unsigned char right_operand){
    if(right_operand >= RX_COUNT){
        printf("Invlaid Rx register address\n");
        exit(0);
    }

    if(right_operand == 0){
        printf("Exception: divide by zero\n");
        return false;
    }

    unsigned char Rx = get_rx_value(right_operand);
    overflow_occured(left_operand, right_operand);
    set_rx_value(right_operand, mem_program[left_operand]/Rx);
    return Rx;
}

#define OPCODE_LDM 19
#define OPCODE_STI 20
#define OPCODE_STR 21

#define OPCODE_ADM 22 
#define OPCODE_SUBM 23
#define OPCODE_MUM 24
#define OPCODE_DUM 25

static const opcode_function_t opcode_functions[INSTRUCTIONS_COUNT] = {
        opcode_add, opcode_sub, opcode_mul,
        opcode_div, opcode_mod, opcode_stp,
        opcode_ldi, opcode_adr, opcode_sur,
        opcode_inc, opcode_dec, opcode_jmp,
        opcode_cmp, opcode_je,  opcode_jne,
        opcode_jl,  opcode_jg,  opcode_jle,
        opcode_jge, opcode_ldm, opcode_sti,
        opcode_str, opcode_adm, opcode_subm,
        opcode_mum, opcode_dum

};


static bool cpu_fetch(void){
    int byte = 0;
    if(IP >= PROGRAM_SIZE){
        /* HALT */
        exit(0);
    }

    /* Fetch */
    IR[0] = mem_program[IP];
    IR[1] = mem_program[LEFT_OPERAND];
    IR[2] = mem_program[RIGHT_OPERAND];

    /* Move to the next insturction */
    IP += INSTRUCTION_SIZE;
    
    return true;
}

static bool cpu_decode(void){
    if(IR[0] >= INSTRUCTIONS_COUNT){
        printf("Invalid instruction\n");
        // IR[0] = 10; // rewrite
        IR[0] = IR[1] = IR[2] = 0;
        exit(0);
    }

    current_instruction = opcode_functions[IR[0]];

    return true;
}

static bool cpu_execute(void){
    OUTPUT = current_instruction(IR[1], IR[2]);
    return true;
}

int main(void){
    // reset
    IP = PROGRAM_BASE;
    IR[0] = IR[1] = IR[2] = 0;

    while(true){
        cpu_fetch();
        cpu_decode();
        cpu_execute();
        printf("output -> %d\n", OUTPUT);
    }

    return 0;
}