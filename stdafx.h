#pragma once
#define _SCL_SECURE_NO_WARNINGS
#include <vtkWin32RenderWindowInteractor.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <memory>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkCommand.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>
#include <array>
#include <vtkProperty.h>
#include <vtkAxesActor.h>
#include <vtkTextProperty.h>
#include <vtkCaptionActor2D.h>
#include <vtkPropCollection.h>
#include <vector>
#include <string>
#include <itkImage.h>
#include <vtkImageImport.h>
#include <itkCommand.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <fstream>
#include <itkMacro.h>
#include <itkOrientImageFilter.h>
#include <vtkOpenGLRenderer.h>
#include <vtkWin32OpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImageSlabReslice.h>
#include "boost/date_time/posix_time/posix_time.hpp" //include all types plus i/o
#include <boost/lexical_cast.hpp>
#include <vtkXMLImageDataWriter.h>
#include <vtkMatrix4x4.h>
#include <assert.h>
#include <vtkImageActor.h>
#include <vtkImageProperty.h>
#include <vtkImageMapper3d.h>
#include <vtkObjectFactory.h>
#include <vtkLightsPass.h>
#include <vtkDefaultPass.h>
#include <vtkRenderPassCollection.h>
#include <vtkSequencePass.h>
#include <vtkCameraPass.h>
#include <vtkRenderState.h>
#include <SDL2\SDL_ttf.h>
#undef main
#include <SDL2/SDL.h>
#undef main
#include <vtkGeneralTransform.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>

#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkAlgorithmOutput.h>
#include <vtkPolyData.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyPaths.h>
#include <vtkAssemblyNode.h>
#include <vtkAssembly.h>
#include <vtkSphereSource.h>
#include <vtkImageFlip.h>

typedef void(_stdcall*FNCallbackDeCarga)(float);

struct ImageDataToDelphi {
	double spacing[3];
	double physicalOrigin[3];
	double uVector[3];
	double vVector[3];
	int imageSize[2];
	unsigned int bufferSize;
	short *bufferData;
};

typedef void(_stdcall *FNCallbackDoDicomReslice)(ImageDataToDelphi outData);