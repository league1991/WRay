#include "stdafx.h"
#include "BoundingBox.h"
#include "MultiBVH.h"
const WBoundingBox WMultiBVH::invalidBox = WBoundingBox(WVector3(0.0f,0.0f,0.0f),WVector3(-0.0f,-0.0f,-0.0f));

WMultiBVH::WMultiBVH():WSimpleBVH()
{
	numInteriors   = numLeafs = 0;
	multiInteriors = NULL;
	multiLeafs     = NULL;
}

WMultiBVH::~WMultiBVH()
{
	this->clearTree();
}
void WMultiBVH::buildEnhancedTree()
{
	int l = nodes.size();
	if (!l)
		return;

	// 分配存储节点的空间
	multiInteriors = (WMultiBVHNode*)allocAligned16(sizeof(WMultiBVHNode) * l);
	multiLeafs     = (WMultiBVHLeaf*)allocAligned16(sizeof(WMultiBVHLeaf) * l);

	int currInterior = 0;
	int currLeaf     = 0;
	nodeMaxDepth = 0;
	buildEnhancedTreeKernel(0, currInterior, currLeaf);
	numInteriors	 = currInterior;
	numLeafs		 = currLeaf;

	buildCacheFriendlyKernel();
}

void WMultiBVH::buildEnhancedTreeKernel(int ithNode, 
										int& currInterior, int& currLeaf, int curDepth)
{
	//printf("node\n");
	nodeMaxDepth = max(nodeMaxDepth, curDepth);
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
			WMultiBVHLeaf& dstLeaf = multiLeafs[currLeaf++];
			makeLeaf(dstLeaf, srcL1);
			dstNode.leaf	[dstIDL1] = &dstLeaf;
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

					WMultiBVHLeaf& dstLeaf = multiLeafs[currLeaf++];
					makeLeaf(dstLeaf, srcL2);
					dstNode.leaf	[dstIDL2] = &dstLeaf;
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
					buildEnhancedTreeKernel(id_L2[j], currInterior, currLeaf, curDepth + 1);
				}
			}
		}
	}
}

void WMultiBVH::buildCacheFriendlyKernel()
{
	struct StackElement
	{
		WMultiBVHNode* pNode;
		int layer;
	};
	// 分配空间存放排列好的节点
	WMultiBVHNode* tempInteriors = (WMultiBVHNode*)allocAligned16(sizeof(WMultiBVHNode) * nodes.size());
	deque<WMultiBVHNode*> stack0;
	stack0.push_back(multiInteriors);
	WMultiBVHNode* dstNode = tempInteriors;
	map<WMultiBVHNode*, WMultiBVHNode*> addressMap;			// 地址对应表

	while(stack0.size())
	{
		// 取一个子树处理
		//StackElement stack1[MBVH_SUBTREE_DEPTH * 4];
		//StackElement* stack1Top = stack1;					// 栈顶指针
		deque<StackElement>stack1;
		StackElement t;
		t.pNode = stack0.back();					// 父节点子树栈
		t.layer = 0;								// 记录节点层数
		stack1.push_back(t);
		stack0.pop_back();						

		while(stack1.size())
		{
			// pop and copy node
			StackElement top = stack1.front();
			WMultiBVHNode* pNode = top.pNode;
			int layer = top.layer;
			stack1.pop_front();
			memcpy((void*)dstNode, (void*)pNode, sizeof(WMultiBVHNode));
			// record address map
			addressMap[pNode] = dstNode;
			dstNode++;
			// process child node
			for (int i = 0; i < 4 ; i++)
			{
				if (!pNode->isLeaf[i] && pNode->child[i])
				{
					if (layer < MBVH_SUBTREE_DEPTH)
					{	// 子树内部的节点
						top.pNode = pNode->child[i];
						top.layer = layer + 1;
						stack1.push_back(top);
					}
					else
					{	// 子树最下层的节点
						stack0.push_back(pNode->child[i]);
					}
				}
			}
		}
	}

	for(int i = 0; i < numInteriors; i++)
	{
		for(int ithChild = 0; ithChild < 4; ithChild++)
		{
			if (!tempInteriors[i].isLeaf[ithChild] && tempInteriors[i].child[ithChild])
			{
				tempInteriors[i].child[ithChild] = addressMap[tempInteriors[i].child[ithChild]];
			}
		}
	}
	freeAligned16(multiInteriors);
	multiInteriors = tempInteriors;
}




bool WMultiBVH::intersect( WRay& r,WDifferentialGeometry& DG, int* endNode /*= NULL*/, int beginNode /*= 0*/ )
{
	if (numInteriors == 0 )
		return false;

	++numIntersect;
	// 分配求交的堆栈
	StackNode* stack = new StackNode[nodeMaxDepth * 3];		// 新建一个栈存储求交数据
	stack->node = multiInteriors;
	stack->t    = 0.0f;										// 把根节点入栈
	StackNode* stackTop = stack;							// 表示当前的栈顶节点	
	// 记录t值以及相交的最近三角形指针
	float t = FLT_MAX;
	WTriangle* nearestTri = NULL;

	// 复制光线,用于对包围盒求交
	__m128 rayO_B[3];
	__m128 rayInvD_B[3];
	__m128 rayTMin, rayTMax;
	rayO_B[0] = _mm_load_ps1(&r.point.x);	rayInvD_B[0] = _mm_set_ps1(1.0f / r.direction.x);
	rayO_B[1] = _mm_load_ps1(&r.point.y);	rayInvD_B[1] = _mm_set_ps1(1.0f / r.direction.y);
	rayO_B[2] = _mm_load_ps1(&r.point.z);	rayInvD_B[2] = _mm_set_ps1(1.0f / r.direction.z);
	rayTMin = _mm_set_ps1(r.tMin);
	rayTMax = _mm_set_ps1(r.tMax);
	char orderIndex =  (r.direction.x <= 0.0f)       | 
		              ((r.direction.y <= 0.0f) << 1) |
					  ((r.direction.z <= 0.0f) << 2); 

	// 复制光线,用于对三角形求交
	__m128 mask;
	unsigned int uMask = 0x80000000;
	mask = _mm_set_ps1(*(reinterpret_cast<float*>(&uMask)));
	__m128i mask2  = _mm_set1_epi32(0x3);
	// w = 0   --> (u, v, w) = (y, z, x)
	// w = 1   --> (u, v, w) = (x, z, y)
	// w = 2   --> (u, v, w) = (x, y, z)
	_MM_ALIGN16 float rayOArray[3][4];
	rayOArray[0][0] = r.point.y;   rayOArray[0][1] = r.point.z;   rayOArray[0][2] = r.point.x; 
	rayOArray[1][0] = r.point.x;   rayOArray[1][1] = r.point.z;   rayOArray[1][2] = r.point.y; 
	rayOArray[2][0] = r.point.x;   rayOArray[2][1] = r.point.y;   rayOArray[2][2] = r.point.z; 
	_MM_ALIGN16 float rayDArray[3][4];
	rayDArray[0][0] = r.direction.y;   rayDArray[0][1] = r.direction.z;   rayDArray[0][2] = r.direction.x;
	rayDArray[1][0] = r.direction.x;   rayDArray[1][1] = r.direction.z;   rayDArray[1][2] = r.direction.y;
	rayDArray[2][0] = r.direction.x;   rayDArray[2][1] = r.direction.y;   rayDArray[2][2] = r.direction.z;

	while (stackTop >= stack)
	{
		// 如果之前压入栈的节点的包围盒与光线交点在
		// 当前算出的最近交点之外，剔除该节点
		if (stackTop->t > t)
		{
			stackTop--;
			continue;
		}
		// 取栈顶节点处理
		WMultiBVHNode& node = *(stackTop->node);
		stackTop--;
		++numTraverseInterior_I;
		//__m128 sum = _mm_add_ps(node.bMin[0], node.bMax[0]);

		// 利用SIMD指令把光线与四个包围盒同时求交,求出最近交点t值存在tMin中
		__m128 t0, t1, tMin, tMax, isHit;
		t0   = _mm_mul_ps(_mm_sub_ps(node.bMin[0], rayO_B[0]), rayInvD_B[0]);
		t1   = _mm_mul_ps(_mm_sub_ps(node.bMax[0], rayO_B[0]), rayInvD_B[0]);
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
		__m128 child   = _mm_load_ps((float*)node.child);
		isHit = _mm_and_ps(isHit, child);

		// 确定子节点处理顺序
		const float* tMinMaxf = (const float*)&tMinMax;
		//char order0[4] = {0, 1, 2, 3};
		/*char order[4];		
		order[3] =  tOrder >> 6;
		order[2] = (tOrder >> 4) & 0x3;
		order[1] = (tOrder >> 2) & 0x3;
		order[0] =  tOrder & 0x3;*/

		// 按顺序处理子节点
		unsigned char tOrder = node.traversalOrder[orderIndex];
		const float* tMinf  = (const float*)&tMin;
		char toPush[4]; char totalPush = 0;
		for (int i = 0; i < 4; ++i)
		{
			char ithChild = (tOrder >> 6) & 0x3;
			tOrder <<= 2;
			unsigned int isHitui = *((unsigned int*)&isHit + ithChild);
			if (isHitui && tMinf[ithChild] < t)//   <------------- should be optimized
			{
				if (node.isLeaf[ithChild])
				{
					++numTraverseLeaf_I;
					WMultiBVHLeaf& leaf = *(node.leaf[ithChild]);
#ifndef PER_TRIANGLE_INTERSECT
					__m128 rayO_u, rayO_v, rayO_w, rayO_xxx;
					__m128 rayD_u, rayD_v, rayD_w, rayD_xxx;


					__m128i axis_w = _mm_and_si128(_mm_srai_epi32(leaf.ci, 25), mask2);//   <------------- should be optimized
					const int*  axis_wf = (const int*)&axis_w;

					rayO_u   = _mm_load_ps(rayOArray[axis_wf[0]]);
					rayD_u   = _mm_load_ps(rayDArray[axis_wf[0]]);
					rayO_v   = _mm_load_ps(rayOArray[axis_wf[1]]);
					rayD_v   = _mm_load_ps(rayDArray[axis_wf[1]]);
					rayO_w   = _mm_load_ps(rayOArray[axis_wf[2]]);
					rayD_w   = _mm_load_ps(rayDArray[axis_wf[2]]);
					rayO_xxx = _mm_load_ps(rayOArray[axis_wf[3]]);
					rayD_xxx = _mm_load_ps(rayDArray[axis_wf[3]]);

					_MM_TRANSPOSE4_PS(rayO_u, rayO_v, rayO_w, rayO_xxx);
					_MM_TRANSPOSE4_PS(rayD_u, rayD_v, rayD_w, rayD_xxx);

					//temporary variables 
					__m128 det, dett, detu, detv;
					__m128 nrv, nru, du, dv, ou, ov;
					/* ----ray-packet/triangle hit test ---- */ 
					//dett = np -(ou*nu+ov*nv+ow) 
					dett = leaf.np;
					dett = _mm_sub_ps(dett,rayO_w);								// dett = np - ow

					du = leaf.nu;
					dv = leaf.nv;
					ou = rayO_u;
					du = _mm_mul_ps(du, ou);									// du = nu * ou
					dett = _mm_sub_ps(dett, du);								// dett = np - ow - nu * ou
					ov = rayO_v;
					dv = _mm_mul_ps(dv, ov);									// dv = nv * ov
					dett = _mm_sub_ps(dett, dv);								// dett = np - ow - nu * ou - nv * ov
					//det =du*nu+dv*nv+dw 
					du = rayD_u;
					dv = rayD_v;
					det = leaf.nu;															
					det = _mm_mul_ps(det, du);									// det = nu * du
					nrv = leaf.nv; 
					nrv = _mm_mul_ps(nrv, dv);									// nrv = nv * dv
					det = _mm_add_ps(det, rayD_w);								// det = nu * du + dw
					det = _mm_add_ps(det, nrv);									// det = nu * du + nv * dv + dw
					//Du = du*dett - (pu-ou)*det 
					//ou = pu;
					ou = _mm_sub_ps(leaf.pu,rayO_u);								// ou = pu - ou
					du  = _mm_mul_ps(du, dett);									// du = du * dett
					nru = _mm_mul_ps(ou, det);									// nru = (pu - ou) * det
					du  = _mm_sub_ps(du, nru);									// du  = du * dett - (pu - ou) * det
					//Dv = dv*dett -  (pv-ov)*det 
					//ov = pv; 
					ov = _mm_sub_ps(leaf.pv, rayO_v);							// ov = pv - ov
					dv  = _mm_mul_ps(dv, dett);									// dv = dv * dett
					nrv = _mm_mul_ps(ov, det);									// nrv = (pv - ov) * det
					dv  = _mm_sub_ps(dv, nrv);									// dv  = dv * dett - (pv - ov) * det
					//detu = (e1vDu – e1u*Dv) 
					nru = leaf.e1v;
					nrv = leaf.e1u; 
					nru = _mm_mul_ps(nru, du);									// nru = e1v * du
					nrv = _mm_mul_ps(nrv, dv);									// nrv = e1v * dv
					detu = _mm_sub_ps(nru, nrv);								// detu = e1v * du - e1v * dv
					//detv = (e0uDv – e0v*Du) 
					nrv = leaf.e0u;
					nru = leaf.e0v;
					nru = _mm_mul_ps(nru, du);									// nru = e0v * du
					nrv = _mm_mul_ps(nrv, dv);									// nrv = e0u * dv
					detv = _mm_sub_ps(nrv, nru);								// detv = e0u * dv - e0v * du

					__m128 tmpdet0, tmpdet1; 
					tmpdet0 = _mm_sub_ps(det, detu);							// tmpdet0 = det - detu - detv
					tmpdet0 = _mm_sub_ps(tmpdet0, detv);
					tmpdet0 = _mm_xor_ps(tmpdet0, detu);
					tmpdet1 = _mm_xor_ps(detv, detu);
					tmpdet0 = _mm_andnot_ps(_mm_or_ps(tmpdet0, tmpdet1), mask);// 如果tmpdet0符号位为1，表示有交点
					// 逼近法算 rdet = 1.0f / det
					//const float twof = 2.0f;
					__m128 two_m = _mm_set_ps1(2.0f);//_mm_load_ps1(&twof);
					__m128 rdet  = _mm_rcp_ps(det);								// rdet = 1.0f / det
					__m128 two_r = _mm_mul_ps(two_m, rdet);						// two_r = 2.0f * rdet
					__m128 det_r_r = _mm_mul_ps(det, _mm_mul_ps(rdet, rdet));	// det_r_r = det * rdet * rdet
					rdet = _mm_sub_ps(two_r, det_r_r);							// 较准确的 1.0f / det 值
					// 算出参数值
					__m128 tx4 = _mm_mul_ps(dett, rdet);

					for (int ithTri = 0; ithTri < 4; ithTri++)
					{
						WTriangle* tri = leaf.triangle[ithTri];
						numTriangleIsect_I += (tri != NULL);
						float*    tx4f = (float*)&tx4;
						unsigned int* tmpdet0ui = (unsigned int*)&tmpdet0;
						if (tri						&& 
							tx4f[ithTri] < t        && 
							tx4f[ithTri] > r.tMin   && 
							tmpdet0ui[ithTri])
						{
							t = tx4f[ithTri];
							nearestTri = tri;
						}
					}
#else
					for (int ithTri = 0; ithTri < 4; ithTri++)
					{
						WTriangle* tri = leaf.triangle[ithTri];
						if (!tri)
							break;
						{
							float temp = tri->intersectTest(r);
							if (temp < t)
							{
								t = temp;
								nearestTri = tri;
							}
						}
					}
#endif
				}
				else
				{
					toPush[totalPush++] = ithChild;//   <------------- should be optimized
					++stackTop;
					stackTop->node = node.child[ithChild];
					float* tMinf   = (float*)&tMin;
					stackTop->t    = tMinf[ithChild];
				}
			}
		}
	}
	bool res = false;
	if (nearestTri)
	{
		nearestTri->intersect(r, DG);
		res = true;
	}
	delete[] stack;
	return res;
}

bool WMultiBVH::isIntersect( WRay& r, int beginNode /*= 0*/ )
{
	if (numInteriors == 0 )
		return false;

	++numIntersectTest;
	// 分配求交的堆栈
	StackNode* stack = new StackNode[nodeMaxDepth * 3];		// 新建一个栈存储求交数据
	stack->node = multiInteriors;
	stack->t    = 0.0f;										// 把根节点入栈
	StackNode* stackTop = stack;							// 表示当前的栈顶节点

	// 复制光线,用于对包围盒求交
	__m128 rayO[3];
	__m128 rayInvD[3];
	__m128 rayTMin, rayTMax;
	rayO[0] = _mm_load_ps1(&r.point.x);	rayInvD[0] = _mm_set_ps1(1.0f / r.direction.x);
	rayO[1] = _mm_load_ps1(&r.point.y);	rayInvD[1] = _mm_set_ps1(1.0f / r.direction.y);
	rayO[2] = _mm_load_ps1(&r.point.z);	rayInvD[2] = _mm_set_ps1(1.0f / r.direction.z);
	rayTMin = _mm_set_ps1(r.tMin);
	rayTMax = _mm_set_ps1(r.tMax);
	char orderIndex =  (r.direction.x <= 0.0f)       | 
		((r.direction.y <= 0.0f) << 1) |
		((r.direction.z <= 0.0f) << 2);

	// 复制光线,用于对三角形求交
	const unsigned int mask1i = 0x80000000;
	const __m128 mask1 = _mm_set_ps1(*(float*)&mask1i);
	const __m128i mask2  = _mm_set1_epi32(0x3);

	_MM_ALIGN16 float rayOArray[3][4];
	rayOArray[0][0] = r.point.y;   rayOArray[0][1] = r.point.z;   rayOArray[0][2] = r.point.x; 
	rayOArray[1][0] = r.point.x;   rayOArray[1][1] = r.point.z;   rayOArray[1][2] = r.point.y; 
	rayOArray[2][0] = r.point.x;   rayOArray[2][1] = r.point.y;   rayOArray[2][2] = r.point.z; 
	_MM_ALIGN16 float rayDArray[3][4];
	rayDArray[0][0] = r.direction.y;   rayDArray[0][1] = r.direction.z;   rayDArray[0][2] = r.direction.x;
	rayDArray[1][0] = r.direction.x;   rayDArray[1][1] = r.direction.z;   rayDArray[1][2] = r.direction.y;
	rayDArray[2][0] = r.direction.x;   rayDArray[2][1] = r.direction.y;   rayDArray[2][2] = r.direction.z;
	// 记录t值以及相交的最近三角形指针
	float t = r.tMax;
	WTriangle* nearestTri = NULL;
	while (stackTop >= stack)
	{
		// 如果之前压入栈的节点的包围盒与光线交点在
		// 当前算出的最近交点之外，剔除该节点
		if (stackTop->t > t)
		{
			stackTop--;
			continue;
		}
		// 取栈顶节点处理
		WMultiBVHNode& node = *(stackTop->node);
		stackTop--;
		++numTraverseInterior_IT;

		// 把光线与四个包围盒同时求交,求出最近交点t值存在tMin中
		__m128 t0, t1, tMin, tMax, isHit;
		t0   = _mm_mul_ps(_mm_sub_ps(node.bMin[0], rayO[0]), rayInvD[0]);
		t1   = _mm_mul_ps(_mm_sub_ps(node.bMax[0], rayO[0]), rayInvD[0]);
		tMin = _mm_min_ps(t0, t1);
		tMax = _mm_max_ps(t0, t1);
		t0   = _mm_mul_ps(_mm_sub_ps(node.bMin[1], rayO[1]), rayInvD[1]);
		t1   = _mm_mul_ps(_mm_sub_ps(node.bMax[1], rayO[1]), rayInvD[1]);
		tMin = _mm_max_ps(tMin, _mm_min_ps(t0, t1));
		tMax = _mm_min_ps(tMax, _mm_max_ps(t0, t1));
		t0   = _mm_mul_ps(_mm_sub_ps(node.bMin[2], rayO[2]), rayInvD[2]);
		t1   = _mm_mul_ps(_mm_sub_ps(node.bMax[2], rayO[2]), rayInvD[2]);
		tMin = _mm_max_ps(tMin, _mm_min_ps(t0, t1));
		tMax = _mm_min_ps(tMax, _mm_max_ps(t0, t1));
		__m128 tMaxMin = _mm_sub_ps(tMax, tMin);
		// isHit = (tMin < tMax) && (r.tMin < tMax) && (r.tMax > tMin)
		isHit = _mm_cmplt_ps(tMin, tMax);	
		isHit = _mm_and_ps(isHit, _mm_cmplt_ps(rayTMin, tMax));
		isHit = _mm_and_ps(isHit, _mm_cmpgt_ps(rayTMax, tMin));

		// 确定子节点处理顺序
		char order[4];
		unsigned char& tOrder = node.traversalOrder[orderIndex];
		order[0] =  tOrder >> 6;
		order[1] = (tOrder >> 4) & 0x3;
		order[2] = (tOrder >> 2) & 0x3;
		order[3] =  tOrder & 0x3;

		// 按顺序处理子节点
		char totalPush = 0;
		for (int i = 0; i < 4; ++i)
		{
			int ithChild = order[i];
			const float* isHitf = (const float*)&isHit;
			const float* tMinf  = (const float*)&tMin;
			if (isHitf[ithChild] && tMinf[ithChild] < t && node.child[ithChild])
			{
				if (node.isLeaf[ithChild])
				{
					++numTraverseLeaf_IT;
					WMultiBVHLeaf& leaf = *(node.leaf[ithChild]);
#ifndef PER_TRIANGLE_INTERSECT_TEST
					__m128 rayO_u, rayO_v, rayO_w, rayO_xxx;
					__m128 rayD_u, rayD_v, rayD_w, rayD_xxx;

					__m128i axis_w = _mm_and_si128(_mm_srai_epi32(leaf.ci, 25), mask2);
					const int*  axis_wi = (const int*)&axis_w;

					rayO_u   = _mm_load_ps(rayOArray[axis_wi[0]]);
					rayD_u   = _mm_load_ps(rayDArray[axis_wi[0]]);
					rayO_v   = _mm_load_ps(rayOArray[axis_wi[1]]);
					rayD_v   = _mm_load_ps(rayDArray[axis_wi[1]]);
					rayO_w   = _mm_load_ps(rayOArray[axis_wi[2]]);
					rayD_w   = _mm_load_ps(rayDArray[axis_wi[2]]);
					rayO_xxx = _mm_load_ps(rayOArray[axis_wi[3]]);
					rayD_xxx = _mm_load_ps(rayDArray[axis_wi[3]]);

					_MM_TRANSPOSE4_PS(rayO_u, rayO_v, rayO_w, rayO_xxx);
					_MM_TRANSPOSE4_PS(rayD_u, rayD_v, rayD_w, rayD_xxx);
					//temporary variables 
					__m128 det, dett, detu, detv;
					__m128 nrv, nru, du, dv, ou, ov;
					/* ----ray-packet/triangle hit test ---- */ 
					//dett = np -(ou*nu+ov*nv+ow) 
					dett = leaf.np;
					dett = _mm_sub_ps(dett,rayO_w);								// dett = np - ow

					du = leaf.nu;
					dv = leaf.nv;
					ou = rayO_u;
					du = _mm_mul_ps(du, ou);									// du = nu * ou
					dett = _mm_sub_ps(dett, du);								// dett = np - ow - nu * ou
					ov = rayO_v;
					dv = _mm_mul_ps(dv, ov);									// dv = nv * ov
					dett = _mm_sub_ps(dett, dv);								// dett = np - ow - nu * ou - nv * ov
					//det =du*nu+dv*nv+dw 
					du = rayD_u;
					dv = rayD_v;
					det = leaf.nu;															
					det = _mm_mul_ps(det, du);									// det = nu * du
					nrv = leaf.nv; 
					nrv = _mm_mul_ps(nrv, dv);									// nrv = nv * dv
					det = _mm_add_ps(det, rayD_w);								// det = nu * du + dw
					det = _mm_add_ps(det, nrv);									// det = nu * du + nv * dv + dw
					//Du = du*dett - (pu-ou)*det 
					//ou = pu;
					ou = _mm_sub_ps(leaf.pu,rayO_u);								// ou = pu - ou
					du  = _mm_mul_ps(du, dett);									// du = du * dett
					nru = _mm_mul_ps(ou, det);									// nru = (pu - ou) * det
					du  = _mm_sub_ps(du, nru);									// du  = du * dett - (pu - ou) * det
					//Dv = dv*dett -  (pv-ov)*det 
					//ov = pv; 
					ov = _mm_sub_ps(leaf.pv, rayO_v);							// ov = pv - ov
					dv  = _mm_mul_ps(dv, dett);									// dv = dv * dett
					nrv = _mm_mul_ps(ov, det);									// nrv = (pv - ov) * det
					dv  = _mm_sub_ps(dv, nrv);									// dv  = dv * dett - (pv - ov) * det
					//detu = (e1vDu – e1u*Dv) 
					nru = leaf.e1v;
					nrv = leaf.e1u; 
					nru = _mm_mul_ps(nru, du);									// nru = e1v * du
					nrv = _mm_mul_ps(nrv, dv);									// nrv = e1v * dv
					detu = _mm_sub_ps(nru, nrv);								// detu = e1v * du - e1v * dv
					//detv = (e0uDv – e0v*Du) 
					nrv = leaf.e0u;
					nru = leaf.e0v;
					nru = _mm_mul_ps(nru, du);									// nru = e0v * du
					nrv = _mm_mul_ps(nrv, dv);									// nrv = e0u * dv
					detv = _mm_sub_ps(nrv, nru);								// detv = e0u * dv - e0v * du

					__m128 tmpdet0, tmpdet1; 
					tmpdet0 = _mm_sub_ps(det, detu);							// tmpdet0 = det - detu - detv
					tmpdet0 = _mm_sub_ps(tmpdet0, detv);
					tmpdet0 = _mm_xor_ps(tmpdet0, detu);
					tmpdet1 = _mm_xor_ps(detv, detu);
					tmpdet0 = _mm_andnot_ps(_mm_or_ps(tmpdet0, tmpdet1), mask1);// 如果tmpdet0符号位为1，表示有交点
					// 逼近法算 rdet = 1.0f / det
					//const float twof = 2.0f;
					__m128 two_m = _mm_set_ps1(2.0f);//_mm_load_ps1(&twof);
					__m128 rdet  = _mm_rcp_ps(det);								// rdet = 1.0f / det
					__m128 two_r = _mm_mul_ps(two_m, rdet);						// two_r = 2.0f * rdet
					__m128 det_r_r = _mm_mul_ps(det, _mm_mul_ps(rdet, rdet));	// det_r_r = det * rdet * rdet
					rdet = _mm_sub_ps(two_r, det_r_r);							// 较准确的 1.0f / det 值

					// 算出参数值
					__m128 tx4 = _mm_mul_ps(dett, rdet);

					for (int ithTri = 0; ithTri < 4; ithTri++)
					{
						WTriangle* tri = leaf.triangle[ithTri];
						const int*tmpdet0i= (const int*)&tmpdet0;
						const float* tx4f = (const float*)&tx4;
						numTriangleIsect_IT += (tri != NULL);
						if (tri && 
							tx4f[ithTri] < r.tMax   && 
							tx4f[ithTri] > r.tMin   && 
							tmpdet0i[ithTri])
						{
							nearestTri = tri;
							goto END;
						}
					}
#else
					for (int ithTri = 0; ithTri < 4; ithTri++)
					{
						WTriangle* tri = leaf.triangle[ithTri];
						if (!tri)
							break;
						{
							float temp = tri->intersectTest(r);
							if (temp < r.tMax)
							{
								nearestTri = tri;
								goto END;
							}
						}
					}
#endif
				}
				else
				{/*
					toPush[totalPush++] = ithChild;*/
					++stackTop;
					stackTop->node = node.child[ithChild];
					float* tMinf   = (float*)&tMin;
					stackTop->t    = tMinf[ithChild];
				}
			}
		}/*
		while (totalPush)
		{
			--totalPush;
			int ithChild = toPush[totalPush];
			++stackTop;
			stackTop->node = node.child[ithChild];
			float* tMinf   = (float*)&tMin;
			stackTop->t    = tMinf[ithChild];
		}*/
	}
END:
	delete[] stack;
	return (nearestTri != NULL);
}

void WMultiBVH::clearTree()
{
	freeAligned16(multiInteriors);
	freeAligned16(multiLeafs);
	multiInteriors = NULL;
	multiLeafs     = NULL;
	numInteriors   = numLeafs   = 0;
	WSimpleBVH::clearTree();
}

void WMultiBVH::drawTree( unsigned int nthBox/*=0*/, float R /*= 0.7*/, float G /*= 0.7*/, float B /*= 0.7*/ )
{
	if (numInteriors == 0)
		return;
	glColor3f(R, G, B);
	WMultiBVHNode* startNode = multiInteriors;
	drawTreeRecursive(startNode);
}

void WMultiBVH::displayNodeInfo()
{
	printf("interior nodes: %d\nleaf nodes: %d\n", numInteriors, numLeafs);
	int leafTris[5];
	leafTris[0] = leafTris[1] = leafTris[2] = leafTris[3] = leafTris[4] = 0;
	for (int ithNode = 0; ithNode < numLeafs; ++ithNode)
	{
		WMultiBVHLeaf& node = multiLeafs[ithNode];
		int i = 0;
		for (; i < 4; i++)
			if (!node.triangle[i])
				break;
		leafTris[i]++;
	}
	printf("node triangles:\n0:%d(%%%f)\n1:%d(%%%f)\n2:%d(%%%f)\n3:%d(%%%f)\n4:%d(%%%f)\naverage:%f\n", 
		leafTris[0], (float)leafTris[0] / numLeafs * 100.0f,
		leafTris[1], (float)leafTris[1] / numLeafs * 100.0f,
		leafTris[2], (float)leafTris[2] / numLeafs * 100.0f,
		leafTris[3], (float)leafTris[3] / numLeafs * 100.0f,
		leafTris[4], (float)leafTris[4] / numLeafs * 100.0f,
		(float)(leafTris[1] + leafTris[2] * 2 + leafTris[3] * 3 + leafTris[4] * 4) / (float)numLeafs);
}

void WMultiBVH::setBBoxToNode( WMultiBVHNode& node, const WBoundingBox& bBox, int ithChild )
{
	float* const bMinf = (float* const)&node.bMin;
	float* const bMaxf = (float* const)&node.bMax;

	bMinf[ithChild] = bBox.pMin.x;
	bMinf[ithChild + 4] = bBox.pMin.y;
	bMinf[ithChild + 8] = bBox.pMin.z;

	bMaxf[ithChild] = bBox.pMax.x;
	bMaxf[ithChild + 4] = bBox.pMax.y;
	bMaxf[ithChild + 8] = bBox.pMax.z;
}

void WMultiBVH::makeLeaf( WMultiBVHLeaf& dstLeaf, WBVHNode& srcLeaf )
{
	for (unsigned int ithTri = 0; ithTri < min(srcLeaf.nTriangles, 4); ++ithTri)
	{
		WTriangle* tri = srcLeaf.tris[ithTri];
#if !defined(PER_TRIANGLE_INTERSECT) && !defined(PER_TRIANGLE_INTERSECT_TEST)
		TriAccel & accel = tri->getAccelerateData();
		unsigned int* const cii = (unsigned int* const)&dstLeaf.ci;
		float* const nuf = (float* const)&dstLeaf.nu;
		float* const nvf = (float* const)&dstLeaf.nv;
		float* const npf = (float* const)&dstLeaf.np;
		float* const puf = (float* const)&dstLeaf.pu;
		float* const pvf = (float* const)&dstLeaf.pv;
		float* const e0uf = (float* const)&dstLeaf.e0u;
		float* const e0vf = (float* const)&dstLeaf.e0v;
		float* const e1uf = (float* const)&dstLeaf.e1u;
		float* const e1vf = (float* const)&dstLeaf.e1v;
		cii[ithTri] = accel.ci;
		nuf[ithTri] = accel.nu;
		nvf[ithTri] = accel.nv;
		npf[ithTri] = accel.np;
		puf[ithTri] = accel.pu;
		pvf[ithTri] = accel.pv;
		e0uf[ithTri] = accel.e0u;
		e0vf[ithTri] = accel.e0v;
		e1uf[ithTri] = accel.e1u;
		e1vf[ithTri] = accel.e1v;
#endif
		dstLeaf.triangle[ithTri] = tri;
	}
	for(int ithTri = min(srcLeaf.nTriangles, 4); ithTri < 4; ++ithTri)
		dstLeaf.triangle[ithTri] = NULL;
}

void WMultiBVH::computeTraversalOrder( WMultiBVHNode& dstNode, const WBVHNode& srcParent, const WBVHNode& srcLeft, const WBVHNode& srcRight )
{
	char splitP = srcParent.type;
	char splitL = srcLeft.type;
	char splitR = srcRight.type;
	char isLinternal = (splitL != WBVHNode::BVHN_LEAF);
	char isRinternal = (splitR != WBVHNode::BVHN_LEAF);
	for (int i = 0; i < 8; i++)
	{
		char order[4] = {0, 1, 2, 3};
		if (isLinternal && ((i >> splitL) & 0x1))
		{
			order[0] = 1;	order[1] = 0;
		}
		if (isRinternal && ((i >> splitR) & 0x1))
		{
			order[2] = 3;	order[3] = 2;
		}
		if ((i >> splitP) & 0x1)
		{
			swap(order[0], order[2]);
			swap(order[1], order[3]);
		}
		dstNode.traversalOrder[i] = (order[3] << 6) | (order[2] << 4) | (order[1] << 2) | order[0];
	}
}

void WMultiBVH::getBBoxFromSrcLeaf( WBVHNode& leaf, WBoundingBox& box )
{
	if (leaf.nTriangles == 0)
	{
		box = invalidBox;
		return;
	}
	box = WBoundingBox(*leaf.tris[0]);
	for (unsigned int i = 1; i < leaf.nTriangles; ++i)
	{
		box.merge(*leaf.tris[i]);
	}
}

void WMultiBVH::drawTreeRecursive( WMultiBVHNode* pNode )
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
			WMultiBVHLeaf& leaf = *node.leaf[i];
			for (int j = 0; j < 4; j++)
			{
				WTriangle* pTri = leaf.triangle[j];
				if (pTri)
				{
					pTri->draw();
				}
			}
		}
	}
}

