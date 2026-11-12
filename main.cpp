#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <optional>
#include <iomanip>
#include <sstream>

const float PI = 3.14159265359f;
const float G = 9.81f;
const float PIXELS_PER_METER = 250.0f;

struct Pendulum {
    float lengthMeters = 1.2f;
    float initialAngleRad = 0.6f;
    float angle = 0.6f;
    float angularVelocity = 0.0f;
    float angularAcceleration = 0.0f;
    bool isPaused = false;

    void reset() {
        angle = initialAngleRad;
        angularVelocity = 0.0f;
        angularAcceleration = 0.0f;
    }

    void update(float dt) {
        if (isPaused) return;
        angularAcceleration = -(G / lengthMeters) * std::sin(angle);
        angularVelocity += angularAcceleration * dt;
        angle += angularVelocity * dt;
    }

    float getTheoreticalPeriod() const {
        return 2.0f * PI * std::sqrt(lengthMeters / G);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ 900, 700 }), "Lab 5 - Pendulum Simulation (Var 2)");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("../resources/fonts/arial.ttf")) {
        font.openFromFile("C:/Windows/Fonts/arial.ttf");
    }

    sf::Text infoText(font, "", 18);
    infoText.setFillColor(sf::Color::White);
    infoText.setPosition({ 20.f, 20.f });

    const sf::Vector2f pivot(450.f, 100.f);
    Pendulum pendulum;
    pendulum.reset();

    sf::CircleShape pivotCircle(6.f);
    pivotCircle.setOrigin({ 6.f, 6.f });
    pivotCircle.setPosition(pivot);
    pivotCircle.setFillColor(sf::Color::Red);

    sf::CircleShape bob(22.f);
    bob.setOrigin({ 22.f, 22.f });
    bob.setFillColor(sf::Color(0, 180, 255));

    sf::VertexArray trajectory(sf::PrimitiveType::LineStrip);
    std::vector<sf::Vector2f> history;
    sf::Clock clock;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (event->is<sf::Event::KeyPressed>()) {
                const auto key = event->getIf<sf::Event::KeyPressed>();

                if (key->code == sf::Keyboard::Key::Up) {
                    pendulum.lengthMeters += 0.05f;
                    history.clear();
                }
                if (key->code == sf::Keyboard::Key::Down) {
                    pendulum.lengthMeters -= 0.05f;
                    history.clear();
                }
                if (key->code == sf::Keyboard::Key::Right) {
                    pendulum.initialAngleRad += 0.05f;
                    pendulum.reset();
                    history.clear();
                }
                if (key->code == sf::Keyboard::Key::Left) {
                    pendulum.initialAngleRad -= 0.05f;
                    pendulum.reset();
                    history.clear();
                }
                if (key->code == sf::Keyboard::Key::Space) {
                    pendulum.isPaused = !pendulum.isPaused;
                }
                if (key->code == sf::Keyboard::Key::R) {
                    pendulum.reset();
                    history.clear();
                }

                pendulum.lengthMeters = std::clamp(pendulum.lengthMeters, 0.4f, 2.2f);
                pendulum.initialAngleRad = std::clamp(pendulum.initialAngleRad, -1.5f, 1.5f);
            }
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        pendulum.update(dt);

        float lengthPx = pendulum.lengthMeters * PIXELS_PER_METER;
        sf::Vector2f bobPos(
            pivot.x + lengthPx * std::sin(pendulum.angle),
            pivot.y + lengthPx * std::cos(pendulum.angle)
        );
        bob.setPosition(bobPos);

        if (!pendulum.isPaused) {
            history.push_back(bobPos);
            if (history.size() > 180) history.erase(history.begin());
            trajectory.clear();
            for (const auto& pt : history) {
                trajectory.append(sf::Vertex{ pt, sf::Color(100, 100, 100, 140) });
            }
        }

        window.clear(sf::Color(25, 25, 30));

        sf::VertexArray rod(sf::PrimitiveType::Lines, 2);
        rod[0].position = pivot;
        rod[0].color = sf::Color(200, 200, 200);
        rod[1].position = bobPos;
        rod[1].color = sf::Color(200, 200, 200);
        window.draw(rod);
        window.draw(trajectory);
        window.draw(pivotCircle);
        window.draw(bob);

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "=== PENDULUM SIMULATION (VAR 2) ===\n"
            << "Status: " << (pendulum.isPaused ? "PAUSED" : "RUNNING") << "\n"
            << "Length (L): " << pendulum.lengthMeters << " m  [Up/Down]\n"
            << "Initial Angle: " << (pendulum.initialAngleRad * 180.0f / PI) << " deg  [Left/Right]\n"
            << "Current Angle: " << (pendulum.angle * 180.0f / PI) << " deg (" << pendulum.angle << " rad)\n"
            << "Angular Vel: " << pendulum.angularVelocity << " rad/s\n"
            << "Theoretical Period (T): " << pendulum.getTheoreticalPeriod() << " s\n\n"
            << "Controls:\n"
            << " [Space] Pause/Resume\n"
            << " [R] Reset to Initial Angle\n"
            << " [Up/Down] Length +/- 0.05 m\n"
            << " [Left/Right] Init Angle +/- ~3 deg";

        infoText.setString(ss.str());
        window.draw(infoText);

        window.display();
    }

    return 0;
}