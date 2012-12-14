//
//  xplanetransform.h
//  ARINC429rcvUDP
//
//  Created by Curd Zechmeister on 12/12/12.
//  Copyright (c) 2012 Curd Zechmeister. All rights reserved.
//

#ifndef ARINC429rcvUDP_xplanetransform_h
#define ARINC429rcvUDP_xplanetransform_h

unsigned int vhf030ToSim( unsigned int value );
int getARINCsdi( unsigned int value );

unsigned int hf205ToSim( unsigned int value );
int getARINChfWord( unsigned int value );


#endif
