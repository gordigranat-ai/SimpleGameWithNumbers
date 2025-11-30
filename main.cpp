#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <memory>
#include <functional>
#include <iostream>
#include <random>
#include <iomanip>

const std::string RESOURCES_DIR = "D:\\С++\\ShaolinNumber2\\resources\\";
const std::string SAVE_FILE = RESOURCES_DIR + "save.dat";
const std::string CONFIG_FILE = RESOURCES_DIR + "config.cfg";

class Config {
public:
    int width = 800;
    int height = 600;
    bool fullscreen = false;

    void load() {
        std::ifstream file(CONFIG_FILE);
        if (file) {
            file >> width >> height >> fullscreen;
            // Validate values
            width = std::max(640, std::min(width, 1920));
            height = std::max(480, std::min(height, 1080));
        }
    }

    void save() {
        std::ofstream file(CONFIG_FILE);
        if (file) {
            file << width << " " << height << " " << fullscreen;
        }
    }
};

class ResourceManager {
public:
    static sf::Font& getFont() {
        static sf::Font font;
        if (!font.loadFromFile(RESOURCES_DIR + "videotype.otf")) {
            throw std::runtime_error("Failed to load font!");
        }
        return font;
    }

    static sf::Texture& getButtonTexture() {
        static sf::Texture texture;
        if (!texture.loadFromFile(RESOURCES_DIR + "knopka.png")) {
            throw std::runtime_error("Failed to load button texture!");
        }
        return texture;
    }

    static sf::SoundBuffer& getClickSound() {
        static sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(RESOURCES_DIR + "mixkit-arcade-game-jump-coin-216.wav")) {
            throw std::runtime_error("Failed to load click sound!");
        }
        return buffer;
    }

    static sf::SoundBuffer& getWinSound() {
        static sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(RESOURCES_DIR + "9f2836f2b6a3690.mp3")) {
            throw std::runtime_error("Failed to load win sound!");
        }
        return buffer;
    }

    static sf::SoundBuffer& getLoseSound() {
        static sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(RESOURCES_DIR + "e285e54b799801b.mp3")) {
            throw std::runtime_error("Failed to load lose sound!");
        }
        return buffer;
    }

    static sf::Texture& getBackgroundTexture() {
        static sf::Texture texture;
        if (!texture.loadFromFile(RESOURCES_DIR + "background1.jpg")) {
            throw std::runtime_error("Failed to load background texture!");
        }
        return texture;
    }

    static sf::Texture& getShopItemTexture() {
        static sf::Texture texture;
        if (!texture.loadFromFile(RESOURCES_DIR + "knopka.png")) {
            throw std::runtime_error("Failed to load shop item texture!");
        }
        return texture;
    }
};

class Button {
public:
    Button(const std::string& text, sf::Vector2f pos, std::function<void()> action, int zIndex = 0,
        float width = 200.f, float height = 50.f, int fontSize = 24)
        : action(std::move(action)), originalPosition(pos), targetPosition(pos), zIndex(zIndex),
        width(width), height(height), fontSize(fontSize) {
        shape.setSize({ width, height });
        shape.setPosition(pos);
        shape.setTexture(&ResourceManager::getButtonTexture());
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Transparent);

        label.setFont(ResourceManager::getFont());
        label.setString(text);
        label.setCharacterSize(fontSize);
        label.setFillColor(sf::Color::White);
        updateTextPosition();

        visible = true;
        clickProcessed = false;
        wasPressed = false;
        hoverEffectActive = true;
    }

    void setVisible(bool isVisible) {
        visible = isVisible;
        if (!visible) {
            clickProcessed = false;
            wasPressed = false;
            isHovered = false;
        }
    }
    bool isVisible() const { return visible; }
    int getZIndex() const { return zIndex; }
    bool wasPressedThisFrame() const { return wasPressed; }
    void resetPressState() { wasPressed = false; }
    void setHoverEffect(bool active) { hoverEffectActive = active; }
    sf::Vector2f getOriginalPosition() const { return originalPosition; }
    void setOriginalPosition(sf::Vector2f pos) { originalPosition = pos; }
    void setSize(float w, float h) {
        width = w;
        height = h;
        shape.setSize({ width, height });
        updateTextPosition();
    }

    void update(const sf::RenderWindow& window, sf::Time deltaTime, bool& buttonPressedThisFrame, bool allowInteraction) {
        if (!visible || !allowInteraction) {
            clickProcessed = false;
            wasPressed = false;
            isHovered = false;
            return;
        }

        wasPressed = false;

        isHovered = shape.getGlobalBounds().contains(
            static_cast<float>(sf::Mouse::getPosition(window).x),
            static_cast<float>(sf::Mouse::getPosition(window).y)
        );

        if (hoverEffectActive) {
            targetPosition = originalPosition + (isHovered ? sf::Vector2f(0, -10.f) : sf::Vector2f(0, 0));
            sf::Vector2f currentPos = shape.getPosition();
            sf::Vector2f newPos = currentPos + (targetPosition - currentPos) * 10.f * deltaTime.asSeconds();
            shape.setPosition(newPos);

            shape.setFillColor(isHovered ?
                sf::Color(255, 255, 255, 180) :
                sf::Color(255, 255, 255, 140));

            shape.setOutlineColor(isHovered ?
                sf::Color(255, 215, 0, 255) :
                sf::Color::Transparent);
        }

        updateTextPosition();

        if (isHovered && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            if (!clickProcessed && !buttonPressedThisFrame) {
                clickProcessed = true;
                wasPressed = true;
                buttonPressedThisFrame = true;
                action();
            }
        }
        else if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            clickProcessed = false;
        }
    }

    void draw(sf::RenderTarget& target) const {
        if (!visible) return;
        target.draw(shape);
        target.draw(label);
    }

private:
    sf::RectangleShape shape;
    sf::Text label;
    std::function<void()> action;
    bool isHovered = false;
    sf::Vector2f originalPosition;
    sf::Vector2f targetPosition;
    bool visible = true;
    bool clickProcessed = false;
    bool wasPressed = false;
    bool hoverEffectActive = true;
    int zIndex = 0;
    float width;
    float height;
    int fontSize;

    void updateTextPosition() {
        label.setPosition(
            shape.getPosition().x + (width - label.getLocalBounds().width) / 2.f,
            shape.getPosition().y + (height - fontSize) / 2.f - 5.f
        );
    }
};

class NumberGuesser {
public:
    enum Difficulty {
        EASY,       // 1-50, unlimited attempts, no timer
        MEDIUM,     // 1-100, 15 attempts, no timer
        HARD,       // 1-200, 10 attempts, no timer
        EXPERT,     // 1-500, 7 attempts, 2 minute timer
        MASTER      // 1-1000, 5 attempts, 1 minute timer
    };

    enum GameState { MENU, PLAYING, ACHIEVEMENTS, DIFFICULTY, GAME_OVER, SHOP, SETTINGS };

    NumberGuesser() {
        config.load();
        createWindow();
        try {
            initResources();
            initGame();
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            window->close();
        }
    }

    void createWindow() {
        if (config.fullscreen) {
            window = std::make_unique<sf::RenderWindow>(sf::VideoMode::getDesktopMode(), "Shaolin Number!", sf::Style::Fullscreen);
        }
        else {
            window = std::make_unique<sf::RenderWindow>(sf::VideoMode(config.width, config.height), "Shaolin Number!", sf::Style::Close | sf::Style::Titlebar);
        }
    }

    void run() {
        sf::Clock clock;
        while (window->isOpen()) {
            sf::Time deltaTime = clock.restart();
            handleEvents();
            update(deltaTime);
            render();
        }
    }

private:
    struct GuessHistory {
        int value;
        std::string hint;
        sf::Color color;

        GuessHistory(int val, const std::string& h, const sf::Color& c)
            : value(val), hint(h), color(c) {
        }
    };

    struct ShopItem {
        std::string name;
        std::string description;
        int cost;
        bool purchased;
        bool active;
        std::function<void()> applyEffect;
        std::function<void()> removeEffect;

        ShopItem(const std::string& n, const std::string& d, int c, bool p, bool a,
            std::function<void()> apply, std::function<void()> remove)
            : name(n), description(d), cost(c), purchased(p), active(a),
            applyEffect(apply), removeEffect(remove) {
        }
    };

    Config config;
    std::unique_ptr<sf::RenderWindow> window;
    GameState state = MENU;
    Difficulty difficulty = MEDIUM;

    int secretNumber = 0;
    int attempts = 0;
    int maxAttempts = 0;
    int bestScore = 999;
    int range = 100;
    int totalPoints = 0;
    bool gameWon = false;
    bool gameLost = false;
    std::vector<GuessHistory> guessHistory;
    std::string inputStr;
    std::string currentHint = "Make your guess!";
    sf::Color inputColor = sf::Color::White;

    sf::Time timeLimit;
    sf::Time timeRemaining;
    sf::Clock gameClock;
    bool timerActive = false;
    bool timeUp = false;

    sf::Music bgMusic;
    sf::Sound clickSound;
    sf::Sound winSound;
    sf::Sound loseSound;
    std::vector<std::unique_ptr<Button>> buttons;
    std::vector<std::unique_ptr<Button>> gameButtons;
    std::vector<std::unique_ptr<Button>> difficultyButtons;
    std::vector<std::unique_ptr<Button>> achievementButtons;
    std::vector<std::unique_ptr<Button>> shopButtons;
    std::vector<std::unique_ptr<Button>> settingsButtons;
    sf::Text title;
    sf::Sprite background;
    sf::Clock titleAnimationClock;
    float titleScale = 1.0f;
    bool titleGrowing = true;
    float titleRotation = 0.0f;
    sf::Color titleColor = sf::Color::White;
    float colorHue = 0.0f;
    float outlineThickness = 0.f;
    bool outlineGrowing = true;
    bool buttonPressedThisFrame = false;
    bool shopButtonPressed = false;

    // Shop items and abilities
    std::vector<ShopItem> shopItems;
    bool showHintAfterWrongGuess = false;
    bool showRangeAfterFewAttempts = false;
    bool extraAttempt = false;
    bool timeExtension = false;
    bool showEvenOdd = false;

    struct Achievement {
        std::string title;
        std::string desc;
        bool unlocked;
        bool justUnlocked = false;

        Achievement(const std::string& t, const std::string& d, bool u)
            : title(t), desc(d), unlocked(u) {
        }
    };
    std::vector<Achievement> achievements;
    sf::Clock achievementDisplayClock;
    std::string lastUnlockedAchievement;

    void initResources() {
        if (!bgMusic.openFromFile(RESOURCES_DIR + "garmoniya-in-yan-278.mp3")) {
            throw std::runtime_error("Failed to load background music!");
        }
        bgMusic.setLoop(true);
        bgMusic.play();

        clickSound.setBuffer(ResourceManager::getClickSound());
        winSound.setBuffer(ResourceManager::getWinSound());
        loseSound.setBuffer(ResourceManager::getLoseSound());

        background.setTexture(ResourceManager::getBackgroundTexture());
        updateBackgroundScale();

        title.setFont(ResourceManager::getFont());
        title.setString("Shaolin Number");

        // Установка фиксированного размера шрифта в зависимости от разрешения
        if (config.width <= 800) {
            title.setCharacterSize(60);
        }
        else if (config.width <= 1024) {
            title.setCharacterSize(70);
        }
        else if (config.width <= 1280) {
            title.setCharacterSize(80);
        }
        else { // Для полноэкранного режима и больших разрешений
            title.setCharacterSize(90);
        }

        title.setFillColor(titleColor);
        title.setOutlineColor(sf::Color(255, 215, 0));
        title.setOutlineThickness(2.f);
        updateTitlePosition();

        achievements = {
            {"Beginner", "Complete first game", false},
            {"Pro", "Win in 5 tries", false},
            {"Legend", "Win on first try", false},
            {"Time Master", "Win on Expert/Master with time left", false},
            {"Perfect Guess", "Win on Master difficulty", false},
            {"Hot Streak", "Win 3 games in a row", false},
            {"Number Ninja", "Win on all difficulty levels", false},
            {"Persistent", "Make 10 wrong guesses in one game", false},
            {"Close Call", "Win with last attempt", false},
            {"Speed Demon", "Win in under 30 seconds", false},
            {"Cold Blooded", "Win with freezing guess", false},
            {"Completionist", "Unlock all achievements", false}
        };

        // Initialize shop items
        shopItems = {
            {"Hint Helper", "Shows hint after wrong guess", 100, false, false,
                [this]() { showHintAfterWrongGuess = true; },
                [this]() { showHintAfterWrongGuess = false; }},

            {"Range Revealer", "Shows range after 3 attempts", 200, false, false,
                [this]() { showRangeAfterFewAttempts = true; },
                [this]() { showRangeAfterFewAttempts = false; }},

            {"Extra Attempt", "+1 attempt in each game", 300, false, false,
                [this]() { extraAttempt = true; },
                [this]() { extraAttempt = false; }},

            {"Time Extender", "+30 sec in timed modes", 400, false, false,
                [this]() { timeExtension = true; },
                [this]() { timeExtension = false; }},

            {"Odd/Even Hint", "Shows if number is odd/even", 150, false, false,
                [this]() { showEvenOdd = true; },
                [this]() { showEvenOdd = false; }}
        };
    }

    float getScaleFactor() const {
        // Base resolution is 800x600
        float scaleX = static_cast<float>(window->getSize().x) / 800.f;
        float scaleY = static_cast<float>(window->getSize().y) / 600.f;
        return std::min(scaleX, scaleY);
    }

    void updateBackgroundScale() {
        sf::Vector2u textureSize = background.getTexture()->getSize();
        background.setScale(
            static_cast<float>(window->getSize().x) / static_cast<float>(textureSize.x),
            static_cast<float>(window->getSize().y) / static_cast<float>(textureSize.y)
        );
    }

    void updateTitlePosition() {
        sf::FloatRect textRect = title.getLocalBounds();
        title.setOrigin(textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f);
        title.setPosition(static_cast<float>(window->getSize().x) / 2.0f, 80.f);
    }

    void initGame() {
        loadProgress();
        createMenu();
        createGameButtons();
        createDifficultyButtons();
        createAchievementButtons();
        createShopButtons();
        createSettingsButtons();
    }

    void createMenu() {
        buttons.clear();
        float buttonWidth = 220.f * getScaleFactor();
        float buttonHeight = 50.f * getScaleFactor();
        float buttonX = window->getSize().x * 0.1f;
        float startY = window->getSize().y * 0.3f;
        float spacing = 25.f * getScaleFactor();

        buttons.push_back(std::make_unique<Button>("Play", sf::Vector2f(buttonX, startY), [this]() {
            clickSound.play();
            startNewGame();
            }, 1, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));

        buttons.push_back(std::make_unique<Button>("Difficulty", sf::Vector2f(buttonX, startY + buttonHeight + spacing), [this]() {
            clickSound.play();
            state = DIFFICULTY;
            updateButtonVisibility();
            }, 2, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));

        buttons.push_back(std::make_unique<Button>("Achievements", sf::Vector2f(buttonX, startY + (buttonHeight + spacing) * 2), [this]() {
            clickSound.play();
            state = ACHIEVEMENTS;
            updateButtonVisibility();
            }, 3, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));

        buttons.push_back(std::make_unique<Button>("Shop", sf::Vector2f(buttonX, startY + (buttonHeight + spacing) * 3), [this]() {
            clickSound.play();
            state = SHOP;
            updateButtonVisibility();
            }, 4, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));

        buttons.push_back(std::make_unique<Button>("Settings", sf::Vector2f(buttonX, startY + (buttonHeight + spacing) * 4), [this]() {
            clickSound.play();
            state = SETTINGS;
            updateButtonVisibility();
            }, 5, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));

        buttons.push_back(std::make_unique<Button>("Exit", sf::Vector2f(buttonX, startY + (buttonHeight + spacing) * 5), [this]() {
            window->close();
            }, 6, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));
    }

    void createGameButtons() {
        gameButtons.clear();
        float buttonWidth = 220.f * getScaleFactor();
        float buttonHeight = 50.f * getScaleFactor();
        float buttonY = window->getSize().y * 0.85f;
        float spacing = 30.f * getScaleFactor();

        gameButtons.push_back(std::make_unique<Button>("Restart", sf::Vector2f(window->getSize().x * 0.25f - buttonWidth / 2, buttonY), [this]() {
            clickSound.play();
            startNewGame();
            }, 1, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));

        gameButtons.push_back(std::make_unique<Button>("Menu", sf::Vector2f(window->getSize().x * 0.75f - buttonWidth / 2, buttonY), [this]() {
            clickSound.play();
            state = MENU;
            updateButtonVisibility();
            }, 2, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));
    }

    void createDifficultyButtons() {
        difficultyButtons.clear();
        float buttonWidth = 200.f * getScaleFactor();
        float buttonHeight = 45.f * getScaleFactor();
        float startY = window->getSize().y * 0.3f;
        float spacingX = 30.f * getScaleFactor();
        float spacingY = 40.f * getScaleFactor();
        float descOffset = 60.f * getScaleFactor();

        // Calculate total width for centered positioning
        float totalWidth = (buttonWidth * 3) + (spacingX * 2);
        float startX = (window->getSize().x - totalWidth) / 2;

        // First row - Easy, Medium, Hard
        difficultyButtons.push_back(std::make_unique<Button>("Easy", sf::Vector2f(startX, startY), [this]() {
            difficulty = EASY;
            clickSound.play();
            state = MENU;
            updateButtonVisibility();
            }, 1, buttonWidth, buttonHeight, static_cast<int>(22 * getScaleFactor())));

        difficultyButtons.push_back(std::make_unique<Button>("Medium", sf::Vector2f(startX + buttonWidth + spacingX, startY), [this]() {
            difficulty = MEDIUM;
            clickSound.play();
            state = MENU;
            updateButtonVisibility();
            }, 2, buttonWidth, buttonHeight, static_cast<int>(22 * getScaleFactor())));

        difficultyButtons.push_back(std::make_unique<Button>("Hard", sf::Vector2f(startX + (buttonWidth + spacingX) * 2, startY), [this]() {
            difficulty = HARD;
            clickSound.play();
            state = MENU;
            updateButtonVisibility();
            }, 3, buttonWidth, buttonHeight, static_cast<int>(22 * getScaleFactor())));

        // Second row - Expert, Master
        float secondRowY = startY + buttonHeight + descOffset + spacingY;
        difficultyButtons.push_back(std::make_unique<Button>("Expert", sf::Vector2f(startX + buttonWidth / 2, secondRowY), [this]() {
            difficulty = EXPERT;
            clickSound.play();
            state = MENU;
            updateButtonVisibility();
            }, 4, buttonWidth, buttonHeight, static_cast<int>(22 * getScaleFactor())));

        difficultyButtons.push_back(std::make_unique<Button>("Master", sf::Vector2f(startX + buttonWidth + spacingX + buttonWidth / 2, secondRowY), [this]() {
            difficulty = MASTER;
            clickSound.play();
            state = MENU;
            updateButtonVisibility();
            }, 5, buttonWidth, buttonHeight, static_cast<int>(22 * getScaleFactor())));

        // Back button
        difficultyButtons.push_back(std::make_unique<Button>("Back", sf::Vector2f(window->getSize().x - buttonWidth - 30.f * getScaleFactor(),
            window->getSize().y - buttonHeight - 30.f * getScaleFactor()), [this]() {
                clickSound.play();
                state = MENU;
                updateButtonVisibility();
            }, 6, buttonWidth, buttonHeight, static_cast<int>(22 * getScaleFactor())));
    }

    void createAchievementButtons() {
        achievementButtons.clear();
        float buttonWidth = 220.f * getScaleFactor();
        float buttonHeight = 50.f * getScaleFactor();
        float spacing = 30.f * getScaleFactor();

        achievementButtons.push_back(std::make_unique<Button>("Back", sf::Vector2f(window->getSize().x - buttonWidth - spacing,
            window->getSize().y - buttonHeight - spacing), [this]() {
                clickSound.play();
                state = MENU;
                updateButtonVisibility();
            }, 1, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));
    }

    void createShopButtons() {
        shopButtons.clear();
        float buttonWidth = 220.f * getScaleFactor();
        float buttonHeight = 50.f * getScaleFactor();
        float spacing = 30.f * getScaleFactor();

        shopButtons.push_back(std::make_unique<Button>("Back", sf::Vector2f(window->getSize().x - buttonWidth - spacing,
            window->getSize().y - buttonHeight - spacing), [this]() {
                clickSound.play();
                state = MENU;
                updateButtonVisibility();
            }, 1, buttonWidth, buttonHeight, static_cast<int>(24 * getScaleFactor())));
    }

    void createSettingsButtons() {
        settingsButtons.clear();
        float buttonWidth = 220.f * getScaleFactor();
        float buttonHeight = 45.f * getScaleFactor();
        float buttonX = window->getSize().x * 0.6f;
        float startY = window->getSize().y * 0.3f;
        float spacing = 25.f * getScaleFactor();

        settingsButtons.push_back(std::make_unique<Button>("800x600", sf::Vector2f(buttonX, startY), [this]() {
            clickSound.play();
            config.width = 800;
            config.height = 600;
            config.fullscreen = false;
            applySettings();
            }, 1, buttonWidth, buttonHeight, static_cast<int>(20 * getScaleFactor())));

        settingsButtons.push_back(std::make_unique<Button>("1024x768", sf::Vector2f(buttonX, startY + buttonHeight + spacing), [this]() {
            clickSound.play();
            config.width = 1024;
            config.height = 768;
            config.fullscreen = false;
            applySettings();
            }, 2, buttonWidth, buttonHeight, static_cast<int>(20 * getScaleFactor())));

        settingsButtons.push_back(std::make_unique<Button>("1280x720", sf::Vector2f(buttonX, startY + (buttonHeight + spacing) * 2), [this]() {
            clickSound.play();
            config.width = 1280;
            config.height = 720;
            config.fullscreen = false;
            applySettings();
            }, 3, buttonWidth, buttonHeight, static_cast<int>(20 * getScaleFactor())));

        settingsButtons.push_back(std::make_unique<Button>("Fullscreen", sf::Vector2f(buttonX, startY + (buttonHeight + spacing) * 3), [this]() {
            clickSound.play();
            config.fullscreen = true;
            applySettings();
            }, 4, buttonWidth, buttonHeight, static_cast<int>(20 * getScaleFactor())));

        settingsButtons.push_back(std::make_unique<Button>("Reset Progress", sf::Vector2f(buttonX, startY + (buttonHeight + spacing) * 4), [this]() {
            clickSound.play();
            resetProgress();
            }, 5, buttonWidth, buttonHeight, static_cast<int>(20 * getScaleFactor())));

        settingsButtons.push_back(std::make_unique<Button>("Back", sf::Vector2f(window->getSize().x - buttonWidth - 30.f * getScaleFactor(),
            window->getSize().y - buttonHeight - 30.f * getScaleFactor()), [this]() {
                clickSound.play();
                state = MENU;
                updateButtonVisibility();
            }, 6, buttonWidth, buttonHeight, static_cast<int>(20 * getScaleFactor())));
    }

    void resetProgress() {
        bestScore = 999;
        totalPoints = 0;

        for (auto& a : achievements) {
            a.unlocked = false;
            a.justUnlocked = false;
        }

        for (auto& item : shopItems) {
            item.purchased = false;
            item.active = false;
            if (item.removeEffect) item.removeEffect();
        }

        // Reset game state
        showHintAfterWrongGuess = false;
        showRangeAfterFewAttempts = false;
        extraAttempt = false;
        timeExtension = false;
        showEvenOdd = false;

        saveProgress();
    }

    void applySettings() {
        config.save();
        createWindow();
        updateBackgroundScale();
        updateTitlePosition();

        // Recreate all buttons with new positions
        initGame();
        updateButtonVisibility();
    }

    void updateButtonVisibility() {
        for (auto& btn : buttons) btn->setVisible(state == MENU);
        for (auto& btn : gameButtons) btn->setVisible(state == PLAYING || state == GAME_OVER);
        for (auto& btn : difficultyButtons) btn->setVisible(state == DIFFICULTY);
        for (auto& btn : achievementButtons) btn->setVisible(state == ACHIEVEMENTS);
        for (auto& btn : shopButtons) btn->setVisible(state == SHOP);
        for (auto& btn : settingsButtons) btn->setVisible(state == SETTINGS);
    }

    void startNewGame() {
        state = PLAYING;
        gameWon = false;
        gameLost = false;
        timeUp = false;
        attempts = 0;
        inputStr.clear();
        guessHistory.clear();
        currentHint = "Make your guess!";

        timerActive = false;

        generateNumber();
        setupDifficultySettings();
        updateButtonVisibility();

        // Apply active shop items
        for (auto& item : shopItems) {
            if (item.active && item.applyEffect) {
                item.applyEffect();
            }
        }

        if (extraAttempt && maxAttempts > 0) {
            maxAttempts++;
        }

        if (timeExtension && timerActive) {
            timeLimit += sf::seconds(30);
            timeRemaining = timeLimit;
        }
    }

    void setupDifficultySettings() {
        switch (difficulty) {
        case EASY:
            range = 50;
            maxAttempts = 0;
            timerActive = false;
            break;
        case MEDIUM:
            range = 100;
            maxAttempts = 15;
            timerActive = false;
            break;
        case HARD:
            range = 200;
            maxAttempts = 10;
            timerActive = false;
            break;
        case EXPERT:
            range = 500;
            maxAttempts = 7;
            timeLimit = sf::seconds(120);
            timeRemaining = timeLimit;
            timerActive = true;
            break;
        case MASTER:
            range = 1000;
            maxAttempts = 5;
            timeLimit = sf::seconds(60);
            timeRemaining = timeLimit;
            timerActive = true;
            break;
        }

        if (timerActive) {
            gameClock.restart();
        }
    }

    void generateNumber() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(1, range);
        secretNumber = distr(gen);
    }

    void handleEvents() {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                saveProgress();
                window->close();
            }

            if (state == PLAYING && event.type == sf::Event::TextEntered) {
                handleInput(event.text.unicode);
            }

            if (state == PLAYING && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    clickSound.play();
                    state = MENU;
                    updateButtonVisibility();
                }
                else if (event.key.code == sf::Keyboard::R) {
                    clickSound.play();
                    startNewGame();
                }
            }

            if (state == SHOP && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    shopButtonPressed = true;
                }
            }
            else if (state == SHOP && event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    shopButtonPressed = false;
                }
            }
        }
    }

    void handleInput(sf::Uint32 code) {
        if (code == '\b') {
            if (!inputStr.empty()) inputStr.pop_back();
        }
        else if (code >= '0' && code <= '9') {
            if (inputStr.size() < 4) inputStr += static_cast<char>(code);
        }
        else if (code == '\r') {
            processGuess();
        }
    }

    void processGuess() {
        if (gameWon || gameLost || timeUp) return;

        try {
            int guess = std::stoi(inputStr);
            if (guess < 1 || guess > range) return;

            attempts++;
            updateTemperature(guess);

            // Формируем основную подсказку
            std::string fullHint = currentHint;

            // Добавляем четность/нечетность, если улучшение активно
            if (showEvenOdd && guess != secretNumber) {
                fullHint += " (" + std::string(secretNumber % 2 == 0 ? "Even" : "Odd") + ")";
            }

            // Добавляем направление, если улучшение активно
            if (showHintAfterWrongGuess && guess != secretNumber) {
                fullHint += " (" + std::string(guess < secretNumber ? "Higher" : "Lower") + ")";
            }

            guessHistory.emplace_back(guess, fullHint, inputColor);

            if (guess == secretNumber) {
                gameWon = true;
                winSound.play();
                if (attempts < bestScore) bestScore = attempts;

                switch (difficulty) {
                case EASY: totalPoints += 10; break;
                case MEDIUM: totalPoints += 25; break;
                case HARD: totalPoints += 50; break;
                case EXPERT: totalPoints += 100; break;
                case MASTER: totalPoints += 200; break;
                }

                checkAchievements();
                checkWinAchievements();
            }
            else if ((maxAttempts > 0 && attempts >= maxAttempts) || timeUp) {
                gameLost = true;
                loseSound.play();
                state = GAME_OVER;
                updateButtonVisibility();
                checkLoseAchievements();
            }

            inputStr.clear();

            if (showRangeAfterFewAttempts && attempts >= 3 && !gameWon && !gameLost) {
                int lower = std::max(1, secretNumber - range / 10);
                int upper = std::min(range, secretNumber + range / 10);
                currentHint = "Range: " + std::to_string(lower) + "-" + std::to_string(upper);
            }
        }
        catch (...) {
            inputStr.clear();
        }
    }

    void updateTemperature(int guess) {
        float diff = std::abs(guess - secretNumber) / static_cast<float>(range);

        if (diff < 0.05f) {
            currentHint = "BOILING HOT!";
            inputColor = sf::Color(255, 0, 0);
        }
        else if (diff < 0.1f) {
            currentHint = "Very Hot";
            inputColor = sf::Color(255, 50, 0);
        }
        else if (diff < 0.2f) {
            currentHint = "Hot";
            inputColor = sf::Color(255, 100, 0);
        }
        else if (diff < 0.3f) {
            currentHint = "Warm";
            inputColor = sf::Color(255, 165, 0);
        }
        else if (diff < 0.4f) {
            currentHint = "Cool";
            inputColor = sf::Color(255, 255, 0);
        }
        else if (diff < 0.6f) {
            currentHint = "Cold";
            inputColor = sf::Color(100, 100, 255);
        }
        else {
            currentHint = "FREEZING!";
            inputColor = sf::Color(0, 0, 255);
            unlockAchievement(10);
        }
    }

    void checkAchievements() {
        if (attempts == 1) unlockAchievement(2);
        if (attempts <= 5) unlockAchievement(1);
        if (!achievements[0].unlocked) unlockAchievement(0);

        if (attempts >= 10) unlockAchievement(7);
        if (maxAttempts > 0 && attempts == maxAttempts - 1) unlockAchievement(8);
    }

    void checkWinAchievements() {
        if (timerActive && (difficulty == EXPERT || difficulty == MASTER) &&
            timeRemaining > sf::Time::Zero) {
            unlockAchievement(3);
        }

        if (difficulty == MASTER) {
            unlockAchievement(4);
        }

        if (timerActive && gameClock.getElapsedTime().asSeconds() < 30) {
            unlockAchievement(9);
        }

        bool allDifficulties = true;
        for (int i = 0; i <= MASTER; ++i) {
            allDifficulties = allDifficulties && (i == difficulty);
        }
        if (allDifficulties) unlockAchievement(6);

        bool allUnlocked = true;
        for (const auto& a : achievements) {
            if (!a.unlocked && a.title != "Completionist") {
                allUnlocked = false;
                break;
            }
        }
        if (allUnlocked) unlockAchievement(11);
    }

    void checkLoseAchievements() {
    }

    void unlockAchievement(int idx) {
        if (idx < 0 || idx >= static_cast<int>(achievements.size())) return;
        if (!achievements[idx].unlocked) {
            achievements[idx].unlocked = true;
            achievements[idx].justUnlocked = true;
            lastUnlockedAchievement = achievements[idx].title;
            achievementDisplayClock.restart();
            saveProgress();
        }
    }

    void saveProgress() {
        std::ofstream file(SAVE_FILE);
        if (file) {
            file << bestScore << "\n";
            file << totalPoints << "\n";
            for (const auto& a : achievements) {
                file << a.unlocked << " ";
            }
            file << "\n";
            for (const auto& item : shopItems) {
                file << item.purchased << " " << item.active << " ";
            }
        }
    }

    void loadProgress() {
        std::ifstream file(SAVE_FILE);
        if (file) {
            file >> bestScore;
            file >> totalPoints;
            for (auto& a : achievements) {
                bool unlocked;
                if (file >> unlocked) a.unlocked = unlocked;
            }
            for (auto& item : shopItems) {
                bool purchased, active;
                if (file >> purchased >> active) {
                    item.purchased = purchased;
                    item.active = active;
                    if (active && item.applyEffect) {
                        item.applyEffect();
                    }
                }
            }
        }
    }

    void purchaseItem(int index) {
        if (index < 0 || index >= static_cast<int>(shopItems.size())) return;
        if (!shopItems[index].purchased && totalPoints >= shopItems[index].cost) {
            totalPoints -= shopItems[index].cost;
            shopItems[index].purchased = true;
            shopItems[index].active = true;
            if (shopItems[index].applyEffect) {
                shopItems[index].applyEffect();
            }
            saveProgress();
        }
        else if (shopItems[index].purchased) {
            shopItems[index].active = !shopItems[index].active;
            if (shopItems[index].active) {
                if (shopItems[index].applyEffect) shopItems[index].applyEffect();
            }
            else {
                if (shopItems[index].removeEffect) shopItems[index].removeEffect();
            }
            saveProgress();
        }
    }

    sf::Color hslToRgb(float h, float s, float l) {
        float c = (1 - std::abs(2 * l - 1)) * s;
        float x = c * (1 - std::abs(std::fmod(h / 60.0f, 2) - 1));
        float m = l - c / 2.0f;

        float r, g, b;
        if (h < 60) { r = c; g = x; b = 0; }
        else if (h < 120) { r = x; g = c; b = 0; }
        else if (h < 180) { r = 0; g = c; b = x; }
        else if (h < 240) { r = 0; g = x; b = c; }
        else if (h < 300) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }

        return sf::Color(
            static_cast<sf::Uint8>((r + m) * 255),
            static_cast<sf::Uint8>((g + m) * 255),
            static_cast<sf::Uint8>((b + m) * 255)
        );
    }

    void update(sf::Time deltaTime) {
        float elapsed = titleAnimationClock.getElapsedTime().asSeconds();
        float delta = deltaTime.asSeconds();

        // Фиксированные параметры анимации для всех разрешений
        if (titleGrowing) {
            titleScale += delta * 0.3f;
            if (titleScale >= 1.1f) titleGrowing = false;
        }
        else {
            titleScale -= delta * 0.3f;
            if (titleScale <= 0.9f) titleGrowing = true;
        }

        titleRotation = 3.0f * std::sin(elapsed * 1.5f);
        colorHue = std::fmod(elapsed * 45.0f, 360.0f);
        titleColor = hslToRgb(colorHue, 0.8f, 0.7f);

        if (outlineGrowing) {
            outlineThickness += delta * 0.8f;
            if (outlineThickness >= 3.f) outlineGrowing = false;
        }
        else {
            outlineThickness -= delta * 0.8f;
            if (outlineThickness <= 1.f) outlineGrowing = true;
        }

        if (timerActive && state == PLAYING && !gameWon && !gameLost) {
            timeRemaining = timeLimit - gameClock.getElapsedTime();
            if (timeRemaining <= sf::Time::Zero) {
                timeUp = true;
                gameLost = true;
                loseSound.play();
                state = GAME_OVER;
                updateButtonVisibility();
            }
        }

        bool anyButtonPressed = false;
        buttonPressedThisFrame = false;

        auto updateButtons = [&](std::vector<std::unique_ptr<Button>>& buttons, bool allowInteraction) {
            for (auto& btn : buttons) {
                btn->update(*window, deltaTime, buttonPressedThisFrame, allowInteraction);
                if (btn->wasPressedThisFrame()) {
                    anyButtonPressed = true;
                }
            }
            };

        switch (state) {
        case MENU:
            updateButtons(buttons, true);
            updateButtons(gameButtons, false);
            updateButtons(difficultyButtons, false);
            updateButtons(achievementButtons, false);
            updateButtons(shopButtons, false);
            updateButtons(settingsButtons, false);
            break;
        case PLAYING:
        case GAME_OVER:
            updateButtons(gameButtons, true);
            updateButtons(buttons, false);
            updateButtons(difficultyButtons, false);
            updateButtons(achievementButtons, false);
            updateButtons(shopButtons, false);
            updateButtons(settingsButtons, false);
            break;
        case DIFFICULTY:
            updateButtons(difficultyButtons, true);
            updateButtons(buttons, false);
            updateButtons(gameButtons, false);
            updateButtons(achievementButtons, false);
            updateButtons(shopButtons, false);
            updateButtons(settingsButtons, false);
            break;
        case ACHIEVEMENTS:
            updateButtons(achievementButtons, true);
            updateButtons(buttons, false);
            updateButtons(gameButtons, false);
            updateButtons(difficultyButtons, false);
            updateButtons(shopButtons, false);
            updateButtons(settingsButtons, false);
            break;
        case SHOP:
            updateButtons(shopButtons, true);
            updateButtons(buttons, false);
            updateButtons(gameButtons, false);
            updateButtons(difficultyButtons, false);
            updateButtons(achievementButtons, false);
            updateButtons(settingsButtons, false);
            break;
        case SETTINGS:
            updateButtons(settingsButtons, true);
            updateButtons(buttons, false);
            updateButtons(gameButtons, false);
            updateButtons(difficultyButtons, false);
            updateButtons(achievementButtons, false);
            updateButtons(shopButtons, false);
            break;
        }

        if (anyButtonPressed) {
            auto resetButtons = [](std::vector<std::unique_ptr<Button>>& buttons) {
                for (auto& btn : buttons) btn->resetPressState();
                };

            switch (state) {
            case MENU: resetButtons(buttons); break;
            case PLAYING:
            case GAME_OVER: resetButtons(gameButtons); break;
            case DIFFICULTY: resetButtons(difficultyButtons); break;
            case ACHIEVEMENTS: resetButtons(achievementButtons); break;
            case SHOP: resetButtons(shopButtons); break;
            case SETTINGS: resetButtons(settingsButtons); break;
            }
        }

        if (achievementDisplayClock.getElapsedTime().asSeconds() > 3.0f) {
            for (auto& a : achievements) {
                a.justUnlocked = false;
            }
        }
    }

    void render() {
        window->clear();
        window->draw(background);

        sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(window->getSize().x), static_cast<float>(window->getSize().y)));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window->draw(overlay);

        switch (state) {
        case MENU: renderMenu(); break;
        case PLAYING: renderGame(); break;
        case ACHIEVEMENTS: renderAchievements(); break;
        case DIFFICULTY: renderDifficulty(); break;
        case GAME_OVER: renderGameOver(); break;
        case SHOP: renderShop(); break;
        case SETTINGS: renderSettings(); break;
        }

        for (const auto& a : achievements) {
            if (a.justUnlocked) {
                renderAchievementUnlocked(a.title);
                break;
            }
        }

        window->display();
    }

    void renderAchievementUnlocked(const std::string& achievementName) {
        float elapsed = achievementDisplayClock.getElapsedTime().asSeconds();
        if (elapsed > 3.0f) return;

        float alpha = 255;
        if (elapsed > 2.5f) {
            alpha = 255 * (3.0f - elapsed) / 0.5f;
        }

        sf::RectangleShape bg(sf::Vector2f(500.f * getScaleFactor(), 80.f * getScaleFactor()));
        bg.setPosition(window->getSize().x / 2 - 250.f * getScaleFactor(), 50.f * getScaleFactor());
        bg.setFillColor(sf::Color(0, 100, 0, static_cast<sf::Uint8>(alpha * 0.8f)));
        bg.setOutlineThickness(2.f * getScaleFactor());
        bg.setOutlineColor(sf::Color(255, 215, 0, static_cast<sf::Uint8>(alpha)));

        sf::Text text("Achievement Unlocked: " + achievementName, ResourceManager::getFont(), static_cast<unsigned int>(24 * getScaleFactor()));
        text.setPosition(window->getSize().x / 2 - text.getLocalBounds().width / 2, 70.f * getScaleFactor());
        text.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));

        window->draw(bg);
        window->draw(text);
    }

    void renderMenu() {
        title.setScale(titleScale, titleScale);
        title.setRotation(titleRotation);
        title.setFillColor(titleColor);
        title.setOutlineThickness(outlineThickness);
        window->draw(title);

        sf::Text pointsText("Points: " + std::to_string(totalPoints), ResourceManager::getFont(), static_cast<unsigned int>(24 * getScaleFactor()));
        pointsText.setPosition(window->getSize().x - pointsText.getLocalBounds().width - 30.f * getScaleFactor(), 30.f * getScaleFactor());
        pointsText.setFillColor(sf::Color::Yellow);
        window->draw(pointsText);

        std::vector<Button*> sortedButtons;
        for (const auto& btn : buttons) {
            sortedButtons.push_back(btn.get());
        }
        std::sort(sortedButtons.begin(), sortedButtons.end(), [](Button* a, Button* b) {
            return a->getZIndex() < b->getZIndex();
            });

        for (auto btn : sortedButtons) {
            btn->draw(*window);
        }
    }

    void renderGame() {
        sf::Text gameTitle("Guess the Number", ResourceManager::getFont(), static_cast<unsigned int>(40 * getScaleFactor()));
        gameTitle.setPosition(static_cast<float>(window->getSize().x) / 2 - gameTitle.getLocalBounds().width / 2, 20.f * getScaleFactor());
        gameTitle.setFillColor(sf::Color::White);
        window->draw(gameTitle);

        std::string difficultyText;
        switch (difficulty) {
        case EASY: difficultyText = "Easy (1-50)"; break;
        case MEDIUM: difficultyText = "Medium (1-100)"; break;
        case HARD: difficultyText = "Hard (1-200)"; break;
        case EXPERT: difficultyText = "Expert (1-500)"; break;
        case MASTER: difficultyText = "Master (1-1000)"; break;
        }

        sf::Text difficultyDisplay(difficultyText, ResourceManager::getFont(), static_cast<unsigned int>(20 * getScaleFactor()));
        difficultyDisplay.setPosition(30.f * getScaleFactor(), 70.f * getScaleFactor());
        difficultyDisplay.setFillColor(sf::Color::Yellow);
        window->draw(difficultyDisplay);

        std::string attemptsText = maxAttempts > 0 ?
            "Attempts: " + std::to_string(attempts) + "/" + std::to_string(maxAttempts) :
            "Attempts: " + std::to_string(attempts);

        sf::Text attemptsDisplay(attemptsText, ResourceManager::getFont(), static_cast<unsigned int>(20 * getScaleFactor()));
        attemptsDisplay.setPosition(window->getSize().x - attemptsDisplay.getLocalBounds().width - 30.f * getScaleFactor(), 70.f * getScaleFactor());
        attemptsDisplay.setFillColor(sf::Color::Yellow);
        window->draw(attemptsDisplay);

        if (timerActive) {
            int seconds = static_cast<int>(timeRemaining.asSeconds());
            int minutes = seconds / 60;
            seconds %= 60;

            std::ostringstream timeStream;
            timeStream << "Time: " << std::setw(2) << std::setfill('0') << minutes << ":"
                << std::setw(2) << std::setfill('0') << seconds;

            sf::Text timerDisplay(timeStream.str(), ResourceManager::getFont(), static_cast<unsigned int>(24 * getScaleFactor()));
            timerDisplay.setPosition(window->getSize().x / 2 - timerDisplay.getLocalBounds().width / 2, 70.f * getScaleFactor());

            if (timeRemaining < sf::seconds(10)) {
                float blink = std::sin(gameClock.getElapsedTime().asSeconds() * 10.f) * 0.5f + 0.5f;
                timerDisplay.setFillColor(sf::Color(255, static_cast<sf::Uint8>(blink * 255), 0));
            }
            else if (timeRemaining < sf::seconds(30)) {
                timerDisplay.setFillColor(sf::Color::Red);
            }
            else {
                timerDisplay.setFillColor(sf::Color::Green);
            }

            window->draw(timerDisplay);
        }

        sf::RectangleShape inputBg(sf::Vector2f(400.f * getScaleFactor(), 80.f * getScaleFactor()));
        inputBg.setPosition(50.f * getScaleFactor(), 120.f * getScaleFactor());
        inputBg.setFillColor(sf::Color(0, 0, 0, 100));
        inputBg.setOutlineThickness(2.f * getScaleFactor());
        inputBg.setOutlineColor(sf::Color::White);
        window->draw(inputBg);

        sf::Text prompt("Enter number (1-" + std::to_string(range) + "):", ResourceManager::getFont(), static_cast<unsigned int>(24 * getScaleFactor()));
        prompt.setPosition(60.f * getScaleFactor(), 130.f * getScaleFactor());
        window->draw(prompt);

        sf::Text input(inputStr, ResourceManager::getFont(), static_cast<unsigned int>(36 * getScaleFactor()));
        input.setPosition(60.f * getScaleFactor(), 160.f * getScaleFactor());
        input.setFillColor(inputColor);
        window->draw(input);

        sf::Text hint(currentHint, ResourceManager::getFont(), static_cast<unsigned int>(30 * getScaleFactor()));
        hint.setPosition(470.f * getScaleFactor(), 140.f * getScaleFactor());
        hint.setFillColor(sf::Color::Yellow);
        window->draw(hint);

        sf::Text historyTitle("Your guesses:", ResourceManager::getFont(), static_cast<unsigned int>(30 * getScaleFactor()));
        historyTitle.setPosition(50.f * getScaleFactor(), 220.f * getScaleFactor());
        window->draw(historyTitle);

        const int maxPerRow = (window->getSize().x - 100 * getScaleFactor()) / static_cast<int>(300 * getScaleFactor());
        const int rowHeight = static_cast<int>(40 * getScaleFactor());

        for (size_t i = 0; i < guessHistory.size(); ++i) {
            int row = static_cast<int>(i) / maxPerRow;
            int col = static_cast<int>(i) % maxPerRow;

            float xPos = 50.f * getScaleFactor() + col * 300.f * getScaleFactor();
            float yPos = 260.f * getScaleFactor() + row * rowHeight;

            if (yPos > window->getSize().y - 100 * getScaleFactor()) {
                guessHistory.erase(guessHistory.begin(), guessHistory.begin() + maxPerRow);
                i -= maxPerRow;
                continue;
            }

            sf::Text guessText(
                std::to_string(guessHistory[i].value) + " (" + guessHistory[i].hint + ")",
                ResourceManager::getFont(), static_cast<unsigned int>(24 * getScaleFactor())
            );
            guessText.setPosition(xPos, yPos);
            guessText.setFillColor(guessHistory[i].color);
            window->draw(guessText);
        }

        if (gameWon) {
            sf::Text win("YOU WIN! Attempts: " + std::to_string(attempts),
                ResourceManager::getFont(), static_cast<unsigned int>(50 * getScaleFactor()));
            win.setPosition(static_cast<float>(window->getSize().x) / 2 - win.getLocalBounds().width / 2, 400.f * getScaleFactor());
            win.setFillColor(sf::Color::Green);
            window->draw(win);

            if (timerActive) {
                int seconds = static_cast<int>(timeRemaining.asSeconds());
                int minutes = seconds / 60;
                seconds %= 60;

                std::ostringstream timeStream;
                timeStream << "Time left: " << std::setw(2) << std::setfill('0') << minutes << ":"
                    << std::setw(2) << std::setfill('0') << seconds;

                sf::Text timeLeft(timeStream.str(), ResourceManager::getFont(), static_cast<unsigned int>(30 * getScaleFactor()));
                timeLeft.setPosition(window->getSize().x / 2 - timeLeft.getLocalBounds().width / 2, 460.f * getScaleFactor());
                timeLeft.setFillColor(sf::Color::Cyan);
                window->draw(timeLeft);
            }
        }

        std::vector<Button*> sortedButtons;
        for (const auto& btn : gameButtons) {
            sortedButtons.push_back(btn.get());
        }
        std::sort(sortedButtons.begin(), sortedButtons.end(), [](Button* a, Button* b) {
            return a->getZIndex() < b->getZIndex();
            });

        for (auto btn : sortedButtons) {
            btn->draw(*window);
        }
    }

    void renderGameOver() {
        sf::Text gameOverText("GAME OVER", ResourceManager::getFont(), static_cast<unsigned int>(60 * getScaleFactor()));
        gameOverText.setPosition(static_cast<float>(window->getSize().x) / 2 - gameOverText.getLocalBounds().width / 2, 150.f * getScaleFactor());
        gameOverText.setFillColor(sf::Color::Red);
        window->draw(gameOverText);

        std::string resultText = timeUp ?
            "Time's up! The number was: " + std::to_string(secretNumber) :
            "Out of attempts! The number was: " + std::to_string(secretNumber);

        sf::Text result(resultText, ResourceManager::getFont(), static_cast<unsigned int>(30 * getScaleFactor()));
        result.setPosition(static_cast<float>(window->getSize().x) / 2 - result.getLocalBounds().width / 2, 250.f * getScaleFactor());
        result.setFillColor(sf::Color::White);
        window->draw(result);

        sf::Text attemptsText("Your attempts: " + std::to_string(attempts), ResourceManager::getFont(), static_cast<unsigned int>(30 * getScaleFactor()));
        attemptsText.setPosition(static_cast<float>(window->getSize().x) / 2 - attemptsText.getLocalBounds().width / 2, 300.f * getScaleFactor());
        attemptsText.setFillColor(sf::Color::Yellow);
        window->draw(attemptsText);

        std::vector<Button*> sortedButtons;
        for (const auto& btn : gameButtons) {
            sortedButtons.push_back(btn.get());
        }
        std::sort(sortedButtons.begin(), sortedButtons.end(), [](Button* a, Button* b) {
            return a->getZIndex() < b->getZIndex();
            });

        for (auto btn : sortedButtons) {
            btn->draw(*window);
        }
    }

    void renderAchievements() {
        sf::Text title("Achievements", ResourceManager::getFont(), static_cast<unsigned int>(50 * getScaleFactor()));
        title.setPosition(static_cast<float>(window->getSize().x) / 2 - title.getLocalBounds().width / 2, 50.f * getScaleFactor());
        title.setFillColor(sf::Color::White);
        window->draw(title);

        const float areaWidth = window->getSize().x - 100.f * getScaleFactor();
        const float areaHeight = window->getSize().y - 200.f * getScaleFactor();
        const float areaX = 50.f * getScaleFactor();
        const float areaY = 120.f * getScaleFactor();

        sf::RectangleShape achievementsBg(sf::Vector2f(areaWidth, areaHeight));
        achievementsBg.setPosition(areaX, areaY);
        achievementsBg.setFillColor(sf::Color(0, 0, 0, 150));
        achievementsBg.setOutlineThickness(2.f * getScaleFactor());
        achievementsBg.setOutlineColor(sf::Color::White);
        window->draw(achievementsBg);

        const float entryHeight = 60.f * getScaleFactor();
        const float entryWidth = areaWidth - 20.f * getScaleFactor();
        const float startX = areaX + 10.f * getScaleFactor();
        const float startY = areaY + 10.f * getScaleFactor();
        const float padding = 10.f * getScaleFactor();

        for (size_t i = 0; i < achievements.size(); ++i) {
            float yPos = startY + i * (entryHeight + padding);

            if (yPos + entryHeight > areaY + areaHeight) continue;

            sf::RectangleShape entryBg(sf::Vector2f(entryWidth, entryHeight));
            entryBg.setPosition(startX, yPos);
            entryBg.setFillColor(sf::Color(0, 0, 0, 100));
            entryBg.setOutlineThickness(1.f * getScaleFactor());
            entryBg.setOutlineColor(achievements[i].unlocked ? sf::Color::Green : sf::Color::Red);
            window->draw(entryBg);

            sf::Text titleText(achievements[i].title, ResourceManager::getFont(), static_cast<unsigned int>(20 * getScaleFactor()));
            titleText.setPosition(startX + 10.f * getScaleFactor(), yPos + 5.f * getScaleFactor());
            titleText.setFillColor(achievements[i].unlocked ? sf::Color::Green : sf::Color(150, 150, 150));
            window->draw(titleText);

            sf::Text descText(achievements[i].desc, ResourceManager::getFont(), static_cast<unsigned int>(16 * getScaleFactor()));
            descText.setPosition(startX + 10.f * getScaleFactor(), yPos + 30.f * getScaleFactor());
            descText.setFillColor(sf::Color::White);
            window->draw(descText);

            sf::Text statusText(achievements[i].unlocked ? "[X]" : "[ ]", ResourceManager::getFont(), static_cast<unsigned int>(20 * getScaleFactor()));
            statusText.setPosition(startX + entryWidth - 40.f * getScaleFactor(), yPos + 20.f * getScaleFactor());
            statusText.setFillColor(achievements[i].unlocked ? sf::Color::Green : sf::Color::Red);
            window->draw(statusText);
        }

        std::vector<Button*> sortedButtons;
        for (const auto& btn : achievementButtons) {
            sortedButtons.push_back(btn.get());
        }
        std::sort(sortedButtons.begin(), sortedButtons.end(), [](Button* a, Button* b) {
            return a->getZIndex() < b->getZIndex();
            });

        for (auto btn : sortedButtons) {
            btn->draw(*window);
        }
    }

    void renderDifficulty() {
        sf::Text title("Select Difficulty", ResourceManager::getFont(), static_cast<unsigned int>(50 * getScaleFactor()));
        title.setPosition(static_cast<float>(window->getSize().x) / 2 - title.getLocalBounds().width / 2, 50.f * getScaleFactor());
        title.setFillColor(sf::Color::White);
        window->draw(title);

        const float buttonWidth = 200.f * getScaleFactor();
        const float buttonHeight = 45.f * getScaleFactor();
        const float spacingX = 30.f * getScaleFactor();
        const float spacingY = 40.f * getScaleFactor();
        const float descOffset = 60.f * getScaleFactor();
        const float totalWidth = (buttonWidth * 3) + (spacingX * 2);
        const float startX = (window->getSize().x - totalWidth) / 2;
        const float startY = window->getSize().y * 0.3f;

        // First row - Easy, Medium, Hard
        std::vector<std::pair<std::string, std::vector<std::string>>> firstRowDifficulties = {
            {"Easy", {"Range: 1-50", "Attempts: Unlimited", "Timer: No"}},
            {"Medium", {"Range: 1-100", "Attempts: 15", "Timer: No"}},
            {"Hard", {"Range: 1-200", "Attempts: 10", "Timer: No"}}
        };

        for (size_t i = 0; i < firstRowDifficulties.size(); ++i) {
            float xPos = startX + i * (buttonWidth + spacingX);
            float yPos = startY;

            // Draw button
            difficultyButtons[i]->draw(*window);

            // Draw description box
            sf::RectangleShape descBox(sf::Vector2f(buttonWidth, 100.f * getScaleFactor()));
            descBox.setPosition(xPos, yPos + buttonHeight + 10.f * getScaleFactor());
            descBox.setFillColor(sf::Color(0, 0, 0, 150));
            descBox.setOutlineThickness(2.f * getScaleFactor());
            descBox.setOutlineColor(sf::Color::White);
            window->draw(descBox);

            // Draw description text
            for (size_t j = 0; j < firstRowDifficulties[i].second.size(); ++j) {
                sf::Text descText(firstRowDifficulties[i].second[j], ResourceManager::getFont(), static_cast<unsigned int>(14 * getScaleFactor()));
                descText.setPosition(xPos + 10.f * getScaleFactor(), yPos + buttonHeight + 20.f * getScaleFactor() + j * 20.f * getScaleFactor());
                descText.setFillColor(sf::Color::White);
                window->draw(descText);
            }
        }

        // Second row - Expert, Master
        std::vector<std::pair<std::string, std::vector<std::string>>> secondRowDifficulties = {
            {"Expert", {"Range: 1-500", "Attempts: 7", "Timer: 2 minutes"}},
            {"Master", {"Range: 1-1000", "Attempts: 5", "Timer: 1 minute"}}
        };

        float secondRowY = startY + buttonHeight + descOffset + spacingY;
        for (size_t i = 0; i < secondRowDifficulties.size(); ++i) {
            float xPos = startX + (buttonWidth + spacingX) * i + buttonWidth / 2;
            float yPos = secondRowY;

            // Draw button
            difficultyButtons[3 + i]->draw(*window);

            // Draw description box
            sf::RectangleShape descBox(sf::Vector2f(buttonWidth, 100.f * getScaleFactor()));
            descBox.setPosition(xPos, yPos + buttonHeight + 10.f * getScaleFactor());
            descBox.setFillColor(sf::Color(0, 0, 0, 150));
            descBox.setOutlineThickness(2.f * getScaleFactor());
            descBox.setOutlineColor(sf::Color::White);
            window->draw(descBox);

            // Draw description text
            for (size_t j = 0; j < secondRowDifficulties[i].second.size(); ++j) {
                sf::Text descText(secondRowDifficulties[i].second[j], ResourceManager::getFont(), static_cast<unsigned int>(14 * getScaleFactor()));
                descText.setPosition(xPos + 10.f * getScaleFactor(), yPos + buttonHeight + 20.f * getScaleFactor() + j * 20.f * getScaleFactor());
                descText.setFillColor(sf::Color::White);
                window->draw(descText);
            }
        }

        // Draw back button
        difficultyButtons.back()->draw(*window);
    }

    void renderShop() {
        sf::Text title("Shop", ResourceManager::getFont(), static_cast<unsigned int>(50 * getScaleFactor()));
        title.setPosition(static_cast<float>(window->getSize().x) / 2 - title.getLocalBounds().width / 2, 50.f * getScaleFactor());
        title.setFillColor(sf::Color::White);
        window->draw(title);

        sf::Text pointsText("Points: " + std::to_string(totalPoints), ResourceManager::getFont(), static_cast<unsigned int>(30 * getScaleFactor()));
        pointsText.setPosition(window->getSize().x / 2 - pointsText.getLocalBounds().width / 2, 100.f * getScaleFactor());
        pointsText.setFillColor(sf::Color::Yellow);
        window->draw(pointsText);

        const float areaWidth = window->getSize().x - 100.f * getScaleFactor();
        const float areaHeight = window->getSize().y - 250.f * getScaleFactor();
        const float areaX = 50.f * getScaleFactor();
        const float areaY = 150.f * getScaleFactor();

        sf::RectangleShape shopBg(sf::Vector2f(areaWidth, areaHeight));
        shopBg.setPosition(areaX, areaY);
        shopBg.setFillColor(sf::Color(0, 0, 0, 150));
        shopBg.setOutlineThickness(2.f * getScaleFactor());
        shopBg.setOutlineColor(sf::Color::White);
        window->draw(shopBg);

        const float itemWidth = areaWidth - 20.f * getScaleFactor();
        const float itemHeight = 80.f * getScaleFactor();
        const float startX = areaX + 10.f * getScaleFactor();
        const float startY = areaY + 10.f * getScaleFactor();
        const float padding = 15.f * getScaleFactor();

        for (size_t i = 0; i < shopItems.size(); ++i) {
            float yPos = startY + i * (itemHeight + padding);

            if (yPos + itemHeight > areaY + areaHeight) break;

            sf::RectangleShape itemBg(sf::Vector2f(itemWidth, itemHeight));
            itemBg.setPosition(startX, yPos);
            itemBg.setFillColor(sf::Color(0, 0, 0, 100));
            itemBg.setOutlineThickness(1.f * getScaleFactor());
            itemBg.setOutlineColor(shopItems[i].purchased ?
                (shopItems[i].active ? sf::Color::Green : sf::Color(100, 255, 100)) :
                sf::Color::Blue);
            window->draw(itemBg);

            sf::Text nameText(shopItems[i].name, ResourceManager::getFont(), static_cast<unsigned int>(20 * getScaleFactor()));
            nameText.setPosition(startX + 10.f * getScaleFactor(), yPos + 5.f * getScaleFactor());
            nameText.setFillColor(sf::Color::White);
            window->draw(nameText);

            sf::Text descText(shopItems[i].description, ResourceManager::getFont(), static_cast<unsigned int>(16 * getScaleFactor()));
            descText.setPosition(startX + 10.f * getScaleFactor(), yPos + 30.f * getScaleFactor());
            descText.setFillColor(sf::Color(200, 200, 200));
            window->draw(descText);

            std::string statusStr;
            if (shopItems[i].purchased) {
                statusStr = shopItems[i].active ? "ACTIVE" : "INACTIVE";
            }
            else {
                statusStr = "Cost: " + std::to_string(shopItems[i].cost);
            }

            sf::Text statusText(statusStr, ResourceManager::getFont(), static_cast<unsigned int>(20 * getScaleFactor()));
            statusText.setPosition(startX + itemWidth - statusText.getLocalBounds().width - 10.f * getScaleFactor(), yPos + 5.f * getScaleFactor());
            statusText.setFillColor(shopItems[i].purchased ?
                (shopItems[i].active ? sf::Color::Green : sf::Color(200, 200, 200)) :
                sf::Color::Yellow);
            window->draw(statusText);

            sf::RectangleShape button(sf::Vector2f(100.f * getScaleFactor(), 30.f * getScaleFactor()));
            button.setPosition(startX + itemWidth - 110.f * getScaleFactor(), yPos + 40.f * getScaleFactor());
            button.setFillColor(sf::Color(0, 0, 0, 150));
            button.setOutlineThickness(1.f * getScaleFactor());
            button.setOutlineColor(sf::Color::White);
            window->draw(button);

            std::string buttonText;
            if (!shopItems[i].purchased) {
                buttonText = "Buy";
            }
            else {
                buttonText = shopItems[i].active ? "Deactivate" : "Activate";
            }

            sf::Text buttonTextObj(buttonText, ResourceManager::getFont(), static_cast<unsigned int>(16 * getScaleFactor()));
            buttonTextObj.setPosition(
                button.getPosition().x + (button.getSize().x - buttonTextObj.getLocalBounds().width) / 2.f,
                button.getPosition().y + 5.f * getScaleFactor()
            );
            buttonTextObj.setFillColor(sf::Color::White);
            window->draw(buttonTextObj);

            if (shopButtonPressed) {
                sf::Vector2f mousePos = window->mapPixelToCoords(sf::Mouse::getPosition(*window));
                if (button.getGlobalBounds().contains(mousePos)) {
                    purchaseItem(static_cast<int>(i));
                    shopButtonPressed = false;
                }
            }
        }

        shopButtons[0]->draw(*window);
    }

    void renderSettings() {
        sf::Text title("Settings", ResourceManager::getFont(), static_cast<unsigned int>(50 * getScaleFactor()));
        title.setPosition(static_cast<float>(window->getSize().x) / 2 - title.getLocalBounds().width / 2, 50.f * getScaleFactor());
        title.setFillColor(sf::Color::White);
        window->draw(title);

        sf::Text resolutionTitle("Resolution:", ResourceManager::getFont(), static_cast<unsigned int>(30 * getScaleFactor()));
        resolutionTitle.setPosition(window->getSize().x * 0.6f - resolutionTitle.getLocalBounds().width / 2, window->getSize().y * 0.2f);
        window->draw(resolutionTitle);

        for (auto& btn : settingsButtons) {
            btn->draw(*window);
        }
    }
};

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    try {
        NumberGuesser game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}