.text
.globl _start
_start:
    # Assuming a0 contains the IEEE 754 representation of the float
    li a0, 0x40490FDB     # Load float (3.14159) into a0 (IEEE 754 representation)
    li a7, 2              # Load ecall number for print_float into a7
    ecall                 # Make ecall

    li a7, 10             # Load ecall number for exit into a7
    ecall                 # Exit the program
