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

class OrientationCubeMK2 {
private:
	vtkRenderer* renderer;
	vtkImageImport *image;
	vtkSmartPointer<vtkActor> cubeActor;
	vtkSmartPointer<vtkImageSlabReslice> thickSlabReslice;
	vtkSmartPointer<vtkImageMapToWindowLevelColors> colorMap;
	vtkSmartPointer<vtkImageActor> actorDaImagem;
	void setup() {
		if (!renderer || !image)
			return;
		//criação do algoritmo de reslice
		thickSlabReslice = vtkSmartPointer<vtkImageSlabReslice>::New();
		thickSlabReslice->TransformInputSamplingOff();
		thickSlabReslice->SetInputConnection(image->GetOutputPort());
		thickSlabReslice->SetOutputDimensionality(2);
		thickSlabReslice->AutoCropOutputOn();
		thickSlabReslice->SetOutputOriginToDefault();
		thickSlabReslice->SetOutputExtentToDefault();
		thickSlabReslice->SetInterpolationModeToLinear();
		thickSlabReslice->SetBlendModeToMean();
		double range[2];
		vtkImageData::SafeDownCast(thickSlabReslice->GetInput())->GetScalarRange(range);
		thickSlabReslice->SetBackgroundLevel(range[0]);
		vtkSmartPointer<vtkTransform> transformTrans = vtkSmartPointer<vtkTransform>::New();
		transformTrans->Translate(image->GetOutput()->GetCenter());
		transformTrans->Update();
		thickSlabReslice->SetResliceTransform(transformTrans);
		//criação do algoritmo do window/level
		colorMap = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
		colorMap->SetInputConnection(thickSlabReslice->GetOutputPort());
		colorMap->SetWindow(350);
		colorMap->SetLevel(50);
		//criação do cube actor;
		auto cubeSource = vtkSmartPointer<vtkCubeSource>::New();
		cubeSource->SetXLength(100);
		cubeSource->SetYLength(100);
		cubeSource->SetZLength(100);
		auto cubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		cubeMapper->SetInputConnection(cubeSource->GetOutputPort());
		cubeActor = vtkSmartPointer<vtkActor>::New();
		cubeActor->SetMapper(cubeMapper);
		cubeActor->GetProperty()->BackfaceCullingOff();
		cubeActor->GetProperty()->SetRepresentationToWireframe();
		cubeActor->GetProperty()->SetColor(0, 0.75, 0);
		cubeActor->GetProperty()->LightingOff();
		cubeActor->GetProperty()->BackfaceCullingOff();
		cubeActor->GetProperty()->SetLineWidth(2);
		cubeActor->SetPosition(image->GetOutput()->GetCenter());
		renderer->AddActor(cubeActor);
		renderer->ResetCamera();
		//criação do actor da imagem
		colorMap->Update();
		actorDaImagem = vtkSmartPointer<vtkImageActor>::New();
		actorDaImagem->SetInputData(colorMap->GetOutput());
		actorDaImagem->SetPosition(image->GetOutput()->GetCenter());
		renderer->AddActor(actorDaImagem);
	}
public:
	OrientationCubeMK2() {
		renderer = nullptr;
		image = nullptr;
		cubeActor = nullptr;
		thickSlabReslice = nullptr;
		colorMap = nullptr;
		actorDaImagem = nullptr;
	}
	void SetRenderer(vtkRenderer* ren) {
		renderer = ren;
		setup();
	}
	void SetImage(vtkImageImport *img) {
		image = img;
		setup();
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
	//Cria a tela do cubo
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
	auto style = vtkSmartPointer<vtkInteractorStyleTrackballActor>::New();
	renderWindowInteractor->SetInteractorStyle(style);
	renderWindow->Render();
	//Cria o reslice cubico
	OrientationCubeMK2 *cube = new OrientationCubeMK2();
	cube->SetRenderer(rendererDaCamadaDaImagem);
	cube->SetImage(imagemImportadaPraVTK);
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