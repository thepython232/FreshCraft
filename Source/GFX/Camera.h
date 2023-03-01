#pragma once

#include "Common.h"
#include "glm/gtc/matrix_access.hpp"

class Camera {
public:
	Camera(float fov, float aspect, float near, float far, glm::vec3 pos = {}, glm::vec3 rot = {}) : pos(pos), rot(rot), fov(glm::radians(fov)), near(near), far(far) {
		proj = glm::scale(glm::perspective(glm::radians(fov), aspect, near, far), glm::vec3{ 1.f, -1.f, -1.f });
		UpdateView();
	}

	void SetPos(glm::vec3 pos) {
		this->pos = pos;
		UpdateView();
	}

	void SetRot(glm::vec3 rot) {
		this->rot = rot;
		UpdateView();
	}

	void UpdateProj(float aspect) {
		proj = glm::scale(glm::perspective(fov, aspect, near, far), glm::vec3{ 1.f, -1.f, -1.f });
	}

	const glm::vec3& GetPos() const { return pos; }
	const glm::vec3& GetRot() const { return rot; }

	const glm::mat4& View() const { return view; }
	const glm::mat4& InvView() const { return invView; }
	const glm::mat4& Proj() const { return proj; }

	glm::vec3 Forward() const { return glm::normalize(glm::vec3(glm::transpose(view)[2])); }

private:
	void UpdateView() {
		float cosPitch = cos(rot.x);
		float sinPitch = sin(rot.x);
		float cosYaw = cos(rot.y);
		float sinYaw = sin(rot.y);
		glm::vec3 xaxis = { cosYaw, 0.f, -sinYaw };
		glm::vec3 yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
		glm::vec3 zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };
		// Create a 4x4 view matrix from the right, up, forward and eye position vectors
		view = {
			xaxis.x, yaxis.x, zaxis.x, 0.f,
			xaxis.y, yaxis.y, zaxis.y, 0.f,
			xaxis.z, yaxis.z, zaxis.z, 0.f,
			-glm::dot(xaxis, pos), -glm::dot(yaxis, pos), -glm::dot(zaxis, pos), 1.f
		};

		invView = glm::inverse(view);
	}

	glm::mat4 view, invView, proj;
	glm::vec3 pos, rot;
	float fov, near, far;
};