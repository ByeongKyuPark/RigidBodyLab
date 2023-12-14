#pragma once
#include <array>
#include <unordered_map>
#include <string>

using std::string;

namespace Rendering 
{
    struct ShaderInfo {
        string vertexShaderPath;
        string fragmentShaderPath;
    };

    // Forward declarations
    using GLenum = unsigned int;
    using GLuint = unsigned int;

    class Shader {
        GLuint programID;

        string ReadShaderFile(const string& filePath);
        bool CompileShader(const std::string& source, GLenum shaderType, GLuint& shaderID);
        bool LinkProgram(GLuint vertexShaderID, GLuint fragmentShaderID);
        void ValidateShader(GLuint shader, const char* file);
        void ValidateProgram(GLuint program);
    public:
        Shader();
        ~Shader();

        bool LoadShader(const string& vertexPath, const string& fragmentPath);
        void Use();

        // Getters for Shader Program ID
        GLuint GetProgramID() const;
    };
}