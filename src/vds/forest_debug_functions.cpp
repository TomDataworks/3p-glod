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
void Forest::PrintForestInfo(Cut *pCut)
{
	unsigned int i;
	cout << endl;
	PrintNodeInfo(iROOT_NODE, pCut, 0);
	PrintTriInfo(pCut);

	cout << endl;
	for (i = 1; i <= mNumNodes; ++i)
	{
		cout << "Node " << i << " subtris: " << flush;
		TriIndex iTri = mpNodes[i].miFirstSubTri;
		while (iTri != iNIL_TRI)
		{
			cout << iTri << " " << flush;
			iTri = mpTris[iTri].miNextSubTri;
		}
		cout << endl;
	}
}

void Forest::PrintForestStructure()
{
	cout << endl;
	PrintNodeInfo(iROOT_NODE, NULL, 0);

}

void Forest::PrintNodeInfo(NodeIndex i, Cut *pCut, int tabs)
{
	int j, k;
	TriIndex iTri, iNextTri;
	NodeIndex iChild;

	if (pCut != NULL)
		if (pCut->mpNodeRefs != NULL)
			if (pCut->mpNodeRefs[i] == NULL)
				return;

	for (j = 0; j < tabs; ++j)
	{
		cout << "  " << flush;
	}

	cout << i << flush;

	NodeIndex cnode = mpNodes[i].mCoincidentVertex;
	if (cnode != iNIL_NODE)
	{
		cout << " { " << flush;
		while ((cnode != iNIL_NODE) && (cnode != i))
		{
			cout << cnode << " ";
			cnode = mpNodes[cnode].mCoincidentVertex;
		}
		cout << "}" << flush;
	}
	
		cout << " - kids: " << flush;
		NodeIndex n = mpNodes[i].miFirstChild;
		while (n != iNIL_NODE)
		{
			cout << n << " " << flush;
			n = mpNodes[n].miRightSibling;
		}

	if (pCut != NULL)
	{
		if (pCut->mpNodeRefs != NULL)
		{
			if (pCut->mpNodeRefs[i] != NULL)
				cout << " - UC: " << pCut->mpRenderer->GetVertexUseCount(pCut->mpNodeRefs[i]->pVertexRenderDatum) << flush;

			unsigned int numlivetris = 0;
			if (pCut->mpNodeRefs[i]->miFirstLiveTri != iNIL_TRI)
				cout << " - LTrs: " << flush;
			for (iTri = pCut->mpNodeRefs[i]->miFirstLiveTri; iTri != iNIL_TRI; iTri = iNextTri)
			{
				k = mpTris[iTri].GetNodeIndex(iTri, i, this, pCut->mpRenderer);

				cout << iTri << " ";
				++numlivetris;

				iNextTri = pCut->mpTriRefs[iTri]->miNextLiveTris[k];
			}
			if (pCut->mpNodeRefs[i] != NULL)
			{
				if (numlivetris != pCut->mpRenderer->GetVertexUseCount(pCut->mpNodeRefs[i]->pVertexRenderDatum))
					cout << "FUGG" << endl;
			}

			iTri = mpNodes[i].miFirstSubTri;
			if (iTri != iNIL_TRI)
				cout << "- STrs: " << flush;
			while (iTri != iNIL_TRI)
			{
				cout << iTri << " ";
				iTri = mpTris[iTri].miNextSubTri;
			}
		}
	}

	cout << endl;

	iChild = mpNodes[i].miFirstChild;

	while (iChild != iNIL_NODE)
	{
		PrintNodeInfo(iChild, pCut, tabs + 1);		
		iChild = mpNodes[iChild].miRightSibling;
	}
}

void Forest::PrintTriInfo(Cut *pCut)
{
	cout << endl << "Tris" << endl;

	for (TriIndex iTri = 1; iTri <= mNumTris; ++iTri)
	{
		cout << "\t" << iTri << " - Cnrs: " << mpTris[iTri].miCorners[0] << " "
			<< mpTris[iTri].miCorners[1] << " " << mpTris[iTri].miCorners[2]
			<< " - Pxs: " << flush;
		
		if (pCut != NULL)
		{
			if (pCut->mpTriRefs != NULL)
			{
				if (pCut->mpTriRefs[iTri] != NULL)
				{
					cout << pCut->mpTriRefs[iTri]->backrefs[0] << " "
						<< pCut->mpTriRefs[iTri]->backrefs[1] << " "
						<< pCut->mpTriRefs[iTri]->backrefs[2];
				}
			}
		}
		cout << " - NLTs: " << flush;
		if (pCut != NULL)
		{
			if (pCut->mpTriRefs != NULL)
			{
				if (pCut->mpTriRefs[iTri] != NULL)
				{
					cout << pCut->mpTriRefs[iTri]->miNextLiveTris[0] << " "
						<< pCut->mpTriRefs[iTri]->miNextLiveTris[1] << " "
						<< pCut->mpTriRefs[iTri]->miNextLiveTris[2];
				}
			}
		}
		cout << endl;
	}
}

bool Forest::CheckIfProxiesMatchProxyBackRefs(Renderer *pRenderer)
{
	bool Ok = true;

	TriIndex i;
//	int k;
	
	for (i = 1; i <= mNumTris; ++i)
	{
// TODO: need to pass renderer in to index into backrefs array
/*		if ((mpTriRefs[i] != NULL) && (mpTriRefs[i]->miProxyBackRefs[0] != iNIL_NODE))
		{
			for (k = 0; k < 3; ++k)
			{
				if (pRenderer->GetProxy(i, k) != pRenderer->GetVertexRenderDatumIndex(mpNodeRefs[mpTriRefs[i]->miProxyBackRefs[k]]->pVertexRenderDatum))
				{
					Ok = false;
					cout << "Tri " << i << " Proxies (" << pRenderer->GetProxy(i, 0) << " " 
						<< pRenderer->GetProxy(i, 1) << " "
						<< pRenderer->GetProxy(i, 2) << ") don't match vertex cache entries of ProxyBackRefs ("
						<< pRenderer->GetVertexRenderDatumIndex(mpNodeRefs[mpTriRefs[i]->miProxyBackRefs[0]]->pVertexRenderDatum) << " " 
						<< pRenderer->GetVertexRenderDatumIndex(mpNodeRefs[mpTriRefs[i]->miProxyBackRefs[1]]->pVertexRenderDatum) << " " 
						<< pRenderer->GetVertexRenderDatumIndex(mpNodeRefs[mpTriRefs[i]->miProxyBackRefs[2]]->pVertexRenderDatum) << ")" << endl;
					cout << "ProxyBackRefs: " << mpTriRefs[i]->miProxyBackRefs[0] << " "
						<< mpTriRefs[i]->miProxyBackRefs[1] << " "
						<< mpTriRefs[i]->miProxyBackRefs[2] << " - Proxies point to nodes: "
						<< pRenderer->GetVertexCacheBackRef(pRenderer->GetProxy(i, 0), this) << " "
						<< pRenderer->GetVertexCacheBackRef(pRenderer->GetProxy(i, 1), this) << " "
						<< pRenderer->GetVertexCacheBackRef(pRenderer->GetProxy(i, 2), this) << endl;
				}
			}
		}
*/	}

	return Ok;
}

void Forest::CheckLiveTriListsC(TriIndex *FirstLiveTris, TriIndex **NextLiveTris)
{
	unsigned int i, k;
	TriIndex LiveTri;

//	for (i = 1; i <= mNumTris; ++i)
//	{
//		cout << "Tri " << i << " corners: " << mpTris[i].miCorners[0] << ", " << 
//			mpTris[i].miCorners[1] << ", " << mpTris[i].miCorners[2] << endl;
//	}

	for (i = 1; i <= mNumNodes; ++i)
	{
//		cout << "Node " << i << " livetris: ";
		LiveTri = FirstLiveTris[i];
		while (LiveTri != iNIL_TRI)
		{
//			cout << LiveTri << " " << flush;
			if ((mpTris[LiveTri].miCorners[0] != i) && (mpTris[LiveTri].miCorners[1] != i) && (mpTris[LiveTri].miCorners[2] != i))
			{
				cerr << endl;
				cerr << "Tri " << LiveTri << " is a livetri of node " << i << " but does not have it as a corner." << endl;
			}
			k = mpTris[LiveTri].GetNodeIndexC(LiveTri, i, *this);
			LiveTri = NextLiveTris[LiveTri][k];
		}
//		cout << endl;
	}
}
