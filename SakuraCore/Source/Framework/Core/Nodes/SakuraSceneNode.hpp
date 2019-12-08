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
#include "Interface/ISObject.h"
#include "Properties/SakuraNodeProperties.h"
#include <algorithm>

using namespace SakuraCore;

namespace SScene
{
	class SakuraSceneNode : SImplements ISTickObject
	{
	public:
		SakuraSceneNode() = default;
		// ISTickObject base interfaces.
		virtual bool Initialize()
		{
			return true;
		};
		virtual void Tick(double deltaTime)
		{
			for (size_t i = 0; i < ChildNodes.size(); i++)
			{
				ChildNodes[i]->Tick(deltaTime);
			}
		};
		// Once as a child, call parent to detach and erase the ref to discard.
		// Root cannot finalize itself.
		virtual void Finalize() override
		{
			if (parentNode != nullptr)
			{
				parentNode->DetachAndFinalize(this);
			}
		};
		// Transform operations.
		virtual STransform GetTransform() { return NodeTransform; }
		virtual XMFLOAT3 GetLocation() { return NodeTransform.Location; }
		virtual XMFLOAT3 GetRotation() { return NodeTransform.Rotation; }
		virtual XMFLOAT3 GetScale() { return NodeTransform.Scale; }
		virtual void SetTransfrom(const STransform& trans) { NodeTransform = trans; }
		virtual void SetTransfrom(const STransform&& trans) { NodeTransform = trans; }
		virtual void SetLocation(float _x, float _y, float _z) 
		{ 
			NodeTransform.Location.x = _x;
			NodeTransform.Location.y = _y;
			NodeTransform.Location.z = _z;
		}
		virtual void SetLocation(const XMFLOAT3& loc) { NodeTransform.Location = loc; }
		virtual void SetLocation(const XMFLOAT3&& loc) { NodeTransform.Location = loc; }
		virtual void SetRotation(float _x, float _y, float _z)
		{
			NodeTransform.Rotation.x = _x;
			NodeTransform.Rotation.y = _y;
			NodeTransform.Rotation.z = _z;
		}
		virtual void SetRotation(const XMFLOAT3& rot) { NodeTransform.Location = rot; }
		virtual void SetRotation(const XMFLOAT3&& rot) { NodeTransform.Location = rot; }
		virtual void SetScale(float _x, float _y, float _z)
		{
			NodeTransform.Scale.x = _x;
			NodeTransform.Scale.y = _y;
			NodeTransform.Scale.z = _z;
		}
		virtual void SetScale(const XMFLOAT3& scale) { NodeTransform.Scale = scale; }
		virtual void SetScale(const XMFLOAT3&& scale) { NodeTransform.Scale = scale; }

		// returns nullptr if not found.
		virtual std::shared_ptr<SakuraSceneNode> GetChild(SGuid id) 
		{ 
			for (size_t i = 0; i < ChildNodes.size(); i++)
			{
				if (ChildNodes[i]->ID == id)
					return ChildNodes[i];
			}
			return nullptr;
		}
		virtual std::shared_ptr<SakuraSceneNode> GetChild(size_t index)
		{
			return ChildNodes[index];
		}
		virtual size_t GetChildNum() { return ChildNodes.size(); }
		virtual SakuraSceneNode* GetParent() { return parentNode; }

		// If a child with same id exists already, return false.
		virtual bool Attach(std::shared_ptr<SakuraSceneNode> child)
		{
			SGuid id = child->GetID();
			if (GetChild(id) == nullptr)
			{
				ChildNodes.push_back(child);
				return true;
			}
			return false;
		}

		// If node not attached to the parent, do nothing and return false. 
		// Detached child node will be attached to parent of this node.
		virtual bool Detach(std::shared_ptr<SakuraSceneNode> child)
		{
			SGuid id = child->GetID();
			bool res = (GetChild(id) != nullptr);
			if (res != false)
			{
				parentNode->Attach(std::move(child));
				return true;
			}
			return (res != 0);
		}

		// Detach child node from this,
		// and then destroy it with all its children. 
		// Return false and do nothing if child is not attached to this.
		virtual bool DetachAndFinalize(SakuraSceneNode* child)
		{
			SGuid id = child->GetID();
			auto res = GetChild(id);
			if (res == nullptr) return false;
			// Remove child from Map.
			ChildNodes.erase(std::remove(std::begin(ChildNodes), std::end(ChildNodes), res), ChildNodes.end());
			return true;
		}

		virtual void AttachTo(std::shared_ptr<SakuraSceneNode> parent)
		{
			parent->Attach(std::shared_ptr<SakuraSceneNode>(this));
		}
	protected:
		// Child node pointer.
		SakuraSceneNode* parentNode = nullptr;
		std::vector<std::shared_ptr<SakuraSceneNode>> ChildNodes;
		STransform NodeTransform;
	};
}