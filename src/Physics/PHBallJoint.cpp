/*
 *  Copyright (c) 2003-2006, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#include "Physics.h"
#ifdef USE_HDRSTOP
#pragma hdrstop
#endif

using namespace PTM;
using namespace std;
namespace Spr{;

//----------------------------------------------------------------------------
// SwingTwist

void SwingTwist::ToQuaternion(Quaterniond& q){
	double psi = SwingDir(), the = Swing(), phi = Twist();
	double cos_the = cos(the / 2), sin_the = sin(the / 2);
	double cos_phi = cos(phi / 2), sin_phi = sin(phi / 2);
	double cos_psiphi = cos(psi - phi / 2), sin_psiphi = sin(psi - phi / 2);
	q.w = cos_the * cos_phi;
	q.x = sin_the * cos_psiphi;
	q.y = sin_the * sin_psiphi;
	q.z = cos_the * sin_phi;
}
void SwingTwist::FromQuaternion(const Quaterniond& q){
	item(0) = atan2(q.w * q.y + q.x * q.z, q.w * q.x - q.y * q.z);
	item(1) = 2 * atan2(sqrt(q.x * q.x + q.y * q.y), sqrt(q.w * q.w + q.z * q.z));
	item(2) = 2 * atan2(q.z, q.w);
}
void SwingTwist::Jacobian(Matrix3d& J){
	double psi = SwingDir();
	double the = max(Rad(1.0), Swing());	// スイング角0でランク落ちするので無理やり回避
	double cos_psi = cos(psi), sin_psi = sin(psi);
	double cos_the = cos(the), sin_the = sin(the);
	J[0][0] = -sin_the * sin_psi;
	J[0][1] =  cos_psi;
	J[0][2] =  sin_the * sin_psi;
	J[1][0] =  sin_the * cos_psi;
	J[1][1] =  sin_psi;
	J[1][2] = -sin_the * cos_psi;
	J[2][0] =  1.0 - cos_the;
	J[2][1] =  0.0;
	J[2][2] =  cos_the;
}
void SwingTwist::Coriolis(Vec3d& c, const Vec3d& sd){
	double cos_psi = cos(SwingDir()), sin_psi = sin(SwingDir());
	double cos_the = cos(Swing()), sin_the = sin(Swing());
	double tmp1 = sd[1] * (sd[0] - sd[2]);
	double tmp2 = sd[0] * (sd[0] - sd[2]);
	double tmp3 = sd[0] * sd[1];
	c[0] = -cos_the * sin_psi * tmp1 - sin_the * cos_psi * tmp2 - sin_psi * tmp3;
	c[1] =  cos_the * cos_psi * tmp1 - sin_the * sin_psi * tmp2 + cos_psi * tmp3;
	c[2] =  sin_the * tmp1;	
}
void SwingTwist::JacobianInverse(Matrix3d& J, const Quaterniond& q){
	const double eps = 1.0e-12;
	double w2z2 = max(eps, q.w * q.w + q.z * q.z);
	double w2z2inv = 1.0 / w2z2;
	double x2y2 = max(eps, 1.0 - w2z2);
	double x2y2inv = 1.0 / x2y2;
	double tmp = sqrt(w2z2inv * x2y2inv);
	double wy_xz = q.w * q.y + q.x * q.z;
	double wx_yz = q.w * q.x - q.y * q.z;
	J[0][0] =  0.5 * (w2z2inv - x2y2inv) * wy_xz;
	J[0][1] = -0.5 * (w2z2inv - x2y2inv) * wx_yz;
	J[0][2] =  1.0;
	J[1][0] =  tmp * wx_yz;
	J[1][1] =  tmp * wy_xz;
	J[1][2] =  0.0;
	J[2][0] =  w2z2inv * wy_xz;
	J[2][1] = -w2z2inv * wx_yz;
	J[2][2] =  1.0;
}

//----------------------------------------------------------------------------
// PHBallJoint
IF_OBJECT_IMP(PHBallJoint, PHJoint)

PHBallJoint::PHBallJoint(){
	axis[0] = axis[1] = axis[2] = false;
	axis[3] = axis[4] = axis[5] = true;
	axisIndex[0] = 3;
	axisIndex[1] = 4;
	axisIndex[2] = 5;
}
void PHBallJoint::SetDesc(const PHConstraintDesc& desc){
	PHConstraint::SetDesc(desc);
	const PHBallJointDesc& descBall = (const PHBallJointDesc&)desc;
	swingUpper  = descBall.swingUpper;
	swingSpring = descBall.swingSpring;
	swingDamper = descBall.swingDamper;
	twistLower  = descBall.twistLower;
	twistUpper  = descBall.twistUpper;
	twistSpring = descBall.twistSpring;
	twistDamper = descBall.twistDamper;
	SetMotorTorque(descBall.torque);
}

void PHBallJoint::UpdateJointState(){
	// 相対quaternionからスイング・ツイスト角を計算
	SwingTwist& angle = (SwingTwist&)position;
	angle.FromQuaternion(Xjrel.q);
	angle.JacobianInverse(Jstinv, Xjrel.q);
	velocity = Jstinv * vjrel.w();
}

void PHBallJoint::SetConstrainedIndex(bool* con){
	con[0] = con[1] = con[2] = true;
	// 可動範囲をチェック
	SwingTwist& angle = (SwingTwist&)position;
	swingOnUpper = (swingUpper > 0 && angle.Swing() >= swingUpper);
	twistOnLower = (twistLower < twistUpper && angle.Twist() <= twistLower);
	twistOnUpper = (twistLower < twistUpper && angle.Twist() >= twistUpper);
	// 可動範囲にかかる場合は回転をSwing/Twistに座標変換する(ModifyJacobian)．
	// 以下3 -> swing方位，4 -> swing角, 5 -> twist角
	con[3] = false;
	con[4] = swingOnUpper;
	con[5] = twistOnLower || twistOnUpper;
}

// ヤコビアンの角速度部分を座標変換してSwingTwist角の時間変化率へのヤコビアンにする
void PHBallJoint::ModifyJacobian(){
	if(swingOnUpper || twistOnLower || twistOnUpper){
		J[0].wv() = Jstinv * J[0].wv();
		J[0].ww() = Jstinv * J[0].ww();
		J[1].wv() = Jstinv * J[1].wv();
		J[1].ww() = Jstinv * J[1].ww();
	}
}

void PHBallJoint::CompBias(){
	double dtinv = 1.0 / scene->GetTimeStep();
	SwingTwist& angle = (SwingTwist&)position;
	db.v() = Xjrel.r * dtinv;
	db.w()[0] = 0.0;
	db.w()[1] = (swingOnUpper ? (angle.Swing() - swingUpper) * dtinv : 0.0);
	db.w()[2] = (twistOnLower ? (angle.Twist() - twistLower) * dtinv :
			   twistOnUpper ? (angle.Twist() - twistUpper) * dtinv : 0.0);
	db *= engine->correctionRate;
}

/*void PHBallJoint::CompError(double dt){
	B.SUBVEC(0, 3) = rjrel;
	B.SUBVEC(3, 3) = qjrel.V();
}*/

void PHBallJoint::Projection(double& f, int k){
	if(k == 4 && swingOnUpper)
		f = min(0.0, f);
	if(k == 5){
		if(twistOnLower)
			f = max(0.0, f);
		if(twistOnUpper)
			f = min(0.0, f);
	}
}

//----------------------------------------------------------------------------
// PHBallJointNode

void PHBallJointNode::CompJointJacobian(){
	PHBallJoint* j = GetJoint();
	SwingTwist& angle = (SwingTwist&)(j->position);
	angle.Jacobian(Jst);
	//Matrix3d test = Jst * Jstinv;
	for(int i = 0; i < 3; i++){
		J[i].v().clear();
		J[i].w() = Jst.col(i);
	}
	PHTreeNodeND<3>::CompJointJacobian();
}
void PHBallJointNode::CompJointCoriolisAccel(){
	PHBallJoint* j = GetJoint();
	cj.v().clear();
	((SwingTwist&)(j->position)).Coriolis(cj.w(), j->velocity);
	//cj.w.clear();
}
void PHBallJointNode::CompRelativePosition(){
	PHBallJoint* j = GetJoint();
	j->Xjrel.r.clear();
	((SwingTwist&)(j->position)).ToQuaternion(j->Xjrel.q);
}
void PHBallJointNode::CompRelativeVelocity(){
	PHBallJoint* j = GetJoint();
	j->vjrel.v().clear();
	j->vjrel.w() = Jst * j->velocity;
}
void PHBallJointNode::CompBias(){
	dA.clear();
	db.clear();
}
void PHBallJointNode::Projection(double& f, int k){
	PHBallJoint* j = GetJoint();
	if(k == 1 && j->swingOnUpper)
		f = min(0.0, f);
	if(k == 2){
		if(j->twistOnLower)
			f = max(0.0, f);
		if(j->twistOnUpper)
			f = min(0.0, f);
	}
}
	
}
