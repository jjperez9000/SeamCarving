#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include "graphics.h"
using namespace std;

/* Image code borrowed from our USA map demo... */

struct Pixel {
  unsigned char r, g, b;
};

int width, height;
Pixel *image;

Pixel white = { 255, 255, 255 };
Pixel black = { 0, 0, 0 };
Pixel blue = {0, 128, 128};
Pixel red = {255, 0, 0};


Pixel &get_pixel(int x, int y)
{
  return image[y*width + x];
}

void read_image(const char *filename)
{
  FILE *fp = fopen (filename, "r");
  int dummy = fscanf (fp, "P6\n%d %d\n255%*c", &width, &height);
  image = new Pixel[width * height];
  dummy = fread (image, width*height, sizeof(Pixel), fp);
  fclose (fp);
}

void write_image(const char *filename)
{
  FILE *fp = fopen (filename, "w");
  fprintf (fp, "P6\n%d %d\n255\n", width, height);
  int dummy = fwrite (image, width*height, sizeof(Pixel), fp);
  fclose (fp);
}

/* To be completed from here on... */
typedef pair<int,int> Node;
bool *visited;
queue<Node> to_visit;
unordered_map<int, int> dist;
unordered_map<int, unsigned char> dim;

bool operator== (Pixel &a, Pixel &b) {  
  	return a.r == b.r && a.g == b.g && a.b == b.b;
}

Pixel &get_pixel(Node xy)
{
  	return image[xy.second * width + xy.first];
}

bool &get_visited(Node xy) {
  	return visited[xy.second*width + xy.first];
}

int get_loc(pair<int, int> xy) {
	return xy.second * width + xy.first;
}

bool valid_loc(pair<int, int> xy) {
    return (xy.first >= 0 && xy.first<width && xy.second < height && xy.second >= 0);
}

void hash_dists() {
	for (int i=0; 255 * pow(.9, i) > 1; i++)
		dim[i] = 255 * pow(.9, i);
}

void make_MEGANODE() {
	for (int i=0; i<height; i++) {
		for (int j=0; j<width; j++) {
			if (get_pixel(j, i) == white) {
				to_visit.push(make_pair(j, i));
				dist[get_loc(to_visit.front())] = 0;
				get_visited(make_pair(j, i)) = true;
			}
            else {
                dist[get_loc(make_pair(j, i))] = width * 500;
            }
		}
	}
}

void blur_nodes() {
	int xAdj[4] = {-1, 1, 0, 0};
	int yAdj[4] = {0, 0, -1, 1};

	while(!to_visit.empty()) {
		Node p = to_visit.front();
		to_visit.pop();
		
		for (int i=0; i<4; i++) {
			Node n = make_pair(p.first+xAdj[i], p.second+yAdj[i]);

			if (valid_loc(n) && !get_visited(n) && dist[get_loc(p)] + 1 < dim.size()) {
				get_visited(n) = true;
				dist[get_loc(n)] = dist[get_loc(p)] + 1;
				get_pixel(n) = {dim[dist[get_loc(n)]], dim[dist[get_loc(n)]], dim[dist[get_loc(n)]]};
				to_visit.push(n);
			}
		}
	}
}

void calculate_blur(void)
{
	visited = new bool[width * height];
	hash_dists();
	make_MEGANODE();
	blur_nodes();
    dim.clear();
}

int main(void)
{
  read_image("paw.ppm");
  calculate_blur();
  write_image("paw2.ppm");
}
