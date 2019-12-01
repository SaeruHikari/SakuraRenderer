#pragma once
#include <concepts>
#include <iostream>

namespace SECS
{
	struct SComponent {
		SComponent() {
#if defined(DEBUG) || defined(_DEBUG)
			std::cout << "Data Com: ";
#endif
		}

		virtual ~SComponent() {

		}
	};
}

