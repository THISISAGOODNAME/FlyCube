#pragma once
#include <platform.h>
#include <utilities.h>
#include <scenebase.h>
#include <algorithm>
#include <math.h>
#include <string>
#include <vector>
#include <list>
#include <ctime>
#include <chrono>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define NORMAL_ATTRIB 1
#define TEXTURE_ATTRIB 2

static const char VERTEX_SHADERP[] =
"#version 300 es\n"
"precision mediump float;\n"
"layout(location = " STRV(POS_ATTRIB) ") in vec2 pos;\n"
"uniform mat4 u_m4MVP;\n"
"void main() {\n"
"    gl_Position = u_m4MVP * vec4(pos, 0.0, 1.0);\n"
"}\n";

static const char FRAGMENT_SHADERP[] =
"#version 300 es\n"
"precision mediump float;\n"
"out vec4 outColor;\n"
"void main() {\n"
"    outColor.rgb = vec3(0.0588, 0.0588, 0.0588);\n"
"    outColor.a = 1.0;\n"
"}\n";

class PrintPolygon : public SceneBase
{
public:
	PrintPolygon()
	{
	}

    void add_point(glm::vec2 val)
	{
		if (!isInput)
			return;
		f_input_line = false;
		point.push_back(val);
		if (point.size() > 1)
		{
			vec.push_back(line(point[point.size() - 2], val));
		}
		init_vao();
	}

    void next_point(glm::vec2 val)
	{
		if (!isInput)
			return;
		if (point.size() >= 1)
		{
			input_line = line(point[point.size() - 1], val);
			f_input_line = true;
		}
		init_vao();
	}

	void endInpit()
	{
		isInput = false;

	}

	void beginInpit()
	{
		isInput = true;
		clear();
	}


	void clear()
	{
		point.clear();
		vec.clear();
		f_input_line = false;
		init_vao();

	}

	bool init()
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		mProgram = createProgram(VERTEX_SHADERP, FRAGMENT_SHADERP);
		if (!mProgram)
			return false;

		loc_u_m4MVP = glGetUniformLocation(mProgram, "u_m4MVP");

		glGenVertexArrays(1, &vaoObjectPoint);
		glGenBuffers(1, &vboVertexPoint);

		glGenVertexArrays(1, &vaoObjectLine);
		glGenBuffers(1, &vboVertexLine);

		glGenVertexArrays(1, &vaoObjectcurLine);
		glGenBuffers(1, &vboVertexcurLine);

		return isInit = true;
	}

	void init_vao()
	{
		glBindVertexArray(vaoObjectPoint);
		glBindBuffer(GL_ARRAY_BUFFER, vboVertexPoint);
		glBufferData(GL_ARRAY_BUFFER, 2 * point.size() * sizeof(glm::vec2), point.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(POS_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(POS_ATTRIB);
		glBindVertexArray(0);

		glBindVertexArray(vaoObjectLine);
		glBindBuffer(GL_ARRAY_BUFFER, vboVertexLine);
		glBufferData(GL_ARRAY_BUFFER, 2 * vec.size() * sizeof(glm::vec2), vec.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(POS_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(POS_ATTRIB);
		glBindVertexArray(0);

		glBindVertexArray(vaoObjectcurLine);
		glBindBuffer(GL_ARRAY_BUFFER, vboVertexcurLine);
		glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), &input_line, GL_STATIC_DRAW);
		glVertexAttribPointer(POS_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(POS_ATTRIB);
		glBindVertexArray(0);
	}

	void draw()
	{
		glClearColor(0.9411f, 0.9411f, 0.9411f, 1.0f);
		glLineWidth(2);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(mProgram);
		glUniformMatrix4fv(loc_u_m4MVP, 1, GL_FALSE, glm::value_ptr(Matrix));

		glBindVertexArray(vaoObjectLine);
		glDrawArrays(GL_LINES, 0, 2 * vec.size());
		glBindVertexArray(0);

		if (f_input_line)
		{
			glBindVertexArray(vaoObjectcurLine);
			glDrawArrays(GL_LINES, 0, 2);
			glBindVertexArray(0);

		}

		glBindVertexArray(vaoObjectPoint);
		glDrawArrays(GL_POINTS, 0, point.size());
		glBindVertexArray(0);

	}

	void resize(int x, int y, int width, int height)
	{
		m_x = x;
		m_y = y;
		m_width = width;
		m_height = height;

		glViewport(m_x, m_y, m_width, m_height);
		GLfloat ratio = (GLfloat)m_height / (GLfloat)m_width;
		if (m_width >= m_height)
			Matrix = glm::ortho(-1.0f / ratio, 1.0f / ratio, -1.0f, 1.0f, -10.0f, 1.0f);
		else
			Matrix = glm::ortho(-1.0f, 1.0f, -1.0f*ratio, 1.0f*ratio, -10.0f, 1.0f);
	}
	void destroy()
	{
	}
private:
	using line = std::pair < glm::vec2, glm::vec2 >;
	line input_line;
	bool f_input_line;
	bool isInput;
	glm::mat4 Matrix;

	std::vector<glm::vec2> point;
	std::vector<line> vec;

	int m_width;
	int m_height;
	int m_x;
	int m_y;
	bool isInit = false;

	GLuint mProgram;

	GLuint vaoObjectcurLine;
	GLuint vboVertexcurLine;

	GLuint vaoObjectPoint;
	GLuint vboVertexPoint;

	GLuint vaoObjectLine;
	GLuint vboVertexLine;

	GLint loc_u_m4MVP;
};