/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafViewCompound.cpp,v $
  Language:  C++
  Date:      $Date: 2005-11-21 18:21:19 $
  Version:   $Revision: 1.12 $
  Authors:   Paolo Quadrani
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

#include "mafViewCompound.h"
#include "mafViewVTK.h"
#include "mafRWI.h"
#include "mafSceneGraph.h"
#include "mafSceneNode.h"
#include "mmgViewWin.h"
#include "mmgGui.h"
#include "mafInteractor.h"
#include "mafAvatar.h"
#include "mafAvatar3D.h"
#include "mmdTracker.h"
#include "mmdMouse.h"

//----------------------------------------------------------------------------
mafCxxTypeMacro(mafViewCompound);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafViewCompound::mafViewCompound( wxString label, int num_row, int num_col, bool external)
: mafView(label, external)
//----------------------------------------------------------------------------
{
  m_ViewRowNum = num_row;
  m_ViewColNum = num_col;
  m_NumOfPluggedChildren  = 0;
  m_DefauldChildView      = 0;
  m_ChildViewList.clear();
  m_PluggedChildViewList.clear();
  m_GuiView = NULL;
  m_SubViewMaximized = -1;
  m_Mouse = NULL;
}
//----------------------------------------------------------------------------
mafViewCompound::~mafViewCompound()
//----------------------------------------------------------------------------
{
  m_PluggedChildViewList.clear();
  for(int i=0; i<m_NumOfChildView; i++)
  {
    cppDEL(m_ChildViewList[i]);
  }
  m_ChildViewList.clear();
}
//----------------------------------------------------------------------------
mafView *mafViewCompound::Copy(mafObserver *Listener)
//----------------------------------------------------------------------------
{
  mafViewCompound *v = new mafViewCompound(m_Label, m_ViewRowNum, m_ViewColNum, m_ExternalFlag);
  v->m_Listener = Listener;
  v->m_Id = m_Id;
  v->m_PluggedChildViewList = m_PluggedChildViewList;
  v->m_NumOfPluggedChildren = m_NumOfPluggedChildren;
  v->Create();
  return v;
}
//----------------------------------------------------------------------------
void mafViewCompound::PlugChildView(mafView *child)
//----------------------------------------------------------------------------
{
  if (child)
  {
    m_PluggedChildViewList.push_back(child);
    m_NumOfPluggedChildren++;
  }
}
//----------------------------------------------------------------------------
void mafViewCompound::Create()
//----------------------------------------------------------------------------
{
  mmgViewWin *w = new mmgViewWin(mafGetFrame(),-1);
  w->SetBackgroundColour(wxColour(102,102,102));
  w->m_Owner = this;
  w->Show(false);
  m_Win = w;

  for(int i=0; i<m_NumOfPluggedChildren; i++)
  {
    m_ChildViewList.push_back(m_PluggedChildViewList[i]->Copy(this));
    m_ChildViewList[i]->GetWindow()->Reparent(m_Win);
    m_ChildViewList[i]->GetWindow()->Show(true);
  }
  for (int f=m_NumOfPluggedChildren; f < m_ViewColNum * m_ViewRowNum; f++)
  {
    m_ChildViewList.push_back(m_PluggedChildViewList[m_NumOfPluggedChildren-1]->Copy(this));
    m_ChildViewList[f]->GetWindow()->Reparent(m_Win);
    m_ChildViewList[f]->GetWindow()->Show(true);
  }
  m_NumOfChildView = m_ChildViewList.size();

  CreateGuiView();
}
//----------------------------------------------------------------------------
void mafViewCompound::CreateGuiView()
//----------------------------------------------------------------------------
{
  m_GuiView = new mmgGui(this);
  m_GuiView->Label("Compound View's GUI",true);
  m_GuiView->Reparent(m_Win);
}
//----------------------------------------------------------------------------
void mafViewCompound::VmeAdd(mafNode *node)
//----------------------------------------------------------------------------
{
  for(int i=0; i<m_NumOfChildView; i++)
    m_ChildViewList[i]->VmeAdd(node);
}
//----------------------------------------------------------------------------
void mafViewCompound::VmeRemove(mafNode *node)
//----------------------------------------------------------------------------
{
  for(int i=0; i<m_NumOfChildView; i++)
    m_ChildViewList[i]->VmeRemove(node);
}
//----------------------------------------------------------------------------
void mafViewCompound::VmeSelect(mafNode *node, bool select)
//----------------------------------------------------------------------------
{
  for(int i=0; i<m_NumOfChildView; i++)
    m_ChildViewList[i]->VmeSelect(node, select);
}
//----------------------------------------------------------------------------
void mafViewCompound::VmeShow(mafNode *node, bool show)
//----------------------------------------------------------------------------
{
  for(int i=0; i<m_NumOfChildView; i++)
    m_ChildViewList[i]->VmeShow(node, show);
}
//----------------------------------------------------------------------------
int mafViewCompound::GetNodeStatus(mafNode *node)
//----------------------------------------------------------------------------
{
  // should be redefined for compounded views
  return m_ChildViewList[m_DefauldChildView]->GetNodeStatus(node);
}
//----------------------------------------------------------------------------
void mafViewCompound::CameraReset(mafNode *node)
//----------------------------------------------------------------------------
{
  for(int i=0; i<m_NumOfChildView; i++)
    m_ChildViewList[i]->CameraReset(node);
}
//----------------------------------------------------------------------------
void mafViewCompound::CameraUpdate()
//----------------------------------------------------------------------------
{
  for(int i=0; i<m_NumOfChildView; i++)
    m_ChildViewList[i]->CameraUpdate();
}
//----------------------------------------------------------------------------
mafSceneGraph *mafViewCompound::GetSceneGraph()
//----------------------------------------------------------------------------
{
  return ((mafViewVTK *)m_ChildViewList[m_DefauldChildView])->GetSceneGraph();
}
//----------------------------------------------------------------------------
mafRWIBase *mafViewCompound::GetRWI()
//----------------------------------------------------------------------------
{
  //return ((mafViewVTK *)m_ChildViewList[m_DefauldChildView])->GetRWI();
  return GetSubView()->GetRWI();
}
//----------------------------------------------------------------------------
bool mafViewCompound::FindPokedVme(mafDevice *device,mafMatrix &point_pose,vtkProp3D *&picked_prop,mafVME *&picked_vme,mafInteractor *&picked_behavior)
//----------------------------------------------------------------------------
{
  mafViewVTK *v = mafViewVTK::SafeDownCast(GetSubView());
  if (v)
    return v->FindPokedVme(device,point_pose,picked_prop,picked_vme,picked_behavior);
  else
    return false;
}
/*//----------------------------------------------------------------------------
mafPipe *mafViewCompound::GetNodePipe(mafNode *vme)
//----------------------------------------------------------------------------
{
  mafSceneGraph *sg = ((mafViewVTK *)m_ChildViewList[m_DefauldChildView])->GetSceneGraph();
  assert(sg);
  mafSceneNode *n = sg->Vme2Node(vme);
  if(!n) 
    return NULL;
  return n->m_Pipe;
}
//----------------------------------------------------------------------------
mmgGui *mafViewCompound::GetNodePipeGUI(mafNode *vme)
//----------------------------------------------------------------------------
{
  return GetNodePipeGUI(vme, m_DefauldChildView);
}
//----------------------------------------------------------------------------
mmgGui *mafViewCompound::GetNodePipeGUI(mafNode *vme, int view_idx)
//----------------------------------------------------------------------------
{
  mafSceneGraph *sg = ((mafViewVTK *)m_ChildViewList[view_idx])->GetSceneGraph();
  assert(sg);
  mafSceneNode *n = sg->Vme2Node(vme);
  if(!n) 
    return NULL;
  if(n->m_Pipe)
  {
    return n->m_Pipe->GetGui();
  }
  return NULL;
}*/
//----------------------------------------------------------------------------
void mafViewCompound::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  switch(maf_event->GetId()) 
  {
    case ID_DEFAULT_CHILD_VIEW:
    {
      mafSceneGraph *sg = GetSceneGraph();
      if(sg)
      {
        mafNode *vme = sg->GetSelectedVme();
        if (vme)
        {
          mafEventMacro(mafEvent(this,VME_MODIFIED,vme));
        }
      }
    }
  	break;
    default:
      mafEventMacro(*maf_event);
  }
}
//-------------------------------------------------------------------------
mmgGui* mafViewCompound::CreateGui()
//-------------------------------------------------------------------------
{
  mafString childview_tooltip;
  childview_tooltip = "set the default child view";

  assert(m_Gui == NULL);
  m_Gui = new mmgGui(this);
  m_Gui->Integer(ID_DEFAULT_CHILD_VIEW,"default child", &m_DefauldChildView, 0, m_NumOfChildView, childview_tooltip);
  return m_Gui;
}
//----------------------------------------------------------------------------
void mafViewCompound::OnSize(wxSizeEvent &event)
//----------------------------------------------------------------------------
{
  m_Size = event.GetSize();
  OnLayout();
}
//----------------------------------------------------------------------------
void mafViewCompound::OnLayout()
//----------------------------------------------------------------------------
{
  assert(m_NumOfPluggedChildren == m_PluggedChildViewList.size());
  if (m_NumOfPluggedChildren == 0)
  {
    return;
  }

  int sw = m_Size.GetWidth();
  int sh = m_Size.GetHeight();

  wxSize gui_size = m_GuiView->GetBestSize();
  int gh = gui_size.GetHeight();

  if(sw<gh || sh<gh) return;

  sh -= gh;
  m_GuiView->SetSize(0,sh,sw,gh);

  if (m_SubViewMaximized == -1)
  {
    // this implement the Fixed SubViews Layout
    int border = 2;
    int x_pos, y_pos;
    int step_width  = (sw-border)  / m_ViewColNum;
    int step_height = (sh-2*border)/ m_ViewRowNum;

    int i = 0;
    for (int r=0; r<m_ViewRowNum; r++)
    {
      for (int c=0; c<m_ViewColNum; c++)
      {
        x_pos = c*(step_width + border);
        y_pos = r*(step_height + border);
        m_ChildViewList[i]->GetWindow()->SetSize(x_pos,y_pos,step_width,step_height);
        i++;
      }
    }
  }
  else
  {
    int i = 0;
    for (int r=0; r<m_ViewRowNum; r++)
    {
      for (int c=0; c<m_ViewColNum; c++)
      {
        if (i == m_SubViewMaximized)
        {
          m_ChildViewList[m_SubViewMaximized]->GetWindow()->SetSize(0,0,sw,sh);
        }
        else
        {
          m_ChildViewList[i]->GetWindow()->SetSize(0,0,0,0);
        }
        i++;
      }
    }
  }
}
//----------------------------------------------------------------------------
void mafViewCompound::SetMouse(mmdMouse *mouse)
//----------------------------------------------------------------------------
{
  m_Mouse = mouse;
  for(int i=0; i<m_NumOfChildView; i++)
  {
    ((mafViewVTK *)m_ChildViewList[i])->SetMouse(mouse);
  }
}
//----------------------------------------------------------------------------
mafView *mafViewCompound::GetSubView()
//----------------------------------------------------------------------------
{
  mafRWIBase *rwi = m_Mouse->GetRWI();
  if (rwi)
  {
    for(int i=0; i<m_NumOfChildView; i++)
    {
      if (((mafViewVTK *)m_ChildViewList[i])->GetRWI() == rwi)
      {
        return m_ChildViewList[i];
      }
    }
  }
  return m_ChildViewList[m_DefauldChildView];
}
//----------------------------------------------------------------------------
int mafViewCompound::GetSubViewIndex()
//----------------------------------------------------------------------------
{
  mafRWIBase *rwi = m_Mouse->GetRWI();
  if (rwi)
  {
    for(int i=0; i<m_NumOfChildView; i++)
    {
      if (((mafViewVTK *)m_ChildViewList[i])->GetRWI() == rwi)
      {
        return i;
      }
    }
  }
  return m_DefauldChildView;
}
//----------------------------------------------------------------------------
void mafViewCompound::MaximizeSubView(int subview_id, bool maximize)
//----------------------------------------------------------------------------
{
  if (m_SubViewMaximized != -1 && maximize)
  {
    return;
  }
  if (subview_id < 0 || subview_id >= m_NumOfChildView)
  {
    mafMessage("Wrong sub-view id !!");
    return;
  }
  m_SubViewMaximized = maximize ? subview_id : -1;
  OnLayout();
}
//----------------------------------------------------------------------------
bool mafViewCompound::Pick(int x, int y)
//----------------------------------------------------------------------------
{
  mafView *sub_view = GetSubView();
  if (sub_view)
  {
    return sub_view->Pick(x,y);
  }
  return false;
}
//----------------------------------------------------------------------------
bool mafViewCompound::Pick(mafMatrix &m)
//----------------------------------------------------------------------------
{
  mafView *sub_view = GetSubView();
  if (sub_view)
  {
    return sub_view->Pick(m);
  }
  return false;
}
//----------------------------------------------------------------------------
void mafViewCompound::GetPickedPosition(double pos[3])
//----------------------------------------------------------------------------
{
  mafView *sub_view = GetSubView();
  if (sub_view)
  {
    sub_view->GetPickedPosition(pos);
  }
}
//----------------------------------------------------------------------------
mafVME *mafViewCompound::GetPickedVme()
//----------------------------------------------------------------------------
{
  mafView *sub_view = GetSubView();
  if (sub_view)
  {
    return sub_view->GetPickedVme();
  }
  return NULL;
}