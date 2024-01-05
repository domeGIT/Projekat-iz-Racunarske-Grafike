#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

unsigned int loadTexture(char const * path);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight
{
    glm::vec3 direction;

    glm::vec3 specular;
    glm::vec3 diffuse;
    glm::vec3 ambient;};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;

    float cutOff;
    float outerCutOff;

    glm::vec3 specular;
    glm::vec3 diffuse;
    glm::vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};



struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    PointLight pointLight;
    DirLight dirLight;
    SpotLight spotLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //=============================pyramid================================================
    float pyramid[]
            {
                //vertices                       //normals                     //tex coordinates
                -0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,0.0f,
                0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,  1.0f,0.0f,
                0.0f,0.5f,0.0f,    0.0f,0.0f,-1.0f,   0.5f,1.0f,

                -0.5f,-0.5f,0.5f,   0.0f,0.0f,1.0f,   0.0f,0.0f,
                0.5f,-0.5f,0.5f,   0.0f,0.0f,1.0f,  1.0f,0.0f,
                0.0f,0.5f,0.0f,    0.0f,0.0f,1.0f,   0.5f,1.0f,

       //         -0.5f,-0.5f,-0.5f,   -1.0f,0.0f,0.0f,   0.0f,1.0f,
       //         -0.5f,-0.5f,0.5f,   -1.0f,0.0f,0.0f,   0.0f,0.0f,
       //        0.0f,0.5f,0.0f,    -1.0f,0.0f,0.0f,   0.5f,1.0f,

       //         0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,  0.0f,1.0f,
        //        0.5f,-0.5f,0.5f,   1.0f,0.0f,0.0f,  0.0f,0.0f,
         //       0.0f,0.5f,0.0f,    1.0f,0.0f,0.0f,   0.5f,1.0f,

         //       0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,  1.0f,1.0f,
          //      0.5f,-0.5f,0.5f,   0.0f,-1.0f,0.0f,  1.0f,0.0f,
           //     0.0f,0.5f,0.0f,    0.0f,-1.0f,0.0f,   0.5f,1.0f,

           //     -0.5f,-0.5f,0.5f,   0.0f,1.0f,0.0f,   0.0f,0.0f,
           //     -0.5f,-0.5f,-0.5f,   0.0f,1.0f,0.0f,   0.0f,1.0f,
          //      0.0f,0.5f,0.0f,    0.0f,1.0f,0.0f,   0.5f,1.0f





            };



    unsigned int pyramidVAO, pyramidVBO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);

    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid), pyramid, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int texture=loadTexture(FileSystem::getPath("resources//textures/teal2.png").c_str());
    //==============================loading shaders=========================================
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader pyramidShader("resources/shaders/cone_of_light.vs", "resources/shaders/cone_of_light.fs");

    //==============================loading models===========================================

    Model campfire("resources/objects/campfire/Campfire.obj");
    campfire.SetShaderTextureNamePrefix("material.");
    Model island("resources/objects/isl/island2.obj");
    island.SetShaderTextureNamePrefix("material.");
    Model log("resources/objects/log/log/13021_Aquarium_Log_v1_L3.obj");
    log.SetShaderTextureNamePrefix("material.");
    Model tree("resources/objects/pine/Tree2.obj");
    tree.SetShaderTextureNamePrefix("material.");
    Model ufo("resources/objects/ufo/UFO.obj");
    ufo.SetShaderTextureNamePrefix("material.");
    Model alien("resources/objects/alien/alien/10469_GrayAlien_v01.obj");
    alien.SetShaderTextureNamePrefix("material.");

    //===============================lighting==================================================
    //pointLight
    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(-2.09394,5.40402,0.707192);
    pointLight.ambient = glm::vec3(150*0.01, 88*0.01, 34*0.01);
    pointLight.diffuse = glm::vec3(155*0.1, 90*0.1, 35*0.1);
    pointLight.specular = glm::vec3(150*0.1, 88*0.1, 34*0.1);

    pointLight.constant = 0.0f;
    pointLight.linear = 0.005f;
    pointLight.quadratic = 8.0f;

    //dirLight
    DirLight& dirLight = programState->dirLight ;
    dirLight.direction = glm::vec3(-1.0f, -1.0f, -1.0f);
    dirLight.ambient =   glm::vec3(0.0f, 0.0f, 0.0f);
    dirLight.diffuse =   glm::vec3( 0.08f, 0.08f, 0.08f);
    dirLight.specular =  glm::vec3(0.1f, 0.1f, 0.1f);

    //spotLight
    SpotLight& spotLight = programState->spotLight;
    spotLight.position=glm::vec3(4.1918+3,7.83968-0.5,0.513466);
    spotLight.direction=glm::vec3(0,-1,0);
    spotLight.cutOff=glm::cos(glm::radians(10.0f));
    spotLight.outerCutOff=glm::cos(glm::radians(60.0f));
    spotLight.ambient=glm::vec3(0.0f, 0.0f, 0.0f);
    spotLight.diffuse=glm::vec3( 5.0f, 5.0f, 5.0f);
    spotLight.specular=glm::vec3(5.0f, 5.0f, 5.0f);
    spotLight.constant = 1.0f;
    spotLight.linear = 0.09f;
    spotLight.quadratic = 0.032f;
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // ======================================render=========================================
        // -----------------------------------setup shaders-------------------------------------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        //pointLight
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        //dirLight
        ourShader.setVec3("dirLight.direction", dirLight.direction);
        ourShader.setVec3("dirLight.ambient", dirLight.ambient);
        ourShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        ourShader.setVec3("dirLight.specular", dirLight.specular);

        //spotLight
        ourShader.setVec3("spotLight.position", spotLight.position);
        ourShader.setVec3("spotLight.direction", spotLight.direction);
        ourShader.setVec3("spotLight.ambient", spotLight.ambient);
        ourShader.setVec3("spotLight.diffuse",spotLight.diffuse/2.0f);
        ourShader.setVec3("spotLight.specular", spotLight.specular);
        ourShader.setFloat("spotLight.constant", spotLight.constant);
        ourShader.setFloat("spotLight.linear", spotLight.linear);
        ourShader.setFloat("spotLight.quadratic", spotLight.quadratic);
        ourShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        ourShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        // ------------------------------------drawing models--------------------------------------

        //Tree 1
        glm::mat4 model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-2.49604,5.01492,-1.477451188));
        model=scale(model,glm::vec3(0.4));
        ourShader.setMat4("model",model);
        tree.Draw(ourShader);

        //Alien 1
         model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-2.9029,5.41774,0.284943));
        model=scale(model,glm::vec3(0.01));
        model=glm::rotate(model,glm::radians(-90.0f),glm::vec3(1,0,0));
        model=glm::rotate(model,glm::radians(64.0702f),glm::vec3(0,0,1));
        ourShader.setMat4("model",model);
        alien.Draw(ourShader);

        //Island 1
        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-2.27991,0.667268,-0.00454823));
        model=scale(model,glm::vec3(0.5));
        ourShader.setMat4("model",model);
        island.Draw(ourShader);

        //Tree 2
        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-1.305,5.11681,-1.19112));
        model=scale(model,glm::vec3(0.4));
        ourShader.setMat4("model",model);
        tree.Draw(ourShader);

        //Tree 3
        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-3.31726,5.11675,-0.963768));
        model=scale(model,glm::vec3(0.4));
        ourShader.setMat4("model",model);
        tree.Draw(ourShader);


        //Log 1

        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-3.57426,5.53347,-0.0779114+0.3));
        model=scale(model,glm::vec3(0.03));
        model=glm::rotate(model,glm::radians(82.0016f),glm::vec3(0,1,0));
        ourShader.setMat4("model",model);
        log.Draw(ourShader);

        //Log 2

        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-1.67368,5.37016,-0.828546));
        model=scale(model,glm::vec3(0.03));
        ourShader.setMat4("model",model);
        log.Draw(ourShader);


        //Alien 2
        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-1.66348,5.35822,-0.0953785));
        model=scale(model,glm::vec3(0.01));
        model=glm::rotate(model,glm::radians(-90.0f),glm::vec3(1,0,0));
        ourShader.setMat4("model",model);
        alien.Draw(ourShader);


        //Campfire
        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(-2.09394,5.40402,0.707192));
        model=scale(model,glm::vec3(0.01));
        ourShader.setMat4("model",model);
        campfire.Draw(ourShader);

        //UFO
        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(7.1918,7.33968,0.513466));
        model=scale(model,glm::vec3(0.8));
        ourShader.setMat4("model",model);
        ufo.Draw(ourShader);

        //Island 2
        model = glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(7.11355,0.628727,0.572346));
        model=scale(model,glm::vec3(0.5));
        ourShader.setMat4("model",model);
        island.Draw(ourShader);

        glDisable(GL_CULL_FACE);

        //-----------------------------------drawing pyramid-------------------------------------------
        pyramidShader.use();
        pyramidShader.setMat4("projection", projection);
        pyramidShader.setMat4("view", view);
        glBindVertexArray(pyramidVAO);
        glBindTexture(GL_TEXTURE_2D,texture);

        model=glm::mat4(1.0f);
        model=glm::translate(model,glm::vec3(7.1918,6.33968,0.513466));
        model=glm::scale(model,glm::vec3(2.1,2.7,2.1));
        model=glm::rotate(model,glm::radians(currentFrame*30),glm::vec3(0,1,0));
        pyramidShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        model=glm::rotate(model,glm::radians(90.0f),glm::vec3(0,1,0));
        pyramidShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //---------------------------------------------------------------------------------------------------------
        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->backpackPosition);
        ImGui::DragFloat("Backpack scale", &programState->backpackScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format ;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
