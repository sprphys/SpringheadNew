#include "CRTryStandingUpController.h"

namespace Spr{;

IF_OBJECT_IMP(CRTryStandingUpController, CRController);

void CRTryStandingUpController::Init(){	
	CRController::Init();

	//大域変数の初期化
	totalStep = 0;
}

void CRTryStandingUpController::Step(){
	totalStep += 1;
	CRController::Step();

	
}


}