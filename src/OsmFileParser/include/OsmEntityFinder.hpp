#ifndef _OSMENTITYFINDER
#define _OSMENTITYFINDER

namespace OsmFileParser
{
	class OsmEntityFinder
	{
		public:
			void findEntities(
				const ::std::function< void() >& nodeCallback 
				
			);


	};
}

#endif // _OSMENTITYFINDER
