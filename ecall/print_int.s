.text
.globl _start
_start:
    li a0, 12345          # Load integer to print into a0
    li a7, 1              # Load ecall number for print_int into a7
    ecall                 # Make ecall

    li a7, 10             # Load ecall number for exit into a7
    ecall                 # Exit the program