#ifndef _PRIMITIVEVISITOR_HPP
#define _PRIMITIVEVISITOR_HPP

#include "Primitive.hpp"
#include "Node.hpp"

namespace OsmFileParser
{
    /**
     * Interface class which implements Visitor pattern for OSM primitives encountered during parsing
     */
    class PrimitiveVisitor
    {
        public:

            /**
             * Virtual destructor ensures child class destructors are called properly
             */
            virtual ~PrimitiveVisitor() { }

            /**
             * Does visitor object wish to have its visit method called for Node primitives?
             *
             * @return True if visit() method should be invoked when Nodes are encountered, else false
             */
            virtual bool shouldVisitNodes() const = 0;

            /**
             * Does visitor object wish to have its visit method called for Way primitives?
             *
             * @return True if visit() method should be invoked when Ways are encountered, else false
             */
            virtual bool shouldVisitWays() const = 0;

            /**
             *  Does visitor object wish to have its visit method called for Relation primitives?
             *
             * @return True if visit() method should be invoked when Relations are encountered, else false
             */
            virtual bool shouldVisitRelations() const = 0;

            /**
             * Does visitor object wish to have its visit method called for Changeset primitives?
             *
             * @return True if visit() method should be invoked when Changesets are encountered, else false
             */
            virtual bool shouldVisitChangesets() const = 0;

            /**
             * Visit method for Nodes
             *
             * @param [in] node Node being visited
             *
             * @note Implementers MUST make this method thread-safe if the ::parse() method is
             *      invoked with number of workers set to be greater than one
             */
            virtual void visit(
                const ::OsmFileParser::OsmPrimitive::Node& node
            ) = 0;

            /**
              * Visit method for Ways
              *
              * @param [in] way Way being visited
              *
              * @note Implementers MUST make this method thread-safe if the ::parse() method is
              *      invoked with number of workers set to be greater than one
              */
            //virtual void visit( const ::OsmFileParser::OsmPrimitive::Way& way ) = 0;

            /**
              * Visit method for Relations
              *
              * @param [in] relation Relation being visited
              *
              * @note Implementers MUST make this method thread-safe if the ::parse() method is
              *      invoked with number of workers set to be greater than one
              */
            //virtual void visit( const ::OsmFileParser::OsmPrimitive::Relation& relation ) = 0;

            /**
              * Visit method for Changesets
              *
              * @param [in] changeset Changeset being visited
              *
              * @note Implementers MUST make this method thread-safe if the ::parse() method is
              *      invoked with number of workers set to be greater than one
              */
            //virtual void visit( const ::OsmFileParser::OsmPrimitive::Changeset& changeset ) = 0;





    };
}

#endif // _PRIMITIVEVISITOR_HPP
