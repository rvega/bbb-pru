#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM 0

#ifndef START_ADDR
#error "START_ADDR must be defined"
#endif

/////////////////////////////////////////////////////////////////////
// Globals
//

unsigned int* shared_ram = NULL;
pthread_t thread;
int finish = 0;


/////////////////////////////////////////////////////////////////////


void load_device_tree_overlay(){
   // Check if device tree overlay is loaded, load if needed.
   int device_tree_overlay_loaded = 0; 
   FILE* f;
   f = fopen("/sys/devices/bone_capemgr.9/slots","rt");
   if(f==NULL){
      printf("Initialisation failed (fopen rt)");
      exit(1);
   }
   char line[256];
   while(fgets(line, 256, f) != NULL){
      if(strstr(line, "PRU-DTO") != NULL){
         device_tree_overlay_loaded = 1; 
      }
   }
   fclose(f);

   if(!device_tree_overlay_loaded){
      f = fopen("/sys/devices/bone_capemgr.9/slots","w");
      if(f==NULL){
         printf("Initialisation failed (fopen)");
         exit(1);
      }
      fprintf(f, "PRU-DTO");
      fclose(f);
      usleep(100000);
   }
}

void init_pru_program(){
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
   prussdrv_init();
   prussdrv_open(PRU_EVTOUT_0);
   prussdrv_pruintc_init(&pruss_intc_initdata);

   prussdrv_load_datafile(PRU_NUM, "./data.bin");
   prussdrv_exec_program_at(PRU_NUM, "./text.bin", START_ADDR);
}

void signal_handler(int signal){
   finish = 1;
}

void* threaded_function(void* param){
   printf("Started thread\n");
   /* struct timeval time; */
   /* unsigned long t1=0; */
   /* unsigned long t2=0; */
   while(1){
      /* gettimeofday(&time,NULL); */
      /* t2 = t1; */
      /* t1 = time.tv_usec + time.tv_sec*1000000; */
      /* printf("Time: %lu \n", t1-t2); */

      // Wait for interrupt from PRU
      prussdrv_pru_wait_event(PRU_EVTOUT_0);
      prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

      // Read number in shared ram
      printf("Got input: %u %u \n", shared_ram[0], shared_ram[1]);
   }

   return NULL;
}

void start_thread(){
   pthread_attr_t attr;
   if(pthread_attr_init(&attr)){
      printf("Cannot start a new thread.\n");
      exit(1);
   }
   if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)){
      printf("Cannot start a new thread.");
      exit(1);
   }
   if(pthread_create(&thread, &attr, &threaded_function, NULL)){
      printf("Cannot start a new thread.");
      exit(1);
   }
}

void stop_thread(){
   while(pthread_cancel(thread)){
      printf("Stopping thread");
   }
   printf("Stopped thread");
}

int main(int argc, const char *argv[]){
   printf("\n\n");

   // Listen to SIGINT signals (program termination)
   signal(SIGINT, signal_handler);

   // Load device tree overlay to enable PRU hardware.
   load_device_tree_overlay();

   // Load and run binary into pru0
   init_pru_program();

   // Get pointer to shared ram
   void* p;
   prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &p);
   shared_ram = (unsigned int*)p;

   // Start input polling thread
   start_thread();

   while(!finish){
      sleep(1);
   }

   printf("Disabling PRU.\n");
   prussdrv_pru_disable(PRU_NUM);
   prussdrv_exit ();

   stop_thread();

   return 0;
}
