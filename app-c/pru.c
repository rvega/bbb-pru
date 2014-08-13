volatile register unsigned int __R31;

int main(int argc, const char *argv[]){
   // Send notification to Host for program completion
   __R31 = 35;
   __halt(); 
   return 0;
}
