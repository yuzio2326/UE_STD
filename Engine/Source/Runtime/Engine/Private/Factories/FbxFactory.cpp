#include "Factories/FbxFactory.h"
#include "Engine/StaticMesh.h"

#include "HierarchyNode.h"
#if !SERVER
#include "Fbx.h"
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

    /*TODO:: skeltalmesh만들고 하나만 로드하도록 하고 할당 해제 하도록 ㄱㄱ
    * 이게 정배긴함 그러면 이걸 이렇게 만들고 나면? 
    */


	TObjectPtr<UStaticMesh> NewStaticMesh;
	// Mesh 정보를 얻어온다
	{
		TArray<FMeshData> MeshData;
		//ExtractFbx(Scene->GetRootNode(), MeshData);
        //임시용 입니다 나중에 skeletal mesh를 만들면 거기에 마저 세팅을 해주세요
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

UHierarchy* UFbxFactory::Get_HierarchyNode(FString pNodeName)
{
    auto	iter = find_if(UHierarchyNodes.begin(), UHierarchyNodes.end(), [&](UHierarchy* pNode)
        {
            if (pNodeName == pNode->Get_Name()) { return true; }

            //return !strcmp(pNodeName, pNode->Get_Name());
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
            }
        }
    }

    for (uint32 i = 0; i < InNode->GetChildCount(); ++i) {
        ExtractFbx(InNode->GetChild(i), OutMeshData);
    }
}
void UFbxFactory::ExtractFbxAnim(fbxsdk::FbxNode* InNode, TArray<FMeshData>& OutMeshData)
{
    // Animation들어있는 Fbx가 여러개의 mesh를 가지고 있을 경우 load 오류가 걸림
   // mesh container 만들면 해결되긴함.. 근데 시간이 있을지는 고민임...

   //ExtractFbx 부분 가지고와서 사용
    fbxsdk::FbxNodeAttribute* NodeAttribute = InNode->GetNodeAttribute();
    if (NodeAttribute != nullptr) {
        fbxsdk::FbxNodeAttribute::EType AttributeType = NodeAttribute->GetAttributeType();

        

        //skeleton
        if (AttributeType == fbxsdk::FbxNodeAttribute::eSkeleton)
        {
            // 일단 animation처리 끝내놓고 임시로 mesh 하나에 skeleton 들고있는 anim mesh fbx를 로드 시키고 나중에 mesh container 만들어서 로드 시키던지 해야할거 같음
               // 위에 ExtractFbx에 넣을지 여기에 마저 처리할지 고민중

               // skeleton 유무 확인 이후 Create skeleton 및 anima setting
            if (m_SetBones == false)
            {
                //UHierarchyNodes.clear();
                m_SetBones = true;
                // Bone 정보 추출
                fbxsdk::FbxSkeleton* Skeleton = static_cast<fbxsdk::FbxSkeleton*>(InNode->GetNodeAttribute());

                //bone의 이름을 가지고 옴R
                const FString BoneName = ANSI_TO_TCHAR(Skeleton->GetName());
                E_LOG(Log, TEXT("ExtractFbxAnim BoneName: {}"), BoneName);

                //하이어라키 만들고 거기 부모자식 관계 설정 시키기
                uint32  iDepth = 0;
                //UHierarchy* pHierarchyNode = UHierarchy::Create(InNode, nullptr, iDepth++);

                //뼈세팅
                //만드는거 OK 됐음
                
                Ready_HierarchyNodes(InNode, nullptr, iDepth);

                //확인용   playanim이랑 getHierarchy할때 사용
                UHierarchyNodes;

                //NewMeshData

                //meshcontainer 를 만들어서 여러개의 메쉬를 로드할까 말까 고민중... 일단 하지말기
                //NewMeshData;
                
                //setup을 어떻게 하지?        NewMeshData->Bones
                //SetUp_HierarchyNodes(InNode, NewMeshData);

                //Ready_Animations(InNode, NewMeshData);    mesh에 해당 함수 만드는 작업 ㄱㄱ
            }
        }

        //mesh
        if (AttributeType == fbxsdk::FbxNodeAttribute::eMesh) {
            string tmp = InNode->GetName();
            const FString FbxNodeName = FString(tmp.begin(), tmp.end());
            //E_LOG(Log, TEXT("ExtractFbx Mesh: {}"), FbxNodeName);

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

                //vertex index buffer
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

            //시점이 mesh 만들때 만들어져야함
            if (bSuccessed) {
                FMeshData& NewMeshData = OutMeshData.emplace_back();
                NewMeshData.Name = FbxNodeName;
                NewMeshData.Vertices = move(Vertices);
                NewMeshData.Indices = move(Indices);
                NewMeshData.NumPrimitives = PolygonCount;


                UHierarchyNodes;


                if (!UHierarchyNodes.empty())
                {
                    //bones 세팅 시점은 이쪽에
                    //NewMeshData.Bones = UHierarchyNodes;
                    //NewMeshData.BoneNumber = UHierarchyNodes.size();
                    SetUp_HierarchyNodes(InNode, NewMeshData);  //matrix 등등 필요한것들이 있어서 이렇게 세팅
                    NewMeshData.BoneNumber = UHierarchyNodes.size(); //뼈 갯수는 사이즈만큼 있으니까 

                    //delete UHierarchyNodes;

                    for (auto& pHierarchyNode : UHierarchyNodes)
                        pHierarchyNode = nullptr;
                    //UHierarchyNodes.resize(0);//clear로 하면 오류가 나니까 

                    //UHierarchyNodes.clear();    //다 사용한거 clear 나중에 알아서 clear함 굳이 할필요 없음


                    m_SetBones = false;         //bone setting끝났으니 false로 반환

                }




                //위에 있는 내용물 가지고 메쉬랑 bones 연결해주기
                /*여기에 setup HierarchyNodes 를 해주는게 ㄱㅊ을거 같음*/

                //이후 애니메이션 데이터를 여기에서 세팅

                const int MaterialCount = Mesh->GetElementMaterialCount();

                //여기에 애니메이션 및 하이어라키 과정을 넣던지 밑에 여기에 있는 과정을 넣어서 한번에 생성하던지 둘중 하나를 할것 같음
                // 한다면 위쪽에 만들어 놓음 bone유무를 가지고 먼저 세팅한다음 t/f 로 구별해서 추가 과정을 해야 하는게 더 나을지도?
                //지금은 일단 고민중
                /* Add ExtractFbxAnim or / add ExtractFbx code into ExtractFbxAnim*/

                // anim 준비 과정을 한 이쯤에서 재생 시키도록 해야하구
                //Ready_Animations();

                //playAnim을 계속 호출할 skeletal mesh가 필요함
            }

            
        }

    }

    //해당 부분 때문에 재귀함수에 2번 들어옴
    uint32 NodeChild = InNode->GetChildCount();
    for (uint32 i = 0; i < NodeChild; ++i)
    {
        ExtractFbxAnim(InNode->GetChild(i), OutMeshData);
    }



}
void UFbxFactory::Ready_HierarchyNodes(fbxsdk::FbxNode* InNode, UHierarchy* pParent, uint32 iDepth)
{

    //여기에서 GetChildCount로 자식 하나하나마다 다 UHierarchy를 만들어 줬었어가지고 나중에 csene에서 destroy 할때
    //해당 부분에 남아있는 UHierarchy때문에 destroy 가 안됌...

    //uint32 ChildHierarchyNode = InNode->GetChildCount();
    //UFbxFactory가 중복이라 이전에 만들어 놓았던 UHierarchyNodes를 가지고와서  pushback하고 있음

    if (!InNode)
        return;
    
    UHierarchy* pCreateHierarchyNode = UHierarchy::Create(InNode, pParent, iDepth++);
    if (nullptr == pCreateHierarchyNode)
        return;

    FString HierarchyName = pCreateHierarchyNode->Get_Name();
    
    //for (auto& pHierarchyNode : UHierarchyNodes)
    //{
    //    if (pHierarchyNode == nullptr)
    //    {
    //        //여기에 넣을까 고민중..
    //    }
    //}

    
    

    UHierarchyNodes.push_back(pCreateHierarchyNode);

    uint32 childCount = InNode->GetChildCount();

    //있으면 찾기
    if (pCreateHierarchyNode != nullptr)
    {
     
        for (uint32 i = 0; i < childCount; ++i)
        {
            //재귀함수로 돌려서 자식 노드 끝가지 만들기
            Ready_HierarchyNodes(InNode->GetChild(i), pCreateHierarchyNode, iDepth);
        }

    }

}
void UFbxFactory::SetUp_HierarchyNodes(fbxsdk::FbxNode* InNode, FMeshData& MeshData)
{
    //mesh 를 가지고 와서 어차피 메쉬 하나만 쓸거니까 InNode에서 가지고 와도 될거 같음
    //자기 자신 로드해서 위에 만들어 놓은 하이어라키 사용 ㄱㄱ
    // 
    //UFbxFactory의 Get_HierarchyNode를 사용해서 BonesName를 세팅하고 mesh data의 Bones에 pushback을 하면 되지 않을까?
    //근데 그냥 바로 생성 하자마자 pushback을 하는게 더 나은거 같기도함 위에 FMeshData에 집어 넣고 여기는 Set_OffsetMatrix관련만 가지고 세팅하면 ㄱㅊ을듯?

    fbxsdk::FbxNodeAttribute* NodeAttribute = InNode->GetNodeAttribute();
    if (NodeAttribute != nullptr) {
        fbxsdk::FbxNodeAttribute::EType AttributeType = NodeAttribute->GetAttributeType();
        if (AttributeType == fbxsdk::FbxNodeAttribute::eMesh) {
            const FString FbxNodeName = ANSI_TO_TCHAR(InNode->GetName());
            FbxMesh* Mesh = InNode->GetMesh();
            if (!Mesh) return;
            FbxSkin* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(0, FbxDeformer::eSkin));
            if (!Skin) return;

            uint32 OwnBoneNum = UHierarchyNodes.size();

            for (uint32 i = 0; i < OwnBoneNum; ++i)
            {

                FbxCluster* Cluster = Skin->GetCluster(i);
                FbxNode* BoneNode = Cluster->GetLink();
                string temp = BoneNode->GetName();
                FString BoneName = FString(temp.begin(), temp.end());
                UHierarchy* pHierarchyNode = Get_HierarchyNode(BoneName);

                // Offset Matrix 변환     FBX는 변환 행렬을 직접 가져와야 함  ㅈㄴ 귀찮네
                FbxAMatrix fbxOffsetMatrix;
                
                FbxAMatrix ClusterOffsetMatrix= Cluster->GetTransformLinkMatrix(fbxOffsetMatrix);

                // FBX 행렬을 Unreal FMatrix로 변환
                FMatrix OffsetMatrix;
                for (int r = 0; r < 4; r++)
                    for (int c = 0; c < 4; c++)
                        OffsetMatrix.m[r][c] = static_cast<float>(ClusterOffsetMatrix[r][c]);
                
                //뼈 세팅한뒤 매쉬에도 세팅
                pHierarchyNode->Set_OffsetMatrix(XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));
                MeshData.Bones.push_back(pHierarchyNode);
                FMatrix FCheckMatrix = MeshData.Bones[i]->Get_OffSetMatrix();

            }

            if (0 == OwnBoneNum)
            {
                UHierarchy* pNode = Get_HierarchyNode(MeshData.BonesName[MeshData.BoneNumber]);
                
                if (nullptr == pNode)
                    return ;

                MeshData.BoneNumber = 1;

                MeshData.Bones.push_back(pNode);

            }
            

        }
    }
    
    //For Check
    //MeshData.Bones = UHierarchyNodes;
    //MeshData.BoneNumber = UHierarchyNodes.size();


    //FbxMesh* Mesh = InNode->GetMesh();
    //if (!Mesh) return;

    //FbxSkin* Skin = static_cast<FbxSkin*>(Mesh->GetDeformer(0, FbxDeformer::eSkin));
    //if (!Skin) return;

#pragma region bone_setting_oldcodes

    // 메시에 영향을 주는 뼈들을 순회 하고
    //for (uint32 i = 0; i < OwnBoneNum; ++i)
    //{
    //    //뼈를 가지고오고
    //    //aiBone* pAIBone = MeshData.Bones[i];
    //    UHierarchy* Meshbone = MeshData.Bones[i];

    //    //뼈이름 가져오기
    //    UHierarchy* pHierarchyNode = Get_HierarchyNode(Meshbone->Get_Name());

    //    //FbxCluster* Cluster = Skin->GetCluster(i);
    //    //FbxNode* BoneNode = Cluster->GetLink();
    //    //const char* BoneName = BoneNode->GetName();

    //    //UHierarchy* pHierarchyNode = Get_HierarchyNode(BoneName);


    //    //FMatrix			OffsetMatrix;
    //    //memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(FMatrix));

    //    // Offset Matrix 변환     FBX는 변환 행렬을 직접 가져와야 함  ㅈㄴ 귀찮네
    //    FbxAMatrix fbxOffsetMatrix;
    //    //Cluster->GetTransformLinkMatrix(fbxOffsetMatrix);

    //    // FBX 행렬을 Unreal FMatrix로 변환
    //    FMatrix OffsetMatrix;
    //    for (int r = 0; r < 4; r++)
    //        for (int c = 0; c < 4; c++)
    //            OffsetMatrix.m[r][c] = static_cast<float>(fbxOffsetMatrix[r][c]);


    //    //뼈 세팅한뒤 매쉬에도 세팅
    //    pHierarchyNode->Set_OffsetMatrix(XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));

    //    //MeshData.Bones.push_back(pHierarchyNode);

    //}

    //if (0 == OwnBoneNum)
    //{

    //    UHierarchy* pNode = Get_HierarchyNode(MeshData.BonesName[MeshData.BoneNumber]);
    //    if (nullptr == pNode)
    //        return;

    //    OwnBoneNum = 1;

    //    MeshData.Bones.push_back(pNode);
    //}
#pragma endregion

}

void UFbxFactory::Ready_Animations(fbxsdk::FbxNode* InNode, FMeshData MeshData)
{
    //임시 설정 X상태입니다


}

#endif