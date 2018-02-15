#include "stdafx.h"
#include "loadVolume.h"
#include "utils.h"
#include "myInteractorStyleTrackballActor.h"
#include "IOrientator.h"
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


class OrientationCubeRenderPass : public vtkRenderPass {
private:
	class ImagePass :public vtkRenderPass {
	public:
		static ImagePass* New() {
			return new ImagePass();
		}
		OrientationCubeRenderPass *outer;
		void Render(const vtkRenderState* s) {
			outer->actorDaImagem->Render(s->GetRenderer());
		}
	};

	class CubePass :public vtkRenderPass {
	public:
		static CubePass* New() {
			return new CubePass();
		}
		OrientationCubeRenderPass *outer;
		void Render(const vtkRenderState* s) {
			outer->actorDoCubo->Render(s->GetRenderer(), outer->actorDoCubo->GetMapper());
		}
	};

	vtkActor* actorDoCubo;
	vtkImageActor* actorDaImagem;
	vtkSmartPointer<vtkLightsPass> lightsPass;
	vtkSmartPointer<ImagePass> imagePass;
	vtkSmartPointer<CubePass> cubePass;

	OrientationCubeRenderPass() {
		lightsPass = vtkSmartPointer<vtkLightsPass>::New();
		imagePass = vtkSmartPointer<ImagePass>::New();
		imagePass->outer = this;
		cubePass = vtkSmartPointer<CubePass>::New();
		cubePass->outer = this;
	}
public:
	static OrientationCubeRenderPass *New() {
		return new OrientationCubeRenderPass();
	}

	void Render(const vtkRenderState* s) {
		lightsPass->Render(s);
		imagePass->Render(s);
		cubePass->Render(s);
	}
	void SetActorsRelevantes(vtkActor* doCubo, vtkImageActor* daImagem) {
		actorDoCubo = doCubo;
		actorDaImagem = daImagem;
	}
};

class OrientationCubeMK2 : public IOrientator {
public:
	enum Interpolacao { NearestNeighbour, Linear, Cubic };
	enum Funcao { MIP, MinP, Composite };
private:
	Interpolacao tipoInterpolacao;
	Funcao tipoFuncao;
	double thickness;
	double window, level;
	vtkRenderer* renderer;
	vtkImageImport *image;
	vtkSmartPointer<vtkActor> cubeActor;
	vtkSmartPointer<vtkImageSlabReslice> thickSlabReslice;
	vtkSmartPointer<vtkImageMapToWindowLevelColors> colorMap;
	vtkSmartPointer<vtkImageActor> actorDaImagem;
	vtkSmartPointer<OrientationCubeRenderPass> renderPass;
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
		colorMap->SetWindow(window);
		colorMap->SetLevel(level);
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
		//setup do renderpass
		renderPass = vtkSmartPointer<OrientationCubeRenderPass>::New();
		renderPass->SetActorsRelevantes(cubeActor, actorDaImagem);
		auto cameraPass = vtkSmartPointer<vtkCameraPass>::New();
		cameraPass->SetDelegatePass(renderPass);
		vtkOpenGLRenderer::SafeDownCast(renderer)->SetPass(cameraPass);
	}
	const short GetBackgroundLevel()const {
		double range[2];
		vtkImageData::SafeDownCast(thickSlabReslice->GetInput())->GetScalarRange(range);
		return range[0];
	}
	void DebugSave() {
		boost::posix_time::ptime current_date_microseconds = boost::posix_time::microsec_clock::local_time();
		long milliseconds = current_date_microseconds.time_of_day().total_milliseconds();
		std::string filename = "c:\\reslice_cubico_mk3\\dump\\" + boost::lexical_cast<std::string>(milliseconds) + ".vti";
		vtkSmartPointer<vtkXMLImageDataWriter> debugsave = vtkSmartPointer<vtkXMLImageDataWriter>::New();
		debugsave->SetFileName(filename.c_str());
		debugsave->SetInputData(thickSlabReslice->GetOutput());
		debugsave->BreakOnError();
		debugsave->Write();
	}
	
public:
	OrientationCubeMK2() {
		renderer = nullptr;
		image = nullptr;
		cubeActor = nullptr;
		thickSlabReslice = nullptr;
		colorMap = nullptr;
		actorDaImagem = nullptr;
		renderPass = nullptr;
		tipoInterpolacao = Linear;
		tipoFuncao = Composite;
		thickness = 1.0;
		window = 350;
		level = 50;
	}
	void SetRenderer(vtkRenderer* ren) {
		renderer = ren;
		setup();
	}
	void SetImage(vtkImageImport *img) {
		image = img;
		setup();
	}

	void UpdateReslice() {
		//se ainda não criou as coisas tenta criar e retorna. Só deve rodar
		//quando estiver estável.
		if (!thickSlabReslice) {
			setup();
			return;
		}
		//configura o novo estado do reslicer
		thickSlabReslice->SetBackgroundLevel(GetBackgroundLevel());
		vtkSmartPointer<vtkTransform> transformTrans = vtkSmartPointer<vtkTransform>::New();
		transformTrans->Translate(actorDaImagem->GetPosition());
		transformTrans->Update();
		vtkSmartPointer<vtkTransform> transformRot = vtkSmartPointer<vtkTransform>::New();
		transformRot->RotateWXYZ(cubeActor->GetOrientationWXYZ()[0], cubeActor->GetOrientationWXYZ()[1], cubeActor->GetOrientationWXYZ()[2], cubeActor->GetOrientationWXYZ()[3]);
		transformRot->Inverse();
		transformTrans->Concatenate(transformRot);
		thickSlabReslice->SetResliceTransform(transformTrans);
		switch (tipoInterpolacao) {
		case NearestNeighbour:
			thickSlabReslice->SetInterpolationModeToNearestNeighbor();
			break;
		case Linear:
			thickSlabReslice->SetInterpolationModeToLinear();
			break;
		case Cubic:
			thickSlabReslice->SetInterpolationModeToCubic();
			break;
		}
		switch (tipoFuncao) {
		case MIP:
			thickSlabReslice->SetBlendModeToMax();
			break;
		case MinP:
			thickSlabReslice->SetBlendModeToMin();
			break;
		case Composite:
			thickSlabReslice->SetBlendModeToMean();
			break;
		}
		thickSlabReslice->SetSlabThickness(thickness);
		thickSlabReslice->Update();
		//teste pra ver se tudo está ok. Se der um extent inválido é tem algo errado e
		//tem que abortar
		vtkImageData* resultado = thickSlabReslice->GetOutput();
		assert(resultado->GetExtent()[1] != -1);
		//agora configura o window/Level
		colorMap->SetWindow(window);
		colorMap->SetLevel(level);
		colorMap->Update();
		//debug
		DebugSave();
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
	auto style = vtkSmartPointer<myInteractorStyleTrackballActor>::New();
	style->SetOperacao(0, VTKIS_ROTATE);
	style->SetOperacao(1, VTKIS_SPIN);
	style->SetOperacao(2, VTKIS_PAN);
	renderWindowInteractor->SetInteractorStyle(style);
	//Cria o reslice cubico
	OrientationCubeMK2 *cube = new OrientationCubeMK2();
	cube->SetRenderer(rendererDaCamadaDaImagem);
	cube->SetImage(imagemImportadaPraVTK);
	//passa o cubo pro style
	style->SetOrientator(cube);
	//agora renderiza
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