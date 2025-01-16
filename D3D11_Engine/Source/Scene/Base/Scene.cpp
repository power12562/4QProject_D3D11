#include "Scene.h"
#include <Manager/SceneManager.h>
#include <Manager/HLSLManager.h>
#include <Manager/ResourceManager.h>
#include <typeinfo>
#include <Utility/ImguiHelper.h>
#include <Utility/Console.h>
#include <Utility/SpinLock.h>
#include <GameObject/Base/GameObject.h>
#include <Component/Camera/Camera.h>
#include <D3DCore/D3D11_GameApp.h>
#include <Math/Mathf.h>
#include <GameObject/Base/CameraObject.h>
#include <algorithm>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui_internal.h>

#include <Physics/PhysicsManager.h>
#include <Physics/PhysicsScene/PhysicsScene.h>
#include <Physics/PhysicsActor/PhysicsActor.h>


Scene::Scene()
{
	constexpr unsigned int ReserveSize = 100000;
	objectList.reserve(ReserveSize);
	dontdestroyonloadList.reserve(ReserveSize);
	Transform::reserveUpdateList(ReserveSize);
}

Scene::~Scene()
{
	hlslManager.ClearSharingShader();
	Resource::ClearResourceManagers();
}

void Scene::FixedUpdate()
{
	for (auto& obj : objectList)
	{
		if (obj && obj->Active)
			obj->FixedUpdate();
	}
}

void Scene::PhysicsUpdate(float fixed_delta_time)
{
	for (auto& obj : objectList)
	{
		if (obj && obj->Active)
		{
			if (PhysicsActor* actor = obj->GetPhysicsActor(); actor != nullptr)
			{
				actor->PropagateTransform(&obj->transform);
			}
		}
	}

	PhysicsManager::GetInstance().GetPhysicsScene()->Update(fixed_delta_time);
	
	for (auto& obj : objectList)
	{
		if (obj && obj->Active)
		{
			if (PhysicsActor* actor = obj->GetPhysicsActor(); actor != nullptr)
			{
				actor->UpdateTransform(&obj->transform);
			}
		}
	}
}

void Scene::Update()
{
	for (auto& obj : objectList)
	{
		if (obj && obj->CheckActive())
		{
			obj->checkActive = obj->Active;
			if (obj->transform.GetChildCount())
				obj->UpdateChildActive(&obj->transform);
		}

		if (obj && obj->Active)
			obj->Update();
	}
}

void Scene::LateUpdate()
{
	for (auto& obj : objectList)
	{
		if (obj && obj->Active)
			obj->LateUpdate();
	}
}

void Scene::Render()
{	
	Transform::UpdateMatrix();
	Transform::ClearUpdateList();
	if (Camera::GetMainCamera())
	{
		for (auto& obj : objectList)
		{
			if (obj && obj->Active)
			{
				obj->Render();
			}		
		}
		D3D11_GameApp::GetRenderer().Render();
	}
	if (UseImGUI)
	{
		ImGUIBegineDraw();
		ImGuizmoDraw();
		ImGUIRender();
		if (!ImGUIPopupQue.empty())
			ImGUIPopupQue.front()();
		ImGUIEndDraw();
	}
	D3D11_GameApp::Present();
}

void Scene::ImGUIBegineDraw()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Scene::ImGuizmoDraw()
{
	using namespace DirectX::SimpleMath;
	using namespace DirectX;
	if (GuizmoSetting.UseImGuizmo)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiContext& imGuiContext = *ImGui::GetCurrentContext();
		if (Camera* mainCamera = Camera::GetMainCamera())
		{
			ImGuizmo::BeginFrame();
			const SIZE& clientSize = D3D11_GameApp::GetClientSize();
			float width = (float)clientSize.cx;
			float height = (float)clientSize.cy;
			ImGuizmo::SetRect(0, 0, width, height);
			const Matrix& cameraVM = mainCamera->GetVM();
			const Matrix& cameraPM = mainCamera->GetPM();

			InputManager::Input& Input = inputManager.input;
			bool isNotRightClickHELD = !Input.IsKey(MouseKeys::rightButton);
			bool isHoveredWindow = imGuiContext.HoveredWindow != nullptr;
			if (Input.IsKeyDown(MouseKeys::leftButton))
				if (!ImGuizmo::IsOver() && !ImGuizmo::IsUsing() && isNotRightClickHELD && !isHoveredWindow)
				{
					const Mouse::State& state = Input.GetMouseState();
					if (state.positionMode == Mouse::Mode::MODE_ABSOLUTE)
					{
						Ray ray = mainCamera->ScreenPointToRay(state.x, state.y);
						ObjectList list = sceneManager.GetObjectList();
						std::erase_if(list, [](GameObject* object)
							{
								return typeid(CameraObject) == typeid(*object);
							});
						std::sort(list.begin(), list.end(), [&mainCamera](GameObject* a, GameObject* b)
							{
								auto fastDistance = [](const Vector3& p1, const Vector3& p2) {
									Vector3 diff = p1 - p2;
									return diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
									};
								float disA = fastDistance(mainCamera->transform.position, a->transform.position);
								float disB = fastDistance(mainCamera->transform.position, b->transform.position);
								return disA < disB;
							});
						float Dist = 0;
						for (auto& obj : list)
						{
							if (obj->Active && obj->GetOBBToWorld().Intersects(ray.position, ray.direction, Dist))
							{
								if (obj->transform.RootParent)
									GuizmoSetting.SelectObject = Input.IsKey(KeyboardKeys::LeftCtrl) ? obj : &obj->transform.RootParent->gameObject;
								else
									GuizmoSetting.SelectObject = obj;
								break;
							}
						}
					}
				}

			if (isNotRightClickHELD)
			{
				//Button Setting				
				if (Input.IsKeyDown(GuizmoSetting.KeySetting.TRANSLATE))
				{
					GuizmoSetting.operation = ImGuizmo::OPERATION::TRANSLATE;
				}
				else if (Input.IsKeyDown(GuizmoSetting.KeySetting.ROTATE))
				{
					GuizmoSetting.operation = ImGuizmo::OPERATION::ROTATE;
				}
				else if (Input.IsKeyDown(GuizmoSetting.KeySetting.SCALE))
				{
					GuizmoSetting.operation = ImGuizmo::OPERATION::SCALE;
				}
				else if (Input.IsKeyDown(GuizmoSetting.KeySetting.UNIVERSAL))
				{
					GuizmoSetting.operation = ImGuizmo::OPERATION::UNIVERSAL;
				}
				else if (Input.IsKeyDown(GuizmoSetting.KeySetting.MODE))
				{
					GuizmoSetting.mode = (GuizmoSetting.mode != ImGuizmo::MODE::WORLD) ? ImGuizmo::MODE::WORLD : ImGuizmo::MODE::LOCAL;
				}
				
				static float dummyMatrix[16]{};
				if (Input.IsKeyDown(KeyboardKeys::Escape))
				{
					ImGuizmo::Enable(false);
					ImGuizmo::Enable(true);
					ImGuizmo::Manipulate(dummyMatrix, dummyMatrix, ImGuizmo::OPERATION(0), ImGuizmo::WORLD, dummyMatrix);
					GuizmoSetting.SelectObject = nullptr;
				}
				else if (GuizmoSetting.SelectObject && Input.IsKeyDown(KeyboardKeys::Delete))
				{
					if (GuizmoSetting.SelectObject != static_cast<GameObject*>(&mainCamera->gameObject))
					{
						ImGuizmo::Enable(false);
						ImGuizmo::Enable(true);
						ImGuizmo::Manipulate(dummyMatrix, dummyMatrix, ImGuizmo::OPERATION(0), ImGuizmo::WORLD, dummyMatrix);
						GameObject::Destroy(GuizmoSetting.SelectObject);
						GuizmoSetting.SelectObject = nullptr;
					}
				}
			}

			if (GuizmoSetting.SelectObject)
			{
				ImGuizmo::OPERATION operation = (ImGuizmo::OPERATION)GuizmoSetting.operation;
				ImGuizmo::MODE mode = (ImGuizmo::MODE)GuizmoSetting.mode;
				//Draw Guizmo
				{
					const float* cameraView = reinterpret_cast<const float*>(&cameraVM);
					const float* cameraProjection = reinterpret_cast<const float*>(&cameraPM);

					bool isParent = GuizmoSetting.SelectObject->transform.RootParent != nullptr;

					Matrix objMatrix = GuizmoSetting.SelectObject->transform.GetWM();
					
					float* pMatrix = reinterpret_cast<float*>(&objMatrix);
					ImGuizmo::Manipulate(cameraView, cameraProjection, operation, mode, pMatrix);

					Transform* parent = GuizmoSetting.SelectObject->transform.Parent;
					if (parent)
					{
						objMatrix *= parent->GetIWM();
					} 
					
					Vector3 postion, scale;
					Quaternion rotation;
					objMatrix.Decompose(scale, rotation, postion);
					if (parent)
					{
						if((GuizmoSetting.SelectObject->transform.localPosition - postion).Length() > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.localPosition = postion;
						if ((GuizmoSetting.SelectObject->transform.localScale - scale).Length() > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.localScale = scale;
						if(Mathf::GetAngleDifference(rotation, GuizmoSetting.SelectObject->transform.localRotation) > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.localRotation = rotation;
					}
					else
					{
						if ((GuizmoSetting.SelectObject->transform.position - postion).Length() > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.position = postion;
						if ((GuizmoSetting.SelectObject->transform.scale - scale).Length() > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.scale = scale;
						if (Mathf::GetAngleDifference(rotation, GuizmoSetting.SelectObject->transform.rotation) > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.rotation = rotation;			
					}
				}
			}	
		}

		//Editer draw
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Game Object"))
			{
				if (ImGui::MenuItem("Open GameObject"))
				{
					ImGui::ShowOpenGameObjectPopup();
				}
				if (ImGui::MenuItem("Save As GameObject"))
				{
					if (GuizmoSetting.SelectObject)
					{
						ImGui::ShowSaveAsGameObjectPopup(GuizmoSetting.SelectObject);
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Scene"))
			{
				if (ImGui::MenuItem("Load Scene"))
				{
					ImGui::ShowLoadScenePopup();
				}
				if (ImGui::MenuItem("Save As Scene"))
				{
					ImGui::ShowSaveAsScenePopup(this);
				}
				if (ImGui::MenuItem("Add Scene"))
				{
					ImGui::ShowAddScenePopup();
				}
				if (ImGui::MenuItem("Sub Scene"))
				{
					ImGui::ShowSubScenePopup();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		//Inspector
		if(GuizmoSetting.SelectObject)
		{
			constexpr float damp = 15.f;
			static std::string windowName;
			static ImVec2 windowSize(300, 200);
			ImVec2 windowPos(io.DisplaySize.x - windowSize.x - damp, damp);
			ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing); // 창 크기 설정
			ImGui::SetNextWindowPos(windowPos, ImGuiCond_Appearing);   // 위치 설정
			windowName = GuizmoSetting.SelectObject->GetNameToString();
			windowName += "##Inspector";
			ImGui::PushID("Inspector");
			ImGui::Begin(windowName.c_str());
			{
				ImGui::Checkbox("Active", &GuizmoSetting.SelectObject->Active);
				ImGui::EditTransform(GuizmoSetting.SelectObject);
				GuizmoSetting.SelectObject->InspectorImguiDraw();
			}
			ImGui::End();
			ImGui::PopID();
		}
		ImGui::ResetGlobalID();
	}
}

void Scene::ImGUIEndDraw()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGui::ResetGlobalID();
}
