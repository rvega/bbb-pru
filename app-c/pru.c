// Macro for accessing a hardware register (32 bit)
#define HWREG(x) (*((volatile unsigned int *)(x)))


volatile register unsigned int __R31;

int main(int argc, const char *argv[]){
   // Shared ram is at address 0x10000.
   // See table 4.7 Local Memory Map in page 204 of manual
   unsigned int* shared_ram = (unsigned int *)0x10000;

   int i;
   unsigned char done = 0;
   unsigned int count = 0;
   while(count<10){
      // Delay
      for(i=0; i<10000000; i++);

      // Writing to register 31 sends interrupt requests to the 
      // ARM system. See section 4.4.1.2.2 Event Interface
      // Mapping (R31): PRU System Events in page 209 of manual. 
      // Not sure why vector output is 4 in this case but this is
      // what they do in
      __R31 = 35;

      shared_ram[0] = count;
      count ++;

      // If bit 30 of register 31 is set. That means someone sent 
      // an interrupt request to this PRU.
      if(__R31 & (1 << 30)){
         // Writing to the SICR register clears the interrupt 
         // request. The address of the register is 0x20024:
         // 0x20000 is the base address for INTC (interrupt
         // controller) registers (page 204) and 0x24 is the
         // offset for the SICR register (page 322, Table 4-102.
         // PRU_ICSS_INTC Registers. 
         // The value 21 is the event number sent from the ARM 
         // (linux) code. Again, not sure why they chose this 
         // number but that's what they do in other examples.
         HWREG(0x20024) = 21;
         count = shared_ram[1];
      }
   }


   // stop pru processing
   __halt(); 

   return 0;
}
