#ifndef PHSHAPE_H
#define PHSHAPE_H

#include <Springhead.h>
#include <Foundation/Object.h>

namespace Spr{;
class CDShape : public CDShapeIf, public Object{
protected:
	Posef pose;
public:
	OBJECTDEFABST(CDShape);
	BASEIMP_OBJECT(Object);
	Posef GetPose() const { return pose; }
	void SetPose(Posef p){ pose = p; }
	
	virtual void CalcBBox(Vec3f& bbmin, Vec3f& bbmax)=0;
};
class CDShapes:public std::vector< UTRef<CDShape> >{
};

}	//	namespace Spr
#endif
