/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mmoMeshImporter.h,v $
  Language:  C++
  Date:      $Date: 2007-04-13 12:03:02 $
  Version:   $Revision: 1.1 $
  Authors:   Stefano Perticoni   
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __mmoMeshImporter_H__
#define __mmoMeshImporter_H__

//----------------------------------------------------------------------------
// Include :
//----------------------------------------------------------------------------
#include "mafOp.h"
#include "mafString.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafVME;
class mafVMEMesh;
class mafEvent;

//----------------------------------------------------------------------------
// mmoMeshImporter :
//----------------------------------------------------------------------------
/** */
class mmoMeshImporter: public mafOp
{
public:
	mmoMeshImporter(const wxString &label = "STLImporter");
	~mmoMeshImporter(); 
	
  mafTypeMacro(mmoMeshImporter, mafOp);

  mafOp* Copy();

	/** Return true for the acceptable vme type. */
	bool Accept(mafNode *node);

	/** Set the filename for the .stl to import */
  void SetFileName(const char *file_name);

  /**
  // Set/Get nodes file name*/
  void SetNodesFileName(const char *name)   {this->m_NodesFileName = name;};
  const char *GetNodesFileName() {return this->m_NodesFileName;};

  /**
  // Set/Get elements file name*/
  void SetElementsFileName(const char *name)   {this->m_ElementsFileName = name;};
  const char *GetElementsFileName() {return this->m_ElementsFileName;};

  /**
  // Set/Get materials file name*/
  void SetMaterialsFileName(const char *name) {this->m_MaterialsFileName = name;};
  const char *GetMaterialsFileName() {return this->m_MaterialsFileName;};

  /** Builds operation's interface. */
	void OpRun();

	/** Import the mesh*/
	int Read();

  virtual void OnEvent(mafEventBase *maf_event);

protected:
  

  /** Create the dialog interface for the importer. */
  virtual void CreateGui();  

  /** This method is called at the end of the operation and result contain the wxOK or wxCANCEL. */
  void OpStop(int result);

  int m_ImporterType;
	mafVMEMesh *m_ImportedVmeMesh;
  mafString	m_FileDir;

  /**
  Nodes file name*/
  mafString m_NodesFileName;

  /**
  Elements file name*/
  mafString m_ElementsFileName;

  /**
  Materials file name*/
  mafString m_MaterialsFileName;
};
#endif