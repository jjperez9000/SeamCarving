#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graphics.h"
using namespace std;

struct Pixel {
    unsigned char r, g, b;
};

int width, height;
Pixel *image;

Pixel white = {255, 255, 255};
Pixel black = {0, 0, 0};
Pixel red = {255, 0, 0};

bool operator==(Pixel &a, Pixel &b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}
bool operator!=(Pixel &a, Pixel &b) { return !(a == b); }

Pixel &get_pixel(int x, int y) { return image[y * width + x]; }

void read_image(const char *filename) {
    FILE *fp = fopen(filename, "r");
    int dummy = fscanf(fp, "P6\n%d %d\n255%*c", &width, &height);
    image = new Pixel[width * height];
    dummy = fread(image, width * height, sizeof(Pixel), fp);
}

typedef pair<int, int> Node;
queue<Node> to_visit;
unordered_map<int, int> dist;
unordered_map<int, unsigned char> dim;

Pixel &get_pixel(Node xy) { return image[xy.second * width + xy.first]; }

bool &get_visited(bool visited[], Node xy) {
    return visited[xy.second * width + xy.first];
}

int get_loc(pair<int, int> xy) { return xy.second * width + xy.first; }

bool valid_loc(pair<int, int> xy) {
    return (xy.first >= 0 && xy.first < width && xy.second < height &&
            xy.second >= 0);
}

void hash_dists() {
    for (int i = 0; 255 * pow(.9, i) > 1; i++) dim[i] = 255 * pow(.9, i);
}

void make_MEGANODE(bool visited[]) {
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            if (get_pixel(j, i) == white) {
                to_visit.push(make_pair(j, i));
                dist[get_loc(to_visit.front())] = 0;
                get_visited(visited, make_pair(j, i)) = true;
            } else
                dist[get_loc(make_pair(j, i))] = width * 500;
}

void blur_nodes(bool visited[]) {
    int xAdj[4] = {-1, 1, 0, 0};
    int yAdj[4] = {0, 0, -1, 1};

    while (!to_visit.empty()) {
        Node p = to_visit.front();
        to_visit.pop();

        for (int i = 0; i < 4; i++) {
            Node n = make_pair(p.first + xAdj[i], p.second + yAdj[i]);

            if (valid_loc(n) && !get_visited(visited, n) &&
                dist[get_loc(p)] + 1 < dim.size()) {
                get_visited(visited, n) = true;
                dist[get_loc(n)] = dist[get_loc(p)] + 1;
                get_pixel(n) = {dim[dist[get_loc(n)]], dim[dist[get_loc(n)]],
                                dim[dist[get_loc(n)]]};
                to_visit.push(n);
            }
        }
    }
}

void calculate_blur(void) {
    bool *visited = new bool[width * height];
    for (int i = 0; i < width * height; i++) visited[i] = false;
    hash_dists();
    make_MEGANODE(visited);
    blur_nodes(visited);
    dist.clear();
    dim.clear();
}

bool valid_loc(int x, int y) { return (x >= 0 && x < width && y < height); }

map<Node, int> weight;
map<Node, Node> pred;
Node get_seam() {
    Node incumbent = make_pair(-100, -100);
    weight[incumbent] = width * 500;

    for (int i = 0; i < width; i++) {
        to_visit.push(make_pair(i, 0));
        pred[make_pair(i, 0)] = make_pair(-1, -1);
        weight[make_pair(i, 0)] = get_pixel(i, 0).r;
        for (int j = 1; j < height; j++) weight[make_pair(i, j)] = width * 500;
    }

    while (!to_visit.empty()) {
        Node xy = to_visit.front();
        to_visit.pop();

        if (xy.second == height - 1 && weight[xy] < weight[incumbent])
            incumbent = xy;

        for (int i = -1; i <= 1; i++) {
            Node neighbor = make_pair(xy.first + i, xy.second + 1);
            if (valid_loc(neighbor.first, neighbor.second) &&
                weight[xy] + get_pixel(neighbor).r < weight[neighbor]) {
                weight[neighbor] = weight[xy] + get_pixel(neighbor).r;
                pred[neighbor] = xy;
                to_visit.push(neighbor);
            }
        }
    }
    return incumbent;
}

void calculate_seam(void) {
    Node s = get_seam();

    while (s != make_pair(-1, -1)) {
        get_pixel(s) = red;
        s = pred[s];
    }
    weight.clear();
    pred.clear();
}

/* Code to find and remove seams */

bool seam_exists(void) {
    for (int x = 0; x < width; x++)
        if (get_pixel(x, 0) == red) return true;
    return false;
}

void remove_seam(void) {
    cout << "Removing seam to decrease width to " << width - 1 << "\n";
    for (int y = 0; y < height; y++) {
        int where_red = -1;
        for (int x = 0; x < width; x++)
            if (get_pixel(x, y) == red)
                if (where_red != -1) {
                    cout << "Error: row " << y << " hass >1 red pixel set\n";
                    exit(0);
                } else
                    where_red = x;
        if (where_red == -1) {
            cout << "Error: row " << y << " has 0 red pixels set\n";
            exit(0);
        }
        for (int x = where_red; x < width - 1; x++)
            get_pixel(x, y) = get_pixel(x + 1, y);
    }

    // Create a new image with one smaller width
    Pixel *orig_image = image;
    width--;
    image = new Pixel[height * width];
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            get_pixel(x, y) = orig_image[y * (width + 1) + x];
    delete[] orig_image;

    calculate_blur();
}

/* Simple 2D interactive graphics code... */

bool show_blurred_image = false;

// Called on each keypress
void keyhandler(int key) {
    if (key == 'q') exit(0);

    // Toggle showing blurred image (off-white pixels)
    if (key == 'b') show_blurred_image = !show_blurred_image;

    // Find and remove one seam
    if (key == 's' && !seam_exists()) calculate_seam();

    // Find and remove 2...9 seams
    if ((key >= '2' && key <= '9') && !seam_exists())
        for (int i = 0; i < key - '0'; i++) {
            calculate_seam();
            remove_seam();
        }
}

// Called any time the scene needs to be re-rendered
void render(void) {
    // Always plot white and red pixels.  Show all other pixels if blur
    // mode is toggled on
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++) {
            Pixel p = get_pixel(x, y);
            if (show_blurred_image || p == white || p == red) {
                set_color(p.r, p.g, p.b);
                draw_pixel(x, y);
            }
        }

    // Fill in empty space at right from shrinking the image with blue
    set_color(0, 0, 255);
    for (int x = width; x < 800; x++) draw_line(x, 0, x, height - 1);
}

// Called periodically (msec is amount of time since last called)
// Return true if a re-render is needed
// Right now this function waits a second after finding a seam to
// remove it (allowing it to be visualized briefly)
bool timer(int msec) {
    static int cumulative = 0;
    static bool found_seam = false;
    cumulative += msec;
    if (cumulative > 1000) {
        cumulative -= 1000;
        if (seam_exists()) {
            if (found_seam) {
                remove_seam();
                found_seam = false;
                return true;
            }
            found_seam = true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    read_image("billboard.ppm");
    calculate_blur();
    init_graphics(argc, argv, width, height, render, keyhandler, timer);
}
