/*
 *  Copyright (c) 2003-2008, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#include "HumanInterface.h"
#pragma hdrstop

namespace Spr {;

IF_OBJECT_IMP_ABST(HIVirtualDevice, Object);

void HIVirtualDevice::Print(std::ostream& o) const{
	int w = o.width();
	o.width(0);
	o << UTPadding(w) << "<DVDevice type:\"" << Type() << "\" name:\"" << Name() << "\"/>" << std::endl;
	o.width(w);
}


}	//	namespace Spr
