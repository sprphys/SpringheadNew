#ifndef DEV_H
#define DEV_H

#include <Springhead.h>
#include <Framework/SprFWApp.h>

using namespace Spr;

class MyApp : public FWApp{
public:
	PHSceneIf* phscene;
	PHSolidIf* pointer;
	double range;
	int timerID;

	void Init(int argc = 0, char* argv[] = 0);
	void UserFunc();
	void ContactAnalysis();
	void Keyboard(int key, int x, int y);
	virtual void TimerFunc(int id);
};


#endif