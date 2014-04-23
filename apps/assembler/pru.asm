#define PRU0_ARM_INTERRUPT      19

.origin 0
.entrypoint START

START:
   MOV r31.b0, PRU0_ARM_INTERRUPT + 16
   HALT
