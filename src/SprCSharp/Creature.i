#	Do not edit. RunSwig.bat will update this file.
%ignore Spr::CRCreatureIf::CreateBody;
%ignore Spr::CRCreatureIf::CreateEngine;
%ignore Spr::CRSdkIf::CreateCreature;
%ignore Spr::CRSdk::CreateSdk;
%module Creature
%include "../../include/SprBase.h"
%include "../../include/Base/Env.h"
%include "../../include/Foundation/SprObject.h"
%include "../../include/Foundation/SprScene.h"
%include "../../include/Creature/SprCRBody.h"
%include "../../include/Creature/SprCRBone.h"
%include "../../include/Creature/SprCREngine.h"
%include "../../include/Creature/SprCRController.h"
%include "../../include/Creature/SprCRCreature.h"
%include "../../include/Creature/SprCRReachController.h"
%include "../../include/Creature/SprCRSdk.h"
%include "../../include/Creature/SprCRTouchSensor.h"
%include "../../include/Creature/SprCRVisualSensor.h"
/*
#define DOUBLECOLON :: 
%include "../../include/Springhead.h"
%include "../../include/base/Env.h"
%include "../../include/Foundation/SprObject.h"
%include "../../include/Foundation/SprScene.h"
%include "../../include/Foundation/SprUTTimer.h"
%include "../../include/Collision/SprCDShape.h"
%include "../../include/Physics/SprPHDef.h"
%include "../../include/Physics/SprPHEngine.h"
%include "../../include/Physics/SprPHFemMesh.h"
%include "../../include/Physics/SprPHFemMeshNew.h"
%include "../../include/Physics/SprPHHaptic.h"
%include "../../include/Physics/SprPHIK.h"
%include "../../include/Physics/SprPHJoint.h"
%include "../../include/Physics/SprPHJointLimit.h"
%include "../../include/Physics/SprPHJointMotor.h"
%include "../../include/Physics/SprPHNodeHandlers.h"
%include "../../include/Physics/SprPHOpObj.h"
%include "../../include/Physics/SprPHScene.h"
%include "../../include/Physics/SprPHSdk.h"
%include "../../include/Physics/SprPHSkeleton.h"
%include "../../include/Physics/SprPHSolid.h"
%include "../../include/Creature/SprCRBody.h"
%include "../../include/Creature/SprCRBone.h"
%include "../../include/Creature/SprCRController.h"
%include "../../include/Creature/SprCRCreature.h"
%include "../../include/Creature/SprCREngine.h"
%include "../../include/Creature/SprCRReachController.h"
%include "../../include/Creature/SprCRSdk.h"
%include "../../include/Creature/SprCRTouchSensor.h"
%include "../../include/Creature/SprCRVisualSensor.h"
%include "../../include/Base/BaseDebug.h"
%include "../../src/Foundation/UTTypeDesc.h"
%include "../Foundation/Foundation.h"
%include "../Foundation/Object.h"
%include "../Foundation/Scene.h"
%include "../Foundation/UTBaseType.h"
%include "../Foundation/UTClapack.h"
%include "../Foundation/UTDllLoader.h"
%include "../Foundation/UTDllLoaderImpl.h"
%include "../Foundation/UTLoadContext.h"
%include "../Foundation/UTLoadHandler.h"
%include "../Foundation/UTMMTimer.h"
%include "../Foundation/UTPath.h"
%include "../Foundation/UTPreciseTimer.h"
%include "../Foundation/UTQPTimer.h"
%include "../Foundation/UTSocket.h"
%include "../Foundation/UTTimer.h"
%include "../Foundation/UTTypeDesc.h"
%include "../Collision/CDBox.h"
%include "../Collision/CDCapsule.h"
%include "../Collision/CDConvex.h"
%include "../Collision/CDConvexMesh.h"
%include "../Collision/CDConvexMeshInterpolate.h"
%include "../Collision/CDCutRing.h"
%include "../Collision/CDDetectorImp.h"
%include "../Collision/CDQuickHull2D.h"
%include "../Collision/CDQuickHull2DImp.h"
%include "../Collision/CDQuickHull3D.h"
%include "../Collision/CDQuickHull3DImp.h"
%include "../Collision/CDRoundCone.h"
%include "../Collision/CDShape.h"
%include "../Collision/CDSphere.h"
%include "../Collision/Collision.h"
%include "../Physics/PHAxisIndex.h"
%include "../Physics/PHBallJoint.h"
%include "../Physics/PHConstraint.h"
%include "../Physics/PHConstraintEngine.h"
%include "../Physics/PHContactDetector.h"
%include "../Physics/PHContactPoint.h"
%include "../Physics/PHContactSurface.h"
%include "../Physics/PHEngine.h"
%include "../Physics/PHFemBase.h"
%include "../Physics/PHFemCollision.h"
%include "../Physics/PHFemEngine.h"
%include "../Physics/PHFemMesh.h"
%include "../Physics/PHFemMeshNew.h"
%include "../Physics/PHFemMeshThermo.h"
%include "../Physics/PHFemPorousWOMove.h"
%include "../Physics/PHFemThermo.h"
%include "../Physics/PHFemVibration.h"
%include "../Physics/PHFixJoint.h"
%include "../Physics/PHForceField.h"
%include "../Physics/PHGear.h"
%include "../Physics/PHGenericJoint.h"
%include "../Physics/PHHapticEngine.h"
%include "../Physics/PHHapticEngineImpulse.h"
%include "../Physics/PHHapticEngineLD.h"
%include "../Physics/PHHapticEngineLDDev.h"
%include "../Physics/PHHapticEngineMultiBase.h"
%include "../Physics/PHHapticEngineSingleBase.h"
%include "../Physics/PHHapticPointer.h"
%include "../Physics/PHHapticRender.h"
%include "../Physics/PHHingeJoint.h"
%include "../Physics/PHIKActuator.h"
%include "../Physics/PHIKEndEffector.h"
%include "../Physics/PHIKEngine.h"
%include "../Physics/PHJoint.h"
%include "../Physics/PHJointLimit.h"
%include "../Physics/PHJointMotor.h"
%include "../Physics/PHOpDecompositionMethods.h"
%include "../Physics/PHOpEngine.h"
%include "../Physics/PHOpGroup.h"
%include "../Physics/PHOpObj.h"
%include "../Physics/PHOpParticle.h"
%include "../Physics/PHPathJoint.h"
%include "../Physics/PHPenaltyEngine.h"
%include "../Physics/PHPoseSpring.h"
%include "../Physics/PHScene.h"
%include "../Physics/PHSdk.h"
%include "../Physics/PHSkeleton.h"
%include "../Physics/PHSliderJoint.h"
%include "../Physics/PHSolid.h"
%include "../Physics/PHSpatial.h"
%include "../Physics/PHSpring.h"
%include "../Physics/PHTreeNode.h"
%include "../Physics/Physics.h"
%include "../Physics/RingBuffer.h"
%include "../Creature/CRBody.h"
%include "../Creature/CRBone.h"
%include "../Creature/CRCreature.h"
%include "../Creature/CREngine.h"
%include "../Creature/CRGazeController.h"
%include "../Creature/CRGrabController.h"
%include "../Creature/CRReachController.h"
%include "../Creature/CRSdk.h"
%include "../Creature/CRTouchSensor.h"
%include "../Creature/CRTrajectoryController.h"
%include "../Creature/CRVisualSensor.h"
*/
