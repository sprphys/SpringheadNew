/*
 *  Copyright (c) 2003-2008, Shoichi Hasegawa and Springhead development team 
 *  All rights reserved.
 *  This software is free software. You can freely use, distribute and modify this 
 *  software. Please deal with this software under one of the following licenses: 
 *  This license itself, Boost Software License, The MIT License, The BSD License.   
 */
#ifndef CDDETECTORIMP_H
#define CDDETECTORIMP_H
#include "CDQuickHull3D.h"
#include "CDConvexMesh.h"
#include "CDSphere.h"
#include "CDBox.h"

namespace Spr {;

#define CD_EPSILON	HULL_EPSILON
#define CD_INFINITE	HULL_INFINITE

class CDFace;
class CDConvex;

/// Shapeの組の状態
struct CDShapePairSt{
	Vec3d normal;				///<	衝突の法線(0から1へ) (Global)
	double depth;				///<	衝突の深さ：最近傍点を求めるために，2物体を動かす距離．
	unsigned lastContactCount;	///<	最後に接触した時刻
	CDShapePairSt():lastContactCount(-2), depth(0){}
};
class CDShapePair: public CDShapePairSt, public Object{
public:
	enum State{		//	接触があった場合だけ値が有効なフラグ。接触の有無は lastContactCountとscene.countを比較しないと分からない。
		NONE,		//	接触していないにも関わらず、列挙された。
		NEW,		//	前回接触なしで今回接触
		CONTINUE,	//	前回も接触
	} state;

	CDConvex* shape[2];			// 判定対象の形状
	Posed shapePoseW[2];		// World系での形状の姿勢のキャッシュ

	//	接触判定結果
	Vec3d	closestPoint[2];		///< 最近傍点（ローカル座標系）
	Vec3d	commonPoint;			///< 交差部分の内点（World座標系）
	Vec3d	center;					///< 2つの最侵入点の中間の点
	Vec3d iNormal;				///<	積分による法線

public:
	CDShapePair(){
	}
	void SetState(const CDShapePairSt& s){
		(CDShapePairSt&)*this = s;
	}	
	///	接触判定
	bool Detect(unsigned ct, const Posed& pose0, const Posed& pose1);
	///	連続接触判定．同時に法線/中心なども計算してしまう．
	bool DetectContinuously(unsigned ct, const Posed& pose0, const Vec3d& delta0, const Posed& pose1, const Vec3d& delta1);

	///	法線の計算
	void CalcNormal();
};
//	デバッグ用ツール
void CallDetectContinuously(std::istream& file, PHSdkIf* sdk);
void SaveDetectContinuously(CDShapePair* sp, unsigned ct, const Posed& pose0, const Vec3d& delta0, const Posed& pose1, const Vec3d& delta1);

///	BBox同士の交差判定．交差していれば true．
bool FASTCALL BBoxIntersection(Posed postureA, Vec3f centerA, Vec3f extentA,
					  Posed postureB, Vec3f centerB, Vec3f extentB);

///	GJKで共有点を見つける
bool FASTCALL FindCommonPoint(const CDConvex* a, const CDConvex* b,
					 const Posed& a2w, const Posed& b2w,
					 Vec3d& v, Vec3d& pa, Vec3d& pb);
/**	GJKで共有点を見つける．連続版
	startからendの間に接触点があるか調べる。
	@return 0:まったく接触なし。-2:startより前に接触あり。-1: endより先に接触あり。
	1: 接触あり。
*/
int FASTCALL ContFindCommonPoint(const CDConvex* a, const CDConvex* b,
	const Posed& a2w, const Posed& b2w, const Vec3d& dir, double start, double end, 
	Vec3d& normal, Vec3d& pa, Vec3d& pb, double& dist);

///	デバッグ用のツール。ファイルに引数を保存する。
void FASTCALL ContFindCommonPointSaveParam(const CDConvex* a, const CDConvex* b,
	const Posed& a2w, const Posed& b2w, const Vec3d& dir, double start, double end, 
	Vec3d& normal, Vec3d& pa, Vec3d& pb, double& dist);
///	デバッグ用のツール。
void ContFindCommonPointCall(std::istream& file, PHSdkIf* sdk);

#if 1
/// GJKで最近傍点対を見つける
void FASTCALL FindClosestPoints(const CDConvex* a, const CDConvex* b,
					   const Posed& a2w, const Posed& b2w,
					   Vec3d& pa, Vec3d& pb);
#endif

class CDFace;


///	ContactAnalysisが使用する凸多面体の面を表す．
class CDContactAnalysisFace{
public:
	class DualPlanes: public std::vector<CDQHPlane<CDContactAnalysisFace>*>{};

	CDFace* face;	///<	面を双対変換した頂点でQuickHullをするので，CDFaceがVtxs.
	int id;			///<	どちらのSolidの面だか表すID．

	//@group CDContactAnalysis が交差部分の形状を求める際に使う作業領域
	//@{
	Vec3f normal;	///<	面の法線ベクトル
	float dist;		///<	原点からの距離
	/**	QuickHullアルゴリズム用ワークエリア．
		一番遠い頂点から見える面を削除したあと残った形状のエッジ部分
		を一周接続しておくためのポインタ．
		頂点→面の接続．	面→頂点は頂点の並び順から分かる．	*/
	CDQHPlane<CDContactAnalysisFace>* horizon;
	//@}
	
	///	QuickHullにとっての頂点．この面を双対変換してできる頂点
	Vec3f GetPos() const { return normal / dist; }
	/**	双対変換を行う．baseに渡す頂点バッファは，双対変換が可能なように
		双対変換の中心が原点となるような座標系でなければならない．	*/
	bool CalcDualVtx(Vec3f* base);

	/**	この面を双対変換してできる頂点を含む面．
		つまり，交差部分の形状を構成する頂点のうち，
		この面にあるもの	*/
	DualPlanes dualPlanes;
	///	交差部分の形状を構成する頂点のうちこの面にあるものの数.
	size_t NCommonVtx(){ return dualPlanes.size(); }
	///	交差部分の形状を構成する頂点のうちこの面にあるもの.
	Vec3f CommonVtx(int i);
	///	デバッグ用表示
	void Print(std::ostream& os) const;
};
inline std::ostream& operator << (std::ostream& os, const CDContactAnalysisFace& f){
	f.Print(os);
	return os;
}

/**	交差部分の解析をするクラス．(交差部分の形状を求める/初回の法線を積分で求める)	*/
class CDContactAnalysis{
public:
	/// \defgroup quickHull QuickHullのための頂点と平面
	//@{
	
	typedef std::vector<CDContactAnalysisFace*> Vtxs;	
	static Vtxs vtxs;					///<	QuickHullの頂点
	typedef std::vector<CDContactAnalysisFace> VtxBuffer;	
	static VtxBuffer vtxBuffer;

	static CDQHPlanes<CDContactAnalysisFace> planes;	///<	面
	bool isValid;						///<	交差部分のポリゴンは有効？
	//@}
	std::vector<Vec3f> tvtxs[2];		///<	対象の2つの凸多面体のCommonPoint系での頂点の座標
	/**	共通部分の形状を求める．
		結果は，共通部分を構成する面を vtxs.begin() から返り値までに，
		共通部分を構成する頂点を， planes.begin から planes.end のうちの
		deleted==false のものに入れて返す．
		cp の shapePoseW に shape[0], shape[1]の頂点をWorld系に変換する
		変換行列が入っていなければならない．	*/
	CDContactAnalysisFace** FindIntersection(CDShapePair* cp);
	/**	交差部分の形状の法線を積分して，衝突の法線を求める．
		物体AとBの衝突の法線は，交差部分の面のうち，Aの面の法線の積分
		からBの面の法線の積分を引いたものになる．	*/
	void IntegrateNormal(CDShapePair* cp);
	/**	法線の計算．前回の法線の向きに物体を動かし，
		物体を離して最近傍点を求める．	*/
	void CalcNormal(CDShapePair* cp);
};

}
#endif
