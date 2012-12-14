/*
 * ARINC 429 Subsystem With UDP
 *
 * This  subsytem  reads  flight  simulator  exported data  from a defined
 * IPC  Message Queue,  performs  ARINC 429  compliant data transformation
 * to the BNR  and BCD formats required by  specific  labels and sends the
 * ARINC 429 payload data to a device driver via UDP.
 *
 * Disclaimer: This software is not DO-178B/C certified and is not for use
 * in actual flight  training  devices. The code and  methos used for data
 * transfer  are not in full compliance with any standards based on  ARINC
 * 419  and/or  ARINC 429. For   more   information  on  DO-178B  and  the
 * ARINC 419/429   standards   please    visit   http://www.rtca.org   and
 * http://www.arinc.com
 *
 * Copyright (C) 2012 Curd S. Zechmeister
 *
 * This  program is   free software: you can redistribute it and/or modify
 * it under  the terms of  the GNU  General Public License as published by
 * the  Free  Software  Foundation, either  version  3 of  the License, or
 * any later version.
 *
 * This   program  is  distributed  in  the  hope  that it will be useful,
 * but  WITHOUT  ANY  WARRANTY;  without  even the  implied  warranty   of
 * MERCHANTABILITY    or  FITNESS  FOR  A  PARTICULAR  PURPOSE.   See  the
 * GNU General Public License for more details.
 *
 * You  should  have received  a  copy  of  the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <math.h>

#define DEGFACTOR        0.000686644

/*
 * Convert a floating point number ot a BNR without format 
 * correction.
 */
unsigned int float2BNR ( float value ) {
    
    unsigned int work = 0;
    
    /*ARINC 429 Adjustment*/
    work = ( (int) value)<<2 | 0x600000;
    
    return( work );
    
} // End float2BNR

/*
 * Convert a floating point Degree number to a BNR without format
 * correction.
 */
unsigned int floatDEG2BNR ( float value ) {
    
    unsigned int work = 0;
    value = ( value / DEGFACTOR);
    work = 0x600000 | ( (int) value )<<2;
    return( work );
    
} // End floatDEG2BNR

/*
 * Convert an Integer to ARINC 429 BCD format with 3 digits output. If a decimal  
 * number  are  to  be  displayed in  BCD encoding,  the input  number has to be  
 * multiplied by 10 to encode the digits to the right of the decimal point. This
 * function  performs  full  4 bit BCD  encoding  and will not  work for certain 
 * ARINC 429 words such as the BCD encoding for the ATC Transponer code.
 *
 * numberDigtis  - number of BCDs to the left of the decimal
 * numberDecimal - number of BCDs to the right of the decimal
 */
unsigned int float2BCD3 ( float value, int numberDigits, int numberDecimal ) {
    
    /* BCD Conversion */
    unsigned int bcdField = 0x00;
    int  work      = 0;
    char digit     = 0x00;
    int  bcdDigits = 0;
    
    if ( numberDecimal ) {
        work = (int) (value*pow(10,numberDecimal));
    } else {
        work = (int) value;
    }
    
    do {
        
        digit = work%10;
        bcdField |= (digit&0x0F)<<(bcdDigits*4);
        
        work /= 10;
        bcdDigits++;
        
    } while ( bcdDigits != (numberDigits+numberDecimal) );
    
    /* ARINC 429 Adjustment */
    bcdField = (bcdField<<6) & 0x1FFFFC;
    return( bcdField );
    
} // end integer2BCD3

/*
 * Convert an Integer to ARINC 429 BCD format with 4 digits output. If a decimal
 * number  are  to  be  displayed in  BCD encoding,  the input  number has to be
 * multiplied by 10 to encode the digits to the right of the decimal point. This
 * function  performs  full  4 bit BCD encoding  and   will not work for certain
 * ARINC 429 words such as the BCD encoding for the ATC Transponer code. 
 *
 * numberDigtis  - number of BCDs to the left of the decimal
 * numberDecimal - number of BCDs to the right of the decimal
 */
unsigned int float2BCD4 ( float value, int numberDigits, int numberDecimal ) {
    
    /* BCD Conversion */
    unsigned int bcdField = 0x00;
    int  work      = 0;
    char digit     = 0x00;
    int  bcdDigits = 0;
    
    if ( numberDecimal ) {
        work = (int) (value*pow(10,numberDecimal));
    } else {
        work = (int) value;
    }
    
    do {
        
        digit = work%10;
        bcdField |= (digit&0x0F)<<(bcdDigits*4);
        
        work /= 10;
        bcdDigits++;
        
    } while ( bcdDigits != (numberDigits+numberDecimal) );
    
    /* ARINC 429 Adjustment */
    bcdField = (bcdField<<2) & 0x1FFFFC;
    return( bcdField );
    
} // end integer2BCD4

/*
 * Compute  odd  partity and set  MSB of the ARINC429 word as the odd parity bit
 */
void parityOdd( unsigned int *value, unsigned int *label ) {
    
    unsigned int x;
    unsigned int parity = 0;

    for (x=0;x<8;x++)
        if ( *label & 0x1<<x )
            parity++;

    for (x=0;x<23;x++) {
        if ( *value & 0x1<<x )
            parity++;
    }
    
    if ( ((parity & 0x1) == 0) )
        *value |= 0x800000;
    
} // End parityOdd
