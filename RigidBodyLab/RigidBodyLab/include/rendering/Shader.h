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

    /*  We need 3 set of shaders programs */
    /*  MAIN_PROG   : Render all the objects in the scene, used for the above 3 passes */
    /*  SKYBOX_PROG : Render the background */
    /*  SPHERE_PROG : Render the relective/refractive sphere */
    enum class ProgType {
        MAIN_PROG = 0,
        SKYBOX_PROG,
        SPHERE_PROG,
        //deferred
        DEFERRED_FORWARD,
        DEFERRED_GEOMPASS, 
        DEFERRED_LIGHTPASS,
        NUM_PROGTYPES
    };
    enum class ShaderType {
        VERTEX_SHADER = 0,
        FRAGMENT_SHADER,
        NUM_SHADERTYPES
    };

    // Forward declarations
    using GLenum = unsigned int;
    using GLuint = unsigned int;

    class Shader {
        GLuint m_programID;

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

        GLuint GetProgramID() const;
    };
}