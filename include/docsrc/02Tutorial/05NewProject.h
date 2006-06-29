/** \page pageNewProject	新規プロジェクト
\contents

ここでは，Visual C++ .NET 2003 を用いて，新しくSpringhead2を用いたプログラムを作成する方法を説明します．
説明にしたがって作成すると \ref pageSampleNewProject ができます．

\section secCreateNewProj 新規プロジェクトの作成
VisualC++を立ち上げて，ファイル(F)-新規作成(N)-プロジェクト(P)を選択して，
下のような「新しいプロジェクト」ダイアログを立ち上げ，「Win32コンソールプロジェクト」
を選択してください．
\image html newProj.png
プロジェクト名や作成場所を指定して，OKでプロジェクトを作成してください．
上の図では，first.vcproj と first.cpp が作成されます．

\section secAddHeaders インクルードディレクトリの追加
作成したプロジェクトを右クリックし，プロパティ(R)でプロパティを表示し，
\image html addIncludeFolder.png 
のように，「すべての構成」について「C/C++ 全般」の「追加のインクルードディレクトリ」に，
「$(SPRINGHEAD2)/include」を追加してください．
これで，Springheadのヘッダファイルがインクルードできるようになります．

\section secAddLibs ライブラリの追加
Springheadを使うためには，ライブラリを追加する必要があります．
新しく作ったプロジェクト(下図のfirst)を右クリックして 追加(D)-既存項目の追加(G)を選び，
\image html addLib.png
\ref pageGettingStarted で作成した lib/win32/SpringheadD.lib と
lib/win32/Springhead.lib を追加します．
これらは，それぞれデバッグ版，リリース版のライブラリなので，それぞれを右クリックして
プロパティ(R)でプロパティを表示し，下図のようにDebug版ではSpringhead.libをビルドから
除外し，Release版では逆にSpringheadD.libをビルドから除外してください．
\image html nonBuildSetting.png

\section secEditSrc ソースファイルの編集
あとは，ソースファイルを編集して自分のプログラムを作れば終わりです．
すごく単純な例を出すと：
<pre>
// first.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
#include "stdafx.h"
#include <springhead.h>
using namespace Spr;

int _tmain(int argc, _TCHAR* argv[])
{
	PHSdkIf* sdk = CreatePHSdk();				//	物理エンジンSDKの作成
	PHSceneIf* scene = sdk->CreateScene();		//	シーンの作成
	PHSolidIf* solid = scene->CreateSolid();	//	剛体の作成
	for(int i=0; i<10; ++i){
		//	シミュレーションを進める
		scene->Step();
		//	剛体の位置を出力：自由落下の様子が数値で出力される
		std::cout << solid->GetPose().Pos() << std::endl;
	}
	return 0;
}
</pre>
のようになります．
