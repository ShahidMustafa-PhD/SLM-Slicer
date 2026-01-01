#include "StlLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <cstdint>

struct StlLoader::Float3Hash {
    size_t operator()(const std::tuple<float, float, float>& t) const {
        auto [x, y, z] = t;
        size_t h1 = std::hash<float>{}(x);
        size_t h2 = std::hash<float>{}(y);
        size_t h3 = std::hash<float>{}(z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

StlLoader::StlLoader(const std::string& filename) {
    loadFile(filename);
}

bool StlLoader::areClose(float a, float b, float eps) const {
    return std::fabs(a - b) < eps;
}

bool StlLoader::float3Equal(const std::tuple<float, float, float>& a,
                            const std::tuple<float, float, float>& b) const {
    return areClose(std::get<0>(a), std::get<0>(b)) &&
           areClose(std::get<1>(a), std::get<1>(b)) &&
           areClose(std::get<2>(a), std::get<2>(b));
}

bool StlLoader::isAsciiSTL(std::ifstream& file) {
    std::string header(80, '\0');
    file.read(&header[0], 80);
    file.seekg(0);
    return header.find("solid") != std::string::npos;
}

void StlLoader::loadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }

    if (isAsciiSTL(file)) {
        std::cout << "Parsing ASCII STL\n";
        parseAsciiSTL(file);
    } else {
        std::cout << "Parsing Binary STL\n";
        parseBinarySTL(file);
    }
}

void StlLoader::parseAsciiSTL(std::ifstream& file) {
    std::unordered_map<std::tuple<float, float, float>, unsigned int, Float3Hash> vertex_map;
    std::string line;
    std::tuple<float, float, float> current_normal;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        if (token == "facet") {
            std::string discard;
            float nx, ny, nz;
            ss >> discard >> nx >> ny >> nz;
            current_normal = {nx, ny, nz};
        } else if (token == "vertex") {
            float x, y, z;
            ss >> x >> y >> z;
            auto key = std::make_tuple(x, y, z);

            auto it = std::find_if(vertex_map.begin(), vertex_map.end(),
                                   [&](const auto& kv) { return float3Equal(kv.first, key); });

            unsigned int index;
            if (it == vertex_map.end()) {
                index = static_cast<unsigned int>(vertices_.size() / 3);
                vertices_.insert(vertices_.end(), {x, y, z});
                vertex_map[key] = index;
            } else {
                index = it->second;
            }

            indices_.push_back(index);

            // One normal per triangle (only at the first vertex in a triangle)
            if (indices_.size() % 3 == 0)
                normals_.push_back(current_normal);
        }
    }
}

void StlLoader::parseBinarySTL(std::ifstream& file) {
    std::unordered_map<std::tuple<float, float, float>, unsigned int, Float3Hash> vertex_map;
    file.seekg(80);
    uint32_t triangle_count;
    file.read(reinterpret_cast<char*>(&triangle_count), sizeof(triangle_count));

    for (uint32_t i = 0; i < triangle_count; ++i) {
        float normal[3], vertices[9];
        uint16_t attr_byte_count;

        file.read(reinterpret_cast<char*>(normal), sizeof(normal));
        file.read(reinterpret_cast<char*>(vertices), sizeof(vertices));
        file.read(reinterpret_cast<char*>(&attr_byte_count), sizeof(attr_byte_count));

        normals_.emplace_back(normal[0], normal[1], normal[2]);

        for (int j = 0; j < 3; ++j) {
            float x = vertices[j * 3 + 0];
            float y = vertices[j * 3 + 1];
            float z = vertices[j * 3 + 2];

            auto key = std::make_tuple(x, y, z);
            auto it = vertex_map.find(key);
            unsigned int index;

            if (it == vertex_map.end()) {
                index = static_cast<unsigned int>(vertices_.size() / 3);
                vertices_.insert(vertices_.end(), {x, y, z});
                vertex_map[key] = index;
            } else {
                index = it->second;
            }

            indices_.push_back(index);
        }
    }
}

const std::vector<float>& StlLoader::getVertices() const {
    return vertices_;
}

const std::vector<unsigned int>& StlLoader::getIndices() const {
    return indices_;
}

const std::vector<std::tuple<float, float, float>>& StlLoader::getNormals() const {
    return normals_;
}
