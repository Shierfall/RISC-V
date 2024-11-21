#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define NUM_REGISTERS 32
#define MEMORY_SIZE (1024 * 1024)

#define OPCODE_LUI       0x37
#define OPCODE_AUIPC     0x17
#define OPCODE_JAL       0x6F
#define OPCODE_JALR      0x67
#define OPCODE_BRANCH    0x63
#define OPCODE_LOAD      0x03
#define OPCODE_STORE     0x23
#define OPCODE_OP_IMM    0x13
#define OPCODE_OP        0x33
#define OPCODE_SYSTEM    0x73

#define FUNCT3_BEQ       0x0
#define FUNCT3_BNE       0x1
#define FUNCT3_BLT       0x4
#define FUNCT3_BGE       0x5
#define FUNCT3_BLTU      0x6
#define FUNCT3_BGEU      0x7

#define FUNCT3_LB        0x0
#define FUNCT3_LH        0x1
#define FUNCT3_LW        0x2
#define FUNCT3_LBU       0x4
#define FUNCT3_LHU       0x5

#define FUNCT3_SB        0x0
#define FUNCT3_SH        0x1
#define FUNCT3_SW        0x2

#define FUNCT3_ADDI      0x0
#define FUNCT3_SLLI      0x1
#define FUNCT3_SLTI      0x2
#define FUNCT3_SLTIU     0x3
#define FUNCT3_XORI      0x4
#define FUNCT3_SRLI_SRAI 0x5
#define FUNCT3_ORI       0x6
#define FUNCT3_ANDI      0x7

#define FUNCT3_ADD_SUB   0x0
#define FUNCT3_SLL       0x1
#define FUNCT3_SLT       0x2
#define FUNCT3_SLTU      0x3
#define FUNCT3_XOR       0x4
#define FUNCT3_SRL_SRA   0x5
#define FUNCT3_OR        0x6
#define FUNCT3_AND       0x7

#define FUNCT7_ADD       0x00
#define FUNCT7_SUB       0x20
#define FUNCT7_SLLI      0x00
#define FUNCT7_SRLI      0x00
#define FUNCT7_SRAI      0x20

#define SYSTEM_ECALL     0x000
#define SYSTEM_EBREAK    0x001

int32_t registers_array[NUM_REGISTERS];
uint32_t pc = 0;
uint8_t memory[MEMORY_SIZE];

#define GET_OPCODE(instr)          (instr & 0x7F)
#define GET_RD(instr)              ((instr >> 7) & 0x1F)
#define GET_FUNCT3(instr)          ((instr >> 12) & 0x7)
#define GET_RS1(instr)             ((instr >> 15) & 0x1F)
#define GET_RS2(instr)             ((instr >> 20) & 0x1F)
#define GET_FUNCT7(instr)          ((instr >> 25) & 0x7F)

int32_t sign_extend(int32_t value, int bits) {
    int32_t mask = 1 << (bits - 1);
    return (value ^ mask) - mask;
}

void initialize() {
    memset(registers_array, 0, sizeof(registers_array));
    memset(memory, 0, sizeof(memory));
    pc = 0;
    registers_array[2] = MEMORY_SIZE;
}

void load_memory(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open binary file");
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(memory, 1, MEMORY_SIZE, file);
    fclose(file);

    if (bytes_read > MEMORY_SIZE) {
        printf("Binary file is too large to fit in memory.\n");
        exit(EXIT_FAILURE);
    }

    if (bytes_read % 4 != 0) {
        printf("Warning: File size is not a multiple of 4 bytes.\n");
    }
}

void print_registers() {
    printf("\n--- Register Contents ---\n");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("x%02d = %d (0x%08X)\n", i, registers_array[i], (uint32_t)registers_array[i]);
    }
    printf("--------------------------\n");
}

void dump_registers_binary() {
    FILE *file = fopen("register_dump.bin", "wb");
    if (!file) {
        perror("Failed to open register dump file");
        exit(EXIT_FAILURE);
    }
    fwrite(registers_array, sizeof(int32_t), NUM_REGISTERS, file);
    fclose(file);
}

void execute_r_type(uint32_t funct7, uint32_t funct3, uint32_t rd, uint32_t rs1, uint32_t rs2) {
    int32_t operand1 = registers_array[rs1];
    int32_t operand2 = registers_array[rs2];
    int32_t result = 0;

    switch (funct3) {
        case FUNCT3_ADD_SUB:
            if (funct7 == FUNCT7_ADD) {
                result = operand1 + operand2;
            } else if (funct7 == FUNCT7_SUB) {
                result = operand1 - operand2;
            } else {
                printf("Unsupported R-type funct7: 0x%X\n", funct7);
                exit(EXIT_FAILURE);
            }
            break;
        case FUNCT3_SLL:
            if (funct7 != FUNCT7_SLLI) {
                printf("Unsupported R-type funct7 for SLL: 0x%X\n", funct7);
                exit(EXIT_FAILURE);
            }
            result = operand1 << (operand2 & 0x1F);
            break;
        case FUNCT3_SLT:
            result = (operand1 < operand2) ? 1 : 0;
            break;
        case FUNCT3_SLTU:
            result = ((uint32_t)operand1 < (uint32_t)operand2) ? 1 : 0;
            break;
        case FUNCT3_XOR:
            result = operand1 ^ operand2;
            break;
        case FUNCT3_SRL_SRA:
            if (funct7 == FUNCT7_SRLI) {
                result = ((uint32_t)operand1) >> (operand2 & 0x1F);
            } else if (funct7 == FUNCT7_SRAI) {
                result = operand1 >> (operand2 & 0x1F);
            } else {
                printf("Unsupported R-type funct7 for SRL/SRA: 0x%X\n", funct7);
                exit(EXIT_FAILURE);
            }
            break;
        case FUNCT3_OR:
            result = operand1 | operand2;
            break;
        case FUNCT3_AND:
            result = operand1 & operand2;
            break;
        default:
            printf("Unsupported R-type funct3: 0x%X\n", funct3);
            exit(EXIT_FAILURE);
    }

    registers_array[rd] = result;
    registers_array[0] = 0;
}

void execute_i_type(uint32_t funct7, uint32_t funct3, uint32_t rd, uint32_t rs1, int32_t imm) {
    int32_t operand1 = registers_array[rs1];
    int32_t result = 0;

    switch (funct3) {
        case FUNCT3_ADDI:
            result = operand1 + imm;
            break;
        case FUNCT3_SLLI:
            if (funct7 != FUNCT7_SLLI) {
                printf("Unsupported I-type funct7 for SLLI: 0x%X\n", funct7);
                exit(EXIT_FAILURE);
            }
            result = operand1 << (imm & 0x1F);
            break;
        case FUNCT3_SLTI:
            result = (operand1 < imm) ? 1 : 0;
            break;
        case FUNCT3_SLTIU:
            result = ((uint32_t)operand1 < (uint32_t)imm) ? 1 : 0;
            break;
        case FUNCT3_XORI:
            result = operand1 ^ imm;
            break;
        case FUNCT3_SRLI_SRAI:
            if (funct7 == FUNCT7_SRLI) {
                result = ((uint32_t)operand1) >> (imm & 0x1F);
            } else if (funct7 == FUNCT7_SRAI) {
                result = operand1 >> (imm & 0x1F);
            } else {
                printf("Unsupported I-type funct7 for SRLI/SRAI: 0x%X\n", funct7);
                exit(EXIT_FAILURE);
            }
            break;
        case FUNCT3_ORI:
            result = operand1 | imm;
            break;
        case FUNCT3_ANDI:
            result = operand1 & imm;
            break;
        default:
            printf("Unsupported I-type funct3: 0x%X\n", funct3);
            exit(EXIT_FAILURE);
    }

    registers_array[rd] = result;
    registers_array[0] = 0;
}

void execute_s_type(uint32_t funct3, uint32_t rs1, uint32_t rs2, int32_t imm) {
    int32_t address = registers_array[rs1] + imm;
    int32_t value = registers_array[rs2];
    int access_size = 4;

    switch (funct3) {
        case FUNCT3_SB:
            access_size = 1;
            break;
        case FUNCT3_SH:
            access_size = 2;
            break;
        case FUNCT3_SW:
            access_size = 4;
            break;
        default:
            printf("Unsupported S-type funct3: 0x%X\n", funct3);
            exit(EXIT_FAILURE);
    }

    if (address < 0 || address + access_size - 1 >= MEMORY_SIZE) {
        printf("Memory access out of bounds at address 0x%X\n", address);
        exit(EXIT_FAILURE);
    }

    if (address % access_size != 0) {
        printf("Misaligned memory access at address 0x%X\n", address);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < access_size; i++) {
        memory[address + i] = (value >> (8 * i)) & 0xFF;
    }
}

void execute_b_type(uint32_t funct3, uint32_t rs1, uint32_t rs2, int32_t imm) {
    int32_t operand1 = registers_array[rs1];
    int32_t operand2 = registers_array[rs2];
    int branch = 0;

    switch (funct3) {
        case FUNCT3_BEQ:
            if (operand1 == operand2) branch = 1;
            break;
        case FUNCT3_BNE:
            if (operand1 != operand2) branch = 1;
            break;
        case FUNCT3_BLT:
            if (operand1 < operand2) branch = 1;
            break;
        case FUNCT3_BGE:
            if (operand1 >= operand2) branch = 1;
            break;
        case FUNCT3_BLTU:
            if ((uint32_t)operand1 < (uint32_t)operand2) branch = 1;
            break;
        case FUNCT3_BGEU:
            if ((uint32_t)operand1 >= (uint32_t)operand2) branch = 1;
            break;
        default:
            printf("Unsupported B-type funct3: 0x%X\n", funct3);
            exit(EXIT_FAILURE);
    }

    if (branch) {
        pc += imm;
    } else {
        pc += 4;
    }
}

void execute_u_type(uint32_t opcode, uint32_t rd, int32_t imm) {
    switch (opcode) {
        case OPCODE_LUI:
            registers_array[rd] = imm;
            break;
        case OPCODE_AUIPC:
            registers_array[rd] = pc + imm;
            break;
        default:
            printf("Unsupported U-type opcode: 0x%X\n", opcode);
            exit(EXIT_FAILURE);
    }

    registers_array[0] = 0;
}

void execute_j_type(uint32_t rd, int32_t imm) {
    registers_array[rd] = pc + 4;
    pc += imm;
}

void execute_jalr(uint32_t rd, uint32_t rs1, int32_t imm) {
    int32_t target = registers_array[rs1] + imm;
    target &= ~1;
    registers_array[rd] = pc + 4;
    pc = target;
}

void handle_system_call(uint32_t instruction) {
    uint32_t funct = (instruction >> 20) & 0xFFF;

    switch (funct) {
        case SYSTEM_ECALL:
            printf("ECALL encountered at PC: 0x%08X\n", pc);
            print_registers();
            dump_registers_binary();
            exit(EXIT_SUCCESS);
            break;
        case SYSTEM_EBREAK:
            printf("EBREAK encountered at PC: 0x%08X\n", pc);
            print_registers();
            dump_registers_binary();
            exit(EXIT_SUCCESS);
            break;
        default:
            printf("Unsupported SYSTEM instruction funct: 0x%X at PC: 0x%08X\n", funct, pc);
            exit(EXIT_FAILURE);
    }
}

void execute_instruction(uint32_t instruction) {
    uint32_t opcode = GET_OPCODE(instruction);
    uint32_t rd = GET_RD(instruction);
    uint32_t funct3 = GET_FUNCT3(instruction);
    uint32_t rs1 = GET_RS1(instruction);
    uint32_t rs2 = GET_RS2(instruction);
    uint32_t funct7 = GET_FUNCT7(instruction);
    int32_t imm = 0;

    switch (opcode) {
        case OPCODE_LUI:
        case OPCODE_AUIPC: {
            imm = instruction & 0xFFFFF000;
            execute_u_type(opcode, rd, imm);
            pc += 4;
            break;
        }
        case OPCODE_JAL: {
            imm = ((instruction >> 21) & 0x3FF) << 1;
            imm |= ((instruction >> 20) & 0x1) << 11;
            imm |= ((instruction >> 12) & 0xFF) << 12;
            imm |= ((instruction >> 31) & 0x1) << 20;
            imm = sign_extend(imm, 21);
            execute_j_type(rd, imm);
            break;
        }
        case OPCODE_JALR: {
            imm = sign_extend((instruction >> 20) & 0xFFF, 12);
            execute_jalr(rd, rs1, imm);
            break;
        }
        case OPCODE_BRANCH: {
            imm = ((instruction >> 7) & 0x1) << 11;
            imm |= ((instruction >> 8) & 0xF) << 1;
            imm |= ((instruction >> 25) & 0x3F) << 5;
            imm |= ((instruction >> 31) & 0x1) << 12;
            imm = sign_extend(imm, 13);
            execute_b_type(funct3, rs1, rs2, imm);
            break;
        }
        case OPCODE_LOAD: {
            imm = sign_extend((instruction >> 20) & 0xFFF, 12);
            int32_t address = registers_array[rs1] + imm;
            int32_t loaded_value = 0;
            int access_size = 4;

            switch (funct3) {
                case FUNCT3_LB: {
                    access_size = 1;
                    if (address < 0 || address >= MEMORY_SIZE) {
                        printf("Memory access out of bounds at address 0x%X\n", address);
                        exit(EXIT_FAILURE);
                    }
                    loaded_value = (int8_t)memory[address];
                    break;
                }
                case FUNCT3_LH: {
                    access_size = 2;
                    if (address < 0 || address + 1 >= MEMORY_SIZE || address % 2 != 0) {
                        printf("Misaligned or out-of-bounds memory access at address 0x%X\n", address);
                        exit(EXIT_FAILURE);
                    }
                    loaded_value = (int16_t)(memory[address] | (memory[address + 1] << 8));
                    break;
                }
                case FUNCT3_LW: {
                    access_size = 4;
                    if (address < 0 || address + 3 >= MEMORY_SIZE || address % 4 != 0) {
                        printf("Misaligned or out-of-bounds memory access at address 0x%X\n", address);
                        exit(EXIT_FAILURE);
                    }
                    loaded_value = memory[address] |
                                   (memory[address + 1] << 8) |
                                   (memory[address + 2] << 16) |
                                   (memory[address + 3] << 24);
                    break;
                }
                case FUNCT3_LBU: {
                    access_size = 1;
                    if (address < 0 || address >= MEMORY_SIZE) {
                        printf("Memory access out of bounds at address 0x%X\n", address);
                        exit(EXIT_FAILURE);
                    }
                    loaded_value = memory[address];
                    break;
                }
                case FUNCT3_LHU: {
                    access_size = 2;
                    if (address < 0 || address + 1 >= MEMORY_SIZE || address % 2 != 0) {
                        printf("Misaligned or out-of-bounds memory access at address 0x%X\n", address);
                        exit(EXIT_FAILURE);
                    }
                    loaded_value = memory[address] | (memory[address + 1] << 8);
                    break;
                }
                default:
                    printf("Unsupported LOAD funct3: 0x%X\n", funct3);
                    exit(EXIT_FAILURE);
            }

            registers_array[rd] = loaded_value;
            registers_array[0] = 0;
            pc += 4;
            break;
        }
        case OPCODE_STORE: {
            int32_t imm_store = ((instruction >> 7) & 0x1F) | (((instruction >> 25) & 0x7F) << 5);
            imm_store = sign_extend(imm_store, 12);
            execute_s_type(funct3, rs1, rs2, imm_store);
            pc += 4;
            break;
        }
        case OPCODE_OP_IMM: {
            if (funct3 == FUNCT3_SLLI || funct3 == FUNCT3_SRLI_SRAI) {
                imm = (instruction >> 20) & 0x1F;
            } else {
                imm = sign_extend((instruction >> 20) & 0xFFF, 12);
            }
            execute_i_type(funct7, funct3, rd, rs1, imm);
            pc += 4;
            break;
        }
        case OPCODE_OP: {
            execute_r_type(funct7, funct3, rd, rs1, rs2);
            pc += 4;
            break;
        }
        case OPCODE_SYSTEM: {
            handle_system_call(instruction);
            pc += 4;
            break;
        }
        default:
            printf("Unsupported opcode: 0x%X at PC: 0x%08X\n", opcode, pc);
            exit(EXIT_FAILURE);
    }

    registers_array[0] = 0;
}

void run() {
    while (pc + 3 < MEMORY_SIZE) {
        uint32_t instruction = 0;
        instruction |= memory[pc];
        instruction |= memory[pc + 1] << 8;
        instruction |= memory[pc + 2] << 16;
        instruction |= memory[pc + 3] << 24;

        printf("PC = 0x%08X | Instruction = 0x%08X\n", pc, instruction);
        execute_instruction(instruction);
    }

    printf("Program execution completed.\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <binary_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    initialize();
    load_memory(argv[1]);
    run();
    print_registers();
    dump_registers_binary();

    return EXIT_SUCCESS;
}