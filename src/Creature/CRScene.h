/*
 *  Copyright (c) 2003-2008, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#ifndef CRSCENE_H
#define CRSCENE_H

#include <Springhead.h>

#include <Foundation/Object.h>
#include <Foundation/Scene.h>

#include <vector>

//@{
namespace Spr{;

class CRScene : public Scene, public CRSceneDesc {
public:
	SPR_OBJECTDEF(CRScene);
	ACCESS_DESC(CRScene);
};

}
//@}

#endif // CRSCENE_H
