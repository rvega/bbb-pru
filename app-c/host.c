#include <stdio.h>
#include <pthread.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM 0

#ifndef START_ADDR
#error "START_ADDR must be defined"
#endif

int main(int argc, const char *argv[]){
   printf("Starting \n");
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
   prussdrv_init();
   prussdrv_open(PRU_EVTOUT_0);
   prussdrv_pruintc_init(&pruss_intc_initdata);

   prussdrv_load_datafile(PRU_NUM, "./data.bin");
   prussdrv_exec_program_at(PRU_NUM, "./text.bin", START_ADDR);

   printf("Waiting\n");
   prussdrv_pru_wait_event(PRU_EVTOUT_0);
   prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

   printf("Done\n");
   prussdrv_pru_disable(PRU_NUM);
   prussdrv_exit ();
   return 0;
}
