#ifndef FWGLUT_H
#define FWGLUT_H
#include<sstream>
#include<string>
#include<vector>
#include<Springhead.h>
#include"Framework/FWGraphicsAdaptee.h"
#include"Framework/SprFWApp.h"
#include"Framework/FWAppInfo.h"
using namespace std;

namespace Spr{;

class FWWinGLUT : public FWWin{
public:
	FWWinGLUT(int wid, const FWWinDesc& d, GRRenderIf* r):
	  FWWin(wid, d, r){}
	void FullScreen();
	void Position(int left, int top);
	void Reshape(int width, int height);
	void SetTitle(UTString t);
};


class FWGLUTDesc{
public:
	FWGLUTDesc();
};

class FWGLUT : public FWGraphicsAdaptee, public FWGLUTDesc{
protected:
	/** glutTimerfuncを管理するクラス */
	/*class FWGLUTTimer : public UTRefCount{
	public:
		//static FWGLUTTimer* timerInstance;
		GTimerFunc* func;					//glutTimerFuncで呼び出す関数
		void SetTimerFunc(GTimerFunc* f);	//funcを設定する
		void GetTimerFunc();				//funcを取得する
	};
	typedef UTRef<FWGLUTTimer> UTRef_FWGLUTTimer;
	typedef vector<UTRef_FWGLUTTimer> FWGLUTTimers;
	FWGLUTTimers glutTimer;
	/// タイマーを取得する
	FWGLUTTimer* GetTimer(int i){
		return (i < (int)glutTimer.size() ? glutTimer[i] : NULL);
	}*/

	/** コールバック関数*/
	static FWGLUT* instance;
	static void SPR_CDECL GlutDisplayFunc();
	static void SPR_CDECL GlutReshapeFunc(int w, int h);
	static void SPR_CDECL GlutIdleFunc();
	static void SPR_CDECL GlutKeyboardFunc(unsigned char key, int x, int y);
	static void SPR_CDECL GlutMouseFunc(int button, int state, int x, int y);
	static void SPR_CDECL GlutMotionFunc(int x, int y);
	static void SPR_CDECL GlutJoystickFunc(unsigned int buttonMask, int x, int y, int z);
	static void SPR_CDECL AtExit();
public:	
	FWGLUT();
	~FWGLUT();

	///	GLUTの初期化を行う。最初にこれを呼ぶ必要がある。
	virtual void Init(int argc, char* argv[]);
	
	/** タイマ */
	/// タイマーを設定する
	virtual void SetTimer(int id, int interval);
	/// タイマーを作成する
	//virtual void AddTimer();
	/// 最後に作成したタイマーの番号を返す
	//virtual int NTimers();
	/// i番目のタイマーにTimerfuncを設定する
	//virtual void SetTimerFunc(GTimerFunc* f ,int i=0);
	/// mainloopを呼ぶ
	//virtual void Loop(int i,double timeStep);
	///GLUTによるTimerをスタートする
	virtual void StartMainLoop();

	/** ウィンドウ */
	///	ウィンドウを作成し、ウィンドウ IDを返す
	virtual FWWin* CreateWin(const FWWinDesc& d=FWWinDesc());
	///	ウィンドウを破棄する
	virtual void DestroyWin(FWWin* w);
	///	カレントウィンドウを設定する
	virtual void SetCurrentWin(FWWin* w);
	///	カレントウィンドウを返す。
	virtual FWWin* GetCurrentWin();	
	///カレントウィンドウのノーマルプレーンを，再描画の必要に応じてマークする
	virtual void PostRedisplay();
	/// Shift,Ctrl,Altのステートを返す
	virtual int Modifiers();

};

}





#endif