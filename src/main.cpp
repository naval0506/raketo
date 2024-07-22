#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <limits>

using namespace std;

struct Joueur {
    string nom;
    int solde;
    vector<int> mises;
    bool continueJeu;
};

int lancerDe() {
    return rand() % 6 + 1;
}

void afficherSoldes(sf::RenderWindow &window, const vector<Joueur>& joueurs, sf::Font &font) {
    for (size_t i = 0; i < joueurs.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setFillColor(sf::Color::Black);
        text.setCharacterSize(24);
        text.setString(joueurs[i].nom + ": " + to_string(joueurs[i].solde) + "$");
        text.setPosition(600, 10 + i * 30);
        window.draw(text);
    }
}

void afficherNotification(sf::RenderWindow &window, const string &message, sf::Font &font) {
    sf::Text text;
    text.setFont(font);
    text.setFillColor(sf::Color::Yellow);
    text.setCharacterSize(24);
    text.setString(message);
    text.setPosition(10, 10);
    window.draw(text);
}

void afficherDe(sf::RenderWindow &window, int resultatDe, const vector<sf::Texture> &textures) {
    sf::Sprite deSprite;
    deSprite.setTexture(textures[resultatDe - 1]);
    deSprite.setPosition(600, 200);
    window.draw(deSprite);
}

void dessinerTableau(sf::RenderWindow &window, const vector<sf::RectangleShape> &cases, sf::Font &font) {
    for (size_t i = 0; i < cases.size(); ++i) {
        window.draw(cases[i]);

        sf::Text text;
        text.setFont(font);
        text.setFillColor(sf::Color::White);
        text.setCharacterSize(24);
        text.setString(to_string(i + 1));
        text.setPosition(cases[i].getPosition().x + 15, cases[i].getPosition().y + 10);
        window.draw(text);
    }
}

bool entrerMontant(sf::RenderWindow &window, const string &promptText, int &montant, sf::Font &font) {
    sf::Text prompt(promptText, font, 24);
    prompt.setFillColor(sf::Color::Black);
    prompt.setPosition(10, 50);

    sf::Text input("", font, 24);
    input.setFillColor(sf::Color::Black);
    input.setPosition(10, 90);

    bool montantEntree = false;
    string montantStr;
    while (window.isOpen() && !montantEntree) {
        window.clear(sf::Color::White);
        window.draw(prompt);
        window.draw(input);
        window.display();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return false;
            }

            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == '\b') {
                    if (!montantStr.empty()) {
                        montantStr.pop_back();
                    }
                } else if (event.text.unicode == '\r') {
                    try {
                        montant = stoi(montantStr);
                        if (montant > 0) {
                            montantEntree = true;
                        } else {
                            montantStr.clear();
                        }
                    } catch (...) {
                        montantStr.clear();
                    }
                } else if (event.text.unicode >= '0' && event.text.unicode <= '9') {
                    montantStr += static_cast<char>(event.text.unicode);
                }
                input.setString(montantStr);
            }
        }
    }
    return true;
}

class Button {
public:
    sf::RectangleShape shape;
    sf::Text text;

    Button(float x, float y, float width, float height, const std::string& textString, sf::Font& font, unsigned int characterSize = 24) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(width, height));
        shape.setFillColor(sf::Color(70, 130, 180)); // SteelBlue color

        text.setFont(font);
        text.setString(textString);
        text.setCharacterSize(characterSize);
        text.setFillColor(sf::Color::White);
        text.setPosition(
            x + (width - text.getLocalBounds().width) / 2,
            y + (height - text.getLocalBounds().height) / 2 - 5
        );
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(text);
    }

    bool isClicked(sf::Vector2i mousePos) {
        return shape.getGlobalBounds().contains(sf::Vector2f(mousePos));
    }
};

bool anyJoueurWithSoldePositif(const vector<Joueur>& joueurs) {
    for (const auto& joueur : joueurs) {
        if (joueur.solde > 0 && joueur.continueJeu) {
            return true;
        }
    }
    return false;
}

void initialiserJoueurs(vector<Joueur>& joueurs) {
    for (auto& joueur : joueurs) {
        cout << "Entrez le nom du joueur: ";
        cin >> joueur.nom;
        while (true) {
            cout << "Entrez le solde initial de " << joueur.nom << " en $: ";
            if (cin >> joueur.solde && joueur.solde > 0) break;
            cout << "Solde invalide. Veuillez réessayer." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        joueur.continueJeu = true;
        joueur.mises.resize(6, 0); // Initialiser les mises à 0 pour chaque case
        cout << endl;
    }
}

bool traiterEvenement(sf::RenderWindow& window, vector<sf::RectangleShape>& cases, Button& boutonValider, vector<Joueur>& joueurs, int& joueurIndex, sf::Font& font) {
    bool miseValidee = false;
    while (window.isOpen() && !miseValidee) {
        window.clear(sf::Color::White);
        dessinerTableau(window, cases, font);
        afficherSoldes(window, joueurs, font);
        afficherNotification(window, joueurs[joueurIndex].nom + ", entrez vos mises et cliquez sur Valider.", font);
        boutonValider.draw(window);
        window.display();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return false;
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    for (int i = 0; i < 6; ++i) {
                        if (cases[i].getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                            int montant;
                            if (entrerMontant(window, "Entrez le montant pour la case " + to_string(i + 1) + ": ", montant, font)) {
                                if (montant <= joueurs[joueurIndex].solde) {
                                    joueurs[joueurIndex].mises[i] += montant;
                                    joueurs[joueurIndex].solde -= montant;
                                } else {
                                    afficherNotification(window, "Mise dépasse le solde.", font);
                                    window.display();
                                    sf::sleep(sf::seconds(2));
                                }
                            }
                        }
                    }
                    if (boutonValider.isClicked(mousePos)) {
                        miseValidee = true;
                    }
                }
            }
        }
    }
    return true;
}

void afficherResultats(sf::RenderWindow& window, int resultatDe, const vector<sf::Texture>& textures, const vector<Joueur>& joueurs, sf::Font& font) {
    window.clear(sf::Color::White);
    afficherNotification(window, "Lancement du dé...", font);
    window.display();
    sf::sleep(sf::seconds(1)); // Réduire le temps de sommeil pour une meilleure expérience utilisateur

    window.clear(sf::Color::White);
    afficherDe(window, resultatDe, textures);
    window.display();
    sf::sleep(sf::seconds(1));

    for (int i = 0; i < joueurs.size(); ++i) {
        int gain = joueurs[i].mises[resultatDe - 1] * 2;
        if (joueurs[i].mises[resultatDe - 1] > 0) {
            afficherNotification(window, "Félicitations " + joueurs[i].nom + ", vous avez gagné " + to_string(gain) + "$!", font);
        } else {
            afficherNotification(window, "Désolé " + joueurs[i].nom + ", vous avez perdu.", font);
        }
        window.display();
        sf::sleep(sf::seconds(1));
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    sf::RenderWindow window(sf::VideoMode(800, 600), "RAKETO - Jeu de Dé");

    sf::Font font;
    if (!font.loadFromFile("fonts/Roboto-Regular.ttf")) { // Changez cette ligne pour charger une police cool comme Roboto
        cerr << "Erreur lors du chargement de la police" << endl;
        return -1;
    }

    Button boutonValider(600, 400, 150, 50, "Valider", font);

    int nombreDeJoueurs;
    while (true) {
        cout << "Bienvenue dans RAKETO" << endl << endl;
        cout << "Entrez le nombre de joueurs: ";
        if (cin >> nombreDeJoueurs && nombreDeJoueurs > 0) break;
        cout << "Nombre de joueurs invalide. Veuillez réessayer." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    vector<Joueur> joueurs(nombreDeJoueurs);
    initialiserJoueurs(joueurs);

    vector<sf::Texture> textures(6);
    for (int i = 0; i < 6; ++i) {
        if (!textures[i].loadFromFile("images/de" + to_string(i + 1) + ".png")) {
            cerr << "Erreur lors du chargement de l'image de de" << i + 1 << ".png" << endl;
            return -1;
        }
    }

    vector<sf::RectangleShape> cases(6);
    for (int i = 0; i < 6; ++i) {
        cases[i].setSize(sf::Vector2f(50, 50));
        cases[i].setFillColor(sf::Color(70, 130, 180));
        cases[i].setPosition(100 + i * 60, 300);
    }

    bool jeuActif = true;
    int joueurIndex = 0;

    while (jeuActif) {
        window.clear(sf::Color::White);
        dessinerTableau(window, cases, font);
        afficherSoldes(window, joueurs, font);

        if (joueurs[joueurIndex].solde > 0 && joueurs[joueurIndex].continueJeu) {
            if (!traiterEvenement(window, cases, boutonValider, joueurs, joueurIndex, font)) {
                return 0;
            }
        }

        joueurIndex = (joueurIndex + 1) % nombreDeJoueurs;

        if (joueurIndex == 0) {
            int resultatDe = lancerDe();
            afficherResultats(window, resultatDe, textures, joueurs, font);

            for (auto& joueur : joueurs) {
                fill(joueur.mises.begin(), joueur.mises.end(), 0);
            }

            if (!anyJoueurWithSoldePositif(joueurs)) {
                jeuActif = false;
            }
        }

        window.display();
    }

    while (window.isOpen()) {
        window.clear(sf::Color::White);
        afficherSoldes(window, joueurs, font);
        afficherNotification(window, "Le jeu est terminé.", font);
        window.display();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
    }

    return 0;
}
