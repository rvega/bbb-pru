#define HWREG(x) (*((volatile unsigned int *)(x)))


volatile register unsigned int __R31;
volatile unsigned int* shared_ram = (volatile unsigned int *)0x10000;

void init_adc(){
   // Enable clock for adc module. CM_WKUP_ADC_TSK_CLKCTL register
   HWREG(0x44e004bc) = 0x02;

   // Disable ADC module temporarily. ADC_CTRL register
   HWREG(0x44e0d040) &= ~(0x01);

   // To calculate sample rate:
   // fs = 24MHz / (CLK_DIV*(OpenDly+Average(14+SampleDly)))
   // We want 48KHz
   unsigned int clock_divider = 1;
   unsigned int open_delay = 4;
   unsigned int average = 3;       // can be 0 (no average), 1 (2 samples), 
                                   // 2 (4 samples),  3 (8 samples) 
                                   // or 4 (16 samples)
   unsigned int sample_delay = 48;

   // Set clock divider (set register to desired value minus one). 
   // ADC_CLKDIV register
   HWREG(0x44e0d04c) = clock_divider - 1;

   // Set values range from 0 to FFF. ADCRANGE register
   HWREG(0x44e0d048) = (0xfff << 16) & (0x000);

   // Disable all steps. STEPENABLE register
   HWREG(0x44e0d054) &= ~(0xff);

   // Unlock step config register. ACD_CTRL register
   HWREG(0x44e0d040) |= (0x01 << 2);

   // Set config for step 1. Average 16, sw mode, continuous mode, 
   // use fifo0, use channel 0. STEPCONFIG1 register
   /* HWREG(0x44e0d064) = 0x0000; */
   HWREG(0x44e0d064) = ((0x0000) | (0x0<<26) | (0x00<<19) | (0x00<<15) | (average<<2) | (0x01));

   // Set delays for step 1. STEPDELAY1 register
   /* HWREG(0x44e0d068) = 0x0000; */
   HWREG(0x44e0d068) = ((0x0000) | (sample_delay - 1)<<24) | open_delay;

   /* // Set config for step 2. Average 16, sw mode, continuous mode,  */
   /* // use fifo1, use channel 1. STEPCONFIG2 register */
   /* HWREG(0x44e0d06c) |= ((0x1<<26) | (0x01<<19) | (0x01<<15) | (0x04<<2) | (0x01)); */

   /* // Set delays for step 2. STEPDELAY2 register */
   /* HWREG(0x44e0d070) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */

   /* // Set config for step 3. Average 16, sw mode, continuous mode,  */
   /* // use fifo0, use channel 2. STEPCONFIG3 register */
   /* HWREG(0x44e0d074) |= ((0x0<<26) | (0x02<<19) | (0x02<<15) | (0x04<<2) | (0x01)); */

   /* // Set delays for step 3. STEPDELAY3 register */
   /* HWREG(0x44e0d078) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */

   /* // Set config for step 4. Average 16, sw mode, continuous mode,  */
   /* // use fifo1, use channel 3. STEPCONFIG4 register */
   /* HWREG(0x44e0d07c) |= ((0x1<<26) | (0x03<<19) | (0x03<<15) | (0x04<<2) | (0x01)); */

   /* // Set delays for step 4. STEPDELAY4 register */
   /* HWREG(0x44e0d080) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */


   /* // Set config for step 5. Average 16, sw mode, continuous mode,  */
   /* // use fifo0, use channel 4. STEPCONFIG5 register */
   /* HWREG(0x44e0d084) |= ((0x0<<26) | (0x04<<19) | (0x04<<15) | (0x04<<2) | (0x01)); */

   /* // Set delays for step 5. STEPDELAY5 register */
   /* HWREG(0x44e0d088) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */

   /* // Set config for step 6. Average 16, sw mode, continuous mode,  */
   /* // use fifo1, use channel 5. STEPCONFIG6 register */
   /* HWREG(0x44e0d08c) |= ((0x1<<26) | (0x05<<19) | (0x05<<15) | (0x04<<2) | (0x01)); */

   /* // Set delays for step 6. STEPDELAY6 register */
   /* HWREG(0x44e0d090) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */

   /* // Set config for step 7. Average 16, sw mode, continuous mode,  */
   /* // use fifo0, use channel 6. STEPCONFIG7 register */
   /* HWREG(0x44e0d094) |= ((0x0<<26) | (0x06<<19) | (0x06<<15) | (0x04<<2) | (0x01)); */

   /* // Set delays for step 7. STEPDELAY7 register */
   /* HWREG(0x44e0d098) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */

   // Lock step config register. ACD_CTRL register
   HWREG(0x44e0d040) &= ~(0x01 << 2);
   
   // Clear FIFO0 by reading from it. FIFO0COUNT, FIFO0DATA registers
   unsigned int count = HWREG(0x44e0d0e4);
   unsigned int data, i;
   for(i=0; i<count; i++){
      data = HWREG(0x44e0d100);
   }

   // Clear FIFO1 by reading from it. FIFO1COUNT, FIFO1DATA registers
   count = HWREG(0x44e0d0f0);
   for (i=0; i<count; i++){
      data = HWREG(0x44e0d200);
   }
   shared_ram[500] = data; // just remove unused value warning;

   // Enable tag channel id. ADC_CTRL register
   HWREG(0x44e0d040) |= 0x02;

   // Enable all steps. STEPENABLE register
   /* HWREG(0x44e0d054) |= 0xfe; */
   HWREG(0x44e0d054) |= 0x2;

   // Enable Module (start sampling). ADC_CTRL register
   HWREG(0x44e0d040) |= 0x01;

}

unsigned int buffer_size = 128;
unsigned int buffer[256]; //128*2

int main(int argc, const char *argv[]){
   // Enable OCP so we can access the whole memory map for the
   // device from the PRU. Clear bit 4 of SYSCFG register
   HWREG(0x26004) &= 0xFFFFFFEF;

   init_adc();

   unsigned int i;
   unsigned int count;
   unsigned int data, channel_id, sample;
   /* unsigned int data; */
   unsigned int buffer_count = 0;
   unsigned int total_count=0;

   while(1){
      // Read samples from fifo0, check that they come from 
      // channel0, put them in a buffer and when block size is 
      // reached, tell arm host to use it.
      // FIFO0COUNT, FIFO0DATA registers
      // TODO: allocate room in shared ram for each channel twice.
      //       this way, the host processor can read from one 
      //       buffer while we fill the other one. Also eliminates
      //       the need for the memcopy routine below.

      for (i=0; i<40; i++);

      __R31 = 0;
      count = HWREG(0x44e0d0e4);
      for(i=0; i<count; i++){
         data = HWREG(0x44e0d100);
         channel_id = (data>>16) & 0x0f;
         if(channel_id==0 && buffer_count<(buffer_size*2)){
            sample = data & 0xfff; 
            buffer[buffer_count] = sample;
            buffer_count ++;
            total_count ++;
         }
      }

      if(buffer_count>=buffer_size && total_count<=144000){
         // put number of available sizes in position 0 and then
         // the rest. 
         shared_ram[0]=buffer_count;
         // TODO: do memcpy instead of this for loop.
         for(i=0; i<buffer_count; i++){
            shared_ram[i+1] = buffer[i];
         }

         // Send IRQ to host.
         __R31 = 35;

         buffer_count = 0;
      }
   }
}
