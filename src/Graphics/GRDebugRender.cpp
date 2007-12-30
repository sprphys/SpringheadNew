/*
 *  Copyright (c) 2003-2006, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#include "Graphics.h"
#ifdef USE_HDRSTOP
#pragma hdrstop
#endif
#include <GL/glut.h>

#include <Physics/PHConstraint.h>

namespace Spr {;
//----------------------------------------------------------------------------
//	GRDebugRender
IF_OBJECT_IMP(GRDebugRender, GRRender);

/// コンストラクタ
GRDebugRender::GRDebugRender(){
	matSampleCount = 24;		// 24種類
	matSample.resize(matSampleCount);
	std::vector<GRMaterialDesc>::iterator itr;
	itr = matSample.begin();
	(itr++)->diffuse = Vec4f(1.0, 0.0, 0.0, 1.0);			// red
	(itr++)->diffuse = Vec4f(0.0, 1.0, 0.0, 1.0);			// green
	(itr++)->diffuse = Vec4f(0.0, 0.0, 1.0, 1.0);			// blue
	(itr++)->diffuse = Vec4f(1.0, 1.0, 0.0, 1.0);			// yellow
	(itr++)->diffuse = Vec4f(0.0, 1.0, 1.0, 1.0);			// cyan
	(itr++)->diffuse = Vec4f(1.0, 0.0, 1.0, 1.0);			// magenta
	(itr++)->diffuse = Vec4f(1.0, 1.0, 1.0, 1.0);			// white
	(itr++)->diffuse = Vec4f(0.5, 0.5, 0.5, 1.0);			// gray
	(itr++)->diffuse = Vec4f(1.0, 0.27, 0.0, 1.0);			// orange
	(itr++)->diffuse = Vec4f(0.198, 0.0, 0.0, 1.0);			// blown
	(itr++)->diffuse = Vec4f(0.676, 0.844, 0.898, 1.0);		// light blue
	(itr++)->diffuse = Vec4f(0.574, 0.438, 0.855, 1.0);		// midium purple
	(itr++)->diffuse = Vec4f(0.0, 0.391, 0.0, 1.0);			// dark green
	(itr++)->diffuse = Vec4f(0.578, 0.0, 0.824, 1.0);		// dark violet
	(itr++)->diffuse = Vec4f(0.0, 0.543, 0.543, 1.0);		// dark cyan
	(itr++)->diffuse = Vec4f(0.676, 1.0, 0.184, 1.0);		// green yellow
	(itr++)->diffuse = Vec4f(0.195, 0.801, 0.195, 1.0);		// lime green
	(itr++)->diffuse = Vec4f(0.801, 0.359, 0.359, 1.0);		// indian red
	(itr++)->diffuse = Vec4f(0.293, 0.0, 0.508, 1.0);		// indigo
	(itr++)->diffuse = Vec4f(0.0, 0.198, 0.198, 1.0);		// green indigo
	(itr++)->diffuse = Vec4f(0.198, 0.398, 0.0, 1.0);		// olive green
	(itr++)->diffuse = Vec4f(0.198, 0.398, 0.797, 1.0);		// navy blue
	(itr++)->diffuse = Vec4f(0.398, 1.0, 0.797, 1.0);		// turquoise blue
	(itr++)->diffuse = Vec4f(0.598, 1.0, 0.398, 1.0);		// emerald green
	for(std::vector<GRMaterialDesc>::iterator it=matSample.begin(); it!= matSample.end(); ++it){
		it->diffuse += Vec4f(0.5,0.5,0.5,1);
		it->diffuse /= 2;
	}
	modeSolid = true;
	modeWire = false;
	renderAxis = false;
	renderForce = false;
}

/// シーン内の全てのオブジェクトをレンダリングする
void GRDebugRender::DrawScene(PHSceneIf* scene){
	if (!scene) return;
	PHSolidIf **solids = scene->GetSolids();
	for (int i = 0; i < scene->NSolids(); ++i){
		this->SetMaterialSample((GRDebugRenderIf::TMaterialSample)i);
		this->DrawSolid(solids[i]);
	}
	for (int i = 0; i < scene->NJoints(); ++i){
		this->DrawConstraint(scene->GetJoint(i));
	}
}

/// 剛体をレンダリングする
void GRDebugRender::DrawSolid(PHSolidIf* so){
	Affinef soaf;
	so->GetPose().ToAffine(soaf);
	this->PushModelMatrix();
	this->MultModelMatrix(soaf);
	
	for(int s=0; s<so->NShape(); ++s){
		CDShapeIf* shape = so->GetShape(s);

		Affinef af;
		so->GetShapePose(s).ToAffine(af);
		this->PushModelMatrix();
		this->MultModelMatrix(af);
		
		CDConvexMeshIf* mesh = DCAST(CDConvexMeshIf, shape);
		if (mesh){
			if(modeSolid)
				DrawMesh(mesh, true);
			if(modeWire)
				DrawMesh(mesh, false);
		}
		CDSphereIf* sphere = DCAST(CDSphereIf, shape);
		if (sphere){
			float r = sphere->GetRadius();
			if(modeSolid)
				glutSolidSphere(r, 16, 16);
			if(modeWire)
				glutWireSphere(r, 16, 16);
		}
		CDCapsuleIf* cap = DCAST(CDCapsuleIf, shape);
		if (cap){
			if(modeSolid)
				DrawCapsule(cap, true);
			if(modeWire)
				DrawCapsule(cap, false);
		}
		CDBoxIf* box = DCAST(CDBoxIf, shape);
		if (box){
			Vec3f boxsize = box->GetBoxSize();
			glScalef(boxsize.x, boxsize.y, boxsize.z);	
			if(modeSolid)
				glutSolidCube(1.0);		
			if(modeWire)
				glutWireCube(1.0);
		}
		this->PopModelMatrix();

		// 剛体のローカル座標軸を表示する．
		if(renderAxis && (modeSolid || modeWire))
			DrawCoordinateAxis(modeSolid);
	}

	this->PopModelMatrix();
}

void GRDebugRender::DrawConstraint(PHConstraintIf* conif){
	PHConstraint* con = conif->Cast();
	Affinef af;
	
	// socket
	(con->solid[0]->GetPose() * con->poseSocket).ToAffine(af);
	this->PushModelMatrix();
	this->MultModelMatrix(af);
	DrawCoordinateAxis(modeSolid);
	this->PopModelMatrix();

	// plug
	(con->solid[1]->GetPose() * con->posePlug).ToAffine(af);
	this->PushModelMatrix();
	this->MultModelMatrix(af);
	DrawCoordinateAxis(modeSolid);
	this->PopModelMatrix();
}

void GRDebugRender::DrawCapsule(CDCapsuleIf* cap, bool solid){
	float r = cap->GetRadius();
	float l = cap->GetLength();
	this->PushModelMatrix();

	DrawCylinder(r, l, 20, solid);

	glTranslatef(0,0,-l/2);
	solid ? glutSolidSphere(r, 20, 20) : glutWireSphere(r, 20, 20);
	glTranslatef(0,0,l);
	solid ? glutSolidSphere(r, 20, 20) : glutWireSphere(r, 20, 20);
	this->PopModelMatrix();
}

void GRDebugRender::DrawMesh(CDConvexMeshIf* mesh, bool solid){
	Vec3f* base = mesh->GetVertices();
	if(solid){
		for (size_t f=0; f<mesh->NFace(); ++f) {	
			CDFaceIf* face = mesh->GetFace(f);
			this->DrawFaceSolid(face, base);
		}
	}
	else{
		for (size_t f=0; f<mesh->NFace(); ++f) {	
			CDFaceIf* face = mesh->GetFace(f);
			this->DrawFaceWire(face, base);
		}
	}
}

void GRDebugRender::DrawCone(float radius, float height, int slice, bool solid){
	if(solid)
		glutSolidCone(radius, height, slice, 1);
	else
		glutWireCone(radius, height, slice, 1);
}

void GRDebugRender::DrawCylinder(float radius, float height, int slice, bool solid){
	// 現状では側面のみ
	glBegin(solid ? GL_QUAD_STRIP : GL_LINES);
	float step = (float)(M_PI * 2.0f) / (float)slice;
	float t = 0.0;
	float x,y;
	for (int i=0; i<=slice; i++) {
		x=sin(t);
		y=cos(t);
		glNormal3f(x, y, 0.0);
		glVertex3f(radius * x, radius * y,  height/2);
		glVertex3f(radius * x, radius * y, -height/2);
		t += step;
	}
	glEnd();
}

void GRDebugRender::DrawAxis(bool solid){
	this->PushModelMatrix();
	this->MultModelMatrix(Affinef::Trn(0.0f, 0.0f, 0.5f));
	DrawCylinder(0.1f, 1.0f, 4, solid);
	this->MultModelMatrix(Affinef::Trn(0.0f, 0.0f, 0.5f));
	DrawCone(0.2f, 0.3f, 8, solid);
	this->PopModelMatrix();
}

void GRDebugRender::DrawCoordinateAxis(bool solid){
	// z
	DrawAxis(solid);
	// x
	this->PushModelMatrix();
	this->MultModelMatrix(Affinef::Rot(0.5f*M_PI, 'y'));
	DrawAxis(solid);
	this->PopModelMatrix();
	// y
	this->PushModelMatrix();
	this->MultModelMatrix(Affinef::Rot(-0.5f*M_PI, 'x'));
	DrawAxis(solid);
	this->PopModelMatrix();	
	/*
	double length=0.5, width=length/10.0;
	this->PushModelMatrix();
	glTranslatef(length/2, 0, 0); glScalef(length, width, width); glutSolidCube(1.0);
	this->PopModelMatrix();
	this->PushModelMatrix();
	glTranslatef(0, length/2, 0); glScalef(width, length, width); glutSolidCube(1.0);
	this->PopModelMatrix();
	this->PushModelMatrix();
	glTranslatef(0, 0, length/2); glScalef(width, width, length); glutSolidCube(1.0);
	this->PopModelMatrix();
	*/

}

void GRDebugRender::DrawFaceSolid(CDFaceIf* face, Vec3f * base){
	int numIndices = face->NIndex();			// (=3 :三角形なので3頂点)
	struct Vtx{
		Vec3f n;
		Vec3f p;
	} vtxs[10];
	assert((size_t)numIndices <= sizeof(vtxs)/sizeof(vtxs[0]));
	for(int v = 0; v < numIndices; ++v)
		vtxs[v].p = base[face->GetIndices()[v]].data;
	Vec3f edge0, edge1;
	edge0 = vtxs[1].p - vtxs[0].p;
	edge1 = vtxs[2].p - vtxs[0].p;
	Vec3f n = (edge0^edge1).unit();
	for(int v = 0; v < numIndices; ++v)
		vtxs[v].n = n;
	SetVertexFormat(GRVertexElement::vfN3fP3f);
	DrawDirect(GRRenderBaseIf::TRIANGLE_FAN, vtxs, numIndices);
}

void GRDebugRender::DrawFaceWire(CDFaceIf* face, Vec3f * base){
	int numIndices = face->NIndex();
	Vec3f vtxs[10];
	assert((size_t)numIndices+1 <= sizeof(vtxs)/sizeof(vtxs[0]));
	int v;
	for(v = 0; v < numIndices; ++v)
		vtxs[v] = base[face->GetIndices()[v]].data;
	vtxs[v] = base[face->GetIndices()[0]].data;
	SetVertexFormat(GRVertexElement::vfP3f);
	DrawDirect(GRRenderBaseIf::LINES, vtxs, numIndices+1);
}

void GRDebugRender::SetMaterialSample(GRDebugRenderIf::TMaterialSample matname){
	int matID = 0;
	if (matname >= GRDebugRenderIf::MATERIAL_SAMPLE_END) {
		matID = (int)matname;
		matID = matID % matSampleCount;
	} else {
		matID = (int)matname;
	}
	this->SetMaterial(matSample[matID]);
}

}	//	Spr

