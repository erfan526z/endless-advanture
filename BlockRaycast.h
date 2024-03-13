#pragma once

#include <glm/glm.hpp>

class BlockRaycast
{
public:
	/*
	Returns: 0: none, 1: x-, 2: x+, 3: y-, 4: y+, 5: z-, 6: z+
	*/
	int cast(glm::vec3 pos, glm::vec3 ray, int& target_x, int& target_y, int& target_z, int x_offset, int z_offset, float reach, float step, unsigned short int (*getBlock)(int, int, int), bool (*canBeTargeted)(unsigned short int)) {
		
		int lastgx = 0, lastgy = -1, lastgz = 0;

		for (float dist = 0.0f; dist < reach; dist += step) {
			glm::vec3 current = pos + ray * dist;
			int glob_x = (int)current.x + x_offset;
			int glob_y = (int)current.y;
			int glob_z = (int)current.z + z_offset;

			if (lastgx == glob_x && lastgy == glob_y && lastgz == glob_z) continue;
			lastgx = glob_x; lastgy = glob_y; lastgz = glob_z;

			if ((current.x < 0.0f))
				glob_x--;
			if ((current.z < 0.0f))
				glob_z--;
			
			if (canBeTargeted(getBlock(glob_x, glob_y, glob_z))) {
				target_x = glob_x;
				target_y = glob_y;
				target_z = glob_z;

				float txbeg = (ray.x > 0) ? ((float)(target_x - x_offset) - pos.x) : ((float)(target_x - x_offset + 1) - pos.x);
				
				float tybeg = (ray.y > 0) ? ((float)(target_y) - pos.y) : ((float)(target_y + 1) - pos.y);
				
				float tzbeg = (ray.z > 0) ? ((float)(target_z - z_offset) - pos.z) : ((float)(target_z - z_offset + 1) - pos.z);
				
				float facxbeg = ray.x / txbeg;
				float facybeg = ray.y / tybeg;
				float faczbeg = ray.z / tzbeg;
				
				float tbeg = min3f(facxbeg, facybeg, faczbeg);

				if (facxbeg == tbeg) {
					if (ray.x > 0)
						return 1;
					else
						return 2;
				}
				if (facybeg == tbeg) {
					if (ray.y > 0)
						return 3;
					else
						return 4;
				}
				if (faczbeg == tbeg) {
					if (ray.z > 0)
						return 5;
					else
						return 6;
				}

				return 7;
			}
		}

		return 0;
	}

private:
	
	float max3f(float a, float b, float c) {
		if (a > b && a > c)
			return a;
		if (b > a && b > c)
			return b;
		return c;
	}

	float min3f(float a, float b, float c) {
		if (a <= 0) a = 99.0f;
		if (b <= 0) b = 99.0f;
		if (c <= 0) c = 99.0f;
		if (a < b && a < c)
			return a;
		if (b < a && b < c)
			return b;
		return c;
	}

};

