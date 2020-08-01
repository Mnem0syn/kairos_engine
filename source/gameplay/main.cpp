#if defined(WIN32) && defined(NDEBUG)

#include <windows.h>

extern int main(int argc, char* argv[]);

////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT) { return main(__argc, __argv); }

#endif // WIN32

#include <application.h>
#include <command_line_parser.h>
#include <globals.h>
#include <logger.h>
#include <messages.h>
#include <window.h>

int main(int argc, char* argv[]) {
    application app;
    app.name = "GameEngine";
    app.organisation = "Kairos";
    application::set_application(&app);

    command_line_parser options(argv[0], " - game");
    options.add_options()("resolution", "Window resolution",
        cxxopts::value<array<int>>()->default_value("1920,1080"))
        ("gl", "The OpenGL version for creating context", cxxopts::value<array<unsigned char>>()->default_value("4,1"))
        ("help", "Show help");

    vec2i window_size_arg{ 0, 0 };

    try {
        parse_result arguments = options.parse(argc, argv);

        auto& gl_context = app.gl_context;
        {
            const auto gl_version = arguments["gl"].as<array<uint8>>();
            const auto antialiasing_samples = 4;
            gl_context = opengl_context{ gl_version[0], gl_version[1], antialiasing_samples, opengl_context::opengl_profile::core };
            gl_context.setup_gl_context();
        }
        log(LoggerName::ENGINE, "New OpenGL context created, version: {}.{}\n", gl_context.major_version, gl_context.minor_version);

        {
            const auto window_size = arguments["resolution"].as<array<int>>();
            window_size_arg.x = window_size[0];
            window_size_arg.y = window_size[1];
            app.main_window = create_window(window_size_arg.x, window_size_arg.y, app.name);
            if (app.native_window() == nullptr) {
                log_error(LoggerName::ENGINE, "An error occured, no window created.");
                return EXIT_FAILURE;
            }
            gl_context.setup_gl_functions();
            setup_default_gl_rendering_options();
        }
    }
    catch (const cxxopts::OptionException& e) {
        log_error(LoggerName::ENGINE, "error parsing command line options: {}\n", e.what());
        return EXIT_FAILURE;
    }

    vec2i framebuffer = app.main_window.size();
    globals::pixel_size = { static_cast<float>(framebuffer.x) / window_size_arg.x,
                           static_cast<float>(framebuffer.y) / window_size_arg.y };
    log(LoggerName::ENGINE, "Pixel size set to {}\n", to_string(globals::pixel_size));

    app.exec();

    return EXIT_SUCCESS;
}