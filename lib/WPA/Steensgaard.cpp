/*
 * Steensgaard.cpp
 *
 *  Created on: 2 Feb. 2021
 *      Author: Yulei Sui
 */

#include "WPA/Steensgaard.h"

using namespace SVF;
using namespace SVFUtil;

static llvm::cl::opt<string> WriteSteens("write-steens",  llvm::cl::init(""),
                                        llvm::cl::desc("Write Steensgaard's analysis results to a file"));
static llvm::cl::opt<string> ReadSteens("read-steens",  llvm::cl::init(""),
                                       llvm::cl::desc("Read Steensgaard's analysis results from a file"));

/*!
 * Steensgaard analysis
 */

void Steensgaard::solveWorklist(){

    processAllAddr();

    // Keep solving until workList is empty.
    while (!isWorklistEmpty())
    {
        NodeID nodeId = popFromWorklist();
        ConstraintNode* node = consCG->getConstraintNode(nodeId);

        /// foreach ptd \in pts(p)
        for(NodeID ptd : getPts(nodeId)){

            /// *p = q : EC(o) == EC(q)
            for (ConstraintEdge* edge : node->getStoreInEdges()){
                ecUnion(edge->getSrcID(),ptd);
            }
            // r = *p : EC(r) == EC(o)
            for (ConstraintEdge* edge : node->getLoadOutEdges()){
                ecUnion(ptd,edge->getDstID());
            }
        }

        /// q = p : EC(q) == EC(p)
        for (ConstraintEdge* edge : node->getCopyOutEdges()){
            ecUnion(edge->getSrcID(),edge->getDstID());
        }
        /// q = &p->f : EC(q) == EC(p)
        for (ConstraintEdge* edge : node->getGepOutEdges()){
            ecUnion(edge->getSrcID(),edge->getDstID());
        }
    }
}

/// merge node into equiv class and merge node's pts into ec's pts
void Steensgaard::ecUnion(NodeID node, NodeID ec){
    unionPts(ec, node);
    setEC(node,ec);
}

/*!
 * Process address edges
 */
void Steensgaard::processAllAddr()
{
    for (ConstraintGraph::const_iterator nodeIt = consCG->begin(), nodeEit = consCG->end(); nodeIt != nodeEit; nodeIt++)
    {
        ConstraintNode * cgNode = nodeIt->second;
        for (ConstraintNode::const_iterator it = cgNode->incomingAddrsBegin(), eit = cgNode->incomingAddrsEnd();
                it != eit; ++it){
            numOfProcessedAddr++;

            const AddrCGEdge* addr = cast<AddrCGEdge>(*it);
            NodeID dst = addr->getDstID();
            NodeID src = addr->getSrcID();
            if(addPts(dst,src))
                pushIntoWorklist(dst);
        }
    }
}
