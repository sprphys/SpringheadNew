#ifndef SPR_FINODEHANDLER_H
#define SPR_FINODEHANDLER_H
#include <Foundation/SprObject.h>

namespace Spr {;
/** \addtogroup gpFileIO	*/
//@{
///	お互いに衝突しない剛体を設定
struct FIInactiveSolids{
	std::vector<std::string> solids;	//	衝突しない剛体のリスト
};

//@}

}	//	namespace Spr

#endif	

