#ifndef _PRIMITIVE
#define _PRIMITIVE

#include <cstdint>

namespace OsmFileParser
{
	namespace OsmPrimitive
	{
		/**
		 * OSM identifiers have been 64-bit signed integers since 2012-ish
		 *
		 * See: http://wiki.openstreetmap.org/wiki/64-bit_Identifiers
		 */
		typedef ::std::int64_t Identifier;

		/**
		 * Base class for OSM primitive entities
		 */
		class Primitive
		{
			public:
				/**
				 * Default constructor, sets all ID values to zero
				 */
				Primitive() : m_primitiveId(0), m_changesetId(0) { }

				/**
				 * Virtual destructor, must be implemented in child classes
				 */
				virtual ~Primitive() { } 

				/**
				 * Get the unique identifier value for the given primititive 
				 *	
				 * @return Unique identifier for this OSM primitive
				 *
				 * @note Only guaranteed to be unique per primitive type
				 */
				virtual ::OsmFileParser::OsmPrimitive::Identifier getPrimitiveId() const 
				{
					return m_primitiveId; 
				}

				/**
				 * Get the changeset ID for this primitive
				 *
				 * @return Changeset ID of the given primitive
				 */
				virtual ::OsmFileParser::OsmPrimitive::Identifier getChangesetId() const 
				{
					return m_changesetId;
				}

			protected:

				/// Primitive ID
				::OsmFileParser::OsmPrimitive::Identifier m_primitiveId;

				/// Changeset ID
				::OsmFileParser::OsmPrimitive::Identifier m_changesetId;

		};
	}
}

#endif // _PRIMITIVE

