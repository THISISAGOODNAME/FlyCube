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

#include <util.h>

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

static const char FRAGMENT_SHADERP_BOX[] =
"#version 300 es\n"
"precision mediump float;\n"
"out vec4 outColor;\n"
"void main() {\n"
"    outColor = vec4(0.75, 0.75, 0.75, 0.5);\n"
"}\n";

class PrintPolygon : public SceneBase, public SingleTon < PrintPolygon >
{
public:
	PrintPolygon()
	{
	}

	void add_point(glm::vec2 val)
	{
		f_input_line = false;

		point.push_back(val);
		if (point.size() > 1)
		{
			vec.push_back(line(point[point.size() - 2], val));
		}
		init_vao_wireframe();
	}

	void future_point(glm::vec2 val)
	{
		if (point.size() >= 1)
		{
			input_line = line(point[point.size() - 1], val);
			f_input_line = true;
		}
		init_vao_wireframe();
	}

	void finish_box()
	{
        vec_ray.clear();
        init_vao_ray();
		m_finish_box = true;
		if (!point.empty())
			point.pop_back();
		elements = { 0, 1, 2, 2, 3, 0 };
		init_vao_box();
		point.clear();
		//vec.clear();
	}

	void clear()
	{
		point.clear();
		vec.clear();
        vec_ray.clear();
        init_vao_ray();
		f_input_line = false;
		init_vao_wireframe();
		m_finish_box = false;
	}

	void clear_point()
	{
		f_input_line = false;
		point.clear();
		if (!vec.empty())
			vec.pop_back();
		init_vao_wireframe();
	}

	bool init()
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		mProgram = createProgram(VERTEX_SHADERP, FRAGMENT_SHADERP);

		mProgramBox = createProgram(VERTEX_SHADERP, FRAGMENT_SHADERP_BOX);

		if (!mProgram || !mProgramBox)
			return false;

		loc_u_m4MVP = glGetUniformLocation(mProgram, "u_m4MVP");
		loc_u_m4MVP_box = glGetUniformLocation(mProgramBox, "u_m4MVP");

		glGenVertexArrays(1, &vaoObjectPoint);
		glGenBuffers(1, &vboVertexPoint);

		glGenVertexArrays(1, &vaoObjectLine);
		glGenBuffers(1, &vboVertexLine);

		glGenVertexArrays(1, &vaoObjectcurLine);
		glGenBuffers(1, &vboVertexcurLine);


        glGenVertexArrays(1, &vaoObjectRay);
        glGenBuffers(1, &vboVertexRay);

		glGenVertexArrays(1, &vaoObjectcurBox);
		glGenBuffers(1, &vboVertexcurBox);
		glGenBuffers(1, &vboElemcurBox);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

		return isInit = true;
	}

	void init_vao_wireframe()
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

	void init_vao_box()
	{
		glBindVertexArray(vaoObjectcurBox);
		glBindBuffer(GL_ARRAY_BUFFER, vboVertexcurBox);
		glBufferData(GL_ARRAY_BUFFER, 2 * point.size() * sizeof(glm::vec2), point.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(POS_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(POS_ATTRIB);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElemcurBox);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(decltype(elements)::value_type), elements.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	void draw()
	{
		glClearColor(0.9411f, 0.9411f, 0.9411f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (!m_finish_box)
			draw_wireframe();
		else
		{
			draw_wireframe();
            draw_ray();
			draw_polygon();
		}
	}

    void set_ray(const std::vector<line> &vec)
    {
        vec_ray = vec;
        init_vao_ray();
    }

    void init_vao_ray()
    {
        glBindVertexArray(vaoObjectRay);
        glBindBuffer(GL_ARRAY_BUFFER, vboVertexRay);
        glBufferData(GL_ARRAY_BUFFER, 2 * vec_ray.size() * sizeof(glm::vec2), vec_ray.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(POS_ATTRIB, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(POS_ATTRIB);
        glBindVertexArray(0);
    }

    void draw_ray()
    {
        glUseProgram(mProgram);
        glUniformMatrix4fv(loc_u_m4MVP, 1, GL_FALSE, glm::value_ptr(Matrix));

        glBindVertexArray(vaoObjectRay);
        glDrawArrays(GL_LINES, 0, 2 * vec_ray.size());
        glBindVertexArray(0);
    }

	void draw_polygon()
	{
		glUseProgram(mProgramBox);
		glUniformMatrix4fv(loc_u_m4MVP_box, 1, GL_FALSE, glm::value_ptr(Matrix));

		glBindVertexArray(vaoObjectcurBox);
		glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glUseProgram(mProgram);
		glUniformMatrix4fv(loc_u_m4MVP, 1, GL_FALSE, glm::value_ptr(Matrix));

		glBindVertexArray(vaoObjectLine);
		glDrawArrays(GL_LINES, 0, 2 * vec.size());
		glBindVertexArray(0);
	}

	void draw_wireframe()
	{
		glLineWidth(2);
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
	line input_line;
	bool f_input_line;
	bool isInput;
	glm::mat4 Matrix;

	bool m_finish_box = false;

	std::vector<glm::vec2> point;
	std::vector<line> vec;
    std::vector<line> vec_ray;
	std::vector<GLuint> elements;

	int m_width;
	int m_height;
	int m_x;
	int m_y;
	bool isInit = false;

	GLuint mProgram;
	GLuint mProgramBox;

	GLuint vaoObjectcurLine;
	GLuint vboVertexcurLine;

	GLuint vaoObjectPoint;
	GLuint vboVertexPoint;

	GLuint vaoObjectLine;
	GLuint vboVertexLine;

	GLuint vaoObjectcurBox;
	GLuint vboVertexcurBox;
	GLuint vboElemcurBox;

    GLuint vaoObjectRay;
    GLuint vboVertexRay;

	GLint loc_u_m4MVP;
	GLint loc_u_m4MVP_box;
};