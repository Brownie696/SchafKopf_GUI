#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>
#include <array>

constexpr int CARD_SIZE_X{145};        
constexpr int CARD_SIZE_Y{260};        
constexpr float CARD_SELECT_SPEED{200.f};

class GUI
{
private:
    // Speichert koordinaten in sprite, koordinaten in Fenseter
    struct Card_Data {sf::IntRect rect; sf::Vector2f pos;};
    // Array für 32 Spielkarten
    std::array<Card_Data,32> card_data;
    // größe und bpp für desktop
    sf::VideoMode desktopmode;
    // Spielefenster
    sf::RenderWindow* window;
    // Textur für hintergrund
    sf::Texture background;
    // Textur für Karten
    sf::Texture spritesheet;
    // Sprite für Hintergrund
    sf::Sprite spriteBG {background};
    // Sprite für Karten
    sf::Sprite spriteCard {spritesheet};
    // Stopuhr um zum zeit messeung pro druchlauf
    sf::Clock cycle;
    // delta zeit zur berchnung der animationen
    sf::Time dt;
    // Startposition vor animation
    float Card_Start_Pos_Y;
    // Auswertung benutzereingaben   
    void InputHandler();
    // Animation Karte
    void MoveCard(Card_Data* data, int index);
    // prüfen ob Mauszeiger auf karte ist
    bool isCardpreselection(Card_Data* data, const sf::Event::MouseMoved* mouse);
    // zeichne aktulles farme und lade in  GPU
    void DrawScreen();
    // merker ob laden von Hintergrundbild erfolgreich war
    bool loadBG;
    // merker ob laden von Kartenbild erfoglreich war
    bool loadCard;
    // Bitmaske auf welcher karte der Mauszeiger ist
    int MousOnCard{0};
public:

    GUI();  // Konstruktor
    ~GUI(); // Destruktor
    void RunGame();
};

#pragma region // privat

// Ausewertung Benutzereingabe
void GUI::InputHandler() {
    //Schleife für aktives event
    while (const std::optional event = window->pollEvent()) {

        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
            event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            // Spielfenster mit ESC schliessen
            window->close();}
        else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
            // Maus wurde bewegt, pürfe ob karte vorgewählt wurde
            for (auto i = card_data.size(); i > 0; i--){
                 if (isCardpreselection(&card_data.at(i-1),mouseMoved)) {
                    // Mauszeiger ist auf Karte
                    MousOnCard = (1 << i-1);
                    // Kann immer nur eine karte zuselben zeit vorgewählt sein, rest muss nicht mehr geprüft werden
                    break;
                }
                MousOnCard = 0;
            }
        }
        // The window was resized
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            //doSomethingWithTheNewSize(resized->size);
        }
    }

}

// Koordinaten berechnung für Kartenanimation
void GUI::MoveCard(Card_Data *data,int index) {
        int comp = (1 << index);
        if ((MousOnCard == comp)) {
            // Maus auf Karte, Bewegekarte nach oben
            data->pos.y = std::max(data->pos.y - CARD_SELECT_SPEED * dt.asSeconds(), Card_Start_Pos_Y - 50.f);
        } else {
            // Maus nicht mehr Karte, Bewegekarte nach unten
            data->pos.y = std::min(data->pos.y + CARD_SELECT_SPEED * dt.asSeconds(), Card_Start_Pos_Y);
        }
        // update karte und position
        spriteCard.setTextureRect(data->rect);
        spriteCard.setPosition(data->pos);
}

// Prüft ob Mauszeiger auf karte ist
bool GUI::isCardpreselection(Card_Data* data, const sf::Event::MouseMoved* mouse) {
    return (mouse->position.x > data->pos.x && mouse->position.x < data->pos.x + CARD_SIZE_X &&
        mouse->position.y > data->pos.y && mouse->position.y < data->pos.y + CARD_SIZE_Y);
}

// Zeichnet aktuelles frame
void GUI::DrawScreen() {
    window->draw(spriteBG);
    for (int i = 0; i < card_data.size(); i++){
        MoveCard(&card_data.at(i),i);
        window->draw(spriteCard);
    }
    window->display();
}
#pragma endregion // privat

#pragma region // public
// Konstruktor
GUI::GUI() {
    // instanzen erstellen und speicher belegen
    this->window = new sf::RenderWindow;
    // pixel grösse von desktop lesen
    desktopmode = sf::VideoMode::getDesktopMode();
    // Spiele fenster erstellen
    window->create(desktopmode,"Scharfkopf",sf::State::Fullscreen);
    // setze framerate
    window->setFramerateLimit(60);
    // lade Hintergrundbild
    loadBG = background.loadFromFile("./resourcen/wooden-floor.jpg");
    if (loadBG) { spriteBG.setTexture(background, true);}
    // lade Spielkarten grafik
    loadCard = spritesheet.loadFromFile("./resourcen/schafkopf_standard.jpg");
    if (loadCard) { spriteCard.setTexture(spritesheet, true);}
    // bestimme Y-Koordinate wo karten reihe angezeiget werden soll
    Card_Start_Pos_Y = desktopmode.size.y - CARD_SIZE_Y - 20.f;
    // bestimme abstand um karten mit im Bild zu haben
    float x_offs = ((desktopmode.size.x - (CARD_SIZE_X / 3) * (card_data.size() + 1)) / 2) - (CARD_SIZE_X / 2);
    // Schleifen index
    int i{0};
    // Schleife initialisiere sprite daten
    for (auto& c : card_data){
        if ((i % 9) == 0){i++;}
        c.rect = {{15 + (i % 10) + ( CARD_SIZE_X * (i % 10))
                 ,229 + (i / 10) + ( CARD_SIZE_Y * (i / 10))},
                 {CARD_SIZE_X,CARD_SIZE_Y}};
        c.pos = {x_offs + (CARD_SIZE_X / 3) * (i-(i/9)), Card_Start_Pos_Y};
        i++;
    }

}

// Destruktor
GUI::~GUI() {
    // gib speicherplatz wieder frei
    delete window;
}

// Spielschleife
void GUI::RunGame() {
    // Spiel kann nicht gestartet werden datein nicht geladen werden konnten
    if (loadBG && loadCard){
        // Schleife bis Fenster geschlossen worden ist
        while (window->isOpen()) {
            // Messung druchlaufzeit für animaitons berechnung
            dt = cycle.restart();
            InputHandler();
            DrawScreen();
        }   
    }
}
#pragma endregion //public

int main() {

    GUI Game;
    Game.RunGame();
    return EXIT_SUCCESS;
}
