/*
 *  Copyright (c) 2003-2008, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#ifndef CR_BODYGEN_H
#define CR_BODYGEN_H

#include <Springhead.h>

#include <Foundation/Object.h>

//@{
namespace Spr{;
// --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
/** @brief �{�f�B
*/
struct CRBodyGenDesc{
	/// �v���O����e���̂ɂ��邩�\�P�b�g����e���̂ɂ��邩
	//enum CRHumanJointOrder{
	enum CRCreatureJointOrder{
		SOCKET_PARENT,
		PLUG_PARENT,
	} jointOrder;

	CRBodyGenDesc(){
	}
};

class CRBodyGen : public CRBodyGenDesc {
protected:
	enum LREnum{LEFTPART=-1, RIGHTPART=+1};

	/// �{�f�B���\�����鍄��
	std::vector< UTRef<PHSolidIf> > solids;	
	
	/// �{�f�B���\������֐�
	std::vector< UTRef<PHJointIf> > joints;

	/// IK�m�[�h
	std::vector< UTRef<PHIKNodeIf> > ikNodes;

	/// ����_
	std::vector< UTRef<PHIKControlPointIf> > ikControlPoints;

	/// ��������V�[��
	PHSceneIf*		phScene;

	/// ��������Sdk
	PHSdkIf*		phSdk;
	
	/** @brief �֐߂����
	*/
	PHJointIf* CreateJoint(PHSolidIf* soChild, PHSolidIf* soParent, PHHingeJointDesc desc);
	PHJointIf* CreateJoint(PHSolidIf* soChild, PHSolidIf* soParent, PHBallJointDesc desc);

	/** @brief CDBox��Inertia���v�Z����i�����e���\���̑Ίp�v�f�ɂ͒l�������āC������ς�0�j
		@param CDBoxDesc::boxsize, PHSolidDesc::mass
		@return �����e���\��
	*/
	Matrix3d CalcBoxInertia(Vec3d boxsize, double mass);

public:
	CRBodyGen(){}
	CRBodyGen(const CRBodyGenDesc& desc, PHSceneIf* s=NULL) 
		: CRBodyGenDesc(desc)
	{
		phScene		= s;
		phSdk		= phScene->GetSdk();
	}

	/** @brief ���������s��
	*/
	virtual void				Init();

	/** @brief ���̂̐��𓾂�
	*/
	virtual int					NSolids();

	/** @brief i�Ԗڂ̍��̂𓾂�
	*/
	virtual PHSolidIf*			GetSolid(int i);

	/** @brief �֐߂̐��𓾂�
	*/
	virtual int					NJoints();

	/** @brief i�Ԗڂ̊֐߂𓾂�
	*/
	virtual PHJointIf*			GetJoint(int i);

	/** @brief �{�[���W���C���g�̐���Ԃ�
	*/
	virtual int NBallJoints();

	/** @brief �q���W�W���C���g�̐���Ԃ�
	*/
	virtual int NHingeJoints();

	/** @brief IK�m�[�h�̐��𓾂�
	*/
	int NIKNodes();

	/** @brief i�Ԗڂ�IK�m�[�h�𓾂�
	*/
	PHIKNodeIf* GetIKNode(int i);

	/** @brief IK����_�̐��𓾂�
	*/
	int NControlPoints();

	/** @brief i�Ԗڂ�IK����_�𓾂�
	*/
	PHIKControlPointIf* GetControlPoint(int i);

	/** @brief �{�f�B�̏d�S���W�𓾂�
	*/
	virtual Vec3d				GetCenterOfMass();
	
	/** @brief �{�f�B�̎��ʂ𓾂�
	*/
	virtual double				GetSumOfMass();
};
}
//@}

#endif // CR_BODYGEN_H