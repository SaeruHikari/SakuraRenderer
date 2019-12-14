#pragma once
#include "Interface/ISObject.h"
#include "Framework/GraphicTypes/GraphicsCommon/GraphicsConfigs.h"

namespace SGraphics
{
	class ISRenderPass;
	class SakuraGraphicsResourceManagerBase;
}

namespace SGraphics
{
	class SFG_PassNode : SImplements SakuraCore::ISSilentObject
	{
	public:
		SFG_PassNode()
			:ISSilentObject()
		{
		}
		template<typename PassClass, typename... Params,
			typename std::enable_if<std::is_convertible<PassClass*, decltype(PassTokenPtr)>::value>::type * = nullptr>
		__forceinline void Create(Params... params)
		{
			mPass = std::make_unique<PassClass>(params...);
		}
		__forceinline auto GetPass()
		{
			return (decltype(PassTokenPtr))(mPass.get());
		}
		template<typename... Params>
		auto Before(Params... params)
		{
			
		}
		template<typename... Params>
		auto After(Params... params)
		{

		}
	private:
		std::string mName;
		std::vector<std::string> mEdges;
		std::vector<std::string> InputResources;
		std::vector<std::string> OutputResources;
		std::unique_ptr<SGraphics::ISRenderPass> mPass;
	};
}