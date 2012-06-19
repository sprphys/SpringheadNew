﻿#include <Physics/PHHapticEngineLD.h>

namespace Spr{;
//----------------------------------------------------------------------------
// PHHapticLoopLD
void PHHapticLoopLD::Step(){
	UpdateInterface();
	HapticRendering();
	LocalDynamics();
}
void PHHapticLoopLD::HapticRendering(){
	PHHapticRenderInfo info;
	info.pointers = GetHapticPointers();
	info.hsolids = GetHapticSolids();
	info.sps = GetSolidPairsForHaptic();
	info.hdt = GetHapticTimeStep();
	info.pdt = GetPhysicsTimeStep();
	info.loopCount = loopCount;
	info.bInterpolatePose = false;
	GetHapticRender()->HapticRendering(info);
}

void PHHapticLoopLD::LocalDynamics(){
	double pdt = GetPhysicsTimeStep();
	double hdt = GetHapticTimeStep();
	for(int i = 0; i < NHapticSolids(); i++){
		PHSolidForHaptic* hsolid = GetHapticSolid(i);
		if(hsolid->doSim == 0) continue;
		if(hsolid->GetLocalSolid()->IsDynamical() == false) continue;
		PHSolid* localSolid = &hsolid->localSolid;
		SpatialVector vel;
		vel.v() = localSolid->GetVelocity();
		vel.w() = localSolid->GetAngularVelocity();
		if(loopCount == 1){
			vel += (hsolid->curb - hsolid->lastb) *  pdt;	// 衝突の影響を反映
		}
		for(int j = 0; j < NHapticPointers(); j++){
			PHHapticPointer* pointer = GetHapticPointer(j);
			PHSolidPairForHaptic* sp = GetSolidPairForHaptic(i, pointer->GetPointerID());
			if(sp->inLocal == 0) continue;
			vel += (sp->A * sp->force) * hdt;			// 力覚ポインタからの力による速度変化
		}
		vel += hsolid->b * hdt;
		localSolid->SetVelocity(vel.v());		
		localSolid->SetAngularVelocity(vel.w());
		localSolid->SetOrientation(( Quaterniond::Rot(vel.w() * hdt) * localSolid->GetOrientation()).unit());
		localSolid->SetCenterPosition(localSolid->GetCenterPosition() + vel.v() * hdt);

 		localSolid->SetUpdated(true);
		localSolid->Step();
	}
}

//----------------------------------------------------------------------------
// PHHapticEngineLD
PHHapticEngineLD::PHHapticEngineLD(){
	hapticLoop = &hapticLoopLD;
	hapticLoop->engineImp = this;
	states = ObjectStatesIf::Create();
}

void PHHapticEngineLD::Step1(){
	lastvels.clear();
	for(int i = 0; i < NHapticSolids(); i++){
		SpatialVector vel;
		vel.v() = GetHapticSolid(i)->sceneSolid->GetVelocity();
		vel.w() = GetHapticSolid(i)->sceneSolid->GetAngularVelocity();
		lastvels.push_back(vel);
	}
}
void PHHapticEngineLD::Step2(){
	// 更新後の速度、前回の速度差から定数項を計算
	for(int i = 0; i < NHapticSolids(); i++){
		// 近傍の剛体のみ
		if(GetHapticSolid(i)->doSim == 0) continue;
		PHSolid* solid = GetHapticSolid(i)->sceneSolid;
		SpatialVector dvel;
		dvel.v() = solid->GetVelocity();
		dvel.w() = solid->GetAngularVelocity();
		GetHapticSolid(i)->curb = (dvel - lastvels[i]) / GetPhysicsTimeStep();
	}

	engine->StartDetection();
	PredictSimulation3D();
}

/// 1対1のshapeで、1点の接触のみ対応
void PHHapticEngineLD::PredictSimulation3D(){
	engine->bPhysicStep = false;
	/** PHSolidForHapticのdosim > 0の物体に対してテスト力を加え，
		すべての近傍物体について，アクセレランスを計算する */
	PHSceneIf* phScene = engine->GetScene();

	//#define DIVIDE_STEP
	#ifdef DIVIDE_STEP
	/// 予測シミュレーションのために現在の剛体の状態を保存する
	states2->SaveState(phScene);		
	///	LCPの直前までシミュレーションしてその状態を保存
	phScene->ClearForce();
	phScene->GenerateForce();
	phScene->IntegratePart1();
	#endif
	/// 予測シミュレーションのために現在の剛体の状態を保存する
	phScene->GetConstraintEngine()->SetBSaveConstraints(true);
	states->Clear();
	states->SaveState(phScene);	
#if 1
	/// テストシミュレーション実行
	for(int i = 0; i < NHapticSolids(); i++){
		if(GetHapticSolid(i)->doSim == 0) continue;
		PHSolidForHaptic* hsolid = GetHapticSolid(i);
		PHSolid* phSolid = hsolid->sceneSolid;
		/// 現在の速度を保存
		SpatialVector curvel, nextvel; 
		curvel.v() = phSolid->GetVelocity();			// 現在の速度
		curvel.w() = phSolid->GetAngularVelocity();		// 現在の角速度		

		//DSTR<<" 力を加えないで1ステップ進める--------------------"<<std::endl;
		/// 何も力を加えないでシミュレーションを1ステップ進める
		#ifdef DIVIDE_STEP
		phScene->IntegratePart2();
		#else
		phScene->Step();
		#endif 
		nextvel.v() = phSolid->GetVelocity();
		nextvel.w() = phSolid->GetAngularVelocity();
		/// アクセレランスbの算出

		hsolid->lastb = hsolid->b;
		double pdt = phScene->GetTimeStep();
		hsolid->b = (nextvel - curvel)/pdt;

		states->LoadState(phScene);						// 現在の状態に戻す

		/// HapticPointerの数だけ力を加える予測シミュレーション
		for(int j = 0; j < NHapticPointers(); j++){
			PHHapticPointer* pointer = GetHapticPointer(j);
			PHSolidPairForHaptic* solidPair = GetSolidPairForHaptic(i, pointer->GetPointerID());
			if(solidPair->inLocal == 0) continue;
			PHShapePairForHaptic* sp = solidPair->shapePairs.item(0, 0);	// 1形状のみ対応
			Vec3d cPoint = sp->shapePoseW[0] * sp->closestPoint[0];		// 力を加える点(ワールド座標)
			Vec3d normal = -1 * sp->normal;

			TMatrixRow<6, 3, double> u;			// 剛体の機械インピーダンス?
			TMatrixRow<3, 3, double> force;		// 加える力
			u.clear(0.0);
			force.clear(0.0);

			float minTestForce = 0.5;		// 最小テスト力

			// 3方向のテスト力をつくる
			Vec3d testForce;
			if(solidPair->force.norm() == 0){
				testForce = minTestForce * normal;
			}else{
				testForce = solidPair->force;
				solidPair->force = Vec3d();
			}
			// テスト力に対して垂直方向のベクトル2本を計算
			Vec3d base1 = testForce.unit();
			Vec3d base2 = Vec3d(1, 0, 0) - (Vec3d(1, 0, 0) * base1) * base1;
			if (base2.norm() > 0.1){
				base2.unitize();
			}else{
				base2 = Vec3d(0, 1, 0) - (Vec3d(0, 1, 0) * base1) * base1;
				base2.unitize();
			}
			Vec3d base3 = base1 ^ base2;
#if 1
			// 垂直なベクトルをもとめブレンド
			force.col(0) = testForce;
			force.col(1) = force.col(0).norm() * (base1 + base2).unit();
			force.col(2) = force.col(0).norm() * (base1 + base3).unit();
#else
			// testForceを含む3方向のベクトル(テント型に）
			// 安定していない
			Vec3d base12 = (base1 + base2).unit();	// base1, base2間のベクトル
			Vec3d base23 = (-base2 - base3).unit();	// -base2, -base3間のベクトル
			Vec3d base32 = (base3 - base2).unit();	// base3, -base2間のベクトル
			double a = testForce.norm() / ( base12 * testForce.unit());
			//DSTR << (a * base12) * testForce.unit() << "," << testForce.norm() <<  std::endl;
			force.col(0) = a * base12;
			force.col(1) = a * (base1 + base23).unit();
			force.col(2) = a * (base1 + base32).unit();
			//DSTR << force << std::endl;
			//DSTR << base12 * base23 << "," << base12 * base32 << std::endl;
			//DSTR << (base12 + base23 + base32).unit() << std::endl;

#endif

			/// テスト力を3方向に加える	
			for(int m = 0; m < 3; m++){
				phSolid->AddForce(force.col(m), cPoint);
				#ifdef DIVIDE_STEP
				phScene->IntegratePart2();
				#else
				phScene->Step();
				#endif
				nextvel.v() = phSolid->GetVelocity();
				nextvel.w() = phSolid->GetAngularVelocity();
				u.col(m) = (nextvel - curvel) / pdt - hsolid->b;
				states->LoadState(phScene);			
				//DSTR << "force.col" << m << " " << force.col(m) << force.col(m).norm() << std::endl;
			}

			solidPair->A = u  * force.inv();	// m/(Ns2)
#if 0
			DSTR << "i = " << i << std::endl;
			DSTR << "j = " << j << std::endl;
			DSTR << "f" << std::endl;
			DSTR << force << std::endl;
			DSTR << "u" << std::endl;
			DSTR << u << std::endl;
			DSTR << "b" << std::endl;
			DSTR << hsolid->b << std::endl;
			DSTR << "A" << std::endl;
			DSTR << solidPair->A << std::endl;
#endif
		}
	}
#endif
	///--------テストシミュレーション終了--------
#ifdef DIVIDE_STEP
	states2->LoadState(phScene);							// 元のstateに戻しシミュレーションを進める
#endif
	engine->bPhysicStep = true;
}

void PHHapticEngineLD::SyncHaptic2Physic(){
#if 1
	// physics <------ haptic
	// PHSolidForHapticの同期
	// PHSolidPairForHaptic(力覚ポインタと近傍の物体)の各種情報の同期
	for(int i = 0; i < hapticLoop->NHapticPointers(); i++){
		PHHapticPointer* hpointer = hapticLoop->GetHapticPointer(i);
		int hpointerID = hpointer->GetPointerID();
		int nNeighbors = hpointer->neighborSolidIDs.size();
		// 近傍物体であるペアだけ同期
		for(int j = 0; j < nNeighbors; j++){
			int solidID = hpointer->neighborSolidIDs[j];
			PHSolidPairForHaptic* hpair = hapticLoop->GetSolidPairForHaptic(solidID, hpointerID);
			PHSolidPairForHaptic* ppair = GetSolidPairForHaptic(solidID, hpointerID);
			PHSolidPairForHapticSt* hst = (PHSolidPairForHapticSt*)hpair;
			PHSolidPairForHapticSt* pst = (PHSolidPairForHapticSt*)ppair;
			*pst = *hst;	// haptic側で保持しておくべき情報を同期
		}
	}
	// LocalDynamicsSimulationの結果をシーンに反映
	for(int i = 0; i < (int)hapticLoop->NHapticSolids(); i++){
		PHSolidForHaptic* hsolid = hapticLoop->GetHapticSolid(i);
		if(hsolid->bPointer) continue;		// ポインタの場合
		if(hsolid->doSim <= 1) continue;	// 局所シミュレーション対象でない場合

		//アクセレランス定数項で反映速度を作る
		double pdt = GetPhysicsTimeStep();
		PHSolid* localSolid = hsolid->GetLocalSolid();
		PHSolidForHaptic* psolid = GetHapticSolid(i);
		SpatialVector b = (psolid->b + (psolid->curb - psolid->lastb)) * pdt;	// モビリティ定数項
		Vec3d v = localSolid->GetVelocity() + b.v();			// 反映速度
		Vec3d w = localSolid->GetAngularVelocity() + b.w();		// 反映角速度

		// 状態の反映
		PHSolid* sceneSolid = hsolid->sceneSolid;
		Vec3d svel = sceneSolid->GetVelocity();
		Vec3d spos = sceneSolid->GetFramePosition();
		sceneSolid->SetVelocity(v);
		sceneSolid->SetAngularVelocity(w);
		sceneSolid->SetPose(localSolid->GetPose());

		PHSolidForHapticSt* hst = (PHSolidForHapticSt*)hsolid;
		PHSolidForHapticSt* pst = (PHSolidForHapticSt*)GetHapticSolid(i);
		*pst = *hst;
	}
#endif
}

void PHHapticEngineLD::SyncPhysic2Haptic(){
	// haptic <------ physics
	// PHSolidForHapticの同期
	for(int i = 0; i < NHapticSolids(); i++){
		PHSolidForHaptic* psolid = GetHapticSolid(i);
		if(psolid->bPointer) continue;
		PHSolidForHaptic* hsolid = hapticLoop->GetHapticSolid(i);
		PHSolidForHapticSt2* pst2 = (PHSolidForHapticSt2*)psolid;
		PHSolidForHapticSt2* hst2 = (PHSolidForHapticSt2*)hsolid;
		*hst2 = *pst2;
		if(psolid->doSim == 1) hsolid->localSolid = psolid->localSolid;		
	}
	// solidpair, shapepairの同期
	// 近傍物体のみ同期させる
	for(int i = 0; i < NHapticPointers(); i++){
		PHHapticPointer* ppointer = GetHapticPointer(i);
		const int ppointerID = ppointer->GetPointerID();
		const int nNeighbors = ppointer->neighborSolidIDs.size();
		for(int j = 0; j < nNeighbors; j++){
			const int solidID = ppointer->neighborSolidIDs[j];
			PHSolidPairForHaptic* hpair = hapticLoop->GetSolidPairForHaptic(solidID, ppointerID);
			PHSolidPairForHaptic* ppair = GetSolidPairForHaptic(solidID, ppointerID);
			*hpair = PHSolidPairForHaptic(*ppair);
		}
	}
}

}