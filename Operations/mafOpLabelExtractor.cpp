/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafOpLabelExtractor.cpp,v $
  Language:  C++
  Date:      $Date: 2007-11-22 13:34:56 $
  Version:   $Revision: 1.2 $
  Authors:   Paolo Quadrani - porting Roberto Mucci 
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


#include "mafOpLabelExtractor.h"

#include "mafDecl.h"
#include "mafOp.h"
#include "mafEvent.h"
#include "mmgGui.h"
#include "mafVME.h"
#include "mafVMEVolumeGray.h"
#include "mafVMESurface.h"

#include "vtkImageData.h"
#include "vtkImageGaussianSmooth.h"
#include "vtkExtractVOI.h"
#include "vtkImageThreshold.h"
#include "vtkMatrix4x4.h"
#include "vtkContourVolumeMapper.h"
#include "vtkStructuredPoints.h"
#include "vtkRectilinearGrid.h"
#include "vtkMAFSmartPointer.h"
#include "vtkPointData.h"
#include "vtkImageToStructuredPoints.h"


//----------------------------------------------------------------------------
mafOpLabelExtractor::mafOpLabelExtractor(const wxString& label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType	= OPTYPE_OP;
  m_Canundo = true;

	m_Vme = NULL;

	m_ValLabel     = 0;
	m_SurfaceName  = "label 0";
  m_SmoothVolume = 0;
  m_RadiusFactor = 0.5;
  m_RadiusFactorAfter = 0.5;
  m_StdDev[0] = m_StdDev[1] = 1.5;
  m_StdDev[2] = 2.0;
  m_StdDevAfter[0] = m_StdDevAfter[1] = 1.5;
  m_StdDevAfter[2] = 2.0;
  m_SamplingRate[0] = m_SamplingRate[1] = 3;
  m_SamplingRate[2] = 1;
}
//----------------------------------------------------------------------------
mafOpLabelExtractor::~mafOpLabelExtractor()
//----------------------------------------------------------------------------
{
	mafDEL(m_Vme);
}
//----------------------------------------------------------------------------
mafOp* mafOpLabelExtractor::Copy()
//----------------------------------------------------------------------------
{
  return (new mafOpLabelExtractor(m_Label));
}
//----------------------------------------------------------------------------
bool mafOpLabelExtractor::Accept(mafNode *vme)
//----------------------------------------------------------------------------
{
	return (vme != NULL && ((mafVME *)vme)->GetOutput()->IsA("mafVMEOutputVolume") && !vme->IsA("mafVMEImage"));
}
//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
enum {
	ID_LABEL = MINID,	
  ID_SMOOTH,
  ID_RADIUS_FACTOR,
  ID_STD_DEVIATION,
  ID_SAMPLING_RATE,
  ID_RADIUS_FACTOR_AFTER,
  ID_STD_DEVIATION_AFTER,
	ID_NAME,
};
//----------------------------------------------------------------------------
void mafOpLabelExtractor::OpRun()   
//----------------------------------------------------------------------------
{
	m_Gui = new mmgGui(this);
	m_Gui->SetListener(this);

	m_Gui->Bool(ID_SMOOTH,_("smooth"),&m_SmoothVolume,0,_("gaussian smooth for extracting big surface"));
  m_Gui->Divider(2);
  m_Gui->Float(ID_RADIUS_FACTOR,_("rad. factor"),&m_RadiusFactor,0.1,MAXFLOAT,0,2,_("max distance to consider for the smooth."));
  m_Gui->Vector(ID_STD_DEVIATION,_("std dev."),m_StdDev, 0.1, MAXFLOAT,2,_("standard deviation for the smooth."));
  m_Gui->Divider();
  m_Gui->Vector(ID_SAMPLING_RATE,_("sample rate"),m_SamplingRate, 1, MAXINT,_("sampling rate for volume sub-sampling."));
  m_Gui->Divider();
  m_Gui->Float(ID_RADIUS_FACTOR_AFTER,_("rad. factor"),&m_RadiusFactorAfter,0.1,MAXFLOAT,0,2,_("max distance to consider for the smooth."));
  m_Gui->Vector(ID_STD_DEVIATION_AFTER,_("std dev."),m_StdDevAfter, 0.1, MAXFLOAT,2,_("standard deviation for the smooth."));
  m_Gui->Divider(2);
  m_Gui->Divider();
  m_Gui->Double(ID_LABEL,_("label"), &m_ValLabel);
	m_Gui->String(ID_NAME,_("name"),&m_SurfaceName);
	m_Gui->Divider();
	m_Gui->OkCancel();

  m_Gui->Enable(ID_RADIUS_FACTOR,m_SmoothVolume != 0);
  m_Gui->Enable(ID_STD_DEVIATION,m_SmoothVolume != 0);
  m_Gui->Enable(ID_RADIUS_FACTOR_AFTER,m_SmoothVolume != 0);
  m_Gui->Enable(ID_STD_DEVIATION_AFTER,m_SmoothVolume != 0);
  m_Gui->Enable(ID_SAMPLING_RATE,m_SmoothVolume != 0);
	ShowGui();
}

//----------------------------------------------------------------------------
void mafOpLabelExtractor::OpDo()
//----------------------------------------------------------------------------
{
  m_Output = m_Vme;
	mafEventMacro(mafEvent(this,VME_ADD,m_Vme));
}
//----------------------------------------------------------------------------
void mafOpLabelExtractor::OpUndo()
//----------------------------------------------------------------------------
{
	mafEventMacro(mafEvent(this,VME_REMOVE,m_Vme)); //serve??
}
//----------------------------------------------------------------------------
void mafOpLabelExtractor::OnEvent(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {
		  case wxOK:
			  ExtractLabel();
			  if(((mafVME *)m_Input)->GetOutput()->GetVTKData() != NULL)
				  OpStop(OP_RUN_OK);
			  else
			  {
				  wxString msg = wxString::Format("Label %d not found!", m_ValLabel);
				  wxMessageBox(msg,"Warning");
				  OpStop(OP_RUN_CANCEL);
			  }
		  break;
		  case wxCANCEL:
			  OpStop(OP_RUN_CANCEL);
		  break;
      case ID_SMOOTH:
        m_Gui->Enable(ID_RADIUS_FACTOR,m_SmoothVolume != 0);
        m_Gui->Enable(ID_STD_DEVIATION,m_SmoothVolume != 0);
        m_Gui->Enable(ID_SAMPLING_RATE,m_SmoothVolume != 0);
        m_Gui->Enable(ID_RADIUS_FACTOR_AFTER,m_SmoothVolume != 0);
        m_Gui->Enable(ID_STD_DEVIATION_AFTER,m_SmoothVolume != 0);
      break;
      default:
          mafEventMacro(*maf_event); 
        break;
    }
	}
}
//----------------------------------------------------------------------------
void mafOpLabelExtractor::ExtractLabel()
//----------------------------------------------------------------------------
{
  vtkMAFSmartPointer<vtkImageToStructuredPoints> imageToSp;
	mafVME *vme = (mafVME *)m_Input;
  vtkDataSet *ds = vme->GetOutput()->GetVTKData();
  vtkMAFSmartPointer<vtkImageData> vol;
  vtkMAFSmartPointer<vtkImageData> imageDataRg; 
  

  double bounds[6];
  int dim[3], xdim, ydim, zdim, slice_size;

  vme->GetOutput()->GetBounds(bounds);

  double xmin = bounds[0];
  double xmax = bounds[1];
  double ymin = bounds[2];
  double ymax = bounds[3];
  double zmin = bounds[4];
  double zmax = bounds[5];	   

  //setting the ImageData for RectilinearGrid
  if (vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(ds))
  {
    double volumeSpacing[3];
    volumeSpacing[0] = VTK_DOUBLE_MAX;
    volumeSpacing[1] = VTK_DOUBLE_MAX;
    volumeSpacing[2] = VTK_DOUBLE_MAX;

    for (int xi = 1; xi < rgrid->GetXCoordinates()->GetNumberOfTuples (); xi++)
    {
      double spcx = rgrid->GetXCoordinates()->GetTuple1(xi)-rgrid->GetXCoordinates()->GetTuple1(xi-1);
      if (volumeSpacing[0] > spcx)
        volumeSpacing[0] = spcx;
    }

    for (int yi = 1; yi < rgrid->GetYCoordinates()->GetNumberOfTuples (); yi++)
    {
      double spcy = rgrid->GetYCoordinates()->GetTuple1(yi)-rgrid->GetYCoordinates()->GetTuple1(yi-1);
      if (volumeSpacing[1] > spcy)
        volumeSpacing[1] = spcy;
    }

    for (int zi = 1; zi < rgrid->GetZCoordinates()->GetNumberOfTuples (); zi++)
    {
      double spcz = rgrid->GetZCoordinates()->GetTuple1(zi)-rgrid->GetZCoordinates()->GetTuple1(zi-1);
      if (volumeSpacing[2] > spcz)
        volumeSpacing[2] = spcz;
    }

    double origin[3];
    origin[0] = bounds[0];
    origin[1] = bounds[2];
    origin[2] = bounds[4];

    int output_extent[6];
    output_extent[0] = 0;
    output_extent[1] = (bounds[1] - bounds[0]) / volumeSpacing[0];
    output_extent[2] = 0;
    output_extent[3] = (bounds[3] - bounds[2]) / volumeSpacing[1];
    output_extent[4] = 0;
    output_extent[5] = (bounds[5] - bounds[4]) / volumeSpacing[2];

    vtkMAFSmartPointer<vtkStructuredPoints> output_data;
    output_data->SetSpacing(volumeSpacing);
    // TODO: here I probably should allow a data type casting... i.e. a GUI widget
    output_data->SetScalarType(ds->GetPointData()->GetScalars()->GetDataType());
    output_data->SetExtent(output_extent);
    output_data->SetUpdateExtent(output_extent);

    rgrid->GetDimensions(dim);
    xdim = dim[0];
    ydim = dim[1];
    zdim = dim[2];
    slice_size = xdim*ydim;
    
    output_data->SetOrigin(xdim ,ydim, zdim);
    output_data->SetDimensions(xdim, ydim, zdim);
    output_data->GetPointData()->SetScalars(rgrid->GetPointData()->GetScalars());
    imageDataRg->Update();

    vol->DeepCopy((vtkImageData *)output_data);
    imageToSp->SetInput(vol);
    imageToSp->Update();
  }
  else
  {
    vtkStructuredPoints *sp = vtkStructuredPoints::SafeDownCast(ds);
    vol->DeepCopy((vtkImageData *)sp);
    imageToSp->SetInput(vol);
    imageToSp->Update();
  }

	vtkMAFSmartPointer<vtkImageThreshold> it;
  it->SetInput(vol);

	double maxValue;
	switch(vol->GetScalarType()) 
	{
	  case VTK_UNSIGNED_SHORT:
      maxValue = VTK_UNSIGNED_SHORT_MAX;
		break;
	  case VTK_SHORT:
			maxValue = VTK_SHORT_MAX;
		break;
		case VTK_UNSIGNED_CHAR:
			maxValue = VTK_UNSIGNED_CHAR_MAX;
		break;
		case VTK_CHAR:
			maxValue = VTK_CHAR_MAX;
		break;
    case VTK_DOUBLE:
      maxValue = VTK_DOUBLE_MAX;
    break;
	  default:
			maxValue = 255;
		break;
	}
  it->SetInValue(maxValue);
  it->SetOutValue(0);
	it->SetOutputScalarType(vol->GetScalarType());
  it->ThresholdBetween(m_ValLabel, m_ValLabel);
  it->Update();

  vtkMAFSmartPointer<vtkImageGaussianSmooth> smooth;
  vtkMAFSmartPointer<vtkImageGaussianSmooth> smoothAfter;

  vtkMAFSmartPointer<vtkExtractVOI> extract;

  if(m_SmoothVolume)
  {
    int b[6];
    smooth->SetInput(it->GetOutput());
    smooth->SetRadiusFactor(m_RadiusFactor);
    smooth->SetStandardDeviation(m_StdDev[0],m_StdDev[1],m_StdDev[2]);
    smooth->SetDimensionality(3);
    smooth->Update();
    smooth->GetOutput()->GetExtent(b);
    extract->SetInput(smooth->GetOutput());
    extract->SetVOI(b);
    extract->SetSampleRate(m_SamplingRate);
    extract->Update();
    smoothAfter->SetInput(extract->GetOutput());
    smoothAfter->SetRadiusFactor(m_RadiusFactorAfter);
    smoothAfter->SetStandardDeviation(m_StdDevAfter[0],m_StdDevAfter[1],m_StdDevAfter[2]);
    smoothAfter->SetDimensionality(3);
    smoothAfter->Update();
    vol->DeepCopy(smoothAfter->GetOutput());
  }
  else
  {
    vol->DeepCopy(it->GetOutput());
  }
 
  vtkMAFSmartPointer<vtkContourVolumeMapper> contourMapper;
  contourMapper->SetInput((vtkDataSet *)imageToSp->GetOutput());
  contourMapper->SetContourValue(m_ValLabel);
	
	vtkMAFSmartPointer<vtkPolyData> surface;
  contourMapper->GetOutput(0, surface);	
	contourMapper->Update();

  mafNEW(m_Vme);
	m_Vme->SetData(surface, 0.0);
  m_Vme->SetName(m_SurfaceName.c_str());
  m_Vme->Update();
}

//----------------------------------------------------------------------------
void mafOpLabelExtractor::SetLabel(double labelValue)
//----------------------------------------------------------------------------
{
  m_ValLabel = labelValue;
}

//----------------------------------------------------------------------------
void mafOpLabelExtractor::SmoothMode(bool smoothMode)
//----------------------------------------------------------------------------
{
  m_SmoothVolume = smoothMode;
}
