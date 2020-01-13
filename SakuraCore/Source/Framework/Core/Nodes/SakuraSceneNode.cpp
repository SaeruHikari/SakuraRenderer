#include "SakuraSceneNode.hpp"

namespace SScene
{
	REFLECTION_EDITOR(
		registration::class_<SakuraSceneNode>("SakuraSceneNode")
		.constructor<>()
		.method("GetParent", &SakuraSceneNode::GetParent)
		.method("GetChildNum", &SakuraSceneNode::GetChildNum)
		.method("GetChild", select_overload<SakuraSceneNode * (size_t)>(&SakuraSceneNode::GetChild))
		.method("GetChild", select_overload<SakuraSceneNode * (SGuid)>(&SakuraSceneNode::GetChild))
		.property("Name", &SakuraSceneNode::GetNodeName, &SakuraSceneNode::SetNodeName)
		.property("Location", &SakuraSceneNode::GetLocation,
			select_overload<void(SakuraMath::SVector)>(&SakuraSceneNode::SetLocation))
		.property("Rotation", &SakuraSceneNode::GetRotation,
			select_overload<void(SakuraMath::SVector)>(&SakuraSceneNode::SetRotation))
		.property("Scale", &SakuraSceneNode::GetScale,
			select_overload<void(SakuraMath::SVector)>(&SakuraSceneNode::SetScale));
	)

	bool SakuraSceneNode::Initialize()
	{
		return true;
	};

	void SakuraSceneNode::Tick(double deltaTime)
	{
		for (size_t i = 0; i < ChildNodes.size(); i++)
		{
			ChildNodes[i]->Tick(deltaTime);
		}
	};

	void SakuraSceneNode::Finalize()
	{
		if (parentNode != nullptr)
		{
			parentNode->DetachAndFinalize(this);
		}
	};

	SakuraCore::STransform SakuraSceneNode::GetTransform()
	{
		return NodeTransform;
	}

	SakuraMath::SVector SakuraSceneNode::GetLocation()
	{
		return NodeTransform.Location;
	}

	SakuraMath::SVector SakuraSceneNode::GetRotation()
	{
		return NodeTransform.Rotation;
	}

	SakuraMath::SVector SakuraSceneNode::GetScale()
	{
		return NodeTransform.Scale;
	}

	SakuraMath::SVector SakuraSceneNode::GetWorldScale()
	{
		return parentNode == nullptr ? NodeTransform.Scale : NodeTransform.Scale * parentNode->GetWorldScale();
	}

	void SakuraSceneNode::SetTransfrom(const STransform trans)
	{
		NodeTransform = trans;
	}

	void SakuraSceneNode::SetTransfrom(STransform&& trans)
	{
		NodeTransform = trans;
	}

	void SakuraSceneNode::SetLocation(float _x, float _y, float _z)
	{
		NodeTransform.Location.x = _x;
		NodeTransform.Location.y = _y;
		NodeTransform.Location.z = _z;
	}

	void SakuraSceneNode::SetLocation(const SakuraMath::SVector loc)
	{
		NodeTransform.Location = loc;
	}

	void SakuraSceneNode::SetLocation(SakuraMath::SVector&& loc)
	{
		NodeTransform.Location = loc;
	}

	void SakuraSceneNode::SetRotation(float _x, float _y, float _z)
	{
		NodeTransform.Rotation.x = _x;
		NodeTransform.Rotation.y = _y;
		NodeTransform.Rotation.z = _z;
	}

	void SakuraSceneNode::SetRotation(SakuraMath::SVector&& rot)
	{
		NodeTransform.Location = rot;
	}

	void SakuraSceneNode::SetRotation(const SakuraMath::SVector rot)
	{
		NodeTransform.Location = rot;
	}

	void SakuraSceneNode::SetScale(float _x, float _y, float _z)
	{
		NodeTransform.Scale.x = _x;
		NodeTransform.Scale.y = _y;
		NodeTransform.Scale.z = _z;
	}

	void SakuraSceneNode::SetScale(SakuraMath::SVector&& scale)
	{
		NodeTransform.Scale = scale;
	}

	void SakuraSceneNode::SetScale(const SakuraMath::SVector scale)
	{
		NodeTransform.Scale = scale;
	}

	SScene::SakuraSceneNode* SakuraSceneNode::GetChild(SGuid id)
	{
		for (size_t i = 0; i < ChildNodes.size(); i++)
		{
			if (ChildNodes[i]->ID == id)
				return ChildNodes[i].get();
		}
		return nullptr;
	}

	SScene::SakuraSceneNode* SakuraSceneNode::GetChild(size_t index)
	{
		return ChildNodes[index].get();
	}

	UINT SakuraSceneNode::GetChildNum()
	{
		return ChildNodes.size();
	}

	SScene::SakuraSceneNode* SakuraSceneNode::GetParent()
	{
		return parentNode;
	}

	bool SakuraSceneNode::Attach(SakuraSceneNode* child)
	{
		SGuid id = child->GetID();
		if (GetChild(id) == nullptr)
		{
			child->parentNode = this;
			ChildNodes.push_back(std::move(std::unique_ptr<SakuraSceneNode>(child)));
			return true;
		}
		return false;
	}

	bool SakuraSceneNode::Detach(SakuraSceneNode* child)
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

	bool SakuraSceneNode::DetachAndFinalize(SakuraSceneNode* child)
	{
		SGuid id = child->GetID();
		for (size_t i = 0; i < ChildNodes.size(); i++)
		{
			if (ChildNodes[i]->ID == id)
			{
				ChildNodes[i].reset();
				ChildNodes.erase(std::remove(std::begin(ChildNodes), std::end(ChildNodes), ChildNodes[i]), ChildNodes.end());
			}
		}
		return true;
	}

	void SakuraSceneNode::AttachTo(SakuraSceneNode* parent)
	{
		parent->Attach(this);
	}

	const std::string& SakuraSceneNode::GetNodeName() const
	{
		return NodeName;
	}

	const void SakuraSceneNode::SetNodeName(const std::string& newName)
	{
		NodeName = newName;
	}

}

