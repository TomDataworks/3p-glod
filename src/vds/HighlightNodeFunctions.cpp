/******************************************************************************
 * Copyright 2004 David Luebke, Brenden Schubert                              *
 *                University of Virginia                                      *
 ******************************************************************************
 * This file is distributed as part of the VDSlib library, and, as such,      *
 * falls under the terms of the VDSlib public license. VDSlib is distributed  *
 * without any warranty, implied or otherwise. See the VDSlib license for     *
 * more details.                                                              *
 *                                                                            *
 * You should have recieved a copy of the VDSlib Open-Source License with     *
 * this copy of VDSlib; if not, please visit the VDSlib web page,             *
 * http://vdslib.virginia.edu/license for more information.                   *
 ******************************************************************************/
void Cut::HighlightFirstLiveTri()
{
	miHighlightedTri = mpNodeRefs[miHighlightedNode]->miFirstLiveTri;
	if (miHighlightedTri != 0)
		PrintHighlightedTriInfo();
}

void Cut::HighlightNextLiveTri()
{
	// not implemented yet
}

void Cut::PrintHighlightedTriInfo()
{
	if (miHighlightedTri != 0)
	{
		cout << "***Highlighted Tri: " << miHighlightedTri << endl;
	}

	// print tri's corners, proxies, patchID

}

void Cut::PrintHighlightedNodeInfo()
{
	cout << "**Highlighted Node: " << miHighlightedNode << endl;
	cout << "\tCoincident Nodes:" << flush;
	NodeIndex node = mpForest->mpNodes[miHighlightedNode].mCoincidentVertex;
	while ((node != Forest::iNIL_NODE) && (node != miHighlightedNode))
	{
		cout << " " << node;
		node = mpForest->mpNodes[node].mCoincidentVertex;
	}
	cout << endl;
	cout << "\tParent: " << mpForest->mpNodes[miHighlightedNode].miParent << endl;
	cout << "\tFirst Child: " << mpForest->mpNodes[miHighlightedNode].miFirstChild << endl;
	cout << "\tLeft Sibling: " << mpForest->mpNodes[miHighlightedNode].miLeftSibling << endl;
	cout << "\tRight Sibling: " << mpForest->mpNodes[miHighlightedNode].miRightSibling << endl;
	cout << "\tPosition: (" << mpForest->mpNodes[miHighlightedNode].mpRenderData->Position.X << ", "
		<< mpForest->mpNodes[miHighlightedNode].mpRenderData->Position.Y << ", "
		<< mpForest->mpNodes[miHighlightedNode].mpRenderData->Position.Z << ")" << endl;
}

void Cut::PrintHighlightedNodeStructure()
{
	cout << endl;
	mpForest->PrintNodeInfo(miHighlightedNode, this, 0);
}

void Cut::UnhighlightNode()
{
	miHighlightedNode = 0;
}

void Cut::HighlightRootNode()
{
	miHighlightedNode = Forest::iROOT_NODE;
	PrintHighlightedNodeInfo();
}

void Cut::HighlightParent()
{
	if (miHighlightedNode != 0)
	{
		if (mpForest->mpNodes[miHighlightedNode].GetParent() >= Forest::iROOT_NODE)
			miHighlightedNode = mpForest->mpNodes[miHighlightedNode].GetParent();
		PrintHighlightedNodeInfo();
	}
}

void Cut::HighlightFirstChild()
{
	if (miHighlightedNode != 0)
	{
		if (mpForest->mpNodes[miHighlightedNode].miFirstChild != Forest::iNIL_NODE)
		{
			if (mpNodeRefs[mpForest->mpNodes[miHighlightedNode].miFirstChild] != NULL)
			{
				miHighlightedNode = mpForest->mpNodes[miHighlightedNode].miFirstChild;
				PrintHighlightedNodeInfo();
			}
		}
	}
}

void Cut::HighlightRightSibling()
{
	if (miHighlightedNode != 0)
	{
		if (mpForest->mpNodes[miHighlightedNode].miRightSibling != Forest::iNIL_NODE)
		{
			if (mpNodeRefs[mpForest->mpNodes[miHighlightedNode].miRightSibling] != NULL)
			{
				miHighlightedNode = mpForest->mpNodes[miHighlightedNode].miRightSibling;
				PrintHighlightedNodeInfo();
			}
		}
	}
}

void Cut::HighlightLeftSibling()
{
	if (miHighlightedNode != 0)
	{
		if (mpForest->mpNodes[miHighlightedNode].miLeftSibling != Forest::iNIL_NODE)
		{
			if (mpNodeRefs[mpForest->mpNodes[miHighlightedNode].miLeftSibling] != NULL)
			{
				miHighlightedNode = mpForest->mpNodes[miHighlightedNode].miLeftSibling;
				PrintHighlightedNodeInfo();
			}
		}
	}
}

void Cut::FoldHighlightedNode()
{
	unsigned int NumTris, BytesUsed;
	//	CheckTriPointers();

	if (miHighlightedNode != 0)
	{
		mpSimplifier->Fold(mpNodeRefs[miHighlightedNode], NumTris, BytesUsed);
	}
}

void Cut::FullyFoldHighlightedNode()
{
	unsigned int NumTris, BytesUsed;
	//	CheckTriPointers();
	
	if (miHighlightedNode != 0)
	{
		FullyFoldNode(miHighlightedNode, NumTris, BytesUsed);
	}
}

void Cut::FullyFoldNode(NodeIndex node, unsigned int &NumTris, unsigned int &BytesUsed)
{
	NodeIndex child;
	child = mpForest->mpNodes[node].miFirstChild;
	
	while (child != Forest::iNIL_NODE)
	{
		FullyFoldNode(child, NumTris, BytesUsed);
		child = mpForest->mpNodes[child].miRightSibling;
	}
	if (mpForest->mpNodes[node].miFirstChild != Forest::iNIL_NODE)
		mpSimplifier->Fold(mpNodeRefs[node], NumTris, BytesUsed);
}

void Cut::FullyUnfoldHighlightedNode()
{
	unsigned int NumTris, BytesUsed;
	//	CheckTriPointers();
	
	if (miHighlightedNode != 0)
	{
		FullyUnfoldNode(miHighlightedNode, NumTris, BytesUsed);
	}
}

void Cut::FullyUnfoldNode(NodeIndex node, unsigned int &NumTris, unsigned int &BytesUsed)
{
	NodeIndex child;
	if (mpForest->mpNodes[node].miFirstChild != Forest::iNIL_NODE)
		mpSimplifier->Unfold(mpNodeRefs[node], NumTris, BytesUsed);

	child = mpForest->mpNodes[node].miFirstChild;
	
	while (child != Forest::iNIL_NODE)
	{
		FullyUnfoldNode(child, NumTris, BytesUsed);
		child = mpForest->mpNodes[child].miRightSibling;
	}
}

void Cut::UnfoldHighlightedNode()
{
	unsigned int NumTris, BytesUsed;
	mpSimplifier->Unfold(mpNodeRefs[miHighlightedNode], NumTris, BytesUsed);
}

