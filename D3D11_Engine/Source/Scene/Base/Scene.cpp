#include "Scene.h"
#include <Manager/SceneManager.h>
#include <D3DCore/D3DRenderer.h>
#include <Manager/HLSLManager.h>
#include <Manager/ResourceManager.h>
#include <D3DCore/D3DSamplerState.h>
#include <typeinfo>
#include <Utility/ImguiHelper.h>
#include <Utility/Console.h>
#include <Utility/SpinLock.h>
#include <GameObject/Base/GameObject.h>
#include <Component/Camera/Camera.h>
#include <D3DCore/D3D11_GameApp.h>
#include <Math/Mathf.h>
#include <GameObject/Base/CameraObject.h>
#include <Core/DXTKInputSystem.h>
#include <algorithm>
#include <ImGuizmo/ImGuizmo.h>
#include <imgui_internal.h>
#include <D3DCore/D3DConstBuffer.h>


Scene::Scene()
{
	constexpr unsigned int ReserveSize = 100000;
	objectList.reserve(ReserveSize);
	dontdestroyonloadList.reserve(ReserveSize);
	d3dRenderer.reserveRenderQueue(ReserveSize);
	Transform::reserveUpdateList(ReserveSize);
}

Scene::~Scene()
{
	hlslManager.ClearSharingShader();
	Resource::ClearResourceManagers();
	D3DConstBuffer::ClearInitFlag();
}

void Scene::FixedUpdate()
{
	for (auto& obj : objectList)
	{
		if (obj && obj->Active)
			obj->FixedUpdate();
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
	d3dRenderer.BegineDraw();
	for (auto& obj : objectList)
	{
		if (obj && obj->Active)
			obj->Render();
	}
	d3dRenderer.EndDraw();
	if (UseImGUI)
	{
		ImGUIBegineDraw();
		ImGuizmoDraw();
		ImGUIRender();
		if (!ImGUIPopupQue.empty())
			ImGUIPopupQue.front()();
		ImGUIEndDraw();
	}
	d3dRenderer.Present();
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
		if (Camera* mainCamera = Camera::GetMainCamera())
		{
			ImGuizmo::BeginFrame();
			D3D11_VIEWPORT& frontView = d3dRenderer.ViewPortsVec.front();
			ImGuizmo::SetRect(frontView.TopLeftX, frontView.TopLeftY, frontView.Width, frontView.Height);

			ImGuiIO& io = ImGui::GetIO();
			ImGuiContext& imGuiContext = *ImGui::GetCurrentContext();
			SIZE clientSize = D3D11_GameApp::GetClientSize();
			const Matrix& cameraVM = mainCamera->GetVM();
			const Matrix& cameraPM = mainCamera->GetPM();

			DXTKInputSystem::InputSystem& Input = DXTKinputSystem.Input;
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
						std::erase_if(list, [](GameObject* object)
							{
								return !object->IsCameraCulling();
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
									GuizmoSetting.SelectObject = Input.IsKey(KeyboardKeys::LeftControl) ? obj : &obj->transform.RootParent->gameObject;
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
				if (Input.IsKeyDown(Keyboard::Keys::Escape))
				{
					ImGuizmo::Enable(false);
					ImGuizmo::Enable(true);
					ImGuizmo::Manipulate(dummyMatrix, dummyMatrix, ImGuizmo::OPERATION(0), ImGuizmo::WORLD, dummyMatrix);
					GuizmoSetting.SelectObject = nullptr;
				}
				else if (GuizmoSetting.SelectObject && Input.IsKeyDown(Keyboard::Keys::Delete))
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
						GuizmoSetting.SelectObject->transform.localPosition = postion;
						GuizmoSetting.SelectObject->transform.localScale = scale;
						if(Mathf::GetAngleDifference(rotation, GuizmoSetting.SelectObject->transform.localRotation) > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.localRotation = rotation;
					}
					else
					{
						GuizmoSetting.SelectObject->transform.position = postion;
						GuizmoSetting.SelectObject->transform.scale	   = scale;
						if (Mathf::GetAngleDifference(rotation, GuizmoSetting.SelectObject->transform.rotation) > Mathf::AngleEpsilon)
							GuizmoSetting.SelectObject->transform.rotation = rotation;			
					}
				}

				//Inspector
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
					}
					ImGui::End();
					ImGui::PopID();
				}
			}	
			
			//Editer
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("Open"))
				{
					{
						if (ImGui::MenuItem("Open GameObject"))
						{
							ImGui::ShowOpenGameObjectPopup();
						}
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Save")) 
				{
					{
						if (ImGui::MenuItem("Save As GameObject"))
						{
							if (GuizmoSetting.SelectObject)
							{
								ImGui::ShowSaveAsGameObjectPopup(GuizmoSetting.SelectObject);
							}						
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
			ImGui::ResetGlobalID();
		}
	}
}

void Scene::ImGUIEndDraw()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGui::ResetGlobalID();
}
