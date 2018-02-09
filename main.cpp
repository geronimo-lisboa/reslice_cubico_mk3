#include "stdafx.h"
#include "loadVolume.h"
#include "utils.h"
class ObserveLoadProgressCommand : public itk::Command
{
public:
	itkNewMacro(ObserveLoadProgressCommand);
	void Execute(itk::Object * caller, const itk::EventObject & event)
	{
		Execute((const itk::Object *)caller, event);
	}

	void Execute(const itk::Object * caller, const itk::EventObject & event)
	{
		if (!itk::ProgressEvent().CheckEvent(&event))
		{
			return;
		}
		const itk::ProcessObject * processObject =
			dynamic_cast< const itk::ProcessObject * >(caller);
		if (!processObject)
		{
			return;
		}
		std::cout << processObject->GetProgress() << std::endl;
	}
};

int main(int argc, char** argv) {
	///Carga da imagem
	ObserveLoadProgressCommand::Pointer prog = ObserveLoadProgressCommand::New();
	const std::string txtFile = "C:\\meus dicoms\\abdomem-feet-first";// "C:\\meus dicoms\\Marching Man";//"C:\\meus dicoms\\abdomem-feet-first";//"C:\\meus dicoms\\Marching Man";//"C:\\meus dicoms\\abdomem-feet-first";//"C:\\meus dicoms\\Marching Man"; //"C:\\meus dicoms\\abdomem-feet-first";
	const std::vector<std::string> lst = GetList(txtFile);
	std::map<std::string, std::string> metadataDaImagem;
	itk::Image<short, 3>::Pointer imagemOriginal = LoadVolume(metadataDaImagem, lst, prog);
	vtkSmartPointer<vtkImageImport> imagemImportadaPraVTK = CreateVTKImage(imagemOriginal);//importa a imagem da itk pra vtk.
	imagemImportadaPraVTK->Update();

	vtkSmartPointer<vtkOpenGLRenderer> rendererDaCamadaDaImagem = vtkSmartPointer<vtkOpenGLRenderer>::New();
	rendererDaCamadaDaImagem->GetActiveCamera()->ParallelProjectionOn();
	rendererDaCamadaDaImagem->SetBackground(0.1, 0.2, 0.4);
	rendererDaCamadaDaImagem->ResetCamera();
	rendererDaCamadaDaImagem->GetActiveCamera()->Zoom(1.5);
	rendererDaCamadaDaImagem->SetLayer(0);
	rendererDaCamadaDaImagem->SetBackground(1, 0, 0);
	vtkSmartPointer<vtkWin32OpenGLRenderWindow> renderWindow = vtkSmartPointer<vtkWin32OpenGLRenderWindow>::New();
	renderWindow->SetNumberOfLayers(1);
	renderWindow->SetSize(500, 500);
	renderWindow->AddRenderer(rendererDaCamadaDaImagem);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindow->Render();

	//A tela dummy PROS PROBLEMAS DO OPENGL
	vtkSmartPointer<vtkRenderer> rendererDummy = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindowDummy = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowDummy->AddRenderer(rendererDummy);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractorDummy = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowDummy->SetInteractor(renderWindowInteractorDummy);
	renderWindowInteractorDummy->Initialize();
	renderWindowInteractorDummy->Start();

	return EXIT_SUCCESS;
}