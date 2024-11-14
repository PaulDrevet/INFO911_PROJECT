#include <cstdio>
#include <iostream>
#include <algorithm>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

struct ColorDistribution {
    float data[8][8][8];  // Histogramme
    int nb;               // Nombre d'echantillons

    ColorDistribution() { reset(); }

    ColorDistribution& operator=(const ColorDistribution& other) = default;

    void reset() {
        nb = 0;
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                for (int k = 0; k < 8; ++k)
                    data[i][j][k] = 0.0f;
    }

    void add(Vec3b color) {
        int r = color[2] / 32;
        int g = color[1] / 32;
        int b = color[0] / 32;
        data[r][g][b] += 1.0f;
        nb += 1;
    }

    void finished() {
        if (nb == 0) return;
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                for (int k = 0; k < 8; ++k)
                    data[i][j][k] /= nb;
    }

    float distance(const ColorDistribution& other) const {
        float dist = 0.0f;
        const float epsilon = 1e-10f;
        for (int i = 0; i < 8; ++i)
            for (int j = 0; j < 8; ++j)
                for (int k = 0; k < 8; ++k) {
                    float diff = data[i][j][k] - other.data[i][j][k];
                    float sum = data[i][j][k] + other.data[i][j][k] + epsilon;
                    dist += (diff * diff) / sum;
                }
        return dist;
    }

    Vec3b getAverageColor() const {
        float rSum = 0, gSum = 0, bSum = 0;
        float total = 0;

        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                for (int k = 0; k < 8; ++k) {
                    float value = data[i][j][k];
                    if (value > 0) {
                        rSum += i * 32 * value + 16;
                        gSum += j * 32 * value + 16;
                        bSum += k * 32 * value + 16;
                        total += value;
                    }
                }
            }
        }

        if (total == 0) return Vec3b(0, 0, 0);

        rSum /= total;
        gSum /= total;
        bSum /= total;

        return Vec3b(static_cast<uchar>(bSum), static_cast<uchar>(gSum), static_cast<uchar>(rSum));
    }

};

// Prototype de la fonction getColorDistribution
ColorDistribution getColorDistribution(Mat input, Point pt1, Point pt2);

// Trouve la plus petite distance entre un histogramme h et une liste d'histogrammes
float minDistance(const ColorDistribution& h, const std::vector<ColorDistribution>& hists) {
    float minDist = std::numeric_limits<float>::max();
    for (const auto& hist : hists) {
        float dist = h.distance(hist);
        if (dist < minDist) {
            minDist = dist;
        }
    }
    return minDist;
}

// Fonction de reconnaissance d'objets
Mat recoObject(Mat input, const std::vector< std::vector<ColorDistribution> >& allColsHists,
    const std::vector<Vec3b>& colors, const int bloc) {
    Mat result = Mat::zeros(input.size(), CV_8UC3);

    for (int y = 0; y <= input.rows - bloc; y += bloc) {
        for (int x = 0; x <= input.cols - bloc; x += bloc) {
            Point pt1(x, y);
            Point pt2(x + bloc, y + bloc);
            ColorDistribution h = getColorDistribution(input, pt1, pt2);

            float minDist = std::numeric_limits<float>::max();
            int minIndex = 0; // Index du groupe d'histogrammes le plus proche

            // Parcourir tous les groupes d'histogrammes (fond + objets)
            for (size_t i = 0; i < allColsHists.size(); ++i) {
                float dist = minDistance(h, allColsHists[i]);
                if (dist < minDist) {
                    minDist = dist;
                    minIndex = static_cast<int>(i);
                }
            }

            // Assigner la couleur correspondante
            Vec3b color = colors[minIndex];
            rectangle(result, pt1, pt2, color, -1);  // Colorie le bloc
        }
    }

    return result;
}


// Definition de getColorDistribution
ColorDistribution getColorDistribution(Mat input, Point pt1, Point pt2) {
    ColorDistribution cd;
    for (int y = pt1.y; y < pt2.y; y++)
        for (int x = pt1.x; x < pt2.x; x++)
            cd.add(input.at<Vec3b>(y, x));
    cd.finished();
    return cd;
}


void showHelp() {
    std::wcout << L"Commandes disponibles :\n";
    std::wcout << L"----------------------------------\n";
    std::wcout << L"'f'  : Geler/Degeler l'image\n";
    std::wcout << L"'h'  : Calculer la distribution de couleur pour la zone centrale\n";
    std::wcout << L"'v'  : Calculer la distance entre la distribution gauche et droite\n";
    std::wcout << L"'b'  : Scanner le fond (background)\n";
    std::wcout << L"'n'  : Ajouter un nouvel objet\n";
    std::wcout << L"'a'  : Ajouter l'histogramme de la zone centrale au dernier objet cree\n";
    std::wcout << L"'r'  : Activer/Desactiver le mode reconnaissance\n";
    std::wcout << L"'q' ou 'ESC' : Quitter\n";
    std::wcout << L"----------------------------------\n";
}


int main(int argc, char** argv) {
    Mat imgInput;
    VideoCapture* pCap = nullptr;
    const int width = 640;
    const int height = 480;
    const int size = 50;
    const int bbloc = 128;

    showHelp();

    pCap = new VideoCapture(0);
    if (!pCap->isOpened()) {
        cout << "Impossible d'ouvrir l'image / la camera ";
        return 1;
    }
    pCap->set(CAP_PROP_FRAME_WIDTH, width);
    pCap->set(CAP_PROP_FRAME_HEIGHT, height);

    (*pCap) >> imgInput;
    if (imgInput.empty()) return 1;

    Point pt1(width / 2 - size / 2, height / 2 - size / 2);
    Point pt2(width / 2 + size / 2, height / 2 + size / 2);

    namedWindow("input", 1);
    imshow("input", imgInput);
    bool freeze = false;
    bool reco = false; // Mode reconnaissance desactive au depart

    // Vecteur de vecteurs pour stocker les distributions de couleur (fond + objets)
    std::vector< std::vector<ColorDistribution> > allColsHists;
    allColsHists.push_back(std::vector<ColorDistribution>()); // Index 0 pour le fond

    // Couleurs pour "fond" et objets
    std::vector<Vec3b> colors = { Vec3b(0, 0, 0) }; // Commence avec la couleur du fond

    while (true) {
        char c = (char)waitKey(50);
        if (pCap != nullptr && !freeze) (*pCap) >> imgInput;
        if (c == 27 || c == 'q') break;
        if (c == 'f') freeze = !freeze;

        if (c == 'h') {
            ColorDistribution cd = getColorDistribution(imgInput, pt1, pt2);
            cout << "Distribution de couleur calculee pour la zone centrale.\n";
        }

        if (c == 'v') {
            Point rightPoint1(width / 2, 0);
            Point rightPoint2(width, height);
            Point leftPoint1(0, 0);
            Point leftPoint2(width / 2, height);

            ColorDistribution leftDist = getColorDistribution(imgInput, leftPoint1, leftPoint2);
            ColorDistribution rightDist = getColorDistribution(imgInput, rightPoint1, rightPoint2);

            float dist = leftDist.distance(rightDist);
            cout << "Distance entre la distribution de couleurs gauche et droite : " << dist << endl;
        }

        if (c == 'b') {
            allColsHists[0].clear();
            for (int y = 0; y <= height - bbloc; y += bbloc) {
                for (int x = 0; x <= width - bbloc; x += bbloc) {
                    Point pt1(x, y);
                    Point pt2(x + bbloc, y + bbloc);
                    ColorDistribution cd = getColorDistribution(imgInput, pt1, pt2);
                    allColsHists[0].push_back(cd);
                }
            }
            int nbHistsBackground = (int)allColsHists[0].size();
            cout << "Nombre de distributions de couleur du fond : " << nbHistsBackground << endl;
        }

        if (c == 'n') {
            // Ajouter un nouvel objet
            allColsHists.push_back(std::vector<ColorDistribution>()); // Nouvel objet
            ColorDistribution cd = getColorDistribution(imgInput, pt1, pt2);
            Vec3b avgColor = cd.getAverageColor();
            colors.push_back(avgColor);
            cout << "Nouvel objet cree. Nombre total d'objets : " << allColsHists.size() - 1 << endl;
        }

        if (c == 'a') {
            if (allColsHists.size() > 1) {
                // Ajouter l'histogramme au dernier objet cree
                ColorDistribution cd = getColorDistribution(imgInput, pt1, pt2);
                allColsHists.back().push_back(cd);
                cout << "Histogramme ajoute à l'objet " << allColsHists.size() - 1 << endl;
                cout << "Nombre d'histogrammes pour cet objet : " << allColsHists.back().size() << endl;
            }
            else {
                cout << "Aucun objet cree. Appuyez sur 'n' pour creer un nouvel objet." << endl;
            }
        }

        if (c == 'r' && allColsHists.size() > 1) {
            reco = !reco; // Active ou desactive le mode reconnaissance
            cout << "Mode reconnaissance " << (reco ? "active" : "desactive") << endl;
        }

        Mat output = imgInput.clone();
        if (reco) {
            Mat gray;
            cvtColor(imgInput, gray, COLOR_BGR2GRAY);
            Mat recoImg = recoObject(imgInput, allColsHists, colors, 8); // Blocs de 8x8
            cvtColor(gray, imgInput, COLOR_GRAY2BGR);
            output = 0.5 * recoImg + 0.5 * imgInput; // Melange reco + camera
        }

        rectangle(output, pt1, pt2, Scalar(255, 255, 255), 1);

        // Afficher la legende des objets
        int legendX = 10;
        int legendY = 30;
        for (size_t i = 0; i < colors.size(); ++i) {
            rectangle(output, Point(legendX, legendY - 20), Point(legendX + 20, legendY), colors[i], -1);
            std::string label = (i == 0) ? "Fond" : "Objet " + std::to_string(i);
            putText(output, label, Point(legendX + 30, legendY - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
            legendY += 30;
        }

        imshow("input", output);
    }

    delete pCap;
    return 0;
}