# INFO911 - Traitement et analyse d'images

## 🤼 Groupe

Paul Drevet<br>
Mathis Mazoyer

## 📖 Sommaire

- [Technologies utilisées](#-technologies-utilisées)
- [Structure du projet](#-structure-du-projet)
- [Description du projet](#-description-du-projet)
- [Liste des commandes](#-liste-des-commandes)
- [Liste des fonctions](#️-liste-des-fonctions)
- [Cas d'utilisation](#-cas-dutilisation)

## 🛠️ Technologies utilisées

[![C++](https://skillicons.dev/icons?i=cpp)]()

------

### Pour le déploiement:

Pour le déploiement, nous avons utilisés les technologies suivantes :

- [C++](https://cplusplus.com): Le C++ est un langage de programmation performant combinant programmation orientée objet et contrôle bas-niveau, idéal pour les logiciels systèmes et les applications exigeantes.

## 🌲 Structure du projet

```sh
./
|
├── imgs # Images du Readme
|
├── Project.cpp # Fichier source du programme
| 
└── README.md 
```

## 📄 Description du projet

Ce projet consiste en une application de **segmentation d'images en temps réel** utilisant la distance entre **histogrammes de couleurs** pour détecter et reconnaître des objets. Le programme apprend à distinguer le fond de l'image et les objets à l'aide de distributions de couleurs. Une fois le fond et les objets mémorisés, il est capable d'identifier et de segmenter les objets dans les futures captures en mode reconnaissance.

## 💻 Liste des commandes

Voici la liste des commandes que vous pourrez utiliser dans le programme :

`q`: Quitter le programme <br>
`f`: Freeze le programme <br>
`b`: Analyser le fond <br>
`a`: Analyser un objet <br>
`r`: Activer la reconnaissance <br>

## ⌨️ Liste des fonctions

1. `ColorDistribution::reset()`
- Paramètres: Aucun
- Réinitialise l'histogramme en mettant tous ses éléments à zéro et réinitialise le compteur d'échantillons (`nb`) à 0.

---

2. `ColorDistribution::add(Vec3b color)`
- Paramètres:
    - `color`: Un vecteur `Vec3b` représentant la couleur (BGR).
- Ajoute un échantillon de couleur à l'histogramme. La couleur est convertie en un indice dans l'espace 8x8x8 en divisant chaque composante par 32, puis l'histogramme est mis à jour et le compteur d'échantillons est incrémenté.

---

3. `ColorDistribution::finished()`
- Paramètres: Aucun
- Normalise l'histogramme en divisant chaque case par le nombre total d'échantillons. Cela permet de représenter les valeurs comme des proportions.

---

4. `ColorDistribution::distance(const ColorDistribution& other)`
- Paramètres:
    - `other`: Un autre histogramme de type `ColorDistribution`.
- Calcule la distance entre deux histogrammes à l'aide de la formule du chi-carré. Cette distance est utilisée pour comparer la similarité entre deux distributions de couleurs.
- La distance est caclulée comme : <br>
![Formule distance](/img/distance_formula.png)<br>
où `diff` est la différence entre les valeurs des deux histogrammes et sum est leur somme. Un petit epsilon est ajouté pour éviter la division par zéro.

---

5. `getColorDistribution(Mat input, Point pt1, Point pt2)`
- Paramètres:
    - `input`: Une image `Mat`
    - `pt1`, `pt2`: Points définissant une zone rectangulaire dans l'image
- Calcule l'histogramme de couleurs pour la région définie par `pt1` et `pt2` dans l'image input.

---

6. `minDistance(const ColorDistribution& h, const std::vector<ColorDistribution>& hists)`
- Paramètres:
    - `h`: L'histogramme de référence.
    - `hists`: Un vecteur d'histogrammes à comparer.
- Trouve la plus petite distance entre l'histogramme h et une liste d'histogrammes. Retourne la distance minimale.

---

7. `recoObject(Mat input, const std::vector<std::vector<ColorDistribution>>& all_col_hists, const std::vector<Vec3b>& colors, const int bloc)`
- Paramètres:
    - `input`: L'image d'entrée.
    - `allColHists`: Un vecteur contenant des groupes d'histogrammes pour le fond et les objets.
    - `colors`: Un vecteur de couleurs correspondant à chaque groupe d'histogrammes.
    - `bloc`: La taille des blocs (ex. 8x8).
- Cette fonction segmente l'image en blocs de taille `bloc`, calcule l'histogramme pour chaque bloc, et détermine s'il appartient au fond ou à un objet en fonction des distances d'histogrammes. Colorie chaque bloc en fonction du groupe le plus proche.

## 📸 Cas d'utilisation

Voici des screenshots de nos utilisations avec le programme : 

Premier test avec un cahier rouge : <br>
![Test 1 une couleur](/img/test_1.png)

Deuxième test avec un cahier rouge et un stabilo jaune : <br>
![Test 2 deux couleur](/img/test_2.png)

! Les reflets impactent le programme, c'est pour cela que nous avons (pour le cahier par exemple) des résultats non parfaits !