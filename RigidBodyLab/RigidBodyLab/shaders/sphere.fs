#version 330 core

uniform samplerCube texCube;

uniform mat4 viewMat;

uniform int sphereRef;

in vec3 view, normal;


out vec4 fragColor;


void main(void)
{
    /*  Compute reflected/refracted vectors in view frame for higher accuracy, 
        then transform them back to world frame for texture sampling.

        If sphereRef = 0 then there's only reflection.
        If sphereRef = 1 then there's only refraction.
        If sphereRef = 2 then there are both reflection and refraction.
        In that case, assume that reflection accounts for 70% of the color.
		
		The refractive index for the sphere is 1.5. For simplicity, we also
		assume that each ray going into the sphere is refracted only once.
    */
        vec3 viewNorm = normalize(view);
        vec3 normalNorm = normalize(normal);
        mat3 invViewMat = transpose(mat3(viewMat));
        vec4 colorReflect, colorRefract;
        vec3 reflectDir, refractDir;
        
        // Handle reflection
        if (sphereRef == 0 || sphereRef == 2)
        {
            reflectDir = invViewMat * reflect(viewNorm, normalNorm);
            colorReflect = texture(texCube, reflectDir);
        }
    
        // Handle refraction
        if (sphereRef == 1 || sphereRef == 2)
        {
            refractDir = refract(viewNorm, normalNorm, 1.f / 1.5f);
            refractDir = invViewMat * refractDir;
            colorRefract = texture(texCube, refractDir);
        }

        // final fragment color
        switch (sphereRef)
        {
            case 0:
                fragColor = colorReflect;
                break;
            case 1:
                fragColor = colorRefract;
                break;
            case 2:
                fragColor = 0.7 * colorReflect + 0.3 * colorRefract;//assuming that reflection accounts for 0.7 of the color
                break;
        }
}