﻿/*
 *  Copyright (c) 2003-2008, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#ifndef SPR_PHENGINEIF_H
#define SPR_PHENGINEIF_H

#include <Foundation/SprObject.h>

namespace Spr{;

struct PHConstraintsIf;
struct PHSceneIf;

/** \addtogroup gpPhysics */
//@{
///	エンジンの基本クラス
struct PHEngineIf : public SceneObjectIf{
public:
	SPR_IFDEF(PHEngine);
	///	実行順序を決めるプライオリティ値．小さいほど早い
	int GetPriority() const;
	///	時間を dt 進める
	void Step();
	///
	PHSceneIf* GetScene();
};

struct PHConstraintEngineIf : PHEngineIf{
public:
	SPR_IFDEF(PHConstraintEngine);

	/** @brief 接触点の集合を返す
	*/
	PHConstraintsIf* GetContactPoints();

	/** @brief velCorrectionRateを書き換える
		@param value 書き換え後の値
	*/
	void SetVelCorrectionRate(double value);

	/** @brief velCorrectionRateを取得する
		@return value velCorrectionRateの値
	*/
	double GetVelCorrectionRate();

	/** @brief posCorrectionRateを書き換える
		@param value 書き換え後の値
	*/
	void SetPosCorrectionRate(double value);

	/** @brief posCorrectionRateを取得する
		@return value posCorrectionRateの値
	*/
	double GetPosCorrectionRate();

	/** @brief contactCorrectionRateを書き換える
		@param value 書き換え後の値
	*/
	void SetContactCorrectionRate(double value);

	/** @brief contactCorrectionRateを取得する
		@return value contactCorrectionRateの値
	*/
	double GetContactCorrectionRate();

	/** @brief bSaveConstraintsを書き換える
		@return value SaveState, LoadStateに， constraints を含めるかどうか．本来不要だが，f, Fが変化する．
	*/
	void SetBSaveConstraints(bool value);

	/** @brief 剛体の速度だけ更新するフラグを設定する
	*/
	void SetUpdateAllSolidState(bool flag);

	/** @brief 面接触を使うフラグを設定する
	*/
	void SetUseContactSurface(bool flag);

	/** @brief shrinkRateの値を設定する
	*/
	void SetShrinkRate(double data);
	
	/** @brief shrinkRateを取得する
		@return value shrinkCorrectionRateの値
	*/
	double GetShrinkRate();

	/** @brief shrinkRateCorrectionの値を設定する
	*/
	void SetShrinkRateCorrection(double data);

	/** @brief shrinkRateCorrectionを取得する
		@return value shrinkRateCorrectionの値
	*/
	double GetShrinkRateCorrection();

	/** @breif 接触領域を表示するための情報を更新するかどうか。FWSceen::EnableRenderContact()が呼び出す。
	*/
	void EnableRenderContact(bool enable);
};

struct PHConstraintEngineDesc{
	int		method;						///< LCPの解法
	int		numIter;					///< 速度更新LCPの反復回数
	int		numIterCorrection;			///< 誤差修正LCPの反復回数
	int		numIterContactCorrection;	///< 接触点の誤差修正LCPの反復回数
	double	velCorrectionRate;			///< 速度のLCPで関節拘束の誤差を修正する場合の誤差修正比率
	double	posCorrectionRate;			///< 位置のLCPで，関節拘束の誤差を修正する場合の誤差修正比率
	double  contactCorrectionRate;		///< 接触の侵入解消のための，速度のLCPでの補正比率．
	double	shrinkRate;					///< LCP初期値を前回の解に対して縮小させる比率
	double	shrinkRateCorrection;
	double	freezeThreshold;			///< 剛体がフリーズする閾値
	double	accelSOR;					///< SOR法の加速係数
	bool	bSaveConstraints;			///< SaveState, LoadStateに， constraints を含めるかどうか．本来不要だが，f, Fが変化する．
	bool	bUpdateAllState;			///< 剛体の速度，位置の全ての状態を更新する．
	bool	bUseContactSurface;			///< 面接触を使う

	PHConstraintEngineDesc(){
		numIter					 = 15;
		numIterCorrection		 = 0;
		numIterContactCorrection = 0;
		velCorrectionRate		 = 0.3;
		posCorrectionRate		 = 0.3;
		contactCorrectionRate	 = 0.1;
		shrinkRate				 = 0.7;
		shrinkRateCorrection	 = 0.7;
		freezeThreshold			 = 0.0;
		accelSOR				 = 1.0;
		bSaveConstraints         = false;
		bUpdateAllState	         = true;
		bUseContactSurface       = false;
	}
};

struct PHGravityEngineIf : PHEngineIf{
public:
	SPR_IFDEF(PHGravityEngine);
};

struct PHPenaltyEngineIf : PHEngineIf{
public:
	SPR_IFDEF(PHPenaltyEngine);

};

struct PHIKEngineIf : PHEngineIf{
public:
	/** @brief IK機能が有効か無効か
	*/
	void Enable(bool bEnabled);
	bool IsEnabled();

	/** @brief IKエンドエフェクタの移動速度限界（1stepあたりの移動量限界を決める）
	*/
	void SetMaxVelocity(double maxVel);
	double GetMaxVelocity();

	/** @brief IKエンドエフェクタの回転角速度限界（1stepあたりの回転量限界を決める）
	*/
	void SetMaxAngularVelocity(double maxAV);
	double GetMaxAngularVelocity();

	/** @brief IKアクチュエータ回転角速度限界
	*/
	void SetMaxActuatorVelocity(double maxAV);
	double GetMaxActuatorVelocity();

	/** @brief ヤコビアンの擬似逆の正則化パラメータ
	*/
	void SetRegularizeParam(double epsilon);
	double GetRegularizeParam();

	/** @brief IK計算結果の関節角速度のノルムがこの値以下になったらIterationを打ち切る
	*/
	void SetIterCutOffAngVel(double epsilon);
	double GetIterCutOffAngVel();

	/** @brief 一時変数の関節角度・剛体姿勢を現実のものに合わせる
	*/
	void ApplyExactState(bool reverse=false);

	/** @brief (IK->FK)の繰り返し回数
	*/
	void SetNumIter(int numIter);
	int GetNumIter();

	SPR_IFDEF(PHIKEngine);
};

struct PHFemEngineIf : PHEngineIf{
public:
	void SetTimeStep(double dt);
	double GetTimeStep();
	SPR_IFDEF(PHFemEngine);
};
struct PHOpEngineIf : PHEngineIf{
public:
	void SetTimeStep(double dt);
	double GetTimeStep();
	void SetGravity(bool gflag);

	SPR_IFDEF(PHOpEngine);
};
struct PHHapticEngineDesc{
	enum HapticEngineMode{
		SINGLE_THREAD = 0,
		MULTI_THREAD,
		LOCAL_DYNAMICS,
	};
	PHHapticEngineDesc();
};

struct PHHapticEngineIf : public PHHapticEngineDesc, PHEngineIf{
public:
	SPR_IFDEF(PHHapticEngine);
	/** @breif HapticEngineを有効化する
	*/
	void EnableHapticEngine(bool b);

	/** @breif HapticEngineのモードを切り替える
		@param mode HapticEngineMode
	*/
	void SetHapticEngineMode(HapticEngineMode mode);

	/** @brief シミュレーションをすすめる。HapticEngineを有効化した場合には
		この関数を使ってシミュレーションをすすめる
	*/
	void StepPhysicsSimulation();

	/** @brief シミュレーションを実行する直前かどうかを返す
	*/
	// bool IsBeforeStepPhysicsSimulation();

	/** @brief シミュレーションを実行した直後かどうかを返す
	*/
	// bool IsAfterStepPhysicsSimulation();

	/** @breif シーングラフの状態(ObjectStatesIf)を開放する。
				動的にオブジェクトを追加する時には直前に呼ぶ必要がある。
	*/
	void ReleaseState();

	///	コールバック関数の型
	typedef void (SPR_CDECL *Callback)(void* arg);

	/** @brief シミュレーションを実行する直前に実行されるコールバックを登録する
	*/
	bool SetCallbackBeforeStep(Callback f, void* arg);

	/** @brief シミュレーションを実行した直後に実行されるコールバックを登録する
	*/
	bool SetCallbackAfterStep(Callback f, void* arg);

};
//@}
}
#endif
