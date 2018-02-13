/*!
 * @file
 * @brief This file contains implemenation of phong vertex and fragment shader.
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <math.h>
#include <assert.h>
#include <string.h>

#include "student/student_shader.h"
#include "student/gpu.h"
#include "student/uniforms.h"

/// \addtogroup shader_side Úkoly v shaderech
/// @{

void phong_vertexShader(
    GPUVertexShaderOutput     *const output,
    GPUVertexShaderInput const*const input,
    GPU const gpu   ){
    /// \todo Naimplementujte vertex shader, který transformuje vstupní vrcholy do clip-space.<br>
    /// <b>Vstupy:</b><br>
    /// Vstupní vrchol by měl v nultém atributu obsahovat pozici vrcholu ve world-space (vec3) a v prvním
    /// atributu obsahovat normálu vrcholu ve world-space (vec3).<br>
    /// <b>Výstupy:</b><br>
    /// Výstupní vrchol by měl v nultém atributu obsahovat pozici vrcholu (vec3) ve world-space a v prvním
    /// atributu obsahovat normálu vrcholu ve world-space (vec3).
    /// Výstupní vrchol obsahuje pozici a normálu vrcholu proto, že chceme počítat osvětlení ve world-space ve fragment shaderu.<br>
    /// <b>Uniformy:</b><br>
    /// Vertex shader by měl pro transformaci využít uniformní proměnné obsahující view a projekční matici.
    /// View matici čtěte z uniformní proměnné "viewMatrix" a projekční matici čtěte z uniformní proměnné "projectionMatrix".
    /// Zachovejte jména uniformních proměnných a pozice vstupních a výstupních atributů.
    /// Pokud tak neučiníte, akceptační testy selžou.<br>
    /// <br>
    /// Využijte vektorové a maticové funkce.
    /// Nepředávajte si data do shaderu pomocí globálních proměnných.
    /// Pro získání dat atributů použijte příslušné funkce vs_interpret* definované v souboru program.h.
    /// Pro získání dat uniformních proměnných použijte příslušné funkce shader_interpretUniform* definované v souboru program.h.
    /// Vrchol v clip-space by měl být zapsán do proměnné gl_Position ve výstupní struktuře.<br>
    /// <b>Seznam funkcí, které jistě použijete</b>:
    ///  - gpu_getUniformsHandle()
    ///  - getUniformLocation()
    ///  - shader_interpretUniformAsMat4()
    ///  - vs_interpretInputVertexAttributeAsVec3()
    ///  - vs_interpretOutputVertexAttributeAsVec3()

    //get handle to all uniforms
    Uniforms const uniformsHandle = gpu_getUniformsHandle(gpu);
    //get uniform location of view matrix
    UniformLocation const viewMatrixLocation = getUniformLocation(gpu, "viewMatrix");

    //get pointer to view matrix
    Mat4 const*const view = shader_interpretUniformAsMat4(uniformsHandle, viewMatrixLocation);
    //get uniform location of projection matrix
    UniformLocation const projectionMatrixLocation = getUniformLocation(gpu, "projectionMatrix");

    //get pointer to projection matrix
    Mat4 const*const proj = shader_interpretUniformAsMat4(uniformsHandle, projectionMatrixLocation);
    Vec3 const*const position = vs_interpretInputVertexAttributeAsVec3(gpu, input, 0);
    Vec3 const*const norm = vs_interpretInputVertexAttributeAsVec3(gpu, input, 1);

    Mat4 mvp;
    multiply_Mat4_Mat4(&mvp, proj, view);

    Vec4 pos4;
    copy_Vec3Float_To_Vec4(&pos4, position, 1.f);

    multiply_Mat4_Vec4(&output->gl_Position, &mvp, &pos4);

    memcpy((Vec3*)output->attributes[0], position->data, sizeof(float)*3);
    memcpy((Vec3*)output->attributes[1], norm->data, sizeof(float)*3);

}

void phong_fragmentShader(
    GPUFragmentShaderOutput     *const output,
    GPUFragmentShaderInput const*const input,
    GPU const gpu   ){
    /// \todo Naimplementujte fragment shader, který počítá phongův osvětlovací model s phongovým stínováním.<br>
    /// <b>Vstup:</b><br>
    /// Vstupní fragment by měl v nultém fragment atributu obsahovat interpolovanou pozici ve world-space a v prvním
    /// fragment atributu obsahovat interpolovanou normálu ve world-space.<br>
    /// <b>Výstup:</b><br>
    /// Barvu zapište do proměnné color ve výstupní struktuře.<br>
    /// <b>Uniformy:</b><br>
    /// Pozici kamery přečtěte z uniformní proměnné "cameraPosition" a pozici světla přečtěte z uniformní proměnné "lightPosition".
    /// Zachovejte jména uniformních proměnný.
    /// Pokud tak neučiníte, akceptační testy selžou.<br>
    /// <br>
    /// Dejte si pozor na velikost normálového vektoru, při lineární interpolaci v rasterizaci může dojít ke zkrácení.
    /// Zapište barvu do proměnné color ve výstupní struktuře.
    /// Shininess faktor nastavte na 40.f
    /// Difuzní barvu materiálu nastavte na čistou zelenou.
    /// Spekulární barvu materiálu nastavte na čistou bílou.
    /// Barvu světla nastavte na bílou.
    /// Nepoužívejte ambientní světlo.<br>
    /// <b>Seznam funkcí, které jistě využijete</b>:
    ///  - shader_interpretUniformAsVec3()
    ///  - fs_interpretInputAttributeAsVec3()

    Uniforms const uniformsHandle = gpu_getUniformsHandle(gpu);

    UniformLocation const lightPositionUniform = getUniformLocation(gpu, "lightPosition");
    UniformLocation const cameraPositionUniform = getUniformLocation(gpu, "cameraPosition");
    const Vec3* lightPosition = shader_interpretUniformAsVec3(uniformsHandle, lightPositionUniform);
    const Vec3* cameraPosition = shader_interpretUniformAsVec3(uniformsHandle, cameraPositionUniform);

    Vec3 dM, sM;
    init_Vec3(&sM, 1.f, 1.f, 1.f);
    init_Vec3(&dM, 0.f, 1.f, 0.f);

    float dL = 1.f;
    float sL = .5f;
    float s = 40.f;

    Vec3 L, L1;
    sub_Vec3(&L1, lightPosition, (Vec3*)input->attributes.attributes[0]);
    normalize_Vec3(&L, &L1);
    Vec3 N;
    normalize_Vec3(&N, (Vec3*)input->attributes.attributes[1]);
    float dF = dot_Vec3(&N, &L);
    dF = (dF < 0 ? 0 : (dF > 1 ? 1 : dF));

    Vec3 V, V1, minusV;
    sub_Vec3(&V1, cameraPosition, (Vec3*)input->attributes.attributes[0]);
    normalize_Vec3(&V, &V1);
    multiply_Vec3_Float(&minusV, &V, -1.f);
    Vec3 R, RNorm;
    reflect(&R, &minusV, &N);
    normalize_Vec3(&RNorm, &R);
    float sF = dot_Vec3(&R, &L);
    sF = (sF < 0 ? 0 : (sF > 1 ? 1 : sF));
    sF = powf(sF, s);
    Vec3 diffuseFinal, specularFinal, final;
    multiply_Vec3_Float(&diffuseFinal, &dM, dL*dF);
    multiply_Vec3_Float(&specularFinal, &sM, sL*sF);
    add_Vec3(&final, &diffuseFinal,  &specularFinal);
    copy_Vec3Float_To_Vec4(&output->color, &final, 1.f);
}

/// @}
