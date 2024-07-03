#pragma once
#include "../core/Types.h"
#include <vector>

namespace VKR
{
	namespace render
	{
		struct Renderable
		{
		private://variables
			std::vector<DBG_Vertex3D> m_Vertices;
		public: // functions
			virtual void AddCommand() = 0;
		};
	}
}