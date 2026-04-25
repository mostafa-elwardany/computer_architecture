# computer_architecture
mostafa ashraf 4/23/2026:
    core stuff should be done now, just missing checking and updating SREG in case of what operation is done
    still need to add print statements to follow the rules on what needs to be printed and when
    as for the text file that will have the assembly code, im waiting for the ta to answer a question regarding it
    anyone who opens this, double check on current logic if there is something i did wrong before continuing


mostafa, updated 4/25/2026:
    SREG should be done now(will check with TA if we need to check for SAR AND SAL cases since in the pdf theyre circular shifts but i assume they should still be done), another thing ill check isthat if SREG should be emptied/reset before each instruction 
    print statements still not done at all
    reading from txt file also not done
    double check on what has been done in terms of logic before continuing!!!!!!!!!!
    quick note is that they wrote this in the pdf to check carry but did it differently
            if( ((temp1 OP temp2) & MASK) == MASK) {
                    Carry = 1;
                    } else {
                    Carry = 0;
                    }