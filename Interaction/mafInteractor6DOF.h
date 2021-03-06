/*=========================================================================

 Program: MAF2
 Module: mafInteractor6DOF
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __mafInteractor6DOF_h
#define __mafInteractor6DOF_h

#include "mafInteractorGenericInterface.h"

class vtkCamera;
class vtkProp3D;
class vtkRenderer;
class mafMatrix;
class mafTransform;
class mafDeviceButtonsPadTracker;
class mafOBB;
class mafAvatar3D;

/** 
  base class for 3D interaction modalities with 6DOF 
  @sa mafInteractor6DOFMove
  @todo
  - rewrite the UpdateDeltaTransform()
*/
class MAF_EXPORT mafInteractor6DOF : public mafInteractorGenericInterface
{
public:
  mafAbstractTypeMacro(mafInteractor6DOF,mafInteractorGenericInterface);
  
  /**  Start the interaction with the selected object */
  virtual int StartInteraction(mafDeviceButtonsPadTracker *tracker,mafMatrix *pose=NULL);
  
  /**  Stop the interaction */
  virtual int StopInteraction(mafDeviceButtonsPadTracker *tracker,mafMatrix *pose=NULL);
  
  /**  Set/Get the current pose matrix */
  virtual void SetTrackerPoseMatrix(mafMatrix *pose);
  mafMatrix *GetTrackerPoseMatrix() {return this->m_TrackerPoseMatrix;}
  
  /**  Stores the current m_TrackerPoseMatrix. */
  void TrackerSnapshot(mafMatrix *pose);
  
  /**
   Update the delta transform, i.e. transform from last snapshot */
  void UpdateDeltaTransform();

  /**  Return pointer to the current input tracker */
  mafDeviceButtonsPadTracker *GetTracker() {return (mafDeviceButtonsPadTracker *)m_Device;}
  void SetTracker(mafDeviceButtonsPadTracker *tracker);

  /** 
    Enable/Disable trigger events processing. Trigger events are StartInteraction
    and StopInteraction events which are used to start/stop the interaction. If eabled
    this flag makes the interactor to ingore these events and to be continuously active.
    Default is false. */
  void SetIgnoreTriggerEvents(int flag) {m_IgnoreTriggerEvents=flag;}
  int GetIgnoreTriggerEvents() {return m_IgnoreTriggerEvents;}
  void IgnoreTriggerEventsOn() {SetIgnoreTriggerEvents(true);}
  void IgnoreTriggerEventsOff() {SetIgnoreTriggerEvents(false);}

  /** redefined to accomplish specific tasks*/
  virtual void SetRenderer(vtkRenderer *ren);

    /** Used to hide default tracker's avatar */
  void HideAvatar();
  
  /** Used to show back default tracker's avatar */
  void ShowAvatar();
  
protected:
  
  mafInteractor6DOF();
  virtual ~mafInteractor6DOF();

  /** reimplemented to manage interaction events from trackers */
  virtual int OnStartInteraction(mafEventInteraction *event);
  /** reimplemented to manage interaction events from trackers */
  virtual int OnStopInteraction(mafEventInteraction *event);
  
  mafMatrix           *m_TrackerPoseMatrix;  
  mafMatrix           *m_StartTrackerPoseMatrix;
  mafMatrix           *m_InverseTrackerPoseMatrix;
  mafMatrix			      *m_InversePoseMatrix;
  mafTransform        *m_DeltaTransform; 
  mafTransform        *m_TmpTransform;   
  
  mafAvatar3D         *m_Avatar;

private:
  mafInteractor6DOF(const mafInteractor6DOF&);  // Not implemented.
  void operator=(const mafInteractor6DOF&);  // Not implemented.
};

#endif 
