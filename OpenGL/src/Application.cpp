#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>


static unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str(); //source needs to exist and assigned at this point to avoid erratic behavior

    int sourceCodeCount = 1;
    glShaderSource(id, sourceCodeCount, &src, nullptr); // Third argument is a pointer to the pointer to the string object
    glCompileShader(id);

    //TODO: Syntax error handling for the shader

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); //Returns a parameter from a shader object. Parameter Options are GL_SHADER_TYPE, GL_DELETE_STATUS, GL_COMPILE_STATUS, GL_INFO_LOG_LENGTH, GL_SHADER_SOURCE_LENGTH
    // i: integer, v: vector

    if (result == GL_FALSE) {  //Shader not compiled successfully
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        // char message[length]; Wouldn't work as we don't know length at compile-time
        char* message = (char*) _malloca(length * sizeof(char)); // alloca (from C) allows you to allocate on the stack dynamically 
        glGetShaderInfoLog(id, length, &length, message);

        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " Shader!" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);
        return 0;
    }
    return id;
}



/*  Args: Actual source code of the shaders as these actual strings vertexShader and fragmentShader
    Function: Code necessary to compile these two shaders.*/
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {

    //Purpose: OpenGL compiles string source codes and link these two together into a single shader program. Give us some unique ID for that shader.

    unsigned int program = glCreateProgram(); //OpenGL is inconsistent about providing unique IDs. Unlike glGenBuffer, we don't pass a reference for the function to store an ID at, glCreateProgram returns an unsigned int ID.

    // TODO: Create Shader objects
    
    //GLuint alternativeVarDeclaration;
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    //Attach both of these programs into our program. Think of like C++ where we have seperate files and we want to link them into one program. 

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    //Links the program object. If any shader objects of type GL_VERTEX_SHADER are attached to program, they will run on the programmable vertex processor.
    //GL_GEOMETRY_SHADER are attached --> they will be used to create an executable that will run on the programmable geometry processor.
    //GL_FRAGMENT_SHADER --> programmable fragment processor.

    // The status of the link operation will be stored as part of the program object's state. This value will be set to GL_TRUE if the program object was linked without errors and is ready for use.

    glLinkProgram(program); 
    glValidateProgram(program); // Checks to see whether the executables contained in program can execute given the current OpenGL state.

    //TODO: Check out glValidateProgramPipeline

    glDeleteShader(vs);   // Since we linked our shader objects to the program, we can delete the intermediates. When you compile something in C++, We get intermediate .obj files.
    glDeleteShader(fs);   // We can delete them since we got an actual program now. 

    //There's also glDetachShader but they are not that necessary but having source code still around is important for debugging. 

    return program;

}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    // Modern OpenGL: Create a vertex bufffer in GPU's RAM to store vertex data as in the previous case
    // glGenBuffers: 
    //      First arg  -> number of buffer object names to be generated
    //      Second arg -> specifies an array in which the generated buffer object names are stored.

    unsigned int bufferID;
    glGenBuffers(1, &bufferID); //ID of the generated buff   er
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);

    //Next up, we gotta specify data/space
    // Route1: No data needed right away, just say how big of a buffer you want and later update it with data
    // Route2: Or pass in the data right away


    //Step 1: Provide OpenGL with data
    float positions[6] = { -0.5f, -0.5f,
                            0.0f, 0.5f,
                            0.5f, -0.5f
    };

    //glBufferData(GL_ARRAY_BUFFER, sizeof(positions));
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float),positions,GL_STATIC_DRAW);

    //As far as OpenGL is concerned, glBufferData just gives a bunch of bytes of data. How does OpenGL
    //interpret that data? 
    //Info that needs to be passed: 1) They are floats, 2) 2 positions per vertex so 6 is really 2 x 3 vertices
        //glVertexAttribPointer --> Very closely tied to the shaders


    // Lesson5: Vertex Attributes and Layouts in OpenGL
    
    // If we had more than just the position attribute, we would have to call
    // this function multiple times to modify each attibute of each vertex by referring
    // to the attributes by their indices. 0 --> Position 1--> Texture 2-->Normal etc.    
    // Eventually we'll start having more than just position, we'll have a struct that
    // makes a single vertex. Then we can use sizeof(vertexStruct)
    // stride argument (sizeof(float)*2)) is the offset used to skip to the next vertex

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2,0);

    //Step 2: Write a shader to explain how GPU uses data

    std::string vertexShader =
        "#version 330 core\n"
        "\n"
        "layout(location = 0) in vec4 position;\n"
        "void main(){\n"
        "   gl_Position  = position;\n"
        "}\n";

    std::string fragmentShader =
        "#version 330 core\n"
        "\n"
        "layout(location = 0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(1.0,0.0,0.0,1.0);\n"
        "}\n";


    unsigned int shader = CreateShader(vertexShader,fragmentShader);
    glUseProgram(shader); // We can also bind the shader

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /*
        //From Lecture 1, where we drew a triangle using Legacy OpenGL
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(0.0f, 0.5f);
        glVertex2f(0.5f, -0.5f);
        glEnd();
        */

        // IMPORTANT QUESTION: How does glDrawArrays know to draw the triangles defined by positions and bufferID?
        // ANSWER: OpenGL is a state machine. The buffer that is CURRENTLY BOUND will be drawn-->bufferID
        //DrawCall Way 1: If you don't have an index buffer
        
        glDrawArrays(GL_TRIANGLES,0,3); //Start drawing from the 0th coordinates, there will be 3 indices/vertices

        //Drawcall Way2: You should have an index buffer for this
        //glDrawElements(GL_TRIANGLES,3)

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteShader(shader);

    glfwTerminate();
    return 0;
}