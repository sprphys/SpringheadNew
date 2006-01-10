#ifndef GRDEVICEGL_H
#define GRDEVICEGL_H

#include <Springhead.h>

namespace Spr{;

/**	OpenGLによるグラフィックス描画の実装	*/
class GRDeviceGL: public GRDevice{
public:
	OBJECTDEF(GRDeviceGL);
	BASEIMP_OBJECT(GRDevice);
	virtual void DrawDirect(TPrimitiveType ty, Vec3f* begin, Vec3f* end);
	virtual void DrawIndexed(TPrimitiveType ty, size_t* begin, size_t* end, Vec3f* vtx);
	virtual void SetMaterial(const GRMaterial& mat);
};

}
#endif
