#ifndef _OSMENTITYPRIMITIVE_H
#define _OSMENTITYPRIMITIVE_H

namespace osmpbf2apidb
{
	class OsmEntityPrimitive
	{
		public:
			OsmEntityPrimitive() { }

			virtual ~OsmEntityPrimitive() { } 

			virtual void writeDataToDbTableFiles() = 0;
	};
}

#endif // _OSMENTITYPRIMITIVE_H
