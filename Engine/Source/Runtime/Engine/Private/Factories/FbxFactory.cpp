#include "Factories/FbxFactory.h"
#include "Engine/StaticMesh.h"

#include "HierarchyNode.h"
#include "Animation.h"

#if !SERVER
#include "Fbx.h"
#include "FbxFactory.h"
#endif

bool UFbxFactory::FactoryCanImport(const FString& Filename)
{
	FString Extension = FPaths::GetExtension(Filename);
	std::transform(Extension.begin(), Extension.end(), Extension.begin(), towlower);
	if (Extension == TEXT("fbx") /*|| Extension == TEXT("obj")*/)
	{
		return true;
	}

	return false;
}

TObjectPtr<UObject> UFbxFactory::FactoryCreateFile(const FName InName, const FString& InFileName, const TCHAR* Params)
{
#if !SERVER
	struct FFbxManagerContext
	{
		FFbxManagerContext(fbxsdk::FbxManager* InFbxManager)
			: Manager(InFbxManager) { }

		~FFbxManagerContext()
		{
			_ASSERT(Manager);
			Manager->Destroy();
			Manager = nullptr;
		}

		fbxsdk::FbxManager* Manager = nullptr;
	};
	FFbxManagerContext Context(fbxsdk::FbxManager::Create());

	fbxsdk::FbxIOSettings* SdkIOSettings = fbxsdk::FbxIOSettings::Create(Context.Manager, IOSROOT);
	Context.Manager->SetIOSettings(SdkIOSettings);
	
	fbxsdk::FbxScene* Scene = LoadFbxScene(Context.Manager, TCHAR_TO_ANSI(InFileName).c_str());
	if (!Scene)
	{
		E_LOG(Error, TEXT("Failed to create Scene."));
		return nullptr;	
	}
#pragma region Check skeletalMesh or StaticMesh

    //bone유무에 따른 static skeletalmesh 설정을 여기서 해야됌.
    //skeletone 보유중인지 확인하는 코드
    //if (Scene->GetRootNode()->GetNodeAttribute()->GetAttributeType() == fbxsdk::FbxNodeAttribute::eSkeleton)
    //{
    //      여기에 skeletalmesh를 생성하도록 ㄱㄱ
    //     
            //TObjectPtr<USkeletalmesh> NewSkeletalmesh;
            //// Mesh 정보를 얻어온다
            //{
            //    TArray<FMeshData> MeshData;
            //    ExtractFbx(Scene->GetRootNode(), MeshData);
            //    //임시용 입니다 나중에 skeletal mesh를 만들면 거기에 마저 세팅을 해주세요
            //    ExtractFbxAnim(Scene->GetRootNode(), MeshData);
            //    Scene->Destroy();

            //    NewSkeletalmesh = NewObject<USkeletalmesh>(nullptr, USkeletalmesh::StaticClass(), InName);
            //    NewSkeletalmesh->Create(MeshData);
            //}
            //return NewSkeletalmesh;
    //}
    //else 
    // {
    //      여기에 staticmesh를 생성하도록 ㄱㄱ
    //}
#pragma endregion
    // 
    //근데 시간이 되긴 할까? 싶어서 일단 냅둠


	TObjectPtr<UStaticMesh> NewStaticMesh;
	// Mesh 정보를 얻어온다
	{
		TArray<FMeshData> MeshData;
		ExtractFbx(Scene->GetRootNode(), MeshData);

        //임시용 입니다 나중에 skeletal mesh를 만들면 거기에 마저 세팅을 해주세요
        // ExtractFbx 내부에서 호출해도 ㄱㅊ은 부분입니다 임시적으로 해당 위치에 놓은겁니다
        // 나중에 ExtractFbx내부에서 호출해서 돌리는게 좋을것 같음
        ExtractFbxAnim(Scene->GetRootNode(), MeshData);



		Scene->Destroy();

		NewStaticMesh = NewObject<UStaticMesh>(nullptr, UStaticMesh::StaticClass(), InName);
		NewStaticMesh->Create(MeshData);
	}
	return NewStaticMesh;
#else
    return nullptr;
#endif
}

UHierarchy* UFbxFactory::Get_HierarchyNode(FString* pNodeName)
{
    auto	iter = find_if(UHierarchyNodes.begin(), UHierarchyNodes.end(), [&](UHierarchy* pNode)
        {
            return pNode->GetName().compare(*pNodeName);
        });

    if (iter == UHierarchyNodes.end())
        return nullptr;

    return *iter;
}

#if !SERVER
fbxsdk::FbxScene* UFbxFactory::LoadFbxScene(fbxsdk::FbxManager* InFbxManager, const char* InFileName)
{
	struct FFbxImporterContext
	{
		FFbxImporterContext(fbxsdk::FbxImporter* InFbxImporter)
			: Importer(InFbxImporter) {
		}

		~FFbxImporterContext()
		{
			_ASSERT(Importer);
			Importer->Destroy();
			Importer = nullptr;
		}

		fbxsdk::FbxImporter* Importer = nullptr;
	};
	FFbxImporterContext Context(fbxsdk::FbxImporter::Create(InFbxManager, "FBX Importer"));

	if (!Context.Importer->Initialize(InFileName, -1, InFbxManager->GetIOSettings()))
	{
		E_LOG(Error, TEXT("UFbxFactory::LoadFbxScene Importer->Initialize failed: {}"), ANSI_TO_TCHAR(Context.Importer->GetStatus().GetErrorString()));
		return nullptr;
	}


	fbxsdk::FbxScene* Scene = fbxsdk::FbxScene::Create(InFbxManager, "FBX Scene");
	if (!Context.Importer->Import(Scene))
	{
		E_LOG(Error, TEXT("UFbxFactory::LoadFbxScene Importer->Import failed: {}"), ANSI_TO_TCHAR(Context.Importer->GetStatus().GetErrorString()));

		Scene->Destroy();
		return nullptr;
	}
	return Scene;
}

void UFbxFactory::ExtractFbx(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData) 
{
    fbxsdk::FbxNodeAttribute* NodeAttribute = InNode->GetNodeAttribute();
    if (NodeAttribute != nullptr) {
        fbxsdk::FbxNodeAttribute::EType AttributeType = NodeAttribute->GetAttributeType();

        if (AttributeType == fbxsdk::FbxNodeAttribute::eMesh) {
            const FString FbxNodeName = ANSI_TO_TCHAR(InNode->GetName());
            E_LOG(Log, TEXT("ExtractFbx Mesh: {}"), FbxNodeName);

            fbxsdk::FbxMesh* Mesh = static_cast<fbxsdk::FbxMesh*>(NodeAttribute);
            _ASSERT(Mesh);
            fbxsdk::FbxLayer* BaseLayer = Mesh->GetLayer(0);
            if (BaseLayer == nullptr) {
                E_LOG(Error, TEXT("BaseLayer is nullptr!"));
                return;
            }

            bool bSuccessed = true;

            const int32 PolygonCount = Mesh->GetPolygonCount();
            const int32 VertexCount = Mesh->GetControlPointsCount();

            // Vertices, Normals 및 UVs 추출
            TArray<FPositionNormalUV> Vertices;
            TArray<uint32> Indices;
            Vertices.reserve(PolygonCount); // 적당히 공간 할당
            std::map<int32, TArray<int32>> VertexMap; // 같은 위치의 정점을 추적하기 위한 맵

            {
                fbxsdk::FbxVector4* FbxVertices = Mesh->GetControlPoints();

                // Position 정보 추출
                for (uint32 i = 0; i < VertexCount; ++i) {
                    FVector3D Position = FVector3D(FbxVertices[i][0], -FbxVertices[i][1], FbxVertices[i][2]);
                    Vertices.emplace_back(Position, FVector3D::Zero);

                    // 같은 위치의 정점을 저장
                    if (!VertexMap.contains(i)) {
                        VertexMap.emplace(i, TArray<int32>());
                    }
                    VertexMap[i].push_back(Vertices.size() - 1);
                }

                if (BaseLayer->GetNormals() == nullptr) {
                    Mesh->InitNormals();
                    Mesh->GenerateNormals();
                }

                fbxsdk::FbxGeometryElementNormal* LayerElementNormal = Mesh->GetElementNormal();
                fbxsdk::FbxLayerElement::EMappingMode NormalMappingMode = LayerElementNormal->GetMappingMode();
                fbxsdk::FbxLayerElement::EReferenceMode NormalReferenceMode = LayerElementNormal->GetReferenceMode();

                fbxsdk::FbxGeometryElementUV* LayerElementUV = Mesh->GetElementUV(0);
                fbxsdk::FbxLayerElement::EMappingMode UVMappingMode = LayerElementUV->GetMappingMode();
                fbxsdk::FbxLayerElement::EReferenceMode UVReferenceMode = LayerElementUV->GetReferenceMode();

                int32 CurrentVertexInstanceIndex = 0;
                for (int32 PolygonIndex = 0; PolygonIndex < PolygonCount; ++PolygonIndex) {
                    const int32 PolygonVertexCount = Mesh->GetPolygonSize(PolygonIndex);
                    if (PolygonVertexCount != 3) {
                        E_LOG(Warning, TEXT("PolygonVertexCount({}) != 3"), PolygonVertexCount);
                        return;
                    }

                    for (int32 CornerIndex = 0; CornerIndex < PolygonVertexCount; ++CornerIndex) {
                        const int32 ControlPointIndex = Mesh->GetPolygonVertex(PolygonIndex, CornerIndex);
                        int32 RealFbxVertexIndex = CurrentVertexInstanceIndex + CornerIndex;

                        int32 NormalMapIndex = (NormalMappingMode == FbxLayerElement::eByControlPoint) ?
                            ControlPointIndex : RealFbxVertexIndex;
                        int32 NormalValueIndex = (NormalReferenceMode == FbxLayerElement::eDirect) ?
                            NormalMapIndex : LayerElementNormal->GetIndexArray().GetAt(NormalMapIndex);

                        FbxVector4 TempValue = LayerElementNormal->GetDirectArray().GetAt(NormalValueIndex);
                        FVector3D Normal = FVector3D(TempValue[0], -TempValue[1], TempValue[2]);

                        // UV 좌표 추출
                        int32 UVMapIndex = (UVMappingMode == FbxLayerElement::eByControlPoint) ?
                            ControlPointIndex : RealFbxVertexIndex;
                        int32 UVIndex = (UVReferenceMode == FbxLayerElement::eDirect) ?
                            UVMapIndex : LayerElementUV->GetIndexArray().GetAt(UVMapIndex);

                        FbxVector2 UVValue = LayerElementUV->GetDirectArray().GetAt(UVIndex);
                        FVector2D UV = FVector2D(UVValue[0], 1.0 - UVValue[1]); // flip the Y of UVs for DirectX

                        // 중복된 위치의 정점을 처리하여 각각의 노멀과 UV를 할당
                        bool bFound = false;
                        for (int32 VertexIndex : VertexMap[ControlPointIndex]) {
                            if (
                                (
                                    Vertices[VertexIndex].Normal == FVector3D::Zero ||
                                    Vertices[VertexIndex].Normal == Normal
                                ) &&
                                (
                                    Vertices[VertexIndex].UV == FVector2D::Zero ||
                                    Vertices[VertexIndex].UV == UV)
                                )
                            {
                                RealFbxVertexIndex = VertexIndex;
                                bFound = true;
                                break;
                            }
                        }

                        // Normal이나 UV가 다르면 신규 정점 생성
                        if (!bFound) {
                            FPositionNormalUV NewVertex = Vertices[ControlPointIndex];
                            NewVertex.Normal = Normal;
                            NewVertex.UV = UV;
                            Vertices.push_back(NewVertex);
                            RealFbxVertexIndex = Vertices.size() - 1;
                            VertexMap[ControlPointIndex].push_back(RealFbxVertexIndex);
                        }

                        Vertices[RealFbxVertexIndex].Normal = Normal;
                        Vertices[RealFbxVertexIndex].UV = UV;

                        Indices.push_back(RealFbxVertexIndex);
                    }
                    CurrentVertexInstanceIndex += PolygonVertexCount;
                }
            }

            if (bSuccessed) {
                FMeshData& NewMeshData = OutMeshData.emplace_back();
                NewMeshData.Name = FbxNodeName;
                NewMeshData.Vertices = move(Vertices);
                NewMeshData.Indices = move(Indices);
                NewMeshData.NumPrimitives = PolygonCount;

                const int MaterialCount = Mesh->GetElementMaterialCount();
                // Material 생성 로직 추가 가능

                //여기에 애니메이션 및 하이어라키 과정을 넣던지 밑에 여기에 있는 과정을 넣어서 한번에 생성하던지 둘중 하나를 할것 같음
                // 한다면 위쪽에 만들어 놓음 bone유무를 가지고 먼저 세팅한다음 t/f 로 구별해서 추가 과정을 해야 하는게 더 나을지도?
                //지금은 일단 고민중
                /* Add ExtractFbxAnim or / add ExtractFbx code into ExtractFbxAnim*/


            }
        }
    }

    for (uint32 i = 0; i < InNode->GetChildCount(); ++i) {
        ExtractFbx(InNode->GetChild(i), OutMeshData);
    }
}

void UFbxFactory::Ready_Animations(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData)
{
    // anim 꺼내올 대상이랑 받을 대상
    // scene 이랑 meshdata 로 꺼내오고 받고 ㄱㄱ
    uint32 OwnAnimStack = InNode->GetScene()->GetSrcObjectCount<fbxsdk::FbxAnimStack>();

    for (uint32 i = 0; i < OwnAnimStack; ++i)
    {
        fbxsdk::FbxAnimStack* pAnimStack = InNode->GetScene()->GetSrcObject<fbxsdk::FbxAnimStack>(i);
        if (!pAnimStack)
            continue;

        //Animation 생성하는 부분
        //둘중에 하나로 만들기
        // UAnimation 만들고 활성화 ㄱㄱ
        UAnimation* pAnimation = UAnimation::Create(pAnimStack, InNode);
        //TObjectPtr<UAnimation> InstanceAnimation = NewObject<UAnimation>(nullptr, StaticClass, UAnimation::StaticClass());
        //LevelToSpawnIn->Actors.push_back(Actor);

        //if (nullptr == pAnimation)
            //return;


        //m_Animations.push_back(pAnimation);
        
    }





}


void UFbxFactory::ExtractFbxAnim(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData)
{
    // Animation들어있는 Fbx가 여러개의 mesh를 가지고 있을 경우 load 오류가 걸림
    // mesh container 만들면 해결되긴함.. 근데 시간이 있을지는 고민임...
    // 일단 animation처리 끝내놓고 임시로 mesh 하나에 skeleton 들고있는 anim mesh fbx를 로드 시키고 나중에 mesh container 만들어서 로드 시키던지 해야할거 같음
    // 위에 ExtractFbx에 넣을지 여기에 마저 처리할지 고민중
    fbxsdk::FbxNodeAttribute* NodeAttribute = InNode->GetNodeAttribute();
    if (NodeAttribute != nullptr)
    {
        fbxsdk::FbxNodeAttribute::EType AttributeType = NodeAttribute->GetAttributeType();

        // Bone 유무 확인후 없으면 그냥 ㄱㄱ 
        if (AttributeType == fbxsdk::FbxNodeAttribute::eSkeleton)
        {
            // Bone 정보 추출
            fbxsdk::FbxSkeleton* Skeleton = static_cast<fbxsdk::FbxSkeleton*>(InNode->GetNodeAttribute());

            //bone의 이름을 가지고 옴
            const FString BoneName = ANSI_TO_TCHAR(Skeleton->GetName());
            E_LOG(Log, TEXT("ExtractFbxAnim BoneName: {}"), BoneName);

            //하이어라키 만들고 거기 부모자식 관계 설정 시키기
            Ready_HierarchyNodes(InNode, nullptr,0);

            //meshcontainer 를 만들어서 여러개의 메쉬를 로드할까 말까 고민중... 일단 하지말기

        }
    }

}
void UFbxFactory::Ready_HierarchyNodes(fbxsdk::FbxNode* InNode, UHierarchy* pParent, uint32 iDepth)
{
    UHierarchy* pHierarchyNode = UHierarchy::Create(InNode, pParent, iDepth++);

    if (pHierarchyNode != nullptr)
    {
        UHierarchyNodes.push_back(pHierarchyNode);
        for (uint32 i = 0; i < InNode->GetChildCount(); ++i)
        {
            //재귀함수로 돌려서 자식 노드 끝가지 만들기
            Ready_HierarchyNodes(InNode->GetChild(i), pHierarchyNode, iDepth);


        }
    }


    

}
#endif