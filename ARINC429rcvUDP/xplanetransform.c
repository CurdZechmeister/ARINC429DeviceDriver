//
//  xplanetransform.c
//  ARINC429rcvUDP
//
//  Created by Curd Zechmeister on 12/12/12.
//  Copyright (c) 2012 Curd Zechmeister. All rights reserved.
// 

#include <stdio.h>

/*
 * Extract the SDI from the provided 24bit ARINC 429 data value.
 */
int getARINCsdi( unsigned int value ) {
   
    int sdi;
    sdi = (value & 0x3);
    
    return( sdi );
    
} // end getARINCsdi

/*
 * LABEL 030 - VHF Communcations
 * BNC and  special  data decode from the  ARINC 429 data field to the simulator
 * compatible data format as an integer representation in Hz.
 */
unsigned int vhf030ToSim( unsigned int value ) {
    
    unsigned int bncData;
    unsigned int simData;
    
    bncData = value;
    simData = ((bncData>>2) & 0xF);
    bncData = value;
    simData += ((bncData>>6) & 0xF) * 10;
    bncData = value;
    simData += ((bncData>>10) & 0xF) * 100;
    bncData = value;
    simData += ((bncData>>14) & 0xF) * 1000;
    bncData = value;
    simData += ((bncData>>18) & 0x7) * 10000;
    simData += 100000;
    
    return ( simData );
    
} // end vhf030ToSim

/*
 * Get the word number from the LABEL 205 HF word field.
 */
int getARINChfWord( unsigned int value ) {
    
    int hfWord;
    hfWord = ((value>>2) & 0x1);
    
    return (hfWord);
}

/*
 * LABEL 205 - HF Communications
 * BNC and special data decode from the  ARINC 429 data field to the simulator
 * compatible data format as an integer representation in Hz.
 */
unsigned int hf205ToSim( unsigned int value ) {
    
    unsigned int bncData;
    unsigned int simData;
    
    bncData = value;
    simData = ((bncData>>3) & 0xF);
    bncData = value;
    simData += ((bncData>>7) & 0xF) * 10;
    bncData = value;
    simData += ((bncData>>11) & 0xF) * 100;
    bncData = value;
    simData += ((bncData>>15) & 0xF) * 1000;
    bncData = value;
    simData += ((bncData>>19) & 0x3) * 10000;
    
    return ( simData );
    
} // end hf205ToSim


