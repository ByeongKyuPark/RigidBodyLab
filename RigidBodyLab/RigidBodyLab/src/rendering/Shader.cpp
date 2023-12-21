#include <rendering/Shader.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

using namespace Rendering;

/******************************************************************************/
/*!
\fn     ReadTextFile
\brief
        Read the content of a text file.
\param  const char fn[]
        Text filename.
\return
        The content of the given text file.
*/
/******************************************************************************/
std::string ReadTextFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

Shader::Shader() : m_programID(0) {}

Shader::~Shader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

string Shader::ReadShaderFile(const string& filePath) {
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        return shaderStream.str();
    }
    catch ([[maybe_unused]]std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filePath << std::endl;
        return "";
    }
}

/******************************************************************************/
/*!
\fn     void ValidateShader(GLuint shader, const char *file)
\brief
Check whether shader files can be compiled successfully.
\param  shader
ID of the shader
\param  file
Shader file name
*/
/******************************************************************************/

void Rendering::Shader::ValidateShader(GLuint shader, const char* file)
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];

    GLsizei length = 0;
    GLint result;

    glGetShaderInfoLog(shader, 512, &length, buffer);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    if (result != GL_TRUE && length > 0)
    {
        std::cerr << "Shader " << file << " compilation error: " << buffer << "\n";
        exit(1);
    }
    else
        std::cout << "Shader " << file << " compilation successful.\n";
}


/******************************************************************************/
/*!
\fn     void ValidateProgram(GLuint program)
\brief
Check whether shader program can be linked successfully.
\param  program
ID of the shader program
*/
/******************************************************************************/

void Rendering::Shader::ValidateProgram(GLuint program)
{
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;
    GLint status;

    /*  Ask OpenGL to give us the log associated with the program */
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status != GL_TRUE && length > 0)
    {
        std::cerr << "Program " << program << " link error: " << buffer << "\n";
        exit(1);
    }
    else
        std::cout << "Program " << program << " link successful.\n";

    /*  Ask OpenGL to validate the program */
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
    {
        std::cerr << "Error validating shader " << program << ".\n";
        exit(1);
    }
    else
    {
        std::cout << "Program " << program << " validation successful.\n";
    }
}

bool Shader::LoadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode = ReadTextFile(vertexPath.c_str());
    std::string fragmentCode = ReadTextFile(fragmentPath.c_str());

    GLuint vertexShaderID, fragmentShaderID;
    if (!CompileShader(vertexCode, GL_VERTEX_SHADER, vertexShaderID)) return false;
    ValidateShader(vertexShaderID, vertexPath.c_str()); // Validate vertex shader

    if (!CompileShader(fragmentCode, GL_FRAGMENT_SHADER, fragmentShaderID)) return false;
    ValidateShader(fragmentShaderID, fragmentPath.c_str()); // Validate fragment shader

    if (!LinkProgram(vertexShaderID, fragmentShaderID)) return false;
    ValidateProgram(m_programID); // Validate the shader program

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return true;
}


void Shader::Use() {
    if (m_programID > 0) {
        glUseProgram(m_programID);
    }
}

// Getters for Shader Program ID
GLuint Shader::GetProgramID() const { return m_programID; }

bool Shader::CompileShader(const std::string& source, GLenum shaderType, GLuint& shaderID) {
    const char* shaderSource = source.c_str();
    shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderSource, nullptr);
    glCompileShader(shaderID);

    GLint success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderID, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

bool Shader::LinkProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShaderID);
    glAttachShader(m_programID, fragmentShaderID);
    glLinkProgram(m_programID);

    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }
    return true;
}
