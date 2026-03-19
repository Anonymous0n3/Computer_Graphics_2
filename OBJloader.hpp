#pragma once

#include <vector>
#include <filesystem>
#include <GL/glew.h>

#include "assets.hpp"

bool loadOBJ(const std::filesystem::path& filename,
	         std::vector <Vertex> & vertices,
	         std::vector <GLuint>& indices);

