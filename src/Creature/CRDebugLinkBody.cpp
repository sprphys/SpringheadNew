/*
 *  Copyright (c) 2003-2008, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#include "CRDebugLinkBody.h"

#ifndef USE_HDRSTOP
#pragma hdrstop
#endif

namespace Spr{

// コンストラクタ
CRDebugLinkBodyDesc::CRDebugLinkBodyDesc(bool enableRange, bool enableFMax){
	soNSolids = 0;
	joNBallJoints = 0;
	joNHingeJoints = 0;
	joNJoints = 0;

	radius  = 0.5;
	length  = 1.2;

	mass = 2.0;

	spring = 5000.0;
	damper = 100.0;

	fMax = 50;
}

void CRDebugLinkBody::CreateBody(){
	PHSolidDesc sDesc;
	{
		sDesc.mass = mass;
	}
	CDCapsuleDesc cDesc;
	{
		cDesc.radius = radius;
		cDesc.length = length;
	}
	if(soNSolids <= 0) return;
	for(unsigned int i = 0; i < soNSolids; i++){
		solids.push_back(phScene->CreateSolid(sDesc));
		solids.back()->AddShape(phSdk->CreateShape(cDesc));
	}
	joNJoints = soNSolids-1;
	PHBallJointDesc bDesc;
	{
		bDesc.spring = spring;
		bDesc.damper = damper;
		bDesc.poseSocket.Pos() = Vec3d(0, 0, length/2);
		bDesc.posePlug.Pos() = Vec3d(0, 0, -length/2);
		bDesc.fMax = fMax;
	}
	for(unsigned int i = 0; i < joNJoints; i++){
		joints.push_back(phScene->CreateJoint(solids[i], solids[i+1], bDesc));
	}
}

void CRDebugLinkBody::InitBody(){}

void CRDebugLinkBody::InitContact(){
	// 自分に属する剛体同士の接触をOff（まだ少なすぎるかも？最低限の接触は残したい（07/09/25, mitake））
	for (unsigned int i=0; i<solids.size(); ++i) {
		for (unsigned int j=0; j<solids.size(); ++j) {
			if (i!=j) {
				phScene->SetContactMode(solids[i], solids[j], PHSceneDesc::MODE_NONE);
			}
		}
	}

	// 自分以外にすでにBodyが居ればそのBodyに属する剛体とのContactも切る
	for (int i=0; i<creature->NBodies(); ++i) {
		CRBodyIf* body = creature->GetBody(i);
		if (DCAST(CRFourLegsAnimalBodyIf,body)!=(this->Cast())) {
			for (int s=0; s<body->NSolids(); ++s) {
				for (unsigned int j=0; j<solids.size(); ++j) {
					phScene->SetContactMode(body->GetSolid(s), solids[j], PHSceneDesc::MODE_NONE);
				}
			}
		}
	}
}

void CRDebugLinkBody::InitControlMode(PHJointDesc::PHControlMode m){}

void CRDebugLinkBody::Init(){}

}