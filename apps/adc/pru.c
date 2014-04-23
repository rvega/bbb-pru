/////////////////////////////////////////////////////////////////////
// UTIL
//
#define HWREG(x) (*((volatile unsigned int *)(x)))
#define min(a,b) (a<b ? a : b)

/////////////////////////////////////////////////////////////////////
// GLOBALS
//

void init_adc();

volatile register unsigned int __R31;
volatile unsigned int* shared_ram;
volatile unsigned int* buffer;

#define block_size 128
#define channels 6

int main(int argc, const char *argv[]){

   // Init globals
   shared_ram = (volatile unsigned int *)0x10000;
   buffer = &(shared_ram[100]); // We'll start putting samples in shared ram 
                                // address 100 and use up to 6KB = 4 bytes 
                                // (size of unsigned int) * block_size * 
                                // num_channels * 2 buffers; 
   // Locals:
   unsigned int i;
   unsigned int fifo0_count;
   /* unsigned int fifo1_count; */
   /* unsigned int min_fifo_count; */
   unsigned int data;
   /* unsigned int channel_id; */
   unsigned int buffer_count = 0;
   unsigned int sample; 
   unsigned int which_buffer = 0; // We'll alternate the memory positions 
                                  // where we put samples in so that the 
                                  // arm cpu can read one of them while we 
                                  // fill the other one.

   init_adc();

   while(1){
      // Read samples from fifo0 and fifo1, check which channel 
      // they come from, put them in a buffer and when block size is 
      // reached, send IRQ to ARM host.
      // FIFO0COUNT, FIFO0DATA, FIFO1COUNT, FIFO1DATA registers
      // TODO: allocate room in shared ram for each channel twice.
      //       this way, the host processor can read from one 
      //       buffer while we fill the other one. Also eliminates
      //       the need for the memcopy routine below.
      //
      // TODO: Do samples come in order (by channel)? if so, it'll be 
      //       easy to send interleaved channels (preferred)

      /* for (i=0; i<40; i++); */
      __R31 = 0;

      // We'll read 6 samples at a time so that the resulting 
      // output is interleaved.
      fifo0_count = HWREG(0x44e0d0e4);
      /* fifo1_count = HWREG(0x44e0d0f0); */
      /* while(fifo0_count>=3 && fifo1_count>=3){ */
         for(i=0; i<fifo0_count; i++){
            data = HWREG(0x44e0d100);
            sample = data & 0xfff; 
            buffer[(which_buffer*block_size*channels) + buffer_count] = sample;
            buffer_count ++;
         }

      /*    for(i=0; i<3; i++){ */
      /*       data = HWREG(0x44e0d200); */
      /*       sample = data & 0xfff;  */
      /*       buffer[(which_buffer*block_size*channels) + buffer_count] = sample; */
      /*       buffer_count ++; */
      /*    } */

      /*    fifo0_count = HWREG(0x44e0d0e4); */
      /*    fifo1_count = HWREG(0x44e0d0f0); */
      /* } */

      if(buffer_count >= block_size*channels){
         // put number of available samples (block size) in position 0,
         // address of buffer in position 1 and IRQ
         shared_ram[0]=100+(which_buffer * block_size * channels);
         shared_ram[1]=buffer_count;
         shared_ram[3] = fifo0_count;
         __R31 = 35;
         buffer_count = 0;
         which_buffer = !which_buffer;
      }
   }
}

void init_adc(){
   // Enable OCP so we can access the whole memory map for the
   // device from the PRU. Clear bit 4 of SYSCFG register
   HWREG(0x26004) &= 0xFFFFFFEF;

   // Enable clock for adc module. CM_WKUP_ADC_TSK_CLKCTL register
   HWREG(0x44e004bc) = 0x02;

   // Disable ADC module temporarily. ADC_CTRL register
   HWREG(0x44e0d040) &= ~(0x01);

   // To calculate sample rate:
   // fs = 24MHz / (CLK_DIV*2*Channels*(OpenDly+Average*(14+SampleDly)))
   // We want 48KHz. (Compromising to 50KHz)
   unsigned int clock_divider = 4;
   unsigned int open_delay = 4;
   unsigned int average = 1;       // can be 0 (no average), 1 (2 samples), 
                                   // 2 (4 samples),  3 (8 samples) 
                                   // or 4 (16 samples)
   unsigned int sample_delay = 4;

   // Set clock divider (set register to desired value minus one). 
   // ADC_CLKDIV register
   HWREG(0x44e0d04c) = clock_divider - 1;

   // Set values range from 0 to FFF. ADCRANGE register
   HWREG(0x44e0d048) = (0xfff << 16) & (0x000);

   // Disable all steps. STEPENABLE register
   HWREG(0x44e0d054) &= ~(0xff);

   // Unlock step config register. ACD_CTRL register
   HWREG(0x44e0d040) |= (0x01 << 2);

   // Set config for step 1. sw mode, continuous mode, 
   // use fifo0, use channel 0. STEPCONFIG1 register
   HWREG(0x44e0d064) = 0x0000 | (0x0<<26) | (0x00<<19) | (0x00<<15) | (average<<2) | (0x01);

   // Set delays for step 1. STEPDELAY1 register
   HWREG(0x44e0d068) = 0x0000 | (sample_delay - 1)<<24 | open_delay;

   // Set config for step 2. sw mode, continuous mode, 
   // use fifo0, use channel 1. STEPCONFIG2 register
   HWREG(0x44e0d06c) = 0x0000 | (0x0<<26) | (0x01<<19) | (0x01<<15) | (average<<2) | (0x01);

   // Set delays for step 2. STEPDELAY2 register
   HWREG(0x44e0d070) = 0x0000 | (sample_delay - 1)<<24 | open_delay;

   // Set config for step 3. sw mode, continuous mode, 
   // use fifo0, use channel 2. STEPCONFIG3 register
   HWREG(0x44e0d074) = 0x0000 | (0x0<<26) | (0x02<<19) | (0x02<<15) | (average<<2) | (0x01);

   // Set delays for step 3. STEPDELAY3 register
   HWREG(0x44e0d078) = 0x0000 | ((sample_delay - 1)<<24) | open_delay;

   // Set config for step 4. sw mode, continuous mode, 
   // use fifo1, use channel 3. STEPCONFIG4 register
   /* HWREG(0x44e0d07c) = 0x0000 | (0x0<<26) | (0x03<<19) | (0x03<<15) | (average<<2) | (0x01); */

   // Set delays for step 4. STEPDELAY4 register
   /* HWREG(0x44e0d080) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */


   // Set config for step 5. sw mode, continuous mode, 
   // use fifo1, use channel 4. STEPCONFIG5 register
   /* HWREG(0x44e0d084) = 0x0000  | (0x0<<26) | (0x04<<19) | (0x04<<15) | (average<<2) | (0x01); */

   // Set delays for step 5. STEPDELAY5 register
   /* HWREG(0x44e0d088) = 0x0000 | ((sample_delay - 1)<<24) | open_delay; */

   // Set config for step 6. sw mode, continuous mode, 
   // use fifo1, use channel 5. STEPCONFIG6 register
   /* HWREG(0x44e0d08c) = 0x0000 | (0x0<<26) | (0x05<<19) | (0x05<<15) | (average<<2) | (0x01); */

   // Set delays for step 6. STEPDELAY6 register
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

   // Enable steps 1-4. STEPENABLE register
   /* HWREG(0x44e0d054) = 0x1e; */
      // Enable steps 1-6. STEPENABLE register
      HWREG(0x44e0d054) = 0x7e;
      // Enable steps 1-3. STEPENABLE register
      /* HWREG(0x44e0d054) = 0xe; */
      // Enable all steps. STEPENABLE register
      /* HWREG(0x44e0d054) |= 0xfe; */

   // Enable Module (start sampling). ADC_CTRL register
   HWREG(0x44e0d040) |= 0x01;
}
