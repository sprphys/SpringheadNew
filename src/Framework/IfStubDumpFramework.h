//	Do not edit. MakeTypeDesc.bat will update this file.
namespace Spr{; 

template <class IF, class MIF, class OBJ> struct NamedObjectIfStubTemplate;
template <class IF, class MIF, class OBJ>
struct FWObjectIfStubTemplate: public NamedObjectIfStubTemplate<IF, MIF, OBJ> {
	virtual PHSolidIf *  GetPHSolid(){
		return ((OBJ*)(MIF*)this)->GetPHSolid();
	}
	virtual GRFrameIf *  GetGRFrame(){
		return ((OBJ*)(MIF*)this)->GetGRFrame();
	}
};
struct FWObjectIf;	class FWObject;
typedef FWObjectIfStubTemplate<FWObjectIf, ObjectIfBuf, FWObject>	FWObjectIfStub;
typedef IfInitTemplate<FWObjectIfStub, FWObject>	FWObjectIfInit;

template <class IF, class MIF, class OBJ> struct SceneIfStubTemplate;
template <class IF, class MIF, class OBJ>
struct FWSceneIfStubTemplate: public SceneIfStubTemplate<IF, MIF, OBJ> {
	virtual PHSceneIf *	 GetPHScene(){
		return ((OBJ*)(MIF*)this)->GetPHScene();
	}
	virtual void SetPHScene(PHSceneIf *  s){
		return ((OBJ*)(MIF*)this)->SetPHScene(s);
	}
	virtual GRSceneIf *	 GetGRScene(){
		return ((OBJ*)(MIF*)this)->GetGRScene();
	}
	virtual void SetGRScene(GRSceneIf *  s){
		return ((OBJ*)(MIF*)this)->SetGRScene(s);
	}
	virtual void Sync(){
		return ((OBJ*)(MIF*)this)->Sync();
	}
	virtual void Step(){
		return ((OBJ*)(MIF*)this)->Step();
	}
	virtual void Draw(GRRenderIf *  grRender, bool debug = false){
		return ((OBJ*)(MIF*)this)->Draw(grRender, debug);
	}
	virtual void AddHumanInterface(HIForceDevice6D *  d){
		return ((OBJ*)(MIF*)this)->AddHumanInterface(d);
	}
};
struct FWSceneIf;	class FWScene;
typedef FWSceneIfStubTemplate<FWSceneIf, ObjectIfBuf, FWScene>	FWSceneIfStub;
typedef IfInitTemplate<FWSceneIfStub, FWScene>	FWSceneIfInit;

template <class IF, class MIF, class OBJ> struct ObjectIfStubTemplate;
template <class IF, class MIF, class OBJ>
struct FWSdkIfStubTemplate: public ObjectIfStubTemplate<IF, MIF, OBJ> {
	virtual FWSceneIf *  CreateScene(const FWSceneDesc &  desc){
		return ((OBJ*)(MIF*)this)->CreateScene(desc);
	}
	virtual int NScene() const {
		return ((OBJ*)(MIF*)this)->NScene();
	}
	virtual FWSceneIf *  GetScene(size_t i){
		return ((OBJ*)(MIF*)this)->GetScene(i);
	}
	virtual PHSdkIf *  GetPHSdk(){
		return ((OBJ*)(MIF*)this)->GetPHSdk();
	}
	virtual GRSdkIf *  GetGRSdk(){
		return ((OBJ*)(MIF*)this)->GetGRSdk();
	}
	virtual FISdkIf *  GetFISdk(){
		return ((OBJ*)(MIF*)this)->GetFISdk();
	}
};
struct FWSdkIf;	class FWSdk;
typedef FWSdkIfStubTemplate<FWSdkIf, ObjectIfBuf, FWSdk>	FWSdkIfStub;
typedef IfInitTemplate<FWSdkIfStub, FWSdk>	FWSdkIfInit;
}	//	namespace Spr; 
