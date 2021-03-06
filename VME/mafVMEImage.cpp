/*=========================================================================

 Program: MAF2
 Module: mafVMEImage
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------



#include "mafVMEImage.h"
#include "mafMatrixInterpolator.h"
#include "mafDataVector.h"
#include "mafDataPipeInterpolatorVTK.h"
#include "mafVMEItemVTK.h"
#include "mafAbsMatrixPipe.h"
#include "vtkDataSet.h"
#include "vtkImageData.h"
//-------------------------------------------------------------------------
mafCxxTypeMacro(mafVMEImage)
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
mafVMEImage::mafVMEImage()
//-------------------------------------------------------------------------
{
}

//-------------------------------------------------------------------------
mafVMEImage::~mafVMEImage()
//-------------------------------------------------------------------------
{
  // data pipe destroyed in mafVME
  // data vector destroyed in mafVMEGeneric
}


//-------------------------------------------------------------------------
mafVMEOutput *mafVMEImage::GetOutput()
//-------------------------------------------------------------------------
{
  // allocate the right type of output on demand
  if (m_Output==NULL)
  {
    SetOutput(mafVMEOutputImage::New()); // create the output
  }
  return m_Output;
}

//-------------------------------------------------------------------------
int mafVMEImage::SetData(vtkImageData *data, mafTimeStamp t, int mode)
//-------------------------------------------------------------------------
{
  return Superclass::SetData(data,t,mode);
}
//-------------------------------------------------------------------------
int mafVMEImage::SetData(vtkDataSet *data, mafTimeStamp t, int mode)
//-------------------------------------------------------------------------
{
  assert(data);
  if (data->IsA("vtkImageData"))
  {
    return Superclass::SetData(data,t,mode);
  }
  
  return MAF_ERROR;
}

//-------------------------------------------------------------------------
char** mafVMEImage::GetIcon() 
//-------------------------------------------------------------------------
{
  #include "mafVMEImage.xpm"
  return mafVMEImage_xpm;
}
