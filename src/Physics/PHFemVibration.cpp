/*
 *  Copyright (c) 2003-2012, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#include <Physics/PHFemVibration.h>
#include <SciLab/SprSciLab.h>
#include <Foundation/UTQPTimer.h>

namespace Spr{;

//* 初期化と行列の作成
/////////////////////////////////////////////////////////////////////////////////////////
PHFemVibration::PHFemVibration(const PHFemVibrationDesc& desc){
	SetDesc(&desc);
	//analysis_mode = PHFemVibrationDesc::ANALYSIS_DIRECT;
	analysis_mode = PHFemVibrationDesc::ANALYSIS_MODAL;
	//integration_mode = PHFemVibrationDesc::INT_EXPLICIT_EULER;
	//integration_mode = PHFemVibrationDesc::INT_IMPLICIT_EULER;
	integration_mode = PHFemVibrationDesc::INT_NEWMARK_BETA;
	IsScilabStarted = false;
} 

void PHFemVibration::Init(){
	// Scilabの起動
	IsScilabStarted = ScilabStart();
	if(!IsScilabStarted){
		DSTR << "Scilab can not start." << std::endl;
	}

	DSTR << "Initializing PHFemVibration" << std::endl;
	/// 刻み時間の設定
	PHSceneIf* scene = GetPHFemMesh()->GetScene()->Cast();
	if(scene) vdt = scene->GetTimeStep();
	else vdt = 0.001;

	/// 全体剛性行列、全体質量行列、全体減衰行列の計算
	/// これらはすべてローカル系
	PHFemMeshNew* mesh = GetPHFemMesh();
	int NTets = mesh->tets.size();
	int NVer= NVertices();
	int NDof = NVer * 3;
	// 各全体行列の初期化
	matKIni.resize(NDof, NDof, 0.0);
	matMIni.resize(NDof, NDof, 0.0);
	matCIni.resize(NDof, NDof, 0.0);

	for(int i = 0; i < NTets; i++){
		// 要素行列の計算
		/// tetが持つ頂点順
		/// 要素剛性行列 u = (u0, v0, w0,  ..., un-1, vn-2, wn-1)として計算
#if 0
		/// 基本変形量版
		/// 行列C(変形-基本変形量）の計算（頂点座標に応じて変わる）
		PTM::TMatrixRow< 1, 4, element_type > vec[4];
		for(int j = 0; j < 4; j++){
			Vec3d pos = mesh->vertices[mesh->tets[i].vertexIDs[j]].pos;
			vec[j].item(0, 0) = 1.0;
			vec[j].item(0, 1) = pos.x;
			vec[j].item(0, 2) = pos.y;
			vec[j].item(0, 3) = pos.z;
		}
		PTM::TMatrixRow< 12, 12, element_type > matCk;
		matCk.clear(0.0);
		for(int j = 0; j < 4; j++){
			int id = 3 * j;
			matCk.vsub_matrix(id, 0, 1, 4) = vec[j];
			matCk.vsub_matrix(id + 1, 4, 1, 4)= vec[j];
			matCk.vsub_matrix(id + 2, 8, 1, 4)= vec[j];
		}
		PTM::TMatrixRow< 12, 12, element_type > matCkInv;
		matCkInv.assign(matCk.inv());

		/// 行列B(ひずみ-基本変形量）（定数）
		PTM::TMatrixRow< 6, 12, element_type > matB;
		matB.clear(0.0);
		matB[0][1] = 1.0;
		matB[1][6] = 1.0;
		matB[2][11] = 1.0;
		matB[3][2] = 1.0;	matB[3][5] = 1.0;
		matB[4][7] = 1.0;	matB[4][10] = 1.0;
		matB[5][3] = 1.0;	matB[5][9] = 1.0;

		/// 弾性係数行列D（応力-ひずみ）の計算
		/// （ヤング率、ポアソン比に応じてかわる）
		double E = GetYoungModulus();
		double v = GetPoissonsRatio();
		double a = 1 - v;
		double b = 1 - 2 * v;
		double c = 0.5 - v;
		double Em;
		if(b == 0.0) Em = DBL_MAX; /// 変形しない。ほんとうは+∞になる。
		else Em = E / ((1 + v) * b);
		PTM::TMatrixRow< 6, 6, element_type > matD;
		matD.clear(0.0);
		matD[0][0] = a; matD[0][1] = v; matD[0][2] = v;
		matD[1][0] = v; matD[1][1] = a; matD[1][2] = v;
		matD[2][0] = v; matD[2][1] = v; matD[2][2] = a;
		matD[3][3] = c;
		matD[4][4] = c;
		matD[5][5] = c;
		matD *= Em;
		
		/// BtDBの計算
		PTM::TMatrixRow< 12, 12, element_type > matBtDB;
		matBtDB.clear(0.0);
		matBtDB = matB.trans() * matD * matB;

		/// 要素剛性行列の計算(エネルギー原理）
		TMatrixRow< 12, 12, element_type > matKe; // 要素剛性行列
		matKe.clear(0.0);
		matKe = matCkInv.trans() * matBtDB * matCkInv * mesh->CompTetVolume(i);
#else
		/// 形状関数版
		/// 形状関数の計算（頂点座標に応じて変わる）
		PTM::TMatrixRow< 4, 4, element_type > matCoeff;
		matCoeff.assign(mesh->CompTetShapeFunctionCoeff(i));
		TVector<4, element_type > b, c, d;	// 形状関数の係数
		b.assign(matCoeff.col(1));
		c.assign(matCoeff.col(2));
		d.assign(matCoeff.col(3));

		/// 行列B（ひずみ-変位）
		PTM::TMatrixRow< 6, 12, element_type > matB;
		matB.clear(0.0);
		matB[0][0] = b[0];	matB[0][3] = b[1];	matB[0][6] = b[2];	matB[0][9] = b[3];
		matB[1][1] = c[0];	matB[1][4] = c[1];	matB[1][7] = c[2];	matB[1][10] = c[3];
		matB[2][2] = d[0];	matB[2][5] = d[1];	matB[2][8] = d[2];	matB[2][11] = d[3];
		matB[3][0] = c[0];	matB[3][1] = b[0];	matB[3][3] = c[1];	matB[3][4] = b[1];	matB[3][6] = c[2];	matB[3][7] = b[2];	matB[3][9] = c[3];	matB[3][10] = b[3];
		matB[4][1] = d[0];	matB[4][2] = c[0];	matB[4][4] = d[1];	matB[4][5] = c[1];	matB[4][7] = d[2];	matB[4][8] = c[2];	matB[4][10] = d[3];	matB[4][11] = c[3];
		matB[5][0] = d[0];	matB[5][2] = b[0];	matB[5][3] = d[1];	matB[5][5] = b[1];	matB[5][6] = d[2];	matB[5][8] = b[2];	matB[5][9] = d[3];	matB[5][11] = b[3];
		element_type div = 1.0 / (6.0 * mesh->CompTetVolume(i));
		matB *= div;

		/// 弾性係数行列Dの計算（応力-ひずみ）
		/// （ヤング率、ポアソン比に応じてかわる）
		double E = GetYoungModulus();
		double v = GetPoissonsRatio();
		double av = 1.0 - v;
		double bv = 1.0 - 2.0 * v;
		double cv = 0.5 - v;
		double Em;
		if(bv == 0.0) Em = DBL_MAX; /// 変形しない。ほんとうは+∞になる。
		else Em = E / ((1.0 + v) * bv);
		PTM::TMatrixRow< 6, 6, element_type > matD;
		PTM::TMatrixRow< 6, 6, element_type > matDs;
		matD.clear(0.0);
		matD[0][0] = av;	matD[0][1] = v;		matD[0][2] = v;
		matD[1][0] = v;		matD[1][1] = av;	matD[1][2] = v;
		matD[2][0] = v;		matD[2][1] = v;		matD[2][2] = av;
		matD[3][3] = cv;
		matD[4][4] = cv;
		matD[5][5] = cv;
		matDs.assign(matD);
		matD *= Em;

		/// 要素剛性行列の計算(エネルギー原理）
		TMatrixRow< 12, 12, element_type > matKe;
		matKe.clear(0.0);
		matKe = matB.trans() * matD * matB * mesh->CompTetVolume(i);
#endif
		/// 質量行列の計算
		TMatrixRow< 12, 12, element_type > matMe;
		matMe.clear(0.0);
		TMatrixRow< 3, 3, element_type > I;
		I.clear(0.0);
		I[0][0] = 1.0; I[1][1] = 1.0; I[2][2] = 1.0;
		for(int j = 0; j < 4; j++){
			for(int k = 0; k < 4; k++){
				int t = j * 3;
				int l = k * 3;
				int h = 3;
				int w = 3;
				if(j == k){
					matMe.vsub_matrix(t, l, h, w) = 2.0 * I;
				}else{
					matMe.vsub_matrix(t, l, h, w) = I;
				}
			}
		}
		matMe *= GetDensity() * mesh->CompTetVolume(i) / 20.0;

		/// 減衰行列（比例減衰）
		TMatrixRow< 12, 12, element_type > matCe;
		matCe.clear(0.0);
		matCe = GetAlpha() * matMe + GetBeta() * matKe;
		
		//全体行列の計算
		//頂点番号順 
		// u = (u0, v0, w0,  ..., un-1, vn-2, wn-1)として計算
		// j:ブロック番号, k:ブロック番号
		for(int j = 0; j < 4; j++){
			for(int k = 0; k < 4; k++){
				int id = mesh->tets[i].vertexIDs[j];
				int id2 = mesh->tets[i].vertexIDs[k];
				int t = id * 3;	int l = id2 * 3;
				int h = 3;		int w = 3;
				int te = j * 3;	int le = k * 3;
				// 全体剛性行列
				matKIni.vsub_matrix(t, l, h, w) += matKe.vsub_matrix(te, le, h, w);
				// 全体質量行列
				matMIni.vsub_matrix(t, l, h, w) += matMe.vsub_matrix(te, le, h, w);
				// 全体減衰行列
				matCIni.vsub_matrix(t, l, h, w) += matCe.vsub_matrix(te, le, h, w);
			}
		}
		//DSTR << "matCk1" << std::endl;	DSTR << matCk1 << std::endl;
		//DSTR << "matCk1Inv" << std::endl;	DSTR << matCk1Inv << std::endl;
		//DSTR << "matCkInv" << std::endl;	DSTR << matCkInv << std::endl;
		//DSTR << "matB" << std::endl;		DSTR << matB << std::endl;
		//DSTR << "matD" << std::endl;		DSTR << matD << std::endl;
		//DSTR << "matBtDB" << std::endl;	DSTR << matBtDB << std::endl;
		//DSTR << "det matKe : "<< matKe.det() << std::endl;
		//DSTR << "matKe" << std::endl;		DSTR << matKe << std::endl;
		//DSTR << "matMe" << std::endl;		DSTR << matMe << std::endl;
		//if(i == 0){
		//	MatrixFileOut(matKe, "matKe0.csv");
		//}
		//if( i == 1){
		//	MatrixFileOut(matKe, "matKe1.csv");		
		//}
	}	
	//DSTR << "det(matKIni) = " << matKIni.det() << std::endl;
	//DSTR << "matKIni" << std::endl;	DSTR << matKIni << std::endl;
	//ScilabDeterminant(matKIni, "matKIni");
	//ScilabFileOut(matKIni, "matKIni.dat");
	//MatrixFileOut(matKIni, "matKini.csv");
	//DSTR << "det(matMIni) = " << matMIni.det() << std::endl;
	//DSTR << "matMIni" << std::endl;	DSTR << matMIni << std::endl;
	//DSTR << "det(matCIni) = " << matCIni.det() << std::endl;
	//DSTR << "matCIni" << std::endl;	DSTR << matCIni << std::endl;

	/// 各種変数の初期化
	matMp.assign(matMIni);
	matKp.assign(matKIni);
	matCp.assign(matCIni);
	xdl.resize(NDof, 0.0);
	vl.resize(NDof, 0.0);
	al.resize(NDof, 0.0);
	fl.resize(NDof, 0.0);
	boundary.resize(NDof, 0.0);
	GetVerticesDisplacement(xdl);		// FemVertexから変位を取ってくる
	CompInitialCondition(matMIni, matKIni, matCIni, fl, xdl, vl, al);
	DSTR << "Initializing Completed." << std::endl;

	// テスト（境界条件の付加）
	vdt = 0.001;
	std::vector< int > veIds = FindVertices(2, Vec3d(0.0, -1.0, 0.0));
	for(int i = 0; i < veIds.size(); i++){
		DSTR << veIds[i] << std::endl;
	}
	Vec3i con = Vec3i(1,1,1);
	for(int i = 0; i < veIds.size(); i++){
		AddBoundaryCondition(veIds[i], con);
	}
	//matCp.clear(0.0);

	ReduceMatrixSize(matMp, matKp, matCp, boundary);
	DSTR << "All matrices has reduced." << std::endl;
	//DSTR << "matMp" << std::endl;	DSTR << matMp << std::endl;
	//DSTR << "matKp" << std::endl;	DSTR << matKp << std::endl;
	//DSTR << "matCp" << std::endl;	DSTR << matCp << std::endl;
	//MatrixFileOut(matKp, "matKp.csv");
	//ScilabDeterminant(matKp, "matKp");
	//ScilabEigenValueAnalysis(matMp, matKp);
#if 0
	// 固有値のサイラボテスト
	VMatrixRe matk;
	matk.resize(5, 5, 0.0);
	matk.item(0, 0) = 200;		matk.item(0, 1) = -100;
	matk.item(1, 0) = -100;	    matk.item(1, 1) = 100;
	matk.item(2, 2) = 20;
	matk.item(3, 3) = 20;
	matk.item(4,4) = 1;

	VMatrixRe matm;
	matm.resize(5, 5, 0.0);
	matm.item(0, 0) = 3;	matm.item(0, 2) = 2;
	matm.item(1, 1) = 3;	matm.item(1, 3) = 2;
	matm.item(2, 0) = 2;	matm.item(2, 2) = 4;
	matm.item(3, 1) = 2;	matm.item(3, 3) = 4;
	matm.item(4,4) = 1;
	DSTR << "ScilabTest" << std::endl;
	ScilabEigenValueAnalysis(matm, matk);
#endif
} 

UTQPTimerFileOut qtimer;
void PHFemVibration::Step(){
	// FemVertexから変位をとってくる
	qtimer.StopCounting("step");
	qtimer.StartCounting("step");
	GetVerticesDisplacement(xdl);
	VVectord xdlp;
	xdlp.assign(xdl);
	VVectord vlp;
	vlp.assign(vl);
	VVectord alp;
	alp.assign(al);
	VVectord flp;
	flp.assign(fl);
	ReduceVectorSize(xdlp, vlp, alp, flp, boundary);

	qtimer.StartCounting("integration");
	switch(analysis_mode){
		case PHFemVibrationDesc::ANALYSIS_DIRECT:
			NumericalIntegration(matMp, matKp, matCp, flp, vdt, xdlp, vlp, alp);
			break;
		case PHFemVibrationDesc::ANALYSIS_MODAL:
			ModalAnalysis(matMp, matKp, matCp, flp, vdt, xdlp, vlp, alp, matMp.height());
			break;
		default:
			break;
	}
	qtimer.StopCounting("integration");

	qtimer.StartCounting("gain");
	fl.clear(0.0);
	// 計算結果をFemVertexに反映
	GainVectorSize(xdlp, vlp, alp, boundary);
	xdl.assign(xdlp);
	vl.assign(vlp);
	al.assign(alp);
	UpdateVerticesPosition(xdl);
	qtimer.StopCounting("gain");

	static int count = 0;
	count++;
	if(count == 5000) qtimer.FileOut("hoge.xls");

#if 0
	DSTR << "vl updated" << std::endl;	DSTR << vl << std::endl;
	DSTR << "xdl updated" << std::endl;	DSTR << xdl << std::endl;
	//for(int i = 0; i < NVertices(); i++){
		Vec3d pos = GetPHFemMesh()->GetVertexPositionL(2);
		//CSVOUT << pos[0] << "," << pos[1] << "," << pos[3] << std::endl;
		pos = GetPHFemMesh()->GetVertexDisplacementL(2);
		CSVOUT << pos[0] << "," << pos[1] << "," << pos[3] << std::endl;
	//}
	DSTR << "///////////////////////////////////////////////" << std::endl;
#endif
}

void PHFemVibration::NumericalIntegration(const VMatrixRe& _M, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v, VVectord& _a){
	/// 数値積分
	switch(integration_mode){
		case PHFemVibrationDesc::INT_EXPLICIT_EULER:
			ExplicitEuler(_M.inv(), _K, _C, _f, _dt, _xd, _v);
			break;
		case PHFemVibrationDesc::INT_IMPLICIT_EULER:
			//ImplicitEuler(_M.inv(), _K, _C, _f, _dt, _xd, _v);
			break;
		case PHFemVibrationDesc::INT_NEWMARK_BETA:
			NewmarkBeta(_M, _K, _C, _f, _dt, _xd, _v, _a, 1.0/4.0);
			break;
		default:
			break;
	}
}

void PHFemVibration::NumericalIntegration(const double& _m, const double& _k, const double& _c, 
	const double& _f, const double& _dt, double& _x, double& _v, double& _a){
	/// 数値積分
	switch(integration_mode){
		case PHFemVibrationDesc::INT_EXPLICIT_EULER:
			ExplicitEuler(1.0/_m, _k, _c, _f, _dt, _x, _v);
			break;
		case PHFemVibrationDesc::INT_IMPLICIT_EULER:
			break;
		case PHFemVibrationDesc::INT_NEWMARK_BETA:
			NewmarkBeta(_m, _k, _c, _f, _dt, _x, _v, _a, 1.0/4.0);
			break;
		default:
			break;

	}
}

// モード解析法（レイリー減衰系）
void PHFemVibration::ModalAnalysis(const VMatrixRe& _M, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v, VVectord& _a, const int nmode){
	//DSTR << "//////////////////////////////////" << std::endl;
	// n:自由度、m:モード次数
	static VVectord evalue;			// 固有値(m)
	static VMatrixRe evector;		// 固有ベクトル(n*m)
	static VVectord w;				// MK系の固有角振動数(m)
	static VVectord wc;				// MKC系の減衰固有角振動数(m)
	static VMatrixRe Mm;			// モード質量行列(m*m)
	static VMatrixRe Km;			// モード剛性行列(m*m)
	static VMatrixRe Cm;			// モード減衰行列(m*m)

	static bool bFirst = true;
	if(bFirst){
		// 固有値・固有ベクトルを求める
		evalue.resize(nmode, 0.0);
		evector.resize(_M.height(), nmode, 0.0);
#if 1
		SubSpace(_M, _K, nmode, 1e-5, evalue, evector);
		DSTR << "eigenvalue springhead" << std::endl;
		DSTR << evalue << std::endl;
		//DSTR << "eigenvector springhead" << std::endl;
		//DSTR << evector << std::endl;
		//MatrixFileOut(evector, "evectorSpringhead.csv");
#else
		VMatrixRe Ms, Ks;
		Ms.assign(_M);
		Ks.assign(_K);
		CompScilabEigenValue(Ms, Ks, evalue, evector);
		evalue.resize(nmode);
		evector.resize(Ms.height(), nmode);
		DSTR << "eigenvalue scilab" << std::endl;
		DSTR << evalue << std::endl;
		//DSTR << "eigenvector scilab" << std::endl;
		//DSTR << evector << std::endl;
		//MatrixFileOut(evector, "evectorScilab.csv");
#endif
		// MK系の固有振動数
		w.resize(evalue.size(), 0.0);
		for(int i = 0; i < (int)w.size(); i++){
			w[i] = sqrt(evalue[i]) / (2.0 * M_PI);
		}
		DSTR << "eigen Vibration Value" << std::endl;
		DSTR << w << std::endl;

		// モード質量、剛性, 減衰行列の計算
		Mm.assign(evector.trans() * _M * evector);
		Km.assign(evector.trans() * _K * evector);
		Cm.assign(evector.trans() * _C * evector);
		bFirst = false;
	}

	VVectord q;		// モード振動ベクトル(m)
	VVectord qv;	// モード振動速度ベクトル(m)
	VVectord qa;	// モード振動加速度ベクトル(m)
	VVectord fm;	// モード外力(m)
	q.assign(evector.trans() * _M * _xd);
	qv.assign(evector.trans() * _M * _v);
	qa.assign(evector.trans() * _M * _a);
	fm.assign(evector.trans() * _f);
#if 0
	// 行列で計算
	NumericalIntegration(Mm, Km, Cm, fm, _dt, q, qv, qa); 
#else
	// 1次独立の連立方程式なので、各方程式毎に計算
	for(int i = 0; i < nmode; i++){
		NumericalIntegration(Mm[i][i], Km[i][i], Cm[i][i], fm[i], _dt, q[i], qv[i], qa[i]);
	}
#endif
	_xd = evector * q;
	_v = evector * qv;
	_a = evector * qa;
}

//* 積分関数
/////////////////////////////////////////////////////////////////////////////////////////
//行列版
void PHFemVibration::CompInitialCondition(const VMatrixRe& _M, const VMatrixRe& _K, const VMatrixRe& _C,
		const VVectord& _f, VVectord& _x, VVectord& _v, VVectord& _a){
	_a = _M.inv() * (- 1.0 * ( _C * _v) - 1.0 * (_K * _x) + _f);
}

void PHFemVibration::ExplicitEuler(const VMatrixRe& _MInv, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v){
	int NDof = NVertices() * 3;
	VVectord tmp;
	tmp.resize(NDof, 0.0);
	tmp = -1 * (_K * _xd) - (_C * _v) + _f;
	_xd += _v * _dt;
	_v += _MInv * tmp * _dt;
}

// 定式化しなおさないといけない.2013.1.3
void PHFemVibration::ImplicitEuler(const VMatrixRe& _MInv, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v){
#if 0
	// 外力をまだ組み込んでない
	int NDof = GetPHFemMesh()->vertices.size() * 3;
	VMatrixRe E;	// 単位行列
	E.resize(NDof, NDof);
	E.clear(0.0);
	for(int i = 0; i < NDof; i++){
		E[i][i] = 1.0;
	}
	VMatrixRe _Kt;
	_Kt.resize(NDof, NDof);
	_Kt.clear(0.0);	
	VMatrixRe _CtInv;
	_CtInv.resize(NDof, NDof);
	_CtInv.clear(0.0);
	VMatrixRe _DInv;
	_DInv.resize(NDof, NDof);
	_DInv.clear(0.0);

	_Kt = _MInv * _K * _dt;
	_CtInv = (E + _MInv * _C * _dt).inv();
	_DInv = (E + _CtInv * _Kt * vdt).inv();

	xdl = _DInv * (xdl + _CtInv * (vl + _Kt * xdlInit) * vdt);
	vl = _CtInv * (vl - _Kt * (xdl - xdlInit));
	//DSTR << "Integrate" << std::endl;
	//DSTR << "_K" << std::endl;
	//DSTR << _K << std::endl;
	//DSTR << "_CInv" << std::endl;
	//DSTR << _CInv << std::endl;
	//DSTR << "_DInv" << std::endl;
	//DSTR << _DInv << std::endl;
#endif
}

void PHFemVibration::NewmarkBeta(const VMatrixRe& _M, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _x, VVectord& _v, VVectord& _a, const double b){
	int NDof = (int)_x.size();
	VMatrixRe _SInv;
	_SInv.resize(NDof, NDof, 0.0);
	VVectord _Ct;
	_Ct.resize(NDof, 0.0);
	VVectord _Kt;
	_Kt.resize(NDof, 0.0);
	VVectord _al;	// 前回の加速度
	_al.assign(_a);

	_SInv = (_M + (0.5 * _C * vdt) + (b * _K * pow(vdt, 2))).inv();
	_Ct = _C * (_v + (0.5 * _a * _dt));
	_Kt = _K * (_x + (_v * _dt) + (_a * pow(_dt, 2) * (0.5 - b)));
	_a = _SInv * (_f - _Ct - _Kt);
	_x += (_v * _dt) + ((0.5 - b) * _al * pow(_dt, 2)) + (_a * b * pow(_dt, 2));	// xの更新が先
	_v += 0.5 * (_al + _a) * _dt;
}

void PHFemVibration::ExplicitEuler(const double& _mInv, const double& _k, const double& _c, 
		const double& _f, const double& _dt, double& _x, double& _v){
	double tmp;
	tmp = -1 * (_k * _x) - (_c * _v) + _f;
	_x += _v * _dt;
	_v += _mInv * tmp * _dt;
}

void PHFemVibration::NewmarkBeta(const double& _m, const double& _k, const double& _c,
		const double& _f, const double& _dt, double& _x, double& _v, double& _a, const double b){
	double _al = _a;	// 前回の加速度
	double _sInv = 1.0 / (_m + (0.5 * _c * vdt) + (b * _k * pow(vdt, 2)));
	double _ct = _c * (_v + (0.5 * _a * _dt));
	double _kt = _k * (_x + (_v * _dt) + (_a * pow(_dt, 2) * (0.5 - b)));
	_a = _sInv * (_f - _ct - _kt);
	_x += (_v * _dt) + ((0.5 - b) * _al * pow(_dt, 2)) + (_a * b * pow(_dt, 2));	// xの更新が先
	_v += 0.5 * (_al + _a) * _dt;
}

#if 0
void PHFemVibration::ModalAnalysis(const VMatrixRe& _M, const VMatrixRe& _K, const VMatrixRe& _C, 
		const VVectord& _f, const double& _dt, VVectord& _xd, VVectord& _v, const int nmode){
	//DSTR << "//////////////////////////////////" << std::endl;
	// 解析+畳み込み積分版？
	// まだ減衰を含んでいない
	// n:自由度、m:モード次数
	static double time = 0.0;		// 経過時間
	static VVectord evalue;			// 固有振動数(m)
	static VMatrixRe evector;		// 固有ベクトル(n*m)
	static VVectord w;				// 固有角振動数(m)
	static VVectord q0;				// モード振動ベクトルの初期値(m)
	static VVectord qv0;			// モード振動速度ベクトルの初期値(m)
	static VMatrixRe fC;			// モード外力の積分cos成分(0列:今回, 1列前回)(m)
	static VMatrixRe fS;			// モード外力の積分sin成分(0列:今回, 1列前回)(m)

	static bool bFirst = true;
	if(bFirst){
		evalue.resize(nmode, 0.0);
		evector.resize(_M.height(), nmode, 0.0);
		SubSpace(_M, _K, nmode, 1e-10, evalue, evector);
		DSTR << "eigenvalue" << std::endl;
		DSTR << evalue << std::endl;
		DSTR << "eigenvector" << std::endl;
		DSTR << evector << std::endl;

		// 初期変位、速度をモード座標系にする
		q0.resize(nmode, 0.0);
		q0 = evector.trans() * _M * _xd;
		qv0.resize(nmode, 0.0);
		qv0 = evector.trans() * _M * _v / _dt;
		fC.resize(nmode, 2, 0.0);
		fS.resize(nmode, 2, 0.0);
		w.resize(evalue.size());
		for(int i = 0; i < (int)w.size(); i++){
			w[i] = sqrt(evalue[i]);	// 固有角振動数
		}
		bFirst = false;
	}

	// 積分
	VVectord q;		// 更新後のモード振動ベクトル(m)
	q.resize(nmode, 0.0);
	VVectord qv;	// 更新後のモード振動速度ベクトル(m)
	qv.resize(nmode, 0.0);
	VVectord fM;	// モード外力(m)
	fM.resize(nmode, 0.0);
	fM =  evector.trans() * _f;

	for(int i = 0; i < nmode; i++){
		double wt = w[i] * time;
		double ftemp = fM[i] * cos(wt);
		fC.item(i, 0) += 0.5 * (ftemp + fC.item(i, 1)) * _dt;
		fC.item(i, 1) = ftemp;
		ftemp = fM[i] * sin(wt);
		fS.item(i, 0) += 0.5 * (ftemp + fS.item(i, 1)) * _dt;
		fS.item(i, 1) = ftemp;
		q[i] = q0[i] * cos(wt) + qv0[i] * sin(wt) / w[i] + (fC.item(i, 0) * sin(wt) - fS.item(i, 0) * cos(wt))/ w[i];
	}
	//DSTR << _f << std::endl;
	//DSTR << fM << std::endl;
	//DSTR << fC << std::endl;
	//DSTR << fS << std::endl;
	//DSTR << time << std::endl;

	_xd = evector * q;
	time += _dt;
}
#endif

//* 計算関数（そのうちTMatirixへ）
/////////////////////////////////////////////////////////////////////////////////////////
/// サブスペース法（同時逆反復法？）
/// VMatrixRe& _M:質量行列（正値対称）, VMatrixRe& _K:剛性行列（正値対称）
/// int nmode:モード次数, double epsilon:収束条件, VVectord& e:固有振動数[Hz], VMatrixRe& v:固有ベクトル(列順）
void PHFemVibration::SubSpace(const VMatrixRe& _M, const VMatrixRe& _K, 
	const int nmode, const double epsilon, VVectord& evalue, VMatrixRe& evector){
	const int size = _M.height();
	if(_K.det() <= 0){
		DSTR << "Stiffness Matrix is not regular matrix." << std::endl;
		return;
	}
	if(nmode > size) assert(0);
	/// 初期化
	evalue.resize(nmode, 0.0);			// 固有値
	evector.resize(size, nmode, 0.0);	// 固有ベクトル
	// 初期値ベクトル
	VMatrixRe y;
	y.resize(size, nmode, 0.0);
	VMatrixRe ylast;
	ylast.resize(size, nmode, 0.0);
	VVectord yini;
	yini.resize(size, 1.0);		// 初期値は1.0
	yini.unitize();
	for(int i = 0; i < nmode; i++){
		y.col(i) = yini;
		ylast.col(i) = yini;
	}	

	/// _M, _Kをコレスキー分解
	// _AInvの計算はコレスキー分解値を使ってfor文で計算したほうが速いはず。
	// 今は速さを気にせず逆行列を計算してる。
	VMatrixRe _Mc;
	_Mc.resize(size, size, 0.0);
	cholesky(_M, _Mc);	
	VMatrixRe _Kc;
	_Kc.resize(size, size, 0.0);
	cholesky(_K, _Kc);
	//DSTR << "cholesky" << std::endl;
	//DSTR << _Mc << std::endl;
	//DSTR << _Kc  << std::endl;

	VMatrixRe _AInv;			
	_AInv.resize(size, size, 0.0);
	_AInv = _Mc.trans() * (_Kc.inv()).trans() * _Kc.inv() * _Mc;

	/// 反復計算
	for(int k = 0; k < nmode; k++){
		VVectord z;
		z.resize(size, 0.0);
		int cnt = 0;
		while(1){
			// zの計算
			z = _AInv * y.col(k);	 
			// グラム・シュミットの直交化
			VVectord c;
			c.resize(size, 0.0);
			for(int i = 0; i < k; i++){
				double a = y.col(i) * z;
				c += a * y.col(i);
			}
			y.col(k) = z - c;
			y.col(k).unitize();

			double error = 0.0;
			error = sqrt((ylast.col(k) - y.col(k)) * (ylast.col(k) - y.col(k)));
			ylast.col(k) = y.col(k);
			if(abs(error) < epsilon) break;
			cnt++;
			if(cnt > 1e5){
				DSTR << "Can not converge in subspace" << std::endl;
				break;
			}
		}
		evector.col(k) = _Mc.trans().inv() * y.col(k);		// 固有ベクトル
		evalue[k] = 1.0 / (y.col(k) * _AInv * y.col(k));	// 固有値
	}
}

//* 各種設定関数
/////////////////////////////////////////////////////////////////////////////////////////
void PHFemVibration::SetAnalysisMode(PHFemVibrationDesc::ANALYSIS_MODE mode){
	analysis_mode = mode;
}

void PHFemVibration::SetIntegrationMode(PHFemVibrationDesc::INTEGRATION_MODE mode){
	integration_mode = mode;
}

void PHFemVibration::GetVerticesDisplacement(VVectord& _xd){
	/// FemVertexから変位を取ってくる
	// u = (u0, v0, w0, ...., un-1, vn-1, wn-1)の順
	int NVer = NVertices();
	_xd.resize(NVer * 3);
	for(int i = 0; i < NVer; i++){
		int id = i * 3;
		Vec3d disp = GetPHFemMesh()->GetVertexDisplacementL(i);
		_xd[id] = disp.x;
		_xd[id + 1] = disp.y;		
		_xd[id + 2] = disp.z;
	}
}

void PHFemVibration::UpdateVerticesPosition(VVectord& _xd){
	/// 計算結果をFemVertexに戻す
	// u = (u0, v0, w0, ...., un-1, vn-1, wn-1)の順
	int NVer = NVertices();
	PHFemMeshNew* mesh = GetPHFemMesh();
	for(int i = 0; i < NVer; i++){
		int id = i * 3;
		Vec3d initialPos = mesh->GetVertexInitPositionL(i);
		GetPHFemMesh()->vertices[i].pos.x = _xd[id] + initialPos.x;
		GetPHFemMesh()->vertices[i].pos.y = _xd[id + 1] + initialPos.y;		
		GetPHFemMesh()->vertices[i].pos.z = _xd[id + 2] + initialPos.z;
	}
}

std::vector< int > PHFemVibration::FindVertices(const int vtxId, const Vec3d _vecl){
	std::vector< int > ve;
	PHFemMeshNew* mesh = GetPHFemMesh();
	std::vector< int > sv = mesh->surfaceVertices;
	Vec3d base = mesh->vertices[vtxId].pos;
	for(int i = 0; i < (int)sv.size(); i++){
		Vec3d rel = mesh->vertices[sv[i]].pos - base;
		double dot = rel * _vecl;
		if(abs(dot) < 1e-5) ve.push_back(sv[i]);
	}
	return ve;
}

bool PHFemVibration::AddBoundaryCondition(VMatrixRe& mat, const int id){
	int n = mat.height();
	if(id > n - 1) return false;
	mat.col(id).clear(0.0);
	mat.row(id).clear(0.0);
	mat.item(id, id) = 1.0;
	return true;
}

bool PHFemVibration::AddBoundaryCondition(const int vtxId, const Vec3i dof = Vec3i(1, 1, 1)){
	int NVer = NVertices();
	if(0 <= vtxId && vtxId <= NVer -1){
		for(int i = 0; i < 3; i++){
			if(dof[i] == 1){
				const int id = vtxId * 3 + i;
				boundary[id] = 1;
			}
		}
		return true;
	}
	return false;
}

bool PHFemVibration::AddBoundaryCondition(const VVector< Vec3i > bcs){ 
	int NVer = NVertices();
	if(NVer != bcs.size()) return false;
	for(int i = 0; i < (int)bcs.size(); i++){
		for(int j = 0; j < 3; j++){
			if(bcs[i][j] == 1){
				const int id = i + j;
				boundary[id] = 1;
			}
		}
	}
	return true;
}

void PHFemVibration::CompBoundaryMatrix(VMatrixRe& _L, VMatrixRe& _R, const VVector< int > bc){
	int n = bc.size();		// 全自由度
	int m = 0;				// 境界条件導入後の自由度
	for(int i = 0; i < n; i++){
		if(bc[i] == 0) m++;
	}
	_L.resize(m, n, 0.0);
	_R.resize(n, m, 0.0);
	int index = 0;
	for(int i = 0; i < n; i++){
		if(bc[i] == 0){
			_L[index][i] = 1.0;
			_R[i][index] = 1.0;
			index++;		
		}
	}
}

void PHFemVibration::ReduceMatrixSize(VMatrixRe& mat, const VVector< int > bc){
	VMatrixRe matL, matR;
	CompBoundaryMatrix(matL, matR, bc);
	VMatrixRe tmp;
	tmp.assign(matL * mat * matR);
	mat.assign(tmp);
}

void PHFemVibration::ReduceMatrixSize(VMatrixRe& _M, VMatrixRe& _K, VMatrixRe& _C, const VVector< int > bc){
	VMatrixRe matL, matR;
	CompBoundaryMatrix(matL, matR, bc);
	VMatrixRe tmp;
	tmp.assign(matL * _M * matR);
	_M.assign(tmp);
	tmp.assign(matL * _K * matR);
	_K.assign(tmp);
	tmp.assign(matL * _C * matR);
	_C.assign(tmp);

}

void PHFemVibration::ReduceVectorSize(VVectord& r, const VVector< int > bc){
	VMatrixRe matL, matR;
	CompBoundaryMatrix(matL, matR, bc);
	VVectord tmp;
	tmp.assign(matL * r);
	r.assign(tmp);
}

void PHFemVibration::ReduceVectorSize(VVectord& _xd, VVectord& _v, VVectord& _a, VVectord& _f,const VVector< int > bc){
	VMatrixRe matL, matR;
	CompBoundaryMatrix(matL, matR, bc);
	VVectord tmp;
	tmp.assign(matL * _xd);
	_xd.assign(tmp);
	tmp.assign(matL * _v);
	_v.assign(tmp);	
	tmp.assign(matL * _a);
	_a.assign(tmp);	
	tmp.assign(matL * _f);
	_f.assign(tmp);
}

void PHFemVibration::GainVectorSize(VVectord& r, const VVector< int > bc){
	VMatrixRe matL, matR;
	CompBoundaryMatrix(matL, matR, bc);
	VVectord tmp;
	tmp.assign(matR * r);
	r.assign(tmp);
}

void PHFemVibration::GainVectorSize(VVectord& _xd, VVectord& _v, VVectord& _a, const VVector< int > bc){
	VMatrixRe matL, matR;
	CompBoundaryMatrix(matL, matR, bc);
	VVectord tmp;
	tmp.assign(matR * _xd);
	_xd.assign(tmp);
	tmp.assign(matR * _v);
	_v.assign(tmp);
	tmp.assign(matR * _a);
	_a.assign(tmp);
}

bool PHFemVibration::AddVertexForceL(int vtxId, Vec3d fL){
	if(0 <= vtxId && vtxId <= NVertices() -1){
		int id = vtxId * 3;
		fl[id] += fL.x;
		fl[id + 1] += fL.y;
		fl[id + 2] += fL.z;
		return true;
	}
	return false;
}


bool PHFemVibration::AddVertexForceW(int vtxId, Vec3d fW){
	if(0 <= vtxId && vtxId <= NVertices() -1){
		Vec3d fL = GetPHFemMesh()->GetPHSolid()->GetOrientation().Inv() * fW;
		AddVertexForceL(vtxId, fL);
		return true;
	}
	return false;
}

bool PHFemVibration::AddVertexForceW(VVector< Vec3d > fWs){
	if(NVertices() != fWs.size()) return false;
	for(int i = 0; i < (int)fWs.size(); i++){
		Vec3d fL = GetPHFemMesh()->GetPHSolid()->GetOrientation().Inv() * fWs[i];
		AddVertexForceL(i, fL);
	}
	return true;
}

void PHFemVibration::AddForce(Vec3d fW, Vec3d posW){
	PHFemMeshNew* mesh = GetPHFemMesh();
	Posed inv = mesh->GetPHSolid()->GetPose().Inv();
	Vec3d fL = inv * fW;
	Vec3d posL = inv * posW;
	std::vector< int > faceIds;
	std::vector< Vec3d > closestPoints;
	if(!FindNeigborFaces(posW, faceIds, closestPoints)) return;
	// 形状関数値から各頂点に加える力を計算
	// faceが1つ:点-面で普通の操作
	// faceが2つ:辺で接している、どちらか1つの面を選べば良い
	// faceが3つ:点で接している。これもどれか1つの面を選べば良い
	// 以上から場合分けする必要がないはず。
	int tetId = mesh->FindTetFromFace(faceIds[0]);
	Vec4d v;
	if(!mesh->CompTetShapeFunctionValue(tetId, posL, v)) return;
	for(int j = 0; j < 4; j++){
		int vtxId = mesh->tets[tetId].vertexIDs[j];
		Vec3d fdiv = v[j] * fL;
		AddVertexForceL(vtxId, fdiv);
		//DSTR << vtxId << " " << fdiv << std::endl;
	}
}

//* scilabデバック
/////////////////////////////////////////////////////////////////////////////////////////

void PHFemVibration::ScilabEigenValueAnalysis(VMatrixRe& _M, VMatrixRe& _K){
	DSTR << "////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	DSTR << "ScilabEigenValueAnalysis Start." << std::endl;
	DSTR << "det(M) springhead2 : " << _M.det() << std::endl;
	DSTR << "det(K) springhead2 : " << _K.det() << std::endl;
#if 0
	VVectord evalue;
	VMatrixRe evector;
	SubSpace(_M, _K, _M.height(), 1e-5, evalue, evector);
	DSTR << "eigenvalue springhead2" << std::endl;
	DSTR << evalue << std::endl;
	DSTR << "eigen vibration value springhead2" << std::endl;
	for(int i = 0; i < (int)evalue.size(); i++){
		evalue[i] = sqrt(evalue[i])/2/M_PI;
	}
	DSTR << evalue << std::endl;
	DSTR << "eigenvector springhead2" << std::endl;
	DSTR << evector << std::endl;
#endif
	if(!IsScilabStarted){
		DSTR << "Scilab has not started" << std::endl;
		return;
	}
	ScilabJob("clear;");
	ScilabSetMatrix("M", _M);
	ScilabJob("detM = det(M);");
	std::cout << "det(M) scilab : ";
	ScilabJob("disp(detM);");	
	ScilabSetMatrix("K", _K);
	ScilabJob("detK = det(K);");
	std::cout << "det(K) scilab : ";
	ScilabJob("disp(detK);");

	ScilabJob("[P D] = spec(inv(M) * K);");
	VMatrixRe D;
	ScilabGetMatrix(D, "D");
	DSTR << "eigenvalue scilab" << std::endl;
	DSTR << D << std::endl;
	std::vector< double > ds;
	for(int i =0; i < D.height(); i++){
		ds.push_back(sqrt(D[i][i]) * 0.5 / M_PI);
	}
	std::sort(ds.begin(), ds.end());
	DSTR << "eigen vibration value scilab" << std::endl;
	for(int i =0; i < D.height(); i++){
		DSTR << ds[i] << std::endl;
	}
	VMatrixRe P;
	ScilabGetMatrix(P, "P");
	DSTR << "eigenvector scilab" << std::endl;
	DSTR << P << std::endl;
	DSTR << "ScilabEigenValueAnalysis End." << std::endl;
	DSTR << "////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
}

struct ScilabEigenValue{
	double e;
	VVector< double > v;
	int id;
		bool operator()(const ScilabEigenValue& a, const ScilabEigenValue& b){
		return a.e < b.e;
	}
};

void PHFemVibration::CompScilabEigenValue(VMatrixRe& _M, VMatrixRe& _K, VVectord& e, VMatrixRe& v){
	if(!IsScilabStarted){
		DSTR << "Scilab has not started" << std::endl;
		return;
	}
	ScilabJob("clear;");
	ScilabSetMatrix("M", _M);
	ScilabSetMatrix("K", _K);

	ScilabJob("[P D] = spec(inv(M) * K);");
	VMatrixRe D;	// 固有値(対角）
	VMatrixRe P;	// 固有ベクトル
	ScilabGetMatrix(D, "D");
	ScilabGetMatrix(P, "P");
	// 固有値を小さい順にソート
	std::vector< ScilabEigenValue > ds;
	ds.resize(D.height());
	for(int i =0; i < ds.size(); i++){
		ds[i].e = D[i][i];
		ds[i].v = P.col(i);
		ds[i].id = i;
	}
	std::sort(ds.begin(), ds.end(), ScilabEigenValue());
	e.resize(ds.size(), 0.0);
	v.resize(ds.size(), ds.size(), 0.0);
	for(int i =0; i < ds.size(); i++){
		e[i] = ds[i].e;
		v.col(i) = ds[i].v;
	}
	// 固有ベクトルを質量正規固有モードに変換
	// Subspaceは正規化されているはず
	VMatrixRe V;
	V.assign((v.trans() * _M * v).inv());
	for(int i = 0; i < V.height(); i++){
		v.col(i) *= sqrt(V.item(i, i));
	}
}

void PHFemVibration::MatrixFileOut(VMatrixRe mat, std::string filename){
	std::ofstream ofs(filename);
	if (!ofs){
		DSTR << "Can not open the file : " << filename << std::endl;
		return;
	}

	ofs.precision(element_limit);
	for (int i = 0; i < (int)mat.height(); i++){
		for(int j = 0; j < (int)mat.width(); j ++){
			ofs << mat.item(i, j) << ",";
		}
		ofs << std::endl;
    }
    ofs.close();
}

std::vector< int > PHFemVibration::FindNeigborTetrahedron(Vec3d pos){
	std::vector< int > tetIds;
	PHFemMeshNew* mesh = GetPHFemMesh();
	std::vector< FemTet > tets = mesh->tets;
	int ntets = (int)tets.size();
	for(int i = 0; i < ntets; i++){
		int in = 0;
		for(int j = 0; j < 4; j++){
			int faceId = tets[i].faceIDs[j];
			// *四面体の面法線を取得したいが、形状内部にある四面体の面法線は正しく取得できていない。
			Vec3d normal = mesh->CompFaceNormal(faceId);
			FemFace face = mesh->faces[faceId];
			Vec3d vp[3];
			vp[0] = mesh->vertices[face.vertexIDs[0]].pos;
			vp[1] = mesh->vertices[face.vertexIDs[1]].pos;
			vp[2] = mesh->vertices[face.vertexIDs[2]].pos;
			Vec3d p0 = vp[0] - pos;		// posからvp[0]方向のベクトル
			double dot = p0 * normal;	// posからfaceまでの距離
			Vec3d ortho = dot * normal;	// posからfaceへの射影ベクトル
			// porthoが四面体内にあるかどうか判定
			// 内積を取って、normalと同方向(内積が=＞0)なら中
			dot = normal * ortho;
			if(dot < 0) break;
			in++;
		}
		if(in == 4) tetIds.push_back(i);
	}
	return tetIds;
}

bool PHFemVibration::FindNeigborFaces(Vec3d pos, std::vector< int >& faceIds, std::vector< Vec3d >& closestPoints){
	// ワールド座標系で計算
	faceIds.clear();
	closestPoints.clear();
	PHFemMeshNew* mesh = GetPHFemMesh();
	Posed pose = mesh->GetPHSolid()->GetPose();
	std::vector< FemFace > faces = mesh->faces;
	int nsf = GetPHFemMesh()->nSurfaceFace;
	double dist = DBL_MAX;
	for(int i = 0; i < nsf; i++){
		Vec3d normal = (pose.Ori() * mesh->CompFaceNormal(i)).unit();
		Vec3d vp[3];
		vp[0] = pose * mesh->vertices[faces[i].vertexIDs[0]].pos;
		vp[1] = pose * mesh->vertices[faces[i].vertexIDs[1]].pos;
		vp[2] = pose * mesh->vertices[faces[i].vertexIDs[2]].pos;
		Vec3d p0 = vp[0] - pos;
		double dot = p0 * normal;
		Vec3d ortho = dot * normal;
		Vec3d portho = pos + ortho;	// posをface上に射影した位置

		// porthoが面内にあるかどうか判定
		// 外積を取って、normalと同じ方向(内積が>0)なら中
		Vec3d vec[2];
		vec[0] = portho - vp[0];
		vec[1] = vp[1] - vp[0];
		if((vec[0] % vec[1]) * normal < 0.0) continue;
		vec[0] = portho - vp[1];	
		vec[1] = vp[2] - vp[1];
		if((vec[0] % vec[1]) * normal < 0.0) continue;
		vec[0] = portho - vp[2];		
		vec[1] = vp[0] - vp[2];
		if((vec[0] % vec[1]) * normal < 0.0) continue;
		if(dot < dist){
			// 前回よりも点-面間の距離が近い場合は近い方を選ぶ
			dist = ortho.norm();
			faceIds.clear();
			faceIds.push_back(i);
			closestPoints.push_back(portho);
		}else if(dot == dist){
			// 前回と距離が同じ場合は加える
			faceIds.push_back(i);
			closestPoints.push_back(portho);
		}
	}
	if(faceIds.size()) return true;
	else	return false;
}


}