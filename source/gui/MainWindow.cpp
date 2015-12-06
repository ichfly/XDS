#include <stdio.h>
#include "Gui.h"


static void on_error(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

MainWindow::MainWindow() {
    // Setup window
    glfwSetErrorCallback(on_error);
    if (!glfwInit())
        exit(1);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "xds", NULL, NULL);

	if (window)
	{

		glfwMakeContextCurrent(window);
		gl3wInit();

		ImGui_ImplGlfwGL3_Init(window, true);


		while (!glfwWindowShouldClose(window))
		{
			ImGuiIO& io = ImGui::GetIO();
			glfwPollEvents();
			ImGui_ImplGlfwGL3_NewFrame();

			ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Test Window");
			ImGui::Text("TODO: Move this to new thread so xds can run in the background.");
			ImGui::Text("Close the window to continue xds execution.");
			ImGui::End();

			// Rendering
			glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
			ImVec4 clear_color = ImColor(114, 144, 154);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui::Render();

			glfwSwapBuffers(window);
		}

		// Cleanup
		ImGui_ImplGlfwGL3_Shutdown();
		glfwTerminate();

	}
}
