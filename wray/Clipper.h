#pragma once

class WBoxClipper
{
	enum AxisType{X_AXIS = 0, Y_AXIS = 1, Z_AXIS = 2};
public:
	WBoxClipper(void);
	WBoxClipper(const WBoundingBox& box);
	~WBoxClipper(void);
	void setClipBox(const WBoundingBox& box);
	void setClipBox(float box[2][3]);
	void displayPolygon(vector<WVector3>& vertices);

	//剪切三角形
	bool clipTriangle(const WTriangle& tri,
							vector<WVector3>& outVertices);

	//bool getClipTriangleBox(const WTriangle& tri,WBoundingBox& box);

	void drawPolygon(vector<WVector3>& vertices);

	bool getClipBox(const WTriangle& tri, float resultBox[2][3]);

private:
	//用于剪切的bbox,会设得大一些
	WBoundingBox clipBox;
	//包围盒数组，按照 minX minY minZ maxX maxY maxZ
	float boxArray[2][3];



};
