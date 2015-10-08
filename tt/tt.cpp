#include "include/GL/GLUT.H"
#include <math.h>
#include <vector>
#include <assert.h>
#include <time.h>
#include <iostream>

using namespace std;

#define PI 3.141592654

int eyeR = 10, eyeA = 0;
float cut = 0.8;

float Xright, Xleft, range;
float width, height;

struct Point2D
{
	bool inf;
	GLfloat x;
	GLfloat y;

	Point2D()
		:x(0)
		,y(0)
		,inf(false)
	{

	}

	Point2D(GLfloat a_x, GLfloat a_y)
		:x(a_x)
		,y(a_y)
		,inf(false)
	{

	}

	float DistanceSqure(const Point2D& a_point)
	{
		float xdiff = x - a_point.x;
		float ydiff = y - a_point.y;

		return xdiff * xdiff + ydiff * ydiff;
	}

	Point2D operator*(float a)
	{
		return Point2D(x * a, y * a);
	}
};

Point2D operator+(const Point2D& p1, const Point2D& p2)
{
	return Point2D(p1.x + p2.x, p1.y + p2.y);
}

Point2D operator-(Point2D& p1, Point2D& p2)
{
	return Point2D(p1.x - p2.x, p1.y - p2.y);
}

bool operator==(Point2D& p1, Point2D& p2)
{
	return p1.x == p2.x && p1.y == p2.y;
}

bool operator!=(Point2D& p1, Point2D& p2)
{
	return p1.x != p2.x || p1.y != p2.y;
}

typedef Point2D Vector2D;

struct Point3D
{
	GLfloat x;
	GLfloat y;
	GLfloat z;

	Point3D(GLfloat a_x, GLfloat a_y, GLfloat a_z)
		:x(a_x)
		,y(a_y)
		,z(a_z)
	{
		
	}
};

class Tool
{
public:
	Tool() : left(9, 12), right(12, 2), pos(10, 10) {}

	static Tool& GetIns() {static Tool ins; return ins;}

	void Move(Point2D& a_pos)
	{
		Point2D shift = a_pos - pos;

		pos = a_pos;
		left = left + shift;
		right = right + shift;
	}

	void Draw()
	{
		glPushMatrix();

		glTranslatef(pos.x, pos.y, 0);
		glutWireSphere(0.1, 15, 15);
		glPopMatrix();
	}

	const Point2D& GetPos() {return pos;}
	const Point2D& GetLeft() {return left;}
	const Point2D& GetRight() {return right;}

private:
	Point2D pos;
	Point2D left;
	Point2D right;
};

#define sTool Tool::GetIns()

typedef Point3D Vector3D;

struct EdgeIntersection
{
	EdgeIntersection(unsigned int& a_e)
		:e(a_e)
	{

	}

	unsigned e;
	Point2D p;
};

class Model
{
public:
	static Model& GetIns();

	void Set(std::vector<Point2D>& a_points);
	void Draw();
	void Cut(Point2D& a_pos);
    bool IsInModel(Point2D& a_point);
    bool IsOnEgde(Point2D& a_point);

private:
	Model();
	void ComputeNormal();
	void ComputeIntersection(Point2D& p1, Point2D& p2, std::vector<EdgeIntersection>& a_result);
	void InsertPoint(EdgeIntersection& a_first, Point2D& a_point, EdgeIntersection& a_last);

	std::vector<Point2D> m_points;
	std::vector<Vector2D> m_normals;
	int m_angle;
};
#define sModel Model::GetIns()

Model& Model::GetIns()
{
	static Model m_ins;
	return m_ins;
}

void Model::Draw()
{
	std::vector<Point2D>::iterator iter = m_points.begin();
	std::vector<Vector2D>::iterator nIter = m_normals.begin();
	do 
	{
		Point2D& pre = *iter;
		++iter;
		if (iter == m_points.end())
		{
			break;
		}
		Point2D& cur = *iter;

		glBegin(GL_QUAD_STRIP);
			glNormal3f(nIter->x, nIter->y, 0);
			glVertex3f(pre.x, pre.y, 0);
			glVertex3f(cur.x, cur.y, 0);

			for (int i = 0; i <= 360; i += m_angle)
			{
				Vector2D& normal = *nIter;
				GLfloat normalX = normal.x, 
						normalY = normal.y * cos(i * PI / 180), 
						normalZ = normal.y * sin(i * PI / 180);
				glNormal3f(normalX, normalY, normalZ);

				GLfloat newX = pre.x, 
						newY = pre.y * cos(i * PI / 180), 
						newZ = pre.y * sin(i * PI / 180);
				glVertex3f(newX, newY, newZ);

				newX = cur.x;
				newY = cur.y * cos(i * PI / 180);
				newZ = cur.y * sin(i * PI / 180);
				glVertex3f(newX, newY, newZ);
			}
		glEnd();

		++nIter;
	} while(iter != m_points.end());
}

void Model::Set( std::vector<Point2D>& a_points )
{
	m_points = a_points;
	ComputeNormal();
}

Model::Model()
:m_angle(3)
{
	
}

void Model::ComputeNormal()
{
	m_normals.clear();

	std::vector<Point2D>::iterator iter = m_points.begin();
	Point2D& tmp1 = *iter;
	++iter;
	Vector2D preV(iter->x - tmp1.x, iter->y - tmp1.y);
	Point2D& tmp2 = *iter;
	++iter;
	Vector2D curV(iter->x - tmp2.x, iter->y - tmp2.y);

	float side = ((preV.x * curV.x + preV.y * curV.y) > 0) ? -1.0 : 1.0;
	m_normals.push_back(Vector2D(-1.0 * side * preV.y, side * preV.x));

	while (iter != m_points.end())
	{
		m_normals.push_back(Vector2D(-1.0 * side * curV.y, side * curV.x));
		Point2D& preP = *iter;
		++iter;
		if (iter == m_points.end())
		{
			break;
		}
		Point2D& curP = *iter;

		curV.x = curP.x - preP.x;
		curV.y = curP.y - preP.y;
	}
}

float cross(Point2D& v1, Point2D& v2)
{
	// 沒有除法，儘量避免誤差。
	return v1.x * v2.y - v1.y * v2.x;
}

float cross(Point2D& o, Point2D& a, Point2D& b)
{
	return (a.x-o.x) * (b.y-o.y) - (a.y-o.y) * (b.x-o.x);
}

bool intersect(Point2D& p1, Point2D& p2, Point2D& p)
{
	return p.x >= fminf(p1.x, p2.x)
		&& p.x <= fmaxf(p1.x, p2.x)
		&& p.y >= fminf(p1.y, p2.y)
		&& p.y <= fmaxf(p1.y, p2.y);
}

float dot(Point2D& o, Point2D& a, Point2D& b)
{
	return (a.x-o.x) * (b.x-o.x) - (a.y-o.y) * (b.y-o.y);
}

Point2D intersectionLine(Point2D& a1, Point2D& a2, Point2D& b1, Point2D& b2)
{
	Point2D a = a2 - a1, b = b2 - b1, s = b1 - a1;

	// 兩線平行，交點不存在。
	// 兩線重疊，交點無限多。
	Point2D inf;
	inf.inf = true;
	if (cross(a, b) == 0) return inf;

	// 計算交點
	return a1 + a * (cross(s, b) / cross(a, b));
}

Point2D intersectionShare(Point2D& a1, Point2D& a2, Point2D& b1, Point2D& b2)
{
	// 確定交點後，剩下的線段端點必須��E韞磏I的不同側。
	if (a1 == b1 && dot(a1, a2, b2) <= 0) return a1;
	if (a1 == b2 && dot(a1, a2, b1) <= 0) return a1;
	if (a2 == b1 && dot(a2, a1, b2) <= 0) return a2;
	if (a2 == b2 && dot(a2, a1, b1) <= 0) return a2;

	// 交點無限多、交點不存在。
	Point2D inf;
	inf.inf = true;
	return inf;
}

Point2D intersectionSegment(Point2D& a1, Point2D& a2, Point2D& b1, Point2D& b2)
{
	double c1 = cross(a1, a2, b1);
	double c2 = cross(a1, a2, b2);
	double c3 = cross(b1, b2, a1);
	double c4 = cross(b1, b2, a2);

	if (c1 * c2 < 0 && c3 * c4 < 0)
		return intersectionLine(a1, a2, b1, b2);

	if (c1 == 0 && c2 == 0)
		return intersectionShare(a1, a2, b1, b2);

	// 兩線不平行、不共線、不交叉，有可能接觸於一點。
	if (c1 == 0 && intersect(a1, a2, b1)) return b1;
	if (c2 == 0 && intersect(a1, a2, b2)) return b2;
	if (c3 == 0 && intersect(b1, b2, a1)) return a1;
	if (c4 == 0 && intersect(b1, b2, a2)) return a2;

	Point2D inf;
	inf.inf = true;
	return inf;
}

// bool point_in_polygon(Point& t)
// {
// 	bool c = false;
// 	for (int i = 0, j = 10-1; i < 10; j = i++)
// 		if ((p[i].y > t.y) != (p[j].y > t.y) &&
// 			t.x < (p[j].x-p[i].x)*(t.y-p[i].y)/(p[j].y-p[i].y)+p[i].x;)
// 			c = !c;
// 	return c;
// }

void Draw()
{
	sModel.Draw();
	sTool.Draw();
// 	glDisable(GL_LIGHTING);
// 	int size = 150;
// 	for (int i = 0; i < size; ++i)
// 	{
// 		for (int j = 0; j < size; ++j)
// 		{
// 			for (int k = 0; k < size; ++k)
// 			{
// 				glBegin(GL_POINT);
// 					glVertex3f(i, j, k);
// 				glEnd();
// 			}
// 		}
// 	}
}

void Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Draw();

	glutSwapBuffers();
}

void RC()
{
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat amb[] = {0.3, 0.3, 0.3, 1.0};
	GLfloat diff[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat pos[] = {10.0, 10.0, 10.0, 0.0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void ChangeCamera()
{
	GLdouble x = eyeR * cos(eyeA * 3.1416 / 180.0);
	GLdouble y = eyeR * sin(eyeA * 3.1416 / 180.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(x, 0, y, 0, 0, 0, 0, 1, 0);

}

void Resize(GLsizei w, GLsizei h)
{
	if (h == 0)
	{
		h = 1;
	}

	width = w;
	height = h;
	Xright = tan(45.0 * PI / 180 / 2.0) * width / height * 1.0;
	Xleft = -Xright;

	glViewport(0, 0, w, h);

	GLfloat aspect = (GLfloat)w / (GLfloat)h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, aspect, 1.0, 425.0);

	ChangeCamera();
}

void keyfunc(unsigned char key, int x, int y)
{
	

	switch (key)
	{
	case 'w': ++eyeR; break;
	case 's': --eyeR; break;
	case 'a': ++eyeA; break;
	case 'd': --eyeA; break;
	case '1': sModel.Cut(sTool.GetPos() + Point2D(-0.1, -0.1));break;
	case '2': sModel.Cut(sTool.GetPos() + Point2D(0, -0.1));break;
	case '3': sModel.Cut(sTool.GetPos() + Point2D(0.1, -0.1));break;
	case '4': sModel.Cut(sTool.GetPos() + Point2D(-0.1, 0));break;
	case '6': sModel.Cut(sTool.GetPos() + Point2D(0.1, 0));break;
	case '7': sModel.Cut(sTool.GetPos() + Point2D(-0.1, 0.1));break;
	case '8': sModel.Cut(sTool.GetPos() + Point2D(0, 0.1));break;
	case '9': sModel.Cut(sTool.GetPos() + Point2D(0.1, 0.1));break;
	}

	ChangeCamera();

	glutPostRedisplay();
}

time_t last = time(NULL);
int fps = 0;

void Idle()
{
	time_t current = time(NULL);

	if (current != last)
	{
		last = current;
		cout << fps << endl;
		fps = 0;
	}

	Render();

	++fps;
}

void BuildModel()
{
	std::vector<Point2D> points;

	points.push_back(Point2D(0.0, 0.0));
	points.push_back(Point2D(0.0, 2.0));
	points.push_back(Point2D(3.0, 2.0));
	points.push_back(Point2D(3.0, 0.0));
	sModel.Set(points);

	sTool.Move(Point2D(4, 1.5));

	Point2D p(0, 1);
	sModel.IsInModel(p);
}

void main()
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("tt");
	glutDisplayFunc(Render);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(keyfunc);
	glutIdleFunc(Idle);

	RC();
	BuildModel();

	glutMainLoop();
}

void RemoveDuplication(std::vector<EdgeIntersection>& a_data)
{
	if (a_data.size() > 1)
	{
		std::vector<EdgeIntersection>::iterator pre = a_data.begin();
		std::vector<EdgeIntersection>::iterator post = pre + 1;
		for (; post != a_data.end();)
		{
			if (pre->p == post->p)
			{
				pre = a_data.erase(pre);
				post = pre + 1;
			}
			else
			{
				++pre;
				++post;
			}
		}
	}
}


void Model::Cut(Point2D& a_pos)
{
	Point2D oriTool = sTool.GetPos();
	sTool.Move(a_pos);
	
	std::vector<EdgeIntersection> toolResult;
	ComputeIntersection(a_pos, oriTool, toolResult);

	if (!toolResult.empty())
	{
		RemoveDuplication(toolResult);

		if (toolResult.size() == 1)
		{
			EdgeIntersection& toolPoint = *toolResult.begin();
			if (toolPoint.p == a_pos || (toolPoint.p == oriTool && !IsInModel(a_pos)) || (IsInModel(oriTool) && !IsInModel(a_pos)))
			{
				return;
			}
			else
			{
				Point2D up(a_pos.x, 10);
				std::vector<EdgeIntersection> upResult;
				ComputeIntersection(a_pos, up, upResult);
				RemoveDuplication(upResult);

				if (upResult.size() == 1)
				{
					EdgeIntersection& upPoint = *upResult.begin();
					if (upPoint.e < toolPoint.e)
					{
						InsertPoint(upPoint, a_pos, toolPoint);
					}
					else if (upPoint.e == toolPoint.e)
					{
						if (m_points[upPoint.e].DistanceSqure(upPoint.p) > m_points[upPoint.e].DistanceSqure(toolPoint.p))
						{
							InsertPoint(toolPoint, a_pos, upPoint);
						}
						else
						{
							InsertPoint(upPoint, a_pos, toolPoint);
						}
					}
					else
					{
						InsertPoint(toolPoint, a_pos, upPoint);
					}
				}
				else
				{
					assert(false);
				}
			}
		}
		else if (toolResult.size() == 2)
		{
			EdgeIntersection& first = *toolResult.begin();
			EdgeIntersection& last= *toolResult.rbegin();
			assert(first.e != last.e && first.p != last.p);

			Point2D p;
			p.inf = true;
			InsertPoint(first, p, last);
		}
		else
		{
			assert(false);
		}

// 		Point2D up(a_pos.x, 10);
// 		std::vector<EdgeIntersection> upResult;
// 		ComputeIntersection(a_pos, up, upResult);
// 		
// 		if (upResult.empty())
// 		{
// 			EdgeIntersection& firstPoint = *toolResult.begin();
// 			unsigned int removeStart = firstPoint.e + 1;
// 			unsigned int removeEnd = removeStart + 1;
// 			if (toolResult.size() == 2)
// 			{
// 				EdgeIntersection& lastPoint = *toolResult.rbegin();
// 				removeEnd = lastPoint.e + 1;
// 				assert(removeEnd >= removeStart);
// 			}
// 			else
// 			{
// 				assert(false);
// 			}
// 
// 			m_points.erase(m_points.begin() + removeStart, m_points.begin() + removeEnd);
// 			
// 			unsigned int insertPos = removeStart;
// 			for (std::vector<EdgeIntersection>::iterator iter = toolResult.begin();
// 				iter != toolResult.end();
// 				++iter)
// 			{
// 				m_points.insert(m_points.begin() + insertPos++, iter->p);
// 			}
// 		}
// 		else
// 		{
// 			EdgeIntersection& toolPoint = *toolResult.begin();
// 			EdgeIntersection& upPoint = *upResult.begin();
// 			
// 			std::vector<EdgeIntersection> firstInsert;
// 			std::vector<EdgeIntersection> lastInsert;
// 
// 			if (toolPoint.e == upPoint.e)
// 			{
// 				if (m_points[toolPoint.e].DistanceSqure(toolPoint.p) > m_points[toolPoint.e].DistanceSqure(upPoint.p))
// 				{
// 					firstInsert = upResult;
// 					lastInsert = toolResult;
// 				}
// 				else if (toolPoint.p == upPoint.p)
// 				{
// 					firstInsert = upResult;
// 					lastInsert = toolResult;
// 				}
// 				else
// 				{
// 					firstInsert = toolResult;
// 					lastInsert = upResult;
// 				}
// 			}
// 			else if (toolPoint.e > upPoint.e)
// 			{
// 				firstInsert = upResult;
// 				lastInsert = toolResult;
// 			}
// 			else
// 			{
// 				firstInsert = toolResult;
// 				lastInsert = upResult;
// 			}
// 
// 			EdgeIntersection& firstPoint = *firstInsert.rbegin();
// 			unsigned int removeStart = firstPoint.e + 1;
// 			EdgeIntersection& lastPoint = *lastInsert.begin();
// 			unsigned int removeEnd = lastPoint.e + 1;
// 			assert(removeEnd >= removeStart);
// 			m_points.erase(m_points.begin() + removeStart, m_points.begin() + removeEnd);
// 
// 			unsigned int insertPos = removeStart;
// 			for (std::vector<EdgeIntersection>::iterator iter = firstInsert.begin();
// 				iter != firstInsert.end();
// 				++iter)
// 			{
// 				m_points.insert(m_points.begin() + insertPos++, iter->p);
// 			}
// 
// 			m_points.insert(m_points.begin() + insertPos++, a_pos);
// 
// 			for (std::vector<EdgeIntersection>::iterator iter = lastInsert.begin();
// 				iter != lastInsert.end();
// 				++iter)
// 			{
// 				m_points.insert(m_points.begin() + insertPos++, iter->p);
// 			}
// 		}

		ComputeNormal();
	}
}

void Model::ComputeIntersection( Point2D& p1, Point2D& p2, std::vector<EdgeIntersection>& a_result )
{
	for (unsigned int i = 0; i < m_points.size(); ++i)
	{
		EdgeIntersection tmpResult(i);

		unsigned int edgeEnd = (i + 1 == m_points.size()) ? 0 : i + 1;
		tmpResult.p = intersectionSegment(m_points[i], m_points[edgeEnd], p1, p2);
		if (!tmpResult.p.inf)
		{
			a_result.push_back(tmpResult);
		}
	}
}

bool Model::IsInModel( Point2D& a_point )
{
	bool c = IsOnEgde(a_point);
	if (c)
	{
		return true;
	}
	int size = m_points.size();
	for (int i = 0, j = size - 1; i < size; j = i++)
		if ((m_points[i].y > a_point.y) != (m_points[j].y > a_point.y) &&
			a_point.x < (m_points[j].x-m_points[i].x)*(a_point.y-m_points[i].y)/(m_points[j].y-m_points[i].y)+m_points[i].x)
			c = !c;
	return c;
}

void Model::InsertPoint( EdgeIntersection& a_first, Point2D& a_point, EdgeIntersection& a_last )
{
	unsigned int insertPos = a_first.e + 1;
	m_points.erase(m_points.begin() + insertPos, m_points.begin() + a_last.e + 1);

	m_points.insert(m_points.begin() + insertPos++, a_first.p);
	if (!a_point.inf)
	{
		m_points.insert(m_points.begin() + insertPos++, a_point);
	}
	m_points.insert(m_points.begin() + insertPos++, a_last.p);
}

bool Model::IsOnEgde( Point2D& a_point )
{
	int size = m_points.size();
	for (int i = 0, j = size - 1; i < size; j = i++)
	{
		if (cross(a_point, m_points[i], m_points[j]) == 0 && dot(a_point, m_points[i], m_points[j]) > 0)
			return true;
	}

	return false;
}
