/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.30
Description:				Sakura scene node virtual class.
Details:
*******************************************************************************************/
#pragma once
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "Framework/Interface/ISObject.h"
#include "Properties/SakuraNodeProperties.h"
#include <algorithm>
#include <minwindef.h>

using namespace SakuraCore;

namespace SScene
{
	class SakuraSceneNode : SImplements ISTickObject
	{
	public:
		SakuraSceneNode() = default;
		// ISTickObject base interfaces.
		virtual bool Initialize();
		virtual void Tick(double deltaTime);
		// Once as a child, call parent to detach and erase the ref to discard.
		// Root cannot finalize itself.
		virtual void Finalize() override;
		// Transform operations.
		virtual STransform GetTransform();
		virtual SakuraMath::SVector GetLocation();
		virtual SakuraMath::SVector GetRotation();
		virtual SakuraMath::SVector GetScale();
		virtual SakuraMath::SVector GetWorldScale();
		virtual void SetTransfrom(const STransform trans);
		virtual void SetTransfrom(STransform&& trans);
		virtual void SetLocation(float _x, float _y, float _z);
		virtual void SetLocation(const SakuraMath::SVector loc);
		virtual void SetLocation(SakuraMath::SVector&& loc);
		virtual void SetRotation(float _x, float _y, float _z);
		virtual void SetRotation(const SakuraMath::SVector rot);
		virtual void SetRotation(SakuraMath::SVector&& rot);
		virtual void SetScale(float _x, float _y, float _z);
		virtual void SetScale(const SakuraMath::SVector scale);
		virtual void SetScale(SakuraMath::SVector&& scale);

		// returns nullptr if not found.
		virtual SakuraSceneNode* GetChild(SGuid id);
		virtual SakuraSceneNode* GetChild(size_t index);
		virtual UINT GetChildNum();
		virtual SakuraSceneNode* GetParent();

		// If a child with same id exists already, return false.
		virtual bool Attach(SakuraSceneNode* child);

		// If node not attached to the parent, do nothing and return false. 
		// Detached child node will be attached to parent of this node.
		virtual bool Detach(SakuraSceneNode* child);

		// Detach child node from this,
		// and then destroy it with all its children. 
		// Return false and do nothing if child is not attached to this.
		virtual bool DetachAndFinalize(SakuraSceneNode* child);

		virtual void AttachTo(SakuraSceneNode* parent);

		virtual const std::string& GetNodeName() const;
		virtual const void SetNodeName(const std::string& newName);

		REFLECTION_ENABLE(ISSilentObject)
		virtual char* GetSClassName() override
		{
			return (char*)"SakuraSceneNode";
		};
	protected:
		// Child node pointer.
		std::string NodeName = "EmptyNode";
		SakuraSceneNode* parentNode = nullptr;
		std::vector<std::unique_ptr<SakuraSceneNode>> ChildNodes;
		SakuraCore::STransform NodeTransform;
	};
}