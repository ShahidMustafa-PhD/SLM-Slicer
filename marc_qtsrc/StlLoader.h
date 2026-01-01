#ifndef STL_LOADER_H
#define STL_LOADER_H

#include <vector>
#include <string>
#include <tuple>

class StlLoader {
public:
    explicit StlLoader(const std::string& filename);

    const std::vector<float>& getVertices() const;
    const std::vector<unsigned int>& getIndices() const;
    const std::vector<std::tuple<float, float, float>>& getNormals() const;

private:
    void loadFile(const std::string& filename);
    bool isAsciiSTL(std::ifstream& file);

    void parseAsciiSTL(std::ifstream& file);
    void parseBinarySTL(std::ifstream& file);

    bool areClose(float a, float b, float eps = 1e-6f) const;
    bool float3Equal(const std::tuple<float, float, float>& a,
                     const std::tuple<float, float, float>& b) const;

    struct Float3Hash;

    std::vector<float> vertices_;
    std::vector<unsigned int> indices_;
    std::vector<std::tuple<float, float, float>> normals_;
};

#endif // STL_LOADER_H
