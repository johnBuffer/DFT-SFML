#include "engine/window/window_context_handler.hpp"
#include "user/initialize.hpp"


int32_t main()
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    pez::render::WindowContextHandler app("Fourier", conf::win::window_size, settings, sf::Style::Fullscreen, 1);
    initialize();

    auto& renderer = pez::core::getRenderer<Renderer>();
    app.getEventManager().addKeyPressedCallback(sf::Keyboard::S, [&](sfev::CstEv) {
        renderer.tracer.clear();
        renderer.addCoefficient();
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::M, [&](sfev::CstEv) {
        renderer.tracer.clear();
        if (renderer.mode == Renderer::Mode::Dual) {
            renderer.mode = Renderer::Mode::Mono;
        } else {
            renderer.mode = Renderer::Mode::Dual;
        }
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::H, [&](sfev::CstEv) {
        renderer.draw_help = !renderer.draw_help;
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::R, [&](sfev::CstEv) {
        renderer.time = 0.0;
        renderer.tracer.clear();
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::P, [&](sfev::CstEv) {
        renderer.draw_tank = !renderer.draw_tank;
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::F, [&](sfev::CstEv) {
        renderer.focus_on_tip_position = !renderer.focus_on_tip_position;
    });

    app.getEventManager().addKeyPressedCallback(sf::Keyboard::X, [&](sfev::CstEv) {
        renderer.slow_mo = !renderer.slow_mo;
    });

    bool clicking = false;
    app.getEventManager().addMousePressedCallback(sf::Mouse::Right, [&](sfev::CstEv) {
        clicking = true;
        // If the signal is not empty, we have to bridge the gap with invisible padding points
        if (!renderer.signal.data.empty()) {
            renderer.signal.addPointFill(app.getWorldMousePosition(), false);
        }
    });
    app.getEventManager().addMouseReleasedCallback(sf::Mouse::Right, [&](sfev::CstEv) {
        clicking = false;
    });

    constexpr uint32_t fps_cap = 60;
    const float dt = 1.0f / static_cast<float>(fps_cap);

    while (app.run()) {
        if (clicking) {
            Vec2 const mouse_position = app.getWorldMousePosition();
            renderer.signal.addPoint(mouse_position, true);
            renderer.signal.closeLoop();
        }

        pez::core::update(dt);
        pez::core::render({80, 80, 80});
    }

    return 0;
}
