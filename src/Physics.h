#pragma once

#include <glm/glm.hpp>

struct PlayerAABB {
	PlayerAABB() {
		xs = xe = ys = ye = zs = ze = 0.0f;
	}

	PlayerAABB(float cx, float cz, float legy, float w, float h) {
		xs = cx - (w / 2.0f);
		xe = cx + (w / 2.0f);
		zs = cz - (w / 2.0f);
		ze = cz + (w / 2.0f);
		ys = legy;
		ye = legy + h;
	}

	PlayerAABB operator+ (glm::vec3& vec) {
		PlayerAABB pab;
		pab.xs = xs + vec.x;
		pab.xe = xe + vec.x;
		pab.ys = ys + vec.y;
		pab.ye = ye + vec.y;
		pab.zs = zs + vec.z;
		pab.ze = ze + vec.z;
		return pab;
	}

	float xs, xe, ys, ye, zs, ze;
};

struct BlockAABB {
	BlockAABB(float x, float y, float z) {
		xs = x;
		ys = y;
		zs = z;
		xe = x + 1.0f;
		ye = y + 1.0f;
		ze = z + 1.0f;
	}

	float clipXCollide(PlayerAABB c, float xa) {
		if (c.ye <= ys || c.ys >= ye)
			return xa;
		if (c.ze <= zs || c.zs >= ze)
			return xa;
		if (xa > 0.0f && c.xe <= xs) {
			float max = xs - c.xe;
			if (max < xa)
				xa = max;
		}
		if (xa < 0.0f && c.xs >= xe) {
			float max = xe - c.xs;
			if (max > xa)
				xa = max;
		}
		return xa;
	}

	float clipYCollide(PlayerAABB c, float ya) {
		if (c.xe <= xs || c.xs >= xe)
			return ya;
		if (c.ze <= zs || c.zs >= ze)
			return ya;
		if (ya > 0.0F && c.ye <= ys) {
			float max = ys - c.ye;
			if (max < ya)
				ya = max;
		}
		if (ya < 0.0F && c.ys >= ye) {
			float max = ye - c.ys;
			if (max > ya)
				ya = max;
		}
		return ya;
	}

	float clipZCollide(PlayerAABB c, float za) {
		if (c.xe <= xs || c.xs >= xe)
			return za;
		if (c.ye <= ys || c.ys >= ye)
			return za;
		if (za > 0.0F && c.ze <= zs) {
			float max = zs - c.ze;
			if (max < za)
				za = max;
		}
		if (za < 0.0F && c.zs >= ze) {
			float max = ze - c.zs;
			if (max > za)
				za = max;
		}
		return za;
	}

	bool collides(PlayerAABB& r) {
		return (r.xs < xe&& r.xe > xs && r.ys < ye&& r.ye > ys && r.zs < ze&& r.ze > zs);
	}

	bool pointInside(float x, float y, float z) {
		return (x < xe&& x > xs && y < ye&& y > ys && z < ze&& z > zs);
	}

	float xs, ys, zs, xe, ye, ze;
};

struct ControlSwitchs {
	bool forward, backward, left, right, jump, sprint, crouch;

	void set(bool forward, bool backward, bool left, bool right, bool jump, bool sprint, bool crouch) {
		this->forward = forward;
		this->backward = backward;
		this->left = left;
		this->right = right;
		this->jump = jump;
		this->sprint = sprint;
		this->crouch = crouch;
	}

	bool playerWantsToWalk() {
		return forward || backward || left || right;
	}

	bool playerWantsToJump() {
		return jump;
	}

	bool playerWantsToSprint() {
		return playerWantsToWalk() && sprint;
	}
};

struct PlayerPhysicalProperties {
	float yaw;
	float body_width;
	float body_height;
	float normal_speed;
	float sprint_speed;
	float crouch_speed;
	glm::vec3 position;
	int xoffset, zoffset;
};

class Physics
{
public:

	bool ground_touch_flag = false;
	float ground_touch_value = 0.0f;

	void player_move(ControlSwitchs cs, PlayerPhysicalProperties ppp, float delta_time, glm::vec3& movement , unsigned short int (*getBlock)(int, int, int), bool (*hasHitbox)(unsigned short int), bool (*isBlockLiquid)(unsigned short int)) {
		static bool on_ground = false;
		static float max_ydc = 0.0f;
		static float xd = 0.0f, yd = 0.0f, zd = 0.0f;
		static PlayerAABB playerAABB;
		static bool last_in_water = false;

		float xo, yo, zo, xa, ya, za, speed, dist, xdc, ydc, zdc, xao, yao, zao;
		int pyl, pxl, pzl;

		float yaw = glm::radians(ppp.yaw);

		xo = ppp.position.x;
		yo = ppp.position.y;
		zo = ppp.position.z;

		int bx = ppp.xoffset + (int) xo;
		int bz = ppp.zoffset + (int) zo;

		unsigned short int block_in = getBlock(bx, (int)(yo + 0.5f), bz);
		bool in_water = isBlockLiquid(block_in);
		float speed_modifier = in_water ? 0.3f : 1.0f;

		xa = 0;
		ya = 0;
		za = 0;

		if (cs.forward) xa += 1.0f;
		if (cs.backward) xa -= 1.0f;
		if (cs.left) za -= 1.0f;
		if (cs.right) za += 1.0f;
		if (cs.jump && on_ground) yd = 0.12f;
		if (cs.jump && (in_water || last_in_water)) yd = 0.08f;

		speed = (cs.sprint) ? ppp.sprint_speed * 1.5f: ppp.normal_speed;
		speed *= speed_modifier;
		speed *= delta_time;
		speed = (on_ground) ? speed : speed * 0.25f;
		dist = xa * xa + za * za;
		if (dist >= 0.01f) {
			dist = speed / (abs(xa) + abs(za));
			xa *= dist;
			za *= dist;
			float sn = sinf(yaw);
			float cs = cosf(yaw);
			xd += xa * cs - za * sn;
			zd += za * cs + xa * sn;
		}

		if (max_ydc > yd)
			max_ydc = yd;

		if(in_water)
			yd = yd - 1.5f * delta_time;
		else
			yd = yd - 0.25f * delta_time;

		xdc = xao = xd;
		ydc = yao = yd;
		zdc = zao = zd;

		pyl = (int)ppp.position.y;
		pxl = (int)ppp.position.x;
		pzl = (int)ppp.position.z;

		playerAABB = PlayerAABB(ppp.position.x, ppp.position.z, ppp.position.y, ppp.body_width, ppp.body_height);
		for (int x = pxl - 1; x < pxl + 2; x++) {
			for (int z = pzl - 1; z < pzl + 2; z++) {
				for (int y = pyl - 2; y < pyl + 3; y++) {
					if (!hasHitbox(getBlock(x + ppp.xoffset, y, z + ppp.zoffset))) continue;
					BlockAABB block(x, y, z);
					ydc = block.clipYCollide(playerAABB, ydc);
				}
			}
		}

		playerAABB = PlayerAABB(ppp.position.x, ppp.position.z, ppp.position.y, ppp.body_width, ppp.body_height);
		for (int x = pxl - 1; x < pxl + 2; x++) {
			for (int z = pzl - 1; z < pzl + 2; z++) {
				for (int y = pyl - 2; y < pyl + 3; y++) {
					if (!hasHitbox(getBlock(x + ppp.xoffset, y, z + ppp.zoffset))) continue;
					BlockAABB block(x, y, z);
					xdc = block.clipXCollide(playerAABB, xdc);
				}
			}
		}

		playerAABB = PlayerAABB(ppp.position.x, ppp.position.z, ppp.position.y, ppp.body_width, ppp.body_height);
		for (int x = pxl - 1; x < pxl + 2; x++) {
			for (int z = pzl - 1; z < pzl + 2; z++) {
				for (int y = pyl - 2; y < pyl + 3; y++) {
					if (!hasHitbox(getBlock(x + ppp.xoffset, y, z + ppp.zoffset))) continue;
					BlockAABB block(x, y, z);
					zdc = block.clipZCollide(playerAABB, zdc);
				}
			}
		}

		if (in_water) ydc = ydc / 2.0f;

		movement = glm::vec3(xdc, ydc, zdc);

		if (ydc > -0.001f && max_ydc < -0.01f) {
			ground_touch_value = max_ydc;
			ground_touch_flag = true;
			max_ydc = 0.0f;
		}

		on_ground = (yao != ydc && yao < 0.0f);

		if (xao != xdc)
			xd = 0.0f;
		if (yao != ydc)
			yd = 0.0f;
		if (zao != zdc)
			zd = 0.0f;

		xd *= 0.91f;
		yd *= 0.99f * speed_modifier;
		zd *= 0.91f;

		if (on_ground) {
			xd *= 0.8f;
			zd *= 0.8f;
		}

		last_in_water = in_water;
	}

};

