/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.30
Description:				Sakura scene node virtual class.
Details:
*******************************************************************************************/
#pragma once
#include <stdlib.h>
#include <unordered_map>
#include <iostream>
#include "Interface/ISObject.h"
#include "Properties/SakuraNodeProperties.h"

namespace SakuraCore
{
	typedef uint32_t SceneNodeID;

	class SakuraSceneNode : SImplements ISTickObject
	{
	public:
		// ISTickObject base interfaces.
		virtual bool Initialize() = 0;
		virtual void Tick(double deltaTime) = 0;
		// Once as a child, call parent to detach and erase the ref to discard.
		// Root cannot finalize itself.
		virtual void Finalize() override
		{
			if (parentNode != nullptr)
			{
				parentNode->DetachAndFinalize(std::shared_ptr<SakuraSceneNode>(this));
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
		virtual size_t GetID() { return ID; }
		virtual void SetID(size_t id) { ID = id; }
		
		// If a child with same id exists already, return false.
		virtual bool Attach(std::shared_ptr<SakuraSceneNode>&& child)
		{
			SceneNodeID id = child->GetID();
			if (ChildNodesMap.find(id) != ChildNodesMap.end())
			{
				ChildNodesMap[id] = child;
				return true;
			}
			return false;
		}

		// If node not attached to the parent, do nothing and return false. 
		// Detached child node will be attached to parent of this node.
		virtual bool Detach(std::shared_ptr<SakuraSceneNode>&& child)
		{
			SceneNodeID id = child->GetID();
			auto res = ChildNodesMap.erase(id);
			if (res != 0)
			{
				parentNode->Attach(std::move(child));
				return true;
			}
			return (res!=0);
		}

		// Detach child node from this,
		// and then destroy it with all its children. 
		// Return false and do nothing if child is not attached to this.
		virtual bool DetachAndFinalize(std::shared_ptr<SakuraSceneNode>&& child)
		{
			SceneNodeID id = child->GetID();
			// Remove child from Map.
			auto res = ChildNodesMap.erase(id);
			return (res != 0);
		}

		virtual void AttachTo(std::shared_ptr<SakuraSceneNode>&& parent)
		{
			parent->Attach(std::shared_ptr<SakuraSceneNode>(this)); 
		}
		// returns nullptr if not found.
		virtual std::shared_ptr<SakuraSceneNode> GetChild(SceneNodeID id) { return ChildNodesMap[id]; }

	protected:
		// Child node pointer.
		SakuraSceneNode* parentNode = nullptr;
		std::unordered_map<SceneNodeID, std::shared_ptr<SakuraSceneNode>> ChildNodesMap;

		STransform NodeTransform;
		// !!!
		SceneNodeID ID = -1;
	private:
		// Hide the constructors that even the child classes won't be able to spawn itself.
		// Only Entity's friend class SceneNodeFactory would spawn one.
		SakuraSceneNode() = default;
		SakuraSceneNode(SceneNodeID id)
			:ID(id)
		{}
	};
}