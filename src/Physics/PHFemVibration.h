/*
 *  Copyright (c) 2003-2012, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */

#ifndef PH_FEM_VIBRATION_H
#define PH_FEM_VIBRATION_H

#include <Physics/PHFemMeshNew.h>
#include <Foundation/Object.h>

namespace Spr{;

class PHFemVibration : public PHFemVibrationDesc, public PHFem{
public:
	SPR_OBJECTDEF(PHFemVibration);
	ACCESS_DESC(PHFemVibration);
	typedef double element_type;
	typedef VMatrixRow< element_type > VMatrixRe;
	typedef VVector< double > VVectord;
	#define element_limit std::numeric_limits< element_type >::max_digits10 	// stream�̐��x

	double vdt;
	PHFemVibrationDesc::ANALYSIS_MODE analysis_mode;
	PHFemVibrationDesc::INTEGRATION_MODE integration_mode;
	VMatrixRe matKIni;	// �S�̍����s��̏����l
	VMatrixRe matMIni;	// �S�̎��ʍs��̏����l
	VMatrixRe matMInv;	// �S�̎��ʍs��̋t�s��
	VMatrixRe matCIni;	// �S�̌����s��̏����l
	VVectord xdl;		// ���_�ψ�(u = (u0, v0, w0, ..., un-1, vn-1, wn-1))
	VVectord vl;		// ���_���x
	VVectord fl;		// �v�Z�p�̊O��
	VMatrixRe matKp;	// �S�̍����s��̈ꕔ
	VMatrixRe matMp;	// �S�̎��ʍs��̈ꕔ
	VMatrixRe matCp;	// �S�̌����s��̈ꕔ
	VVectord xdlp;		// ���_�ψʂ̈ꕔ(u = (u0, v0, w0, ..., un-1, vn-1, wn-1))
	VVectord vlp;		// ���_���x�̈ꕔ
	VVectord flp;		// �v�Z�p�̊O�͂̈ꕔ

	VVector< int > boundary;	// ���E����ID

	PHFemVibration(const PHFemVibrationDesc& desc = PHFemVibrationDesc());
	/// ������
	virtual void Init();
	//virtual void CompStiffnessMatrix();
	//virtual void CompMassMatrix();
	//virtual void CompDampingMatrix();

	/// �V�~�����[�V�����X�e�b�v
	virtual void Step();
	virtual void NumericalIntegration(const VMatrixRe& _M, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v);
	virtual void ModalAnalysis(const VMatrixRe& _M, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v, const int nmode);

	/// ���Ԑϕ�
	/// _M:���ʍs��A_K:�����s��A_C:�����s��A_f:�O�́A_dt:�ϕ����݁A_xd:�ψʁA_v:���x
	virtual void ExplicitEuler(const VMatrixRe& _MInv, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v);
	virtual void ImplicitEuler(const VMatrixRe& _MInv, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v);
	virtual void NewmarkBeta(const VMatrixRe& _M, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v, const double b = 1.0 /6.0);

	/// �ŗL�l���
	virtual void SubSpace(const VMatrixRe& _M, const VMatrixRe& _K, 
							const int nmode, const double epsilon, VVectord& e, VMatrixRe& v);

	/// �e��ݒ�
	void SetTimeStep(double dt){ vdt = dt; }
	double GetTimeStep(){ return vdt; }
	void SetYoungModulus(double value){ young = value; }
	double GetYoungModulus(){ return young; }
	void SetPoissonsRatio(double value){ poisson = value; }
	double GetPoissonsRatio(){ return poisson; }
	void SetDensity(double value) { density = value; }
	double GetDensity(){ return density; }
	void SetAlpha(double value){ alpha = value; }
	double GetAlpha(){ return alpha; }
	void SetBeta(double value){ beta = value; }
	double GetBeta(){ return beta; }
	void SetAnalysisMode(PHFemVibrationDesc::ANALYSIS_MODE mode);
	void SetIntegrationMode(PHFemVibrationDesc::INTEGRATION_MODE mode);

	// FemVertex���璸�_�ψʂ��擾���A�v�Z�ł���`�ɕϊ�����
	void GetVerticesDisplacement(VVectord& _xd);
	// FemVertex�ɒ��_�ψʂ�������
	void UpdateVerticesPosition(VVectord& _xd);

	std::vector< int > FindVertices(const int vtxId, const Vec3d _vecl);
	// ���E������������(�s��Ɣԍ��j
	bool AddBoundaryCondition(VMatrixRe& mat, const int id);
	// ���E������������(���_�j
	bool AddBoundaryCondition(const int vtxId, const Vec3i dof);
	// ���E������������(���_���j
	bool AddBoundaryCondition(const VVector< Vec3i > bcs);
	// ���E�����𔽉f�����邽�߂̍s����v�Z����
	void CompBoundaryMatrix(VMatrixRe& _L, VMatrixRe& _R, const VVector< int > bc);
	// ���E�����ɉ����čs��̎��R�x���팸����
	void ReduceMatrixSize(VMatrixRe& mat, const VVector< int > bc);
	// ���E�����ɉ����čs�񎩗R�x���팸����
	void ReduceMatrixSize(VMatrixRe& _M, VMatrixRe& _K, VMatrixRe& _C, const VVector< int > bc);
	// ���E�����ɉ����ăx�N�g���̎��R�x���팸����
	void ReduceVectorSize(VVectord& r, const VVector< int > bc);
	// ���E�����ɉ����ăx�N�g���̎��R�x���팸����
	void ReduceVectorSize(VVectord& _xd, VVectord& _v, VVectord& _f,const VVector< int > bc);
	// �x�N�g���̎��R�x�����ɖ߂�
	void GainVectorSize(VVectord& r, const VVector< int > bc);
	// �x�N�g���̎��R�x�����ɖ߂�
	void GainVectorSize(VVectord& _xd, VVectord& _v, const VVector< int > bc);

	// ���_�ɗ͂�������i���[�J�����W�n�j
	bool AddVertexForceL(int vtxId, Vec3d fL);
	// ���_�ɗ͂�������i���[���h���W�n�j
	bool AddVertexForceW(int vtxId, Vec3d fW);
	// ���_�Q�ɗ͂�������i���[���h���W�n�j
	bool AddVertexForceW(VVector< Vec3d > fWs);
	// �͂�������
	void AddForce(Vec3d fW, Vec3d posW);

	/// scilab�f�o�b�N
	bool IsScilabStarted;	/// scilab���X�^�[�g���Ă��邩�ǂ����̃t���O
	/// scilab�œǂݍ��܂���dat�`���t�@�C�����o��
	template < class AD >
	void ScilabFileOut(PTM::MatrixImp<AD>& a, const std::string filename = "scimat.dat"){
		if(!IsScilabStarted){
			DSTR << "Scilab has not started" << std::endl;
			return;
		}		
		ScilabJob("clear;");
		ScilabSetMatrix("A", a);
		std::stringstream str;
		str << "fprintfMat('" << filename << "', A, '%Lf');";
		ScilabJob(str.str().c_str());
	}

	/// �s�񎮌v�Z
	template < class AD >
	void ScilabDeterminant(PTM::MatrixImp<AD>& a, const std::string name = ""){
		DSTR << "////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
		DSTR << "Scilab Determinant Start." << std::endl;	
		DSTR << "det(" << name << ") springhead2 : " << a.det() << std::endl;
		if(!IsScilabStarted){
			DSTR << "Scilab has not started" << std::endl;
			return;
		}
		ScilabJob("clear;");
		ScilabSetMatrix("A", a);
		ScilabJob("detA = det(A);");
		DSTR << "Determinant of scilab is written in console." << std::endl;
		std::cout << "det("<< name << ") scilab : ";
		ScilabJob("disp(detA);");	
		DSTR << "Scilab Determinant End." << std::endl;	
		DSTR << "////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	}
	/// �ŗL�l�ŗL�x�N�g���v�Z
	void ScilabEigenValueAnalysis(VMatrixRe& _M, VMatrixRe& _K);

	/// �s��̃t�@�C���o��
	void MatrixFileOut(VMatrixRe mat, std::string filename);

	/// ������
	std::vector< int > FindNeigborTetrahedron(Vec3d pos);
	bool FindNeigborFaces(Vec3d pos, std::vector< int >& faceIds, std::vector< Vec3d >& closestPoints);
};

}

#endif