/*=========================================================================

 Program: MAF2
 Module: mafAvatar3D
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __mafAvatar3D_h
#define __mafAvatar3D_h

#include "mafAvatar.h"
#include "mafMatrix.h"

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class mafInteractor6DOF;
class mafDeviceButtonsPadTracker;
class mafTransform;
class mafCameraTransform;
class mafView;
class mafEventBase;
class vtkMAFRayCast3DPicker;
class vtkTextMapper;
class vtkActor2D;
class vtkOutlineSource;
class vtkTransform;
class vtkCellPicker;
class vtkProp3D;
class vtkActor;
class vtkAssemblyPath;;
class vtkRenderer;
class vtkRenderWindowInteractor;

/**
  Avatars are entities moving in the virtual world according to user's 
  movements, and thus representing thier presence in the virtual world.
  Specialized classes must set the Avatar variable with a vtkProp3D,
  this class will take care of adding it to the current renderer, moving,
  hiding and showing it at the right time. The Avatar moves according to an
  interactor, which must also be set by subclasses.
  @todo
  - implement rendering by means of a CameraUpdate event sent to the interaction manager
  - make the avatar have its own GUI plugged into the device gui settings
  - refactoring to have the avatar picking nodes (VME)
 */
class MAF_EXPORT mafAvatar3D : public mafAvatar
{
public:  
  //----------------------------------------------------------------------------
  //    GUI Constants
  //----------------------------------------------------------------------------
  enum AVATAR_3D_GUI_ID
  {
    ID_FITTING_COMBO = mafAvatar::ID_LAST,
    ID_WBOX_BOOL,
    ID_DEBUG_TEXT,
    ID_DEBUG_TEXT_POSITION,
    ID_COORDS_COMBO,
    ID_LAST
  };

  enum AVATAR_COORDS_TYPE
  {
    TRACKER_COORDS=0,
    CANONICAL_COORDS,
    WORLD_COORDS,
  };

  enum AVATAR_COORDS_SCALING
  {
    NO_SCALE=0,
    TRACKER_TO_WORLD_SCALE,
    CANONICAL_TO_WORLD_SCALE
  };

  mafAbstractTypeMacro(mafAvatar3D,mafAvatar);
 
  /**  
    Set/Get the renderer this avatar is attached to. When the rederer is
    changed, all props created by this avatar are moved to the new renderer
    and the interaction is changed accordingly */
  virtual void SetRenderer(vtkRenderer *ren);
  
  /**  
    Hide the 3D cursor, i.e. remove actor from renderer's list. This is
    usually used to compute bounding box without considering avatars */
  virtual void Hide();

  /**  
    Show the 3D cursor. This function is conditional, in the 
    sense the old visibility state is retained when showing back. */
  virtual void Show();
  
  /** Return picker 3D used by 3DAvatar for picking */
  vtkMAFRayCast3DPicker *GetPicker3D();

  /** Return picker 2D used by 3DAvatar for 2D picking */
  vtkCellPicker *GetPicker2D();

  /** return the picker used by this avatar: from the picker is possible to obtain information 
      about the picked object. */
  virtual vtkAbstractPropPicker *GetPicker();
  
  /** Display a box showing where input coords are mapped.
      This can be used for debug to understand if the tracker's
      settings and the mapping function are correct */
  void SetDisplayWorkingBox(int vis);
  int GetDisplayWorkingBox(){ return m_DisplayWorkingBox;}
  void DisplayWorkingBoxOn() {SetDisplayWorkingBox(true);}
  void DisplayWorkingBoxOff() {SetDisplayWorkingBox(false);}

  /** 
    Display the debug text showing trackers coordinate. This can 
    be used to debug the code. */
  void SetDisplayDebugText(int vis);
  int GetDisplayDebugText() {return m_DisplayDebugText;}

  /** where to display the text */
  void SetDebugTextPosition(double *pos) {SetDebugTextPosition(pos[0],pos[1]);}
  void SetDebugTextPosition(double posx,double posy);
  double *GetDebugTextPosition() {return m_DebugTextPosition;}
  
  /** 
    Display the original tracker's coords in the debug text area.
    This can be used to know the tracked volume size to
    be set in the trackers' settings. */
  void SetCoordsFrame(int frame) {m_CoordsFrame=frame;}
  int GetCoordsFrame() {return m_CoordsFrame;}
  

  /**  Process events coming from tracker */
  virtual void OnEvent(mafEventBase *event);

  /** return the tracker this avatar is the tracker attached to this avatar */
  virtual void SetTracker(mafDeviceButtonsPadTracker *tracker);

  /**  Transform a matrix from Canonical to World space*/
  void CanonicalToWorld(const mafMatrix &pose,mafMatrix &dest);
  void CanonicalToWorld(mafMatrix &pose) {CanonicalToWorld(pose,pose);}
  void CanonicalToWorld(mafTransform *trans);

  void WorldToCanonical(const mafMatrix &pose,mafMatrix &dest);
  void WorldToCanonical(mafMatrix &pose) {WorldToCanonical(pose,pose);}
  void WorldToCanonical(mafTransform *trans);


  /** 
    Transform a matrix from Tracker frame to World frame, the function
    can optionally avoid to map on the destination matrix some of the 
    6DOF: e.g. not using the scale or the rotational matrix */
  void TrackerToWorld(mafMatrix &tracker_pose,mafMatrix &world_pose,int use_scale=1,int use_rot=1, int use_trans=1);
  void WorldToTracker(mafMatrix &world_pose,mafMatrix &tracker_pose,int use_scale=1,int use_rot=1, int use_trans=1);

  
  /** map tracker coords to display normalized coords */
  void TrackerToDisplay(mafMatrix &tracker_pose,double xy[2]);

  /** map world coords to tracker coords */
  void WorldToDisplay(mafMatrix &world_pose,double xy[2]);

  /** map world coords to normalized display coords*/
  void WorldToNormalizedDisplay(mafMatrix &world_pose,double xy[2]);
  

  /**  return transform between canonical and world space */
  mafCameraTransform *GetCanonicalToWorldTransform() {return m_CanonicalToWorldTransform;}

   /** 
    Set mapping rule, i.e. the policy to be used for scaling the
    tracked volume to the world volume. */
  void SetFittingMode(int type);

  /** return the mapping rule currently in use */
  int GetFittingMode() {return m_FittingMode;}

  /** 
    return the last pose matrix of the avatar's 3d actor in world coordinates.  
    */
  mafMatrix &GetLastPoseMatrix() {return m_LastPoseMatrix;}

  /** Create the dialog that show the interface for settings. */
  virtual void CreateGui();

protected:
  mafAvatar3D();
  virtual ~mafAvatar3D();

  /**
   Internally used to set the last pose matrix.
   @todo: Why should we use this instead of using the tracker's one? I'd like to remove this Marco. 28-10-2004: */
  void SetLastPoseMatrix(mafMatrix &matrix);

  virtual int InternalStore(mafStorageElement *node);
  virtual int InternalRestore(mafStorageElement *node);

  /** redefined to add the Cursor actor into the selected renderer */
  virtual int InternalInitialize();

  /** redefined to remove Cursor from renderer */
  virtual void InternalShutdown();

  /** internally used to update debug text on the screen */
  virtual void UpdateDebugText(const char *title, mafMatrix &pose);

  /** process a move event */
  virtual void OnMove3DEvent(mafEventInteraction *e);
  
  /** process an event signaling a change of the tracked bounds */
  void OnUpdateBoundsEvent(mafDeviceButtonsPadTracker *tracker);

  /** process event sent after a camera reset */
  virtual void OnPostResetCamera(mafEventBase *event);

  /** Internally used to redefine the 3D picker class */  
  void SetPicker3D(vtkMAFRayCast3DPicker *picker);

  vtkProp3D*          m_WorkingBoxActor; ///< Prop3D of the working box
  vtkOutlineSource*   m_WorkingBox; ///< Working box the tracker coords are mapped to

  vtkTextMapper*      m_DebugTextMapper;
  vtkActor2D*         m_DebugTextActor;

  mafMatrix           m_LastPoseMatrix;

  vtkMAFRayCast3DPicker* m_Picker3D; ///< Used to pick in a VTK Render window
  vtkCellPicker*      m_Picker2D;


  mafCameraTransform* m_CanonicalToWorldTransform;
  
  int                 m_FittingMode; ///< specify the current mapping rule from canonical to world
  
  int                 m_DisplayWorkingBox; ///< Whether display the working box
  int                 m_DisplayDebugText; ///< Whether display Debug text
  int                 m_CoordsFrame; ///< Whether to display the original tracker's coords 

  double*              m_DebugTextPosition;

private:
  mafAvatar3D(const mafAvatar3D&);  // Not implemented.
  void operator=(const mafAvatar3D&);  // Not implemented.
};

#endif 
