volatile register unsigned int __R31;

int main(int argc, const char *argv[]){
   // Loop a bit to make time
   int i, j;
   for(i=0; i<0x00a00000; i++){
      for(j=0; j<5; j++);
   }

   // Send notification to Host for program completion
   __R31 = 35;
   __halt(); 
   return 0;
}
