/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafGUITransformMouse.cpp,v $
  Language:  C++
  Date:      $Date: 2008-07-25 08:44:32 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "mafGUITransformMouse.h"
#include "mafDecl.h"

#include "mafGUI.h"
#include "mafGUIButton.h"

#include "mmiGenericMouse.h"
#include "mmiCompositorMouse.h"

#include "mafMatrix.h"
#include "mafAbsMatrixPipe.h"
#include "mafTransform.h"
#include "mafVME.h"
#include "mafVMEOutput.h"

#include "vtkMatrix4x4.h"
#include "vtkRenderer.h"

//----------------------------------------------------------------------------
mafGUITransformMouse::mafGUITransformMouse(mafVME *input, mafObserver *listener)
//----------------------------------------------------------------------------
{
  assert(input);

  m_Listener = listener;
  m_InputVME = input;
  m_Gui = NULL;
  
  IsaRotate = NULL;
  IsaTranslate = NULL;
  IsaRoll = NULL;

  RotationConstraintId = VIEW_PLANE;
  TranslationConstraintId = VIEW_PLANE;
  
  m_RefSysVME = m_InputVME;
  OldInteractor = NULL;

  CreateISA();
  CreateGui();
  
  AttachInteractorToVme();
}
//----------------------------------------------------------------------------
mafGUITransformMouse::~mafGUITransformMouse() 
//----------------------------------------------------------------------------
{ 
  DetachInteractorFromVme();    
  mafDEL(IsaCompositor); 
}

//----------------------------------------------------------------------------
void mafGUITransformMouse::CreateGui()
//----------------------------------------------------------------------------
{
  m_Gui = new mafGUI(this);
  m_Gui->Divider(2);
  m_Gui->Label("mouse interaction", true);
  m_Gui->Label("left mouse: rotate");
  m_Gui->Label("middle mouse: translate");
  m_Gui->Label("left mouse + ctrl: rotate around view normal");
  m_Gui->Divider();

  // rotation axes
  wxString rot_axes[5] = {"x", "y", "z", "view","normal view"};

  // translation axes
	wxString translation_type[10] = {"x", "y", "z", "view","normal view", "xy", "xz", "yz", "surface snap","surface snap with normal || x"}; 

  // rotation constraints
  m_Gui->Label("rotation constraints");
	m_Gui->Combo(ID_ROTATION_AXES,"",&RotationConstraintId,5,rot_axes);

  // translation constraints
  m_Gui->Label("translation constraints");
	m_Gui->Combo(ID_TRASLATION_AXES,"",&TranslationConstraintId,10,translation_type);

	m_Gui->Divider();

  m_Gui->Update();
}

//----------------------------------------------------------------------------
void mafGUITransformMouse::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  switch(maf_event->GetId())
  {
    case ID_ROTATION_AXES:
    {
      // set rotation constraint;
      if (RotationConstraintId == X_AXIS)
      {
        IsaRotate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaRotate->GetRotationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::LOCK, mmiConstraint::LOCK);
      }
      else if (RotationConstraintId == Y_AXIS)
      {
        IsaRotate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaRotate->GetRotationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::FREE, mmiConstraint::LOCK);
      }
      else if (RotationConstraintId == Z_AXIS)
      {
        IsaRotate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaRotate->GetRotationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::LOCK, mmiConstraint::FREE);
      }
      else if (RotationConstraintId == VIEW_PLANE)
      {
        IsaRotate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
				//Modified by Matteo 30-05-2006
				//IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::LOCK, mmiConstraint::FREE);
        //End Matteo
				IsaRotate->GetRotationConstraint()->GetRefSys()->SetTypeToView();
      }
			else if (RotationConstraintId == NORMAL_VIEW_PLANE)
			{
				IsaRotate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        //IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
				//Modified by Matteo 30-05-2006
				IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::LOCK, mmiConstraint::FREE);
        //End Matteo
				IsaRotate->GetRotationConstraint()->GetRefSys()->SetTypeToView();
			}
    }
    break;
    case ID_TRASLATION_AXES:
    {
      // set translation constraint;
      if (TranslationConstraintId == X_AXIS)
      {
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::LOCK, mmiConstraint::LOCK);   
      }
      else if (TranslationConstraintId == Y_AXIS)
      {
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::FREE, mmiConstraint::LOCK);        
      }
      else if (TranslationConstraintId == Z_AXIS)
      {                                             
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::LOCK, mmiConstraint::FREE);        
      }      
      else if (TranslationConstraintId == VIEW_PLANE)
      {
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToView();
				//Modified by Matteo 30-05-2006
				//IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());
				//IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
				//IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
				//End Matteo
			}
			else if (TranslationConstraintId == NORMAL_VIEW_PLANE)
      {
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToView();
				//Modified by Matteo 30-05-2006
				IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());
				IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
				IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
				//End Matteo
			}

      if (TranslationConstraintId == XY_PLANE)
      {
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
      }
      else if (TranslationConstraintId == XZ_PLANE)  
      {
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::LOCK, mmiConstraint::FREE);       
      }
      else if (TranslationConstraintId == YZ_PLANE)
      {               
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::FREE, mmiConstraint::FREE);   
      }
      // isa gen is sending matrix to the operation
      else if (TranslationConstraintId == SURFACE_SNAP)
      {
        IsaTranslate->SurfaceSnapOn();
        IsaTranslate->SurfaceNormalOff();
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToView();
				//Modified by Matteo 30-05-2006
				IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());
				IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
				IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
				//End Matteo
      }
      else if (TranslationConstraintId == NORMAL_SURFACE)
      {
        IsaTranslate->SurfaceSnapOff();
        IsaTranslate->SurfaceNormalOn();
       
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToView();
        //Modified by Matteo 30-05-2006
        IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetPivotRefSys()->SetTypeToCustom(m_RefSysVME->GetOutput()->GetAbsMatrix());
        IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
        //End Matteo
      }
    }
    break;

    case ID_TRANSFORM:
    {
      // forward transform events to listener operation; the operation will move the vme
      maf_event->SetSender(this);
      mafEventMacro(*maf_event);
    }
    break;

    default:
    {
      mafEventMacro(*maf_event);
    }
    break;
  }
}

//----------------------------------------------------------------------------
void mafGUITransformMouse::CreateISA()
//----------------------------------------------------------------------------
{
  OldInteractor = m_InputVME->GetBehavior();

  // Create the isa compositor:
  IsaCompositor = mmiCompositorMouse::New();

  // default aux ref sys is the vme ref sys
  m_RefSysVME = m_InputVME;

  mafMatrix *absMatrix;
  absMatrix = m_RefSysVME->GetOutput()->GetAbsMatrix();
  //----------------------------------------------------------------------------
	// create the rotate behavior
	//----------------------------------------------------------------------------
  
  IsaRotate = IsaCompositor->CreateBehavior(MOUSE_LEFT);
  
  IsaRotate->SetListener(this); 
  IsaRotate->SetVME(m_InputVME);
  IsaRotate->GetRotationConstraint()->GetRefSys()->SetTypeToView();
  IsaRotate->GetRotationConstraint()->GetRefSys()->SetMatrix(absMatrix);
  
  IsaRotate->GetPivotRefSys()->SetTypeToView();
  IsaRotate->GetPivotRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetMatrix());
  
  //IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
	//Modified by Matteo 30-05-2006
	IsaRotate->GetRotationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::LOCK, mmiConstraint::FREE);
  //End Matteo
  IsaRotate->EnableRotation(true);
  
	//----------------------------------------------------------------------------
	// create the translate behavior
	//----------------------------------------------------------------------------
  
  IsaTranslate = IsaCompositor->CreateBehavior(MOUSE_MIDDLE);
  
  IsaTranslate->SetListener(this);
  IsaTranslate->SetVME(m_InputVME);
  IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetTypeToView();
  // set the pivot point
  IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetMatrix(absMatrix);
  IsaTranslate->GetPivotRefSys()->SetTypeToCustom(absMatrix);

  IsaTranslate->GetTranslationConstraint()->SetConstraintModality(mmiConstraint::FREE, mmiConstraint::FREE, mmiConstraint::LOCK);
  IsaTranslate->EnableTranslation(true);

  //----------------------------------------------------------------------------
	// create the roll behavior
	//----------------------------------------------------------------------------
  
  IsaRoll = IsaCompositor->CreateBehavior(MOUSE_LEFT_CONTROL);
  
  // isa gen is sending matrix to the operation
  IsaRoll->SetListener(this);
  IsaRoll->SetVME(m_InputVME);
  IsaRoll->GetRotationConstraint()->GetRefSys()->SetTypeToView();
  IsaRoll->GetRotationConstraint()->GetRefSys()->SetMatrix(absMatrix);
  
  IsaRoll->GetPivotRefSys()->SetTypeToView();
  IsaRoll->GetPivotRefSys()->SetMatrix(absMatrix);
  
  IsaRoll->GetRotationConstraint()->SetConstraintModality(mmiConstraint::LOCK, mmiConstraint::LOCK, mmiConstraint::FREE);
  IsaRoll->EnableRotation(true);
}

//----------------------------------------------------------------------------
void mafGUITransformMouse::EnableWidgets(bool enable)
//----------------------------------------------------------------------------
{
  m_Gui->Enable(ID_ROTATION_AXES, enable);
  m_Gui->Enable(ID_TRASLATION_AXES, enable);

  if (enable == true)
  {
    AttachInteractorToVme();
  }
  else
  {
    DetachInteractorFromVme();
  }
}

//----------------------------------------------------------------------------
void mafGUITransformMouse::AttachInteractorToVme()
//----------------------------------------------------------------------------
{
  m_InputVME->SetBehavior(IsaCompositor);
}

//----------------------------------------------------------------------------
void mafGUITransformMouse::DetachInteractorFromVme()
//----------------------------------------------------------------------------
{
  m_InputVME->SetBehavior(OldInteractor);
}

//----------------------------------------------------------------------------
void mafGUITransformMouse::RefSysVmeChanged()
//----------------------------------------------------------------------------
{
  IsaTranslate->GetTranslationConstraint()->GetRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());
  IsaTranslate->GetPivotRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());

  IsaRotate->GetRotationConstraint()->GetRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());
  IsaRotate->GetPivotRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());

  IsaRoll->GetRotationConstraint()->GetRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());
  IsaRoll->GetPivotRefSys()->SetMatrix(m_RefSysVME->GetOutput()->GetAbsMatrix());
}