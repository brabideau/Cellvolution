#include <cmath>
#include <vector>
#include <random>

#include <GLFW/glfw3.h>
#pragma comment(lib, "opengl32.lib")



class cell
{
	public:
	double x, y; // position
	double dir_x, dir_y; // velocity
	double size, max_size, health, metabolism; // size
	double hue; // color
	int offspring; // number of offspring
	double o_size; // size of each offspring
};

// global variables 
std::vector<cell> flock;

float size = 20; //average size
float speed = .00025;
float maxspeed = 1;
float growth = .001;
int starting_number = 1;
float drift = .01;


void init_cell(double x, double y, float &speed, float &size) {

	std::random_device engine;
	auto uniform = std::uniform_real_distribution<float>(0, 1);
	auto normal = std::normal_distribution<float>(0, 1);
	auto intgen = std::uniform_int_distribution<int>(1, 5);
	
	cell c;

	c.x = x;
	c.y = y;
	c.dir_x = .25 * normal(engine);
	c.dir_y = .25 * normal(engine);
	c.size = abs(size + size * normal(engine) / 4 );
	c.max_size = c.size + size * abs(normal(engine));
	c.metabolism = growth + growth * uniform(engine);
	c.health = c.size * 2;
	c.hue = 3.14159 * uniform(engine); 
	c.offspring = intgen(engine);
	c.o_size = .25 * c.max_size * uniform(engine);

	flock.push_back(c);
}

cell cell_split(cell& c){

	std::random_device engine;
	auto uniform = std::uniform_real_distribution<float>(0, 1);
	auto normal = std::normal_distribution<float>(0, 1);
	auto intgen = std::uniform_int_distribution<int>(-1, 1);

	cell o;

	o.x = c.x;
	o.y = c.y;
	o.dir_x = .25 * normal(engine);
	o.dir_y = .25 * normal(engine);
	o.size = c.o_size * (1 + drift * normal(engine));
	o.max_size = c.max_size * (1 + drift * normal(engine));
	o.metabolism = c.metabolism * (1 + drift * normal(engine));
	o.health = o.size * 2;
	o.hue = c.hue * (1 + drift * normal(engine));
	o.offspring = abs(c.offspring + intgen(engine));
	o.o_size = c.o_size * (1 + drift * normal(engine));
	
	return o;
}



void on_mouse_button(GLFWwindow * win, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(win, &x, &y);

		// flock.push_back({ (float)x, (float)y, 0, 0, 30 });
		init_cell(x, y, speed, size);
	}
}


int main()
{
	int width, height;
	width = 1100;
	height = 700;

	glfwInit();
	GLFWwindow * win = glfwCreateWindow(width, height, "OpenGL example program", nullptr, nullptr);

	// glfwGetFramebufferSize(win, &width, &height);

	std::random_device pos_engine;
	auto init_pos = std::uniform_real_distribution<float>(0, 1);

	// generate all the initial cells
	for (int i = 0; i < starting_number; i++)
	{
		init_cell(width * init_pos(pos_engine), height * init_pos(pos_engine), speed, size);
	}

	glfwMakeContextCurrent(win);

	//glfwSetKeyCallback(win, on_key);
	glfwSetMouseButtonCallback(win, on_mouse_button);

	double t0 = glfwGetTime();
	while (!glfwWindowShouldClose(win)) // The actual animatoin starts
	{
		// Handle input
		glfwPollEvents();

		// Compute timestep
		double t1 = glfwGetTime();
		float timestep = (float)(t1 - t0);
		t0 = t1;

		// Update cell stats
		for (auto & c : flock)
		{
			// Update size & health:
			if (c.size < c.max_size) {
				c.size += c.metabolism;
			}

			if (c.health < c.size * 2){
				c.health += c.metabolism * 2;
			}


			// Movement calculation
			for (auto & target : flock)
			{
				if (&c == &target) continue;

				float dist = sqrt(pow(target.x - c.x, 2) + pow(target.y - c.y, 2));

				float factor = abs(c.hue - target.hue);
				if (factor > 3.14159 / 2) {
					factor = (3.14159 / 2) - factor;
				}

				factor = 3.14159 / 4 - factor;

				c.dir_x += speed * factor * (target.x - c.x) / dist;
				c.dir_y += speed * factor * (target.y - c.y) / dist;
			}

			c.x += c.dir_x;
			c.y += c.dir_y;

			if (c.x > width) { c.x -= width; }
			if (c.x < 0) { c.x += width; }

			if (c.y > height) { c.y -= height; }
			if (c.y < 0) { c.y += height; }


			std::vector<cell> litter;
			// Gives birth
			if (c.health >= c.max_size * 1.99) {
				for (int i = 0; i < c.offspring; i++)
				{
					litter.push_back(cell_split(c));
				}
			}

			for (auto & o : litter) {
				flock.push_back(o);
			}

		}


		// Draw some stuff
		glfwMakeContextCurrent(win);

		glClearColor(.05, .1, .3, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glLoadIdentity();
		glOrtho(0, width, height, 0, -1, +1);


		for (cell b : flock)
		{
			float hue = b.hue;

			glBegin(GL_POLYGON);
			glColor3f(abs(sin(hue)), abs(sin(hue + 1)), abs(sin(hue + 2)));
			glVertex2f(b.x + b.size, b.y + b.size);
			glVertex2f(b.x + b.size, b.y - b.size);
			glVertex2f(b.x - b.size, b.y - b.size);
			glVertex2f(b.x - b.size, b.y + b.size);
			glEnd();
		}



		glfwSwapBuffers(win);


		glfwDestroyWindow(win);
		glfwTerminate();
		return 0;

	}
} // end main

