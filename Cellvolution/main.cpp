#include <cmath>
#include <vector>
#include <random>
#include <algorithm>

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

float size = 20; //average size
float speed = 5;
double maxspeed = .001;
float growth = .001;
int start_population = 5;
int max_population = 100;
float drift = .03;

int width = 1100;
int height = 700;

std::vector<cell> flock;
std::vector<cell> litter;
//std::flock.reserve(100);


cell cell_new(double x, double y, float &speed, float &size) {

	std::random_device engine;
	auto uniform = std::uniform_real_distribution<float>(0, 1);
	auto normal = std::normal_distribution<float>(0, 1);
	auto intgen = std::uniform_int_distribution<int>(1, 5);
	
	cell c;

	c.x = x;
	c.y = y;
	c.dir_x = speed * normal(engine);
	c.dir_y = speed * normal(engine);
	c.size = abs(size + size * normal(engine) / 4 );
	c.max_size = c.size + size * abs(normal(engine));
	c.metabolism = growth + growth * uniform(engine);
	c.health = c.size * 2;
	c.hue = 3.14159 * uniform(engine); 
	c.offspring = intgen(engine);
	c.o_size = c.max_size * (uniform(engine)+1) *.125;

	return c;
}

cell cell_split(cell& c){

	std::random_device engine;
	auto uniform = std::uniform_real_distribution<float>(0, 1);
	auto normal = std::normal_distribution<float>(0, 1);
	auto intgen = std::uniform_int_distribution<int>(-1, 1);

	cell o;

	o.x = c.x + c.size * normal(engine);
	o.y = c.y + c.size * normal(engine);
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

		flock.push_back(cell_new(x, y, speed, size));
		
	}
}


int main()
{

	glfwInit();
	GLFWwindow * win = glfwCreateWindow(width, height, "Cellvolution", nullptr, nullptr);

	// glfwGetFramebufferSize(win, &width, &height);

	std::random_device pos_engine;
	auto init_pos = std::uniform_real_distribution<float>(0, 1);

	// generate all the initial cells
	for (int i = 0; i < start_population; i++)
	{
		flock.push_back(cell_new(width * init_pos(pos_engine), height * init_pos(pos_engine), speed, size));

	}

	glfwMakeContextCurrent(win);

	//glfwSetKeyCallback(win, on_key);
	glfwSetMouseButtonCallback(win, on_mouse_button);

	double t0 = glfwGetTime();

	while (!glfwWindowShouldClose(win)) // The actual animation starts
	{
		// Handle input
		glfwPollEvents();

		// Compute timestep
		double t1 = glfwGetTime();
		float timestep = (float)((t1 - t0)/8);
		t0 = t1;

		// Make the window
		glfwMakeContextCurrent(win);

		glClearColor(.05, .1, .3, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glLoadIdentity();
		glOrtho(0, width, height, 0, -1, +1);

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

				if (dist == 0) continue;

				float factor = abs(c.hue - target.hue);
				if (factor > 3.14159 / 2) {
					factor = (3.14159 / 2) - factor;
				}

				factor = 3.14159 / 4 - factor;

				c.dir_x += factor * (target.x - c.x) / dist;
				//c.dir_x = std::min(c.dir_x, maxspeed);
				c.dir_y += factor * (target.y - c.y) / dist;
			//	c.dir_y = std::min(c.dir_y, maxspeed);
			}

			c.x += c.dir_x * timestep; 
			c.y += c.dir_y * timestep;


			// Wrap on edge of screen
			//if (c.x > width || c.x < 0) {
			//	c.x -= width * abs(c.x) / c.x;
			//}

			//if (c.y > height || c.y < 0) {
			//	c.y -= height * abs(c.y) / c.y;
			//}

			if (c.x > width) { c.x -= width; }
			if (c.x < 0) { c.x += width; }

			if (c.y > height) { c.y -= height; }
			if (c.y < 0) { c.y += height; }


			// Gives birth

			if (c.health >= c.max_size * 1.99) {
				float total_size = 0;
				for (int i = 0; i < c.offspring; i++)
				{
					cell o = cell_split(c);
					litter.push_back(o);
					total_size += o.size;
				}

				c.health -= total_size;
			}


			// Draw the cell
			float hue = c.hue;

			glBegin(GL_POLYGON);
			glColor3f(abs(sin(hue)), abs(sin(hue + 1)), abs(sin(hue + 2)));
			glVertex2f(c.x + c.size, c.y + c.size);
			glVertex2f(c.x + c.size, c.y - c.size);
			glVertex2f(c.x - c.size, c.y - c.size);
			glVertex2f(c.x - c.size, c.y + c.size);
			glEnd();
		}

		glfwSwapBuffers(win);


		for (auto & o : litter) {
			cell new_cell{ o.x, o.y, o.dir_x, o.dir_y, o.size, o.max_size, o.health, o.metabolism, o.hue, o.offspring, o.o_size };

			flock.push_back(new_cell);
		}

		litter.clear();
	}


		


		glfwDestroyWindow(win);
		glfwTerminate();
		return 0;

	} // end main

