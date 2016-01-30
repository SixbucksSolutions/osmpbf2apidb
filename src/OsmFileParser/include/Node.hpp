#ifndef _NODE
#define _NODE

#include "Primitive.hpp"

namespace OsmFileParser
{
	namespace OsmPrimitive
	{
		class Node : public ::OsmFileParser::OsmPrimitive::Primitive
		{
			public:
				
				Node( 
					const ::OsmFileParser::OsmPrimitive::Identifier nodeId
				)
				{ m_primitiveId = nodeId; }

				virtual ~Node() { }
		};
	}
}

#endif // _NODE

