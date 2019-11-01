// NoshiningC.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/freeglut.h>
constexpr size_t WINDOWSIZE_X = 800, WINDOWSIZE_Y = 800;
constexpr size_t TERRSIZE = 50;
constexpr size_t RAYNUM = 720;
constexpr size_t ILLUMINATION = 10;
std::string mappath = R"(D:\DataExchange\NoshiningMap\map50.csv)";
using TERRMAP = int[TERRSIZE][TERRSIZE];
using RAY = float[4];
inline bool equ0(double x)
{
	return fabs(x) < 2 * FLT_EPSILON;
}

inline int objonmap(size_t x, size_t y, TERRMAP tm)
{
	return tm[x][y];
}

bool projectray(RAY ray, TERRMAP m)
{
	float tpast = 0;
	if (objonmap(ray[0], ray[1], m) == 1)
		return true;
	while (true)
	{
		float nextcollision[2];
		if (equ0(ray[2]))
			nextcollision[0] = 1000;
		else
		{
			int nextborder = ray[2] > 0 ? 1 : 0;
			nextcollision[0] = (nextborder - (ray[0] - floor(ray[0]))) / (ray[2]);
		}

		if (equ0(ray[3]))
			nextcollision[1] = 1000;
		else
		{
			int nextborder = ray[3] > 0 ? 1 : 0;
			nextcollision[1] = (nextborder - (ray[1] - floor(ray[1]))) / (ray[3]);
		}


		float nextcollisiontime = 0.001; //+0.1 to enter surface, or skip over thinned surface
		if (nextcollision[0] > nextcollision[1])
			nextcollisiontime += nextcollision[1];
		else
			nextcollisiontime += nextcollision[0];

		tpast += nextcollisiontime;
		if (tpast > 1.4142 * TERRSIZE)
			throw "unable to project ray: endless map!!!";

		ray[0] += ray[2] * nextcollisiontime;
		ray[1] += ray[3] * nextcollisiontime;
		size_t onblockposx = size_t(ray[0]), onblockposy = size_t(ray[1]);
		if (onblockposx < 0 || onblockposx >= TERRSIZE || onblockposy < 0 || onblockposy >= TERRSIZE)
			return false;
		if (objonmap(ray[0], ray[1], m) == 1)
			return true;
	}
}

void map_csv_read(std::string path, TERRMAP m)
{
	//读文件
	std::ifstream inFile(path, std::ios::in);
	std::string lineStr;
	size_t x = 0, y = 0;
	while (std::getline(inFile, lineStr))
	{
		std::stringstream ss(lineStr);//来自sstream
		std::string str;
		while (getline(ss, str, ','))
		{
			if (str.length() == 0)
				m[x][y] = 0;
			else
				m[x][y] = atoi(str.c_str());
			x++;
		}
		x = 0;
		y++;
	}
}

void DrawMap(TERRMAP m, float rayorg[2])
{
	TERRMAP shinedmap = { 0 };
	memcpy(shinedmap, m, TERRSIZE * TERRSIZE * sizeof(int));

	for (long long i = 0; i < RAYNUM; i++)
	{
		float deg = i * 2 * M_PI / RAYNUM;
		RAY ray = { rayorg[0],rayorg[1],cos(deg),sin(deg) };
		bool ret = projectray(ray, m);
		if (ret)
			shinedmap[int(ray[0])][int(ray[1])]++;
	}
	for (long long x = 0; x < TERRSIZE; x++)
	{
		for (size_t y = 0; y < TERRSIZE; y++)
		{
			switch (shinedmap[x][y])
			{
			case 0:
				continue;
				break;
			case 1:
				glColor3f(0, 0.1, 0.1);
				break;
			default:
				glColor3f(shinedmap[x][y] / float(RAYNUM) * ILLUMINATION, 0.1, 0.1);
				break;
			}
			float width = 2.0 / TERRSIZE;
			glRectf(x * width - 1, y * width - 1, (x + 1) * width - 1, (y + 1) * width - 1);
		}
	}
}

size_t frameidx = 0;
TERRMAP terrmap;
float posRaysource[2] = { 5.5,5.5 };
void drawFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);
	DrawMap(terrmap, posRaysource);
	glutSwapBuffers();
	frameidx += 1;
}

void mouseFunc(int x, int y)
{
	posRaysource[0] = float(TERRSIZE) * x / WINDOWSIZE_X;
	posRaysource[1] = float(TERRSIZE) * (WINDOWSIZE_Y - y) / WINDOWSIZE_Y;
}

int main(int argc, char** argv)
{
	//prog init
	map_csv_read(mappath, terrmap);

	//gl init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glClearColor(0.0, 0.0, 0.7, 1.0);
	glutInitWindowSize(WINDOWSIZE_X, WINDOWSIZE_Y);
	glutCreateWindow("Noshining");
	glutDisplayFunc(drawFunc);
	glutIdleFunc(drawFunc);
	glutMotionFunc(mouseFunc);
	glutMainLoop();
}
