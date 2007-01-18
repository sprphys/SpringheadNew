#include "Device.h"
#pragma hdrstop
#include <HumanInterface/HISdk.h>
#include <HumanInterface/DRKeyMouseWin32.h>
#include <Windows.h>
#include <iomanip>

namespace Spr {;

//	DRKeyMouseWin32

IF_OBJECT_IMP(DRKeyMouseWin32, HIRealDevice);

DRKeyMouseWin32::KeyMouse::KeyMouse(DRKeyMouseWin32* w):win32(w){
	strcpy(name, Type());
	strcat(name, "Win32");
}
void DRKeyMouseWin32::KeyMouse::SetHandler(DVKeyMouseHandler* h){
	DRKeyMouseWin32* kmw32 = RealDevice()->Cast();
	kmw32->SetHandler(h);
}
int DRKeyMouseWin32::KeyMouse::GetKeyState(int key){
	int rv = DVKeyMouseIf::NONE;
	short got = ::GetKeyState(key);
	if (got & 0x1) rv |= DVKeyMouseIf::TOGGLE_ON;
	if (got & 0x8000) rv |= DVKeyMouseIf::PRESS;
	return rv;
}
///	マウス位置取得関数	0が最新，1以上は過去の履歴を取得
DVKeyMouseIf::DVMousePosition DRKeyMouseWin32::KeyMouse::GetMousePosition(int count){
	MOUSEMOVEPOINT points[64];
	int nPoint = GetMouseMovePointsEx(sizeof(MOUSEMOVEPOINT), NULL, points, 64, GMMP_USE_HIGH_RESOLUTION_POINTS);
	DVKeyMouseIf::DVMousePosition rv;
	if (count < nPoint){
		rv.time = points[count].time;
		rv.x = points[count].x;
		rv.y = points[count].y;
	}else{
		rv.time = 0;
		rv.x = 0;
		rv.y = 0;
	}
	return rv;
};

//---------------------------------------------------------------------
//	DRKeyMouseWin32
//

DRKeyMouseWin32::DRKeyMouseWin32(): handler(NULL){
	strcpy(name, Type());
	strcat(name, "Win32");
}
bool DRKeyMouseWin32::Init(){
	return true;
}
void DRKeyMouseWin32::Register(HISdkIf* intf){
	HISdk* sdk = intf->Cast();
	sdk->RegisterVirtualDevice((new DRKeyMouseWin32::KeyMouse(this))->Cast());
}
inline int ConvertKeyState(unsigned fwKeys){
	int keyState=0;
	if (fwKeys & MK_LBUTTON) keyState |= DVKeyMouseHandler::LBUTTON;
	if (fwKeys & MK_RBUTTON) keyState |= DVKeyMouseHandler::RBUTTON;
	if (fwKeys & MK_SHIFT) keyState |= DVKeyMouseHandler::SHIFT;
	if (fwKeys & MK_CONTROL) keyState |= DVKeyMouseHandler::CONTROL;
	if (fwKeys & MK_MBUTTON) keyState |= DVKeyMouseHandler::MBUTTON;
	return keyState;
}
bool DRKeyMouseWin32::PreviewMessage(void* m){
	if (!handler) return false;
	MSG* msg = (MSG*)m;
	bool bKeyDown = false;
	switch (msg->message){
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		bKeyDown = true;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:{
		if (!bKeyDown) ReleaseCapture();
		int keyState = ConvertKeyState(msg->wParam);  // key flags		
		int x = (short)LOWORD(msg->lParam);  // horizontal position of cursor 
		int y = (short)HIWORD(msg->lParam);  // vertical position of cursor 
		if (bKeyDown) SetCapture(msg->hwnd);
		return handler->OnKey(bKeyDown, VK_LBUTTON, keyState, x, y);
		}break;
	case WM_MOUSEMOVE:{
		unsigned fwKeys = msg->wParam;  // key flags 
		int keyState = ConvertKeyState(msg->wParam);  // key flags		
		int x = (short)LOWORD(msg->lParam);	// horizontal position of cursor 
		int y = (short)HIWORD(msg->lParam);	// vertical position of cursor 
		return handler->OnMouseMove(keyState, x, y, 0);
		}break;
	case WM_MOUSEWHEEL:{
		unsigned fwKeys = LOWORD(msg->wParam);		
		int keyState = ConvertKeyState(fwKeys);		// key flags		
		int zDelta = (short) HIWORD(msg->wParam);   // wheel rotation
		int x = (short) LOWORD(msg->lParam);		// horizontal position of pointer
		int y = (short) HIWORD(msg->lParam);		// vertical position of pointer
		return handler->OnMouseMove(keyState, x, y, zDelta);
		}break;
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:{
		int keyState = ConvertKeyState(msg->wParam);        // key flags 
		int x = (short)LOWORD(msg->lParam);  // horizontal position of cursor 
		int y = (short)HIWORD(msg->lParam);  // vertical position of cursor 
		return handler->OnDoubleClick(x, y, keyState);
		}break;
	case WM_KEYDOWN:{
		int nVirtKey = (int) msg->wParam;
/*		int keyState = ReadKeyState();
		TODO hase x y を取る
		int x = ;
		GetMouseMovePointsEx()
		return handler->OnKey(true, nVirtKey, keyState, x, y);
		*/
		}break;
	default:
		return false;
	}
	return true;
}

}	//	namespace Spr
