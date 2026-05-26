#include "curve_path_loader.h"
#include <fstream>

inline static std::vector<std::string> Split(const std::string& str, char delimeter)
{
	std::vector<std::string> subStrings{};
	std::string subStr{};
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] != delimeter)
		{
			subStr.append(1, str[i]);
		}
		else if (str[i] == delimeter && subStr.size() > 0)
		{
			subStrings.push_back(subStr);
			subStr.clear();
		}
	}
	subStrings.push_back(subStr);
	subStr.clear();

	return subStrings;
}

std::vector<glm::vec3> LoadCurvePath(const std::filesystem::path& curve_filepath)
{
	std::ifstream file(curve_filepath);

	std::vector<glm::vec3> curve_points{};

	if (file.is_open())
	{
		while (!file.eof())
		{
			std::string point = "";
			std::getline(file, point);

			auto p_components = Split(point, ' ');

			if (p_components.size() == 3)
				curve_points.emplace_back(std::stof(p_components[0]), std::stof(p_components[1]), std::stof(p_components[2]));
		}
	}

	return curve_points;
}
