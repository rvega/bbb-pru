#include <stdio.h>
#include <pthread.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM 0

mutex lock;
int finish;

void* interrupt_thread(void* arg){
   while(1){
      prussdrv_pru_wait_event(PRU_EVTOUT_0);

      finish = 1;

      prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
   }
}


int main(int argc, const char *argv[]){
   finish = 0;
   printf("Starting \n");
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
   prussdrv_init();
   prussdrv_open(PRU_EVTOUT_0);
   prussdrv_pruintc_init(&pruss_intc_initdata);
   prussdrv_exec_program (PRU_NUM, "./pru.bin");

   printf("Waiting\n");
   /* prussdrv_pru_wait_event(PRU_EVTOUT_0); */

   // TODO: use shced_get_priority_max and sched_priority_min to set 
   // the correct priority for the thread
   prussdrv_start_irqthread(PRU_EVTOUT_0, sched_get_priority_max(SCHED_FIFO)-2, interrupt_thread);

   while(!finish){
      sleep(1); 
   }

   //  Disable PRU and close memory mappings
   prussdrv_pru_disable(PRU_NUM);
   prussdrv_exit ();
   printf("Done\n");

   
   return 0;
}
