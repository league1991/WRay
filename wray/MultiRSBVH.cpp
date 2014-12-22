#include "stdafx.h"

WMultiRSBVH::WMultiRSBVH()
{

}

WMultiRSBVH::~WMultiRSBVH()
{
}

void WMultiRSBVH::intersect( WRayGroup& rayGroup )
{
	// 初始化任务栈
	unsigned short maxStackNode = (short)(nodeMaxDepth * 1.5f);
	WTask* taskStack = new WTask[maxStackNode];	
	WTask* taskTop = taskStack;								// 栈顶指针
	taskTop->isLeaf = 0;									// 初始化栈顶，指向第一个节点
	taskTop->pInterior = multiInteriors;
	taskTop->SIMDlane = 0;
	taskTop->numRay = rayGroup.numRays;

	// 初始化光线栈
	unsigned short* activeRayStack[4];
	unsigned short maxStackRay  = maxStackNode * rayGroup.numRays;
	activeRayStack[0] = new unsigned short[maxStackRay * 4];
	memset((void*)activeRayStack[0], 0, maxStackRay * 4 * sizeof(short));
	activeRayStack[1] = activeRayStack[0] + maxStackRay;
	activeRayStack[2] = activeRayStack[1] + maxStackRay;
	activeRayStack[3] = activeRayStack[2] + maxStackRay;
	__m128i numActive4 = _mm_set_epi32(0, 0, 0,rayGroup.numRays);	// 栈顶索引
	int* const numActive = (int*)&numActive4;
	for (unsigned short nthRay = 0; nthRay < rayGroup.numRays; ++nthRay)	// 光线ID入栈
		activeRayStack[0][nthRay] = nthRay;

	__m128i one4 = _mm_set1_epi32(1);

	while (taskTop >= taskStack)
	{
		WTask* currTask = taskTop--;
		numActive[currTask->SIMDlane] -= currTask->numRay;
		int rayPtr = numActive[currTask->SIMDlane];
		if (!currTask->isLeaf)
		{
			WMultiBVHNode& node = *(currTask->pInterior);
			__m128 gResult4 = _mm_castsi128_ps(_mm_set1_epi32(0));
			__m128 length4  = _mm_set_ps1(0.0f);
			__m128i newCount4  = _mm_set1_epi32(0);

			for (unsigned short i = 0; i < currTask->numRay; ++i)
			{
				unsigned short rayID = activeRayStack[currTask->SIMDlane][rayPtr + i];

				// 复制光线,用于对包围盒求交
				__m128 rayO_B[3];
				__m128 rayInvD_B[3];
				__m128 rayTMin, rayTMax;
				rayO_B[0] = _mm_set_ps1(rayGroup.ori[0][rayID]);
				rayO_B[1] = _mm_set_ps1(rayGroup.ori[1][rayID]);
				rayO_B[2] = _mm_set_ps1(rayGroup.ori[2][rayID]);
				rayInvD_B[0] = _mm_set_ps1(rayGroup.invDir[0][rayID]);
				rayInvD_B[1] = _mm_set_ps1(rayGroup.invDir[1][rayID]);
				rayInvD_B[2] = _mm_set_ps1(rayGroup.invDir[2][rayID]);
				rayTMin = _mm_set_ps1(rayGroup.ori_min[rayID * 4 + 3]);
				rayTMax = _mm_set_ps1(rayGroup.dir_max[rayID * 4 + 3]);/*
				rayO_B[0] = rayO_B[1] = rayO_B[2] = rayTMin = _mm_load_ps(rayGroup.ori_min + rayID * 4);
				rayInvD_B[0] = rayInvD_B[1] = rayInvD_B[2] = rayTMax = _mm_div_ps(_mm_set_ps1(1.0f),_mm_load_ps(rayGroup.dir_max + rayID * 4));
				_MM_TRANSPOSE4_PS(rayO_B[0], rayO_B[1], rayO_B[2], rayTMin);
				_MM_TRANSPOSE4_PS(rayInvD_B[0], rayInvD_B[1], rayInvD_B[2], rayTMax);
				rayTMax = _mm_set_ps1(rayGroup.dir_max[rayID * 4 + 3]);*/


				// 把光线与四个包围盒同时求交,求出最近交点t值存在tMin中
				__m128 t0, t1, tMin, tMax, isHit;
				t0   = _mm_mul_ps(_mm_sub_ps(node.bMin[0], rayO_B[0]), rayInvD_B[0]);
				t1   = _mm_mul_ps(_mm_sub_ps(node.bMax[0], rayO_B[0]), rayInvD_B[0]);//   <------------- should be optimized
				tMin = _mm_min_ps(t0, t1);
				tMax = _mm_max_ps(t0, t1);
				t0   = _mm_mul_ps(_mm_sub_ps(node.bMin[1], rayO_B[1]), rayInvD_B[1]);
				t1   = _mm_mul_ps(_mm_sub_ps(node.bMax[1], rayO_B[1]), rayInvD_B[1]);
				tMin = _mm_max_ps(tMin, _mm_min_ps(t0, t1));
				tMax = _mm_min_ps(tMax, _mm_max_ps(t0, t1));
				t0   = _mm_mul_ps(_mm_sub_ps(node.bMin[2], rayO_B[2]), rayInvD_B[2]);
				t1   = _mm_mul_ps(_mm_sub_ps(node.bMax[2], rayO_B[2]), rayInvD_B[2]);
				tMin = _mm_max_ps(tMin, _mm_min_ps(t0, t1));
				tMax = _mm_min_ps(tMax, _mm_max_ps(t0, t1));
				__m128 tMinMax = _mm_add_ps(tMin, tMax);
				// isHit = (tMin < tMax) && (r.tMin < tMax) && (r.tMax > tMin)
				isHit = _mm_cmplt_ps(tMin, tMax);	
				isHit = _mm_and_ps(isHit, _mm_cmplt_ps(rayTMin, tMax));
				isHit = _mm_and_ps(isHit, _mm_cmpgt_ps(rayTMax, tMin));
				length4 = _mm_add_ps(length4, _mm_and_ps(isHit, tMin));

				activeRayStack[0][numActive[0]] = rayID;
				activeRayStack[1][numActive[1]] = rayID;
				activeRayStack[2][numActive[2]] = rayID;
				activeRayStack[3][numActive[3]] = rayID;

				__m128i delta = _mm_and_si128(_mm_castps_si128(isHit), one4);
				numActive4 = _mm_add_epi32(delta, numActive4);
				newCount4  = _mm_add_epi32(delta, newCount4);
				gResult4   = _mm_or_ps(gResult4, isHit);
			}

			// 根据光线与包围盒相交的平均长度对子节点排序
			//if (/*_mm_movemask_ps(gResult4)*/1)
			{
				char order[4] = {0, 1, 2, 3};
				float* length = (float*)&length4;

				// 按照平均长度从大到小排序
				for (char i = 0; i < 3; ++i)
					for (char j = i + 1; j < 4; ++j)
						if (length[i] < length[j])
						{
							swap(length[i], length[j]);
							swap(order[i], order[j]);
						}

				// 再按排好的顺序入栈
				const int* gResult = (int*)&gResult4;
				const int* newCount = (int*)&newCount4;
				for (char i = 0; i < 4; ++i)
				{
					const char childID = order[i];
					if (gResult[childID] && node.child[childID])
					{
						taskTop++;
						taskTop->isLeaf = node.isLeaf[childID];
						taskTop->numRay = newCount[childID];
						taskTop->pInterior = node.child[childID];
						taskTop->SIMDlane = childID;
					}
				}
			}
		}
		else
		{
			WMultiRSBVHLeaf& node = *(currTask->pLeaf);
			WRayPacket rp;
			unsigned short ithRay = 3;
			for (ithRay = 3; ithRay < currTask->numRay; ithRay += 4)
			{
				// 每次使用4条光线对三角形求交
				// 加载光线，要求光线坐标数组对齐到16字节
				unsigned short* baseRay = activeRayStack[currTask->SIMDlane] + rayPtr + ithRay;
				unsigned short rayID = *baseRay;
				__m128 ori3 = _mm_load_ps(rayGroup.ori_min + rayID * 4);
				__m128 dir3 = _mm_load_ps(rayGroup.dir_max + rayID * 4);

				rayID = *(--baseRay);
				rp.ori[2] = _mm_load_ps(rayGroup.ori_min + rayID * 4);
				rp.dir[2] = _mm_load_ps(rayGroup.dir_max + rayID * 4);

				rayID = *(--baseRay);
				rp.ori[1] = _mm_load_ps(rayGroup.ori_min + rayID * 4);
				rp.dir[1] = _mm_load_ps(rayGroup.dir_max + rayID * 4);

				rayID = *(--baseRay);
				rp.ori[0] = _mm_load_ps(rayGroup.ori_min + rayID * 4);
				rp.dir[0] = _mm_load_ps(rayGroup.dir_max + rayID * 4);

				// 转置后,ori[0:3] = {xxxx yyyy zzzz (min)(min)(min)(min)}
				//       dir[0:3] = {xxxx yyyy zzzz (max)(max)(max)(max)}
				_MM_TRANSPOSE4_PS(rp.ori[0], rp.ori[1], rp.ori[2], ori3);
				_MM_TRANSPOSE4_PS(rp.dir[0], rp.dir[1], rp.dir[2], dir3);

				rp.tMin = ori3;		rp.tMax = dir3;

				WTriangle** pTri = node.triangles;

				// 4条光线与节点内的三角形逐个求交，并根据求交结果决定是否更新光线信息
				char maxLeafTris = 4;
				while(*pTri && maxLeafTris--)
				{
					__m128 result = (*pTri)->intersectTest(rp);
					float* pResultf = (float*)&result;
					unsigned int* pResulti = (unsigned int*)&result;
					for (int i = 0; i < 4; ++i)
					{
						if (pResulti[i] != 0xffffffff)
						{
							float* tMax = (float*)&rp.tMax;
							tMax[i] = pResultf[i];
							rayGroup.isectTriangle[baseRay[i]] = *pTri;
							rayGroup.dir_max[baseRay[i] * 4 + 3] = pResultf[i];
						}
					}
					++pTri;
				}
			}
			// 对剩下的光线求交
			ithRay -= 3;
			int remain = currTask->numRay - ithRay;			// 剩下的光线数
			for (int i = 0; i < remain; ++ithRay, ++i)
			{
				unsigned short rayID = activeRayStack[currTask->SIMDlane][rayPtr + ithRay];
				WRay r;
				int beginID = rayID * 4;
				r.point.v[0] = rayGroup.ori_min[beginID];
				r.point.v[1] = rayGroup.ori_min[beginID + 1];
				r.point.v[2] = rayGroup.ori_min[beginID + 2];
				r.tMin       = rayGroup.ori_min[beginID + 3];
				r.direction.v[0] = rayGroup.dir_max[beginID];
				r.direction.v[1] = rayGroup.dir_max[beginID + 1];
				r.direction.v[2] = rayGroup.dir_max[beginID + 2];
				r.tMax		 = rayGroup.dir_max[beginID + 3];

				// 光线与节点内的三角形逐个求交，并根据求交结果决定是否更新光线信息
				WTriangle** pTri = &node.triangles[0];
				char maxLeafTris = WBVHNode::maxTrianglesPerLeaf;
				while(*pTri && maxLeafTris--)
				{
					float t = (*pTri)->intersectTest(r);
					if (t < rayGroup.dir_max[beginID + 3])
					{
						rayGroup.dir_max[beginID + 3] = t;
						rayGroup.isectTriangle[rayID] = *pTri;
					}
					++pTri;
				}
			}
		}
	}
	// 如有交点， 求出DifferentialGeometry
	for (unsigned short ithRay = 0; ithRay < rayGroup.numRays; ++ithRay)
	{
		WTriangle* isectTri = rayGroup.isectTriangle[ithRay];
		WRay r;
		int beginID = ithRay * 4;
		r.point.v[0] = rayGroup.ori_min[beginID];
		r.point.v[1] = rayGroup.ori_min[beginID + 1];
		r.point.v[2] = rayGroup.ori_min[beginID + 2];
		r.tMin       = rayGroup.ori_min[beginID + 3];
		r.direction.v[0] = rayGroup.dir_max[beginID];
		r.direction.v[1] = rayGroup.dir_max[beginID + 1];
		r.direction.v[2] = rayGroup.dir_max[beginID + 2];
		r.tMax		 = rayGroup.dir_max[beginID + 3];
		if (isectTri)
			isectTri->intersect(r, rayGroup.DG[ithRay]);
	}
	delete[] taskStack;
	delete[] activeRayStack[0];
}

bool WMultiRSBVH::isIntersect( WRay& r, int beginNode /*= 0*/ )
{
	return false;
}

void WMultiRSBVH::buildEnhancedTree()
{
	int l = nodes.size();
	if (!l)
		return;

	// 分配存储节点的空间
	multiInteriors = (WMultiBVHNode*)allocAligned16(sizeof(WMultiBVHNode) * l);
	multiRSLeafs     = new WMultiRSBVHLeaf[l];

	int currInterior = 0;
	int currLeaf     = 0;
	buildEnhancedTreeKernel(0, currInterior, currLeaf);
	numInteriors	 = currInterior;
	numLeafs		 = currLeaf;

	buildCacheFriendlyKernel();
}

void WMultiRSBVH::buildEnhancedTreeKernel( int ithNode, int& currInterior, int& currLeaf )
{
	//printf("node\n");
	WBVHNode& srcParent = nodes[ithNode];						// 当前子树的根节点
	WMultiBVHNode& dstNode = multiInteriors[currInterior];	// 要构建的四叉树节点
	int id_L1[2] = {ithNode + 1, srcParent.farNode};			// 第一层子节点序号

	// 计算表示传输顺序的traversalOrder数组
	computeTraversalOrder(dstNode, srcParent, nodes[id_L1[0]], nodes[id_L1[1]]);

	for (int i = 0; i < 2; ++i)
	{
		WBVHNode& srcL1 = nodes[id_L1[i]];
		if (srcL1.type == WBVHNode::BVHN_LEAF)
		{	// 第一层为叶节点时
			// 一个节点设为叶节点，另一个设为空（通过把包围盒设为无效包围盒）
			int dstIDL1 = i << 1;

			// 设置节点包围盒
			WBoundingBox box;
			getBBoxFromSrcLeaf(srcL1, box);
			setBBoxToNode(dstNode, box, dstIDL1);
			setBBoxToNode(dstNode, invalidBox, dstIDL1 | 0x1);

			// 申请一个叶节点并设置
			WMultiRSBVHLeaf& dstLeaf = multiRSLeafs[currLeaf++];
			for (int ithTri = 0; ithTri < WBVHNode::maxTrianglesPerLeaf; ++ithTri)
				dstLeaf.triangles[ithTri] = srcL1.tris[ithTri];

			dstNode.rsLeaf	[dstIDL1] = &dstLeaf;
			dstNode.isLeaf	[dstIDL1] = 1;
			dstNode.leaf	[dstIDL1 | 0x1] = NULL;
			dstNode.isLeaf	[dstIDL1 | 0x1] = 0;
		}
		else
		{
			int id_L2[2] = {id_L1[i] + 1, srcL1.farNode};
			for (int j = 0; j < 2; ++j)
			{
				WBVHNode& srcL2 = nodes[id_L2[j]];
				int dstIDL2 = (i << 1) | j; // dstIDL1 = i * 2 + j
				if (srcL2.type == WBVHNode::BVHN_LEAF)
				{	// 第二层的叶节点
					WBoundingBox box;
					getBBoxFromSrcLeaf(srcL2, box);
					setBBoxToNode(dstNode, box, dstIDL2);

					WMultiRSBVHLeaf& dstLeaf = multiRSLeafs[currLeaf++];
					for (int ithTri = 0; ithTri < WBVHNode::maxTrianglesPerLeaf; ++ithTri)
						dstLeaf.triangles[ithTri] = srcL2.tris[ithTri];
					dstNode.rsLeaf	[dstIDL2] = &dstLeaf;
					dstNode.isLeaf	[dstIDL2] = 1;
				}
				else
				{	// 第二层仍然是内部节点
					WBoundingBox box;
					getBBoxFromInterior(box, srcL2);
					setBBoxToNode(dstNode, box, dstIDL2);
					dstNode.isLeaf	[dstIDL2] = 0;
					currInterior++;
					dstNode.child	[dstIDL2] = &multiInteriors[currInterior];
					buildEnhancedTreeKernel(id_L2[j], currInterior, currLeaf);
				}
			}
		}
	}
}

void WMultiRSBVH::displayNodeInfo()
{
	displayNodeKernel(multiInteriors);
}

void WMultiRSBVH::displayNodeKernel( WMultiBVHNode* pNode )
{
	printf("interior\n");
	for (int i = 0; i < 4; ++i)
	{
		if (!pNode->child[i])
			continue;
		if (pNode->isLeaf[i])
		{
			printf("leaf\n");
			WMultiRSBVHLeaf* pLeaf = pNode->rsLeaf[i];
			for (int t = 0; t < WBVHNode::maxTrianglesPerLeaf; ++t)
			{
				printf("%p\n", pLeaf->triangles[t]);
			}
		}
		else
			displayNodeKernel(pNode->child[i]);
	}
}

void WMultiRSBVH::drawTreeRecursive( WMultiBVHNode* pNode )
{
	WMultiBVHNode& node = *pNode;
	float* pMin[3], *pMax[3];
	pMin[0] = (float*)&node.bMin[0];	pMax[0] = (float*)&node.bMax[0];
	pMin[1] = (float*)&node.bMin[1];	pMax[1] = (float*)&node.bMax[1];
	pMin[2] = (float*)&node.bMin[2];	pMax[2] = (float*)&node.bMax[2];
	for (int i = 0; i < 4; i++)
	{
		WBoundingBox box(WVector3(*pMin[0],*pMin[1],*pMin[2]),
			WVector3(*pMax[0],*pMax[1],*pMax[2]));
		box.draw();
		pMin[0]++; pMax[0]++;
		pMin[1]++; pMax[1]++;
		pMin[2]++; pMax[2]++;

		if (node.child[i] != NULL && node.isLeaf[i] == 0)
		{
			drawTreeRecursive(node.child[i]);
		}
		else if (node.isLeaf[i])
		{
			glColor3f(1,1,1);
			WMultiRSBVHLeaf& leaf = *node.rsLeaf[i];
			for (int j = 0; j < 4; j++)
			{
				WTriangle* pTri = leaf.triangles[j];
				if (pTri)
				{
					pTri->draw();
				}
			}
			glColor3f(0,1,0);
		}
	}
}

void WRayGroup::allocSpace(short nRay)
{
	numRays = nRay;
	float* ptr = (float*)WAccelerator::allocAligned16(sizeof(float) * nRay * 14);
	ori_min = ptr;
	dir_max = ptr + nRay * 4;
	ori[0]  = dir_max + nRay * 4;
	ori[1]  = ori[0]  + nRay;
	ori[2]  = ori[1]  + nRay;
	invDir[0]  = ori[2]  + nRay;
	invDir[1]  = invDir[0] + nRay;
	invDir[2]  = invDir[1] + nRay;

	isectTriangle = new WTriangle*[nRay];
	memset((void*)isectTriangle, 0, sizeof(WTriangle*) * nRay);

	DG = new WDifferentialGeometry[nRay];
}

void WRayGroup::freeSpace()
{
	WAccelerator::freeAligned16(ori_min);
	delete[] isectTriangle;
	delete[] DG;
	numRays = 0;
	ori_min = dir_max = ori[0] = ori[1] = ori[2] = invDir[0] = invDir[1] = invDir[2] = NULL;
	isectTriangle = NULL;
	DG = NULL;
}

WRayGroup::~WRayGroup()
{
	freeSpace();
}

WRayGroup::WRayGroup()
{
	numRays = 0;
	ori_min = dir_max = ori[0] = ori[1] = ori[2] = invDir[0] = invDir[1] = invDir[2] = NULL;
	isectTriangle = NULL;
	DG = NULL;
}
